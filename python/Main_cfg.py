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

import random
import string


# Metadata
__author__ = 'Andrey Popov'
__email__ = 'Andrey.Popov@cern.ch'


# The skeleton PAT configuration
from PhysicsTools.PatAlgos.patTemplate_cfg import *


# The configuration supports options that can be given in the command line or pycfg_params in the
# [CMSSW] section of the CRAB configuration. An example of calling syntax:
#    cmsRun Main_cfg.py runOnData=False,HLTProcess=REDIGI311X
from UserCode.SingleTop.VarParsing import VarParsing  # multicrab bugfix
options = VarParsing ('python')

options.register('runOnData', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Indicates whether it runs on the real data')
options.register('HLTProcess', 'HLT', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'The name of the HLT process')
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
options.register('runFlavourAnalyzer', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Run FlavourAnalyzer to save the info on heavy flavours')
options.register('sourceFile', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'The name of the source file')
options.register('runOnFastSim', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Indicates whether FastSim is processed')
options.register('run53XSpecific', True, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Indicates whether the input dataset was reconstructed in 53X')

options.parseArguments()


# Make the shortcuts to access some of the configuration options easily
runOnData = options.runOnData
elChan = (options.channels.find('e') != -1)
muChan = (options.channels.find('m') != -1)


# Set the default global tag
if len(options.globalTag) == 0:
    if runOnData:
        options.globalTag = 'FT53_V21A_AN6'
    else:
        options.globalTag = 'START53_V27'
process.GlobalTag.globaltag = options.globalTag + '::All'


# Rerun jet probability calibration when running over simulation [1-2]. Real data form 22Jan2013
# rereco is fine
# [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagPOG?rev=169#2012_Data_and_MC_EPS13_prescript
# [2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagJetProbabilityCalibration?rev=24#Calibration_in_53x_Data_and_MC
if not runOnData:
    process.GlobalTag.toGet = cms.VPSet(
        cms.PSet(record = cms.string('BTagTrackProbability2DRcd'),
           tag = cms.string('TrackProbabilityCalibration_2D_MC53X_v2'),
           connect = cms.untracked.string('frontier://FrontierPrep/CMS_COND_BTAU')),
        cms.PSet(record = cms.string('BTagTrackProbability3DRcd'),
           tag = cms.string('TrackProbabilityCalibration_3D_MC53X_v2'),
           connect = cms.untracked.string('frontier://FrontierPrep/CMS_COND_BTAU')))


# Define the input files to be used for testing
if runOnData:
    from PhysicsTools.PatAlgos.patInputFiles_cff import filesSingleMuRECO
    process.source.fileNames = filesSingleMuRECO
else:
    from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValProdTTbarAODSIM
    process.source.fileNames = filesRelValProdTTbarAODSIM

if len(options.sourceFile) > 0:
    process.source.fileNames = cms.untracked.vstring(options.sourceFile)

# Set a specific event range here (useful for debuggin)
#process.source.eventsToProcess = cms.untracked.VEventRange('1:2807803')

# Set the maximum number of events to process for a local run (it is overiden by CRAB)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

# Reduce the verbosity for a local run
process.MessageLogger.cerr.FwkReport.reportEvery = 100


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
    src = cms.InputTag('offlinePrimaryVertices'),
    cut = cms.string('!isFake & ndof >= 4. & abs(z) < 24. & position.Rho < 2.'))

paths.append(process.goodOfflinePrimaryVertices)


# Specify the JEC needed
inputJetCorrLabel = ('AK5PFchs', ['L1FastJet', 'L2Relative', 'L3Absolute'])
if runOnData:
    inputJetCorrLabel[1].append('L2L3Residual')


# PF2PAT setup. The code snippet is inspired by [1]. In order to make life simplier, the (empty)
# postfix is hard-coded. See [2] for information on MET corrections with PFBRECO and PAT
# [1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/PhysicsTools/PatExamples/test/patTuple_52x_jec_cfg.py?revision=1.1&view=markup
# [2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis#Type_I_II_0_with_PF2PAT
from PhysicsTools.PatAlgos.tools.pfTools import usePF2PAT
usePF2PAT(process, runPF2PAT = True, runOnMC = not runOnData, postfix = '',
    jetAlgo = 'AK5', jetCorrections = inputJetCorrLabel,
    pvCollection = cms.InputTag('goodOfflinePrimaryVertices'),
    typeIMetCorrections = runOnData,  # in case of MC the corrections are added with MET unc. tool
    outputModules = [])

# A recommended setting for JEC with CHS
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetEnCorPFnoPU2012
process.pfPileUp.checkClosestZVertex = False


# We do not consider tau-leptons in the analysis (they are included in jets)
process.pfNoTau.enable = False



# Define the leptons
from UserCode.SingleTop.ObjectsDefinitions_cff import *

eleQualityCuts = DefineElectrons(process, process.patPF2PATSequence, runOnData)
muQualityCuts = DefineMuons(process, process.patPF2PATSequence, runOnData)


# Add the PF2PAT sequence to the paths
paths.append(process.patPF2PATSequence)


# Include the event filters
from UserCode.SingleTop.EventFilters_cff import ApplyEventFilters
ApplyEventFilters(process, runOnData, goodVertices = 'goodOfflinePrimaryVertices',
    runOnFastSim = options.runOnFastSim, run53XFilters = options.run53XSpecific)
paths.append(process.eventFiltersSequence)


# Define the MET
METCollections = DefineMETs(process, paths, runOnData, inputJetCorrLabel[1][-1])


# Define the jets
DefineJets(process, paths, runOnData)


# The loose event selection
process.countTightPatElectrons = process.countPatElectrons.clone(
    src = 'patElectronsForEventSelection',
    minNumber = 1, maxNumber = 999)
process.countTightPatMuons = process.countPatMuons.clone(
    src = 'patMuonsForEventSelection',
    minNumber = 1, maxNumber = 999)
process.countHighPtPatJets = process.countPatJets.clone(
    src = 'patJetsForEventSelection',
    minNumber = 2, maxNumber = 999)

if elChan:
    process.elPath += process.countTightPatElectrons
if muChan:
    process.muPath += process.countTightPatMuons
paths.append(process.countHighPtPatJets)



# Modules to save the needed information to the ROOT file
# Save the info on the specified triggers
process.trigger = cms.EDFilter('TriggerResults',
    whiteList = cms.vstring(
        r'^HLT_(PF)?Jet\\d+_v\\d+$', r'^HLT_(Iso)?Mu\\d+_', r'^HLT_Ele\\d+_', r'^HLT_DoubleMu\\d+_',
        r'^HLT_DoubleEle\\d+_', r'^HLT_RelIso\\d+'),
    blackList = cms.vstring(
        r'_Jpsi_', r'_PFMT\\d+', r'_R\\d+_', r'Photon\\d+_', r'PFTau\\d+', r'_HT\\d+_',
        r'_PFMHT\\d+_', r'Acoplanarity', r'_L1', r'_Track', r'Displaced', r'Dimuon', r'_Deta',
        r'_PFHT\\d+_', r'_Rsq', r'_Mass', r'_trackless_', r'_HFT\\d+', r'_PFMET', r'_WCand',
        r'_FJHT\\d+'),
    filter = cms.bool(False),
    dumper = cms.bool(True),
    orMode = cms.bool(False),
    triggerProcessName = cms.string(options.HLTProcess))

# Save the event content
process.eventContent = cms.EDAnalyzer('PlainEventContent',
    runOnData = cms.bool(runOnData),
    electrons = cms.InputTag('nonIsolatedLoosePatElectrons'),
    eleSelection = eleQualityCuts,
    muons = cms.InputTag('nonIsolatedLoosePatMuons'),
    muSelection = muQualityCuts,
    jets = cms.InputTag('analysisPatJets'),
    jetCut = cms.string('pt > 20.'),
    softJetCut = cms.string('pt > 10.'),
    JERSystJets = cms.VInputTag('smearedPatJetsResUp', 'smearedPatJetsResDown'),
    METs = cms.VInputTag(*METCollections),
    saveHardInteraction = cms.bool(options.saveHardInteraction),
    generator = cms.InputTag('generator'),
    genParticles = cms.InputTag('genParticles'),
    primaryVertices = cms.InputTag('offlinePrimaryVertices'),
    PUInfo = cms.InputTag('addPileupInfo'),
    rho = cms.InputTag('kt6PFJets', 'rho'))

paths.append(process.trigger, process.eventContent)

# Save the info on heavy flavour quarks
if options.runFlavourAnalyzer:
    process.flavourAnalyzer = cms.EDAnalyzer('FlavourAnalyzer',
        genParticles = cms.InputTag('genParticles'),
        generator = cms.InputTag('generator'),
        genJets = cms.InputTag('ak5GenJets'),
        saveGenJets = cms.bool(False),
        saveMinimalisticChains = cms.bool(True))
    paths.append(process.flavourAnalyzer)

# In case one of the channels is not requested for the processing, remove it
if not elChan:
    process.elPath = cms.Path()
if not muChan:
    process.muPath = cms.Path()


# The output file for the analyzers
postfix = '_' + string.join([random.choice(string.letters) for i in range(3)], '')

process.TFileService = cms.Service('TFileService',
    fileName = cms.string(options.outputName + postfix + '.root'))


# Remove the output module (it is declared in patTemplate_cfg)
process.outpath.remove(process.out)
del process.out

