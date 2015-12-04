""" This module defines a general configuration for analyses involving t-channel single top-quark
    production. It performs a very loose event selection (at least one semi-tight lepton, at least
    two jet with pt > 30 GeV/c). Necessary event cleaning (mostly recommended for MET analyses) is
    applied and quality cuts for different physical objects are defined. Corrected MET as well as
    all the corresponding systematics are calculated. Isolation requirements for charged leptons are
    dropped (they are applied only for jet clustering and MET systematics due to lepton energy
    scale).
    
    The results are saved with the help of dedicated EDAnalyzer's. No EDM output is produced.
    
    The workflow can be controlled through the VarParsing options defined in the code below.
    """

import sys
import random
import string
import re


# Create a process
import FWCore.ParameterSet.Config as cms
process = cms.Process('Analysis')


# Enable MessageLogger
process.load('FWCore.MessageLogger.MessageLogger_cfi')

# Reduce verbosity
process.MessageLogger.cerr.FwkReport.reportEvery = 100


# Ask to print a summary in the log
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True))


# Parse command-line options
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

options.register('runOnData', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Indicates whether it runs on the real data')
options.register('globalTag', '', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'The relevant global tag')
# The outputName is postfixed with ".root" automatically
options.register('outputName', 'sample', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'The name of the output ROOT file')
# The leptonic channels to be processed. 'e' stands for electron, 'm' -- for muon
options.register('channels', 'em', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'The leptonic channels to process')
options.register('saveHardInteraction', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    'Save information about the status 3 particles, except for the initial section')
options.register('saveHeavyFlavours', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Saves information about heavy-flavour quarks in parton shower')
options.register('saveGenJets', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Save information about generator-level jets')
options.register('sourceFile', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'The name of the source file')
options.register('runOnFastSim', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Indicates whether FastSim is processed')
options.register('jetSel', '2j30', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Selection on jets. E.g. 2j30 means that an event must contain at least 2 jets with '
    'pt > 30 GeV/c')

options.parseArguments()


# Make the shortcuts to access some of the configuration options easily
runOnData = options.runOnData
elChan = (options.channels.find('e') != -1)
muChan = (options.channels.find('m') != -1)


# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, options.globalTag)

# Set the default global tag if user has not given any. They are set as recommended for JEC
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/JECDataMC?rev=98
if len(options.globalTag) == 0:
    if runOnData:
        options.globalTag = '74X_dataRun2_v5 '
    else:
        options.globalTag = '74X_mcRun2_asymptotic_v4'
    
    print 'WARNING: No global tag provided. Will use the default one (' + options.globalTag + ')'


# Parse jet selection
jetSelParsed = re.match(r'(\d+)j(\d+)', options.jetSel)
if jetSelParsed is None:
    print 'Cannot parse jet selection "' + options.jetSel + '". Aborted.'
    sys.exit(1)
 
minNumJets = int(jetSelParsed.group(1))
jetPtThreshold = int(jetSelParsed.group(2))
print 'Will select events with at least', minNumJets, 'jets with pt >', jetPtThreshold, 'GeV/c.'


# Define the input files to be used for testing
process.source = cms.Source('PoolSource')

if runOnData:
    from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValSingleMuMINIAOD
    process.source.fileNames = filesRelValSingleMuMINIAOD
else:
    from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValTTbarPileUpMINIAODSIM
    process.source.fileNames = filesRelValTTbarPileUpMINIAODSIM
# process.source.fileNames = cms.untracked.vstring('/store/relval/...')

if len(options.sourceFile) > 0:
    process.source.fileNames = cms.untracked.vstring(options.sourceFile)

# Set a specific event range here (useful for debuggin)
# process.source.eventsToProcess = cms.untracked.VEventRange('1:5')

# Set the maximum number of events to process for a local run (it is overiden by CRAB)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))


# Define the paths. There is one path per each channel (electron or muon).
# Note that every module is guarenteed to run only once per event despite it can be included
# into several paths
process.elPath = cms.Path()
process.muPath = cms.Path()

# Make a simple class to add modules to all the paths simultaneously
class PathManager:
    def __init__(self, *paths_):
        self.paths = []
        for p in paths_:
            self.paths.append(p)
    
    def append(self, *modules):
        for p in self.paths:
            for m in modules:
                p += m

paths = PathManager(process.elPath, process.muPath)


# Filter on the first vertex properties. The produced vertex collection is the same as in [1]
# [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetEnCorPFnoPU2012
process.goodOfflinePrimaryVertices = cms.EDFilter('FirstVertexFilter',
    src = cms.InputTag('offlineSlimmedPrimaryVertices'),
    cut = cms.string('!isFake & ndof >= 4. & abs(z) < 24. & position.Rho < 2.'))

paths.append(process.goodOfflinePrimaryVertices)


# Define the leptons
from Analysis.PECTuples.ObjectsDefinitions_cff import *

eleQualityCuts, eleIDMaps = DefineElectrons(process, paths)
muQualityCuts = DefineMuons(process, paths)


# Include the event filters
# from Analysis.PECTuples.EventFilters_cff import ApplyEventFilters
# ApplyEventFilters(process, runOnData, goodVertices = 'goodOfflinePrimaryVertices',
#     runOnFastSim = options.runOnFastSim, run53XFilters = options.run53XSpecific)
# paths.append(process.eventFiltersSequence)


# Define the jets
DefineJets(process, paths)


# The loose event selection
process.countTightPatElectrons = cms.EDFilter('PATCandViewCountFilter',
    src = cms.InputTag('patElectronsForEventSelection'),
    minNumber = cms.uint32(1), maxNumber = cms.uint32(999))
process.countTightPatMuons = cms.EDFilter('PATCandViewCountFilter',
    src = cms.InputTag('patMuonsForEventSelection'),
    minNumber = cms.uint32(1), maxNumber = cms.uint32(999))

process.countGoodJets = cms.EDFilter('PATCandViewCountMultiFilter',
    src = cms.VInputTag('analysisPatJets'),
    cut = cms.string('pt > ' + str(jetPtThreshold)),
    minNumber = cms.uint32(minNumJets), maxNumber = cms.uint32(999))
# if not runOnData:
#     process.countGoodJets.src = cms.VInputTag('analysisPatJets', 'smearedPatJets',
#      'smearedPatJetsResUp', 'smearedPatJetsResUp',
#      'shiftedPatJetsEnUpForCorrMEt', 'shiftedPatJetsEnDownForCorrMEt')

if elChan:
    process.elPath += process.countTightPatElectrons
if muChan:
    process.muPath += process.countTightPatMuons
paths.append(process.countGoodJets)



# Modules to save the needed information to the ROOT file
# Save the info on the specified triggers. The set of triggers is aligned with the menu used in
# CMSSW_7_2_2_patch1
process.trigger = cms.EDFilter('SlimTriggerResults',
    triggers = cms.vstring(
        'Mu40', 'IsoMu20_eta2p1_IterTrk02', 'IsoMu24_eta2p1_IterTrk02',
        'Ele27_eta2p1_WP85_Gsf', 'Ele32_eta2p1_WP85_Gsf',
        'IsoMu20_eta2p1_IterTrk02_TriCentralPFJet60_50_35',
        'IsoMu20_eta2p1_IterTrk02_TriCentralPFJet40',
        'IsoMu20_eta2p1_IterTrk02_CentralPFJet30_BTagCSV',
        'IsoMu24_eta2p1_IterTrk02_TriCentralPFJet60_50_35',
        'IsoMu24_eta2p1_IterTrk02_TriCentralPFJet40',
        'IsoMu24_eta2p1_IterTrk02_CentralPFJet30_BTagCSV',
        'Ele27_eta2p1_WP85_Gsf_TriCentralPFJet40', 'Ele27_eta2p1_WP85_Gsf_TriCentralPFJet60_50_35',
        'Ele27_eta2p1_WP85_Gsf_CentralPFJet30_BTagCSV', 'Ele32_eta2p1_WP85_Gsf_TriCentralPFJet40',
        'Ele32_eta2p1_WP85_Gsf_TriCentralPFJet60_50_35',
        'Ele32_eta2p1_WP85_Gsf_CentralPFJet30_BTagCSV'),
    filter = cms.bool(False),
    savePrescales = cms.bool(options.runOnData),
    triggerBits = cms.InputTag('TriggerResults', processName = 'HLT'),
    triggerPrescales = cms.InputTag('patTrigger'))

# Save the event content
process.eventID = cms.EDAnalyzer('PECEventID')

process.eventContent = cms.EDAnalyzer('PlainEventContent',
    runOnData = cms.bool(runOnData),
    electrons = cms.InputTag('analysisPatElectrons'),
    eleIDMaps = cms.VInputTag(eleIDMaps),
    eleSelection = eleQualityCuts,
    muons = cms.InputTag('analysisPatMuons'),
    muSelection = muQualityCuts,
    jets = cms.InputTag('analysisPatJets'),
    jetMinPt = cms.double(20.),
    jetMinRawPt = cms.double(10.),
    met = cms.InputTag('slimmedMETs'),
    generator = cms.InputTag('generator'),
    primaryVertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
    puInfo = cms.InputTag('addPileupInfo'),
    rho = cms.InputTag('fixedGridRhoFastjetAll'))
    #^ There are several various versions of rho stored in an event. Apparently, this is the one
    # used for JEC [1]
    # [1] https://hypernews.cern.ch/HyperNews/CMS/get/jes/497.html?inline=-1

paths.append(process.trigger, process.eventID, process.eventContent)


# Save information about the hard interaction
if options.saveHardInteraction:
    process.hardInteraction = cms.EDAnalyzer('HardInteractionInfo',
        genParticles = cms.InputTag('prunedGenParticles'),
        saveExtraParticles = cms.vuint32(6, 23, 24, 25))
    paths.append(process.hardInteraction)


# Save information on heavy-flavour quarks
# if options.saveHeavyFlavours:
#     process.heavyFlavours = cms.EDAnalyzer('PartonShowerOutcome',
#         absPdgId = cms.vint32(4, 5),
#         genParticles = cms.InputTag('genParticles'))
#     paths.append(process.heavyFlavours)


# Save information on generator-level jets
if options.saveGenJets:
    process.genJets = cms.EDAnalyzer('GenJetsInfo',
        jets = cms.InputTag('slimmedGenJets'),
        cut = cms.string('pt > 8.'),  # the pt cut is synchronised with JME-13-005
        saveFlavourCounters = cms.bool(True))
    paths.append(process.genJets)


# In case one of the channels is not requested for the processing, remove it
if not elChan:
    process.elPath = cms.Path()
if not muChan:
    process.muPath = cms.Path()


# The output file for the analyzers
postfix = '_' + string.join([random.choice(string.letters) for i in range(3)], '')

process.TFileService = cms.Service('TFileService',
    fileName = cms.string(options.outputName + postfix + '.root'))
