"""Configuration for cmsRun to produce PEC tuples from MiniAOD.

The configuration uses reconstructed objects as defined in the module
ObjectsDefinitions_cff.  They are exploited to perform a loose event
selection requiring the presence of a loosely defined electron or muon
and, possibly, some jets.  The selection on jets is easily configurable,
and jet systematic uncertainties are taken into account when this
selection is evaluated.  In addition, anomalous or otherwise problematic
events are rejected using configuration provided in the module
EventFilters_cff.

After reconstructed objects are defined and the loose event selection is
performed, relevant reconstructed objects as well as some
generator-level properties are saved in a ROOT file with the help of a
set of dedicated EDAnalyzers.  The jobs does not produce and EDM output.

Behaviour can be controlled with a number of command-line options (see
their list in the code).
"""

import sys
import random
import string
import re


# Create a process
import FWCore.ParameterSet.Config as cms
process = cms.Process('Analysis')


# Enable MessageLogger and reduce its verbosity
process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100


# Ask to print a summary in the log
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    allowUnscheduled = cms.untracked.bool(True)
)


# Parse command-line options.  In addition to the options defined below,
# use several standard ones: inputFiles, outputFile, maxEvents.
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')

options.register(
    'globalTag', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'The relevant global tag'
)
# Leptonic channels to be processed.  Here 'e' and 'm' stand for
# electron and muon respectively.
options.register(
    'channels', 'em', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'The leptonic channels to process'
)
options.register(
    'jetSel', '0j30', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Selection on jets. E.g. 2j30 means that an event must contain at least 2 jets with '
    'pt > 30 GeV/c'
)
options.register(
    'runOnData', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Indicates whether it runs on the real data'
)
options.register(
    'isPromptReco', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'In case of data, distinguishes PromptReco and ReReco. Ignored for simulation'
)
options.register(
    'triggerProcessName', 'HLT', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Name of process that evaluated trigger decisions'
)
options.register(
    'saveLHEWeightVars', True, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Indicates whether LHE-level variations of event weights should be stored'
)
options.register(
    'labelLHEEventProduct', 'externalLHEProducer', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Label to access LHEEventProduct'
)
options.register(
    'saveGenParticles', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Save information about the hard(est) interaction and selected particles'
)
options.register(
    'saveGenJets', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Save information about generator-level jets'
)

# Override defaults for automatically defined options
options.setDefault('maxEvents', 100)
options.setType('outputFile', VarParsing.varType.string)
options.setDefault('outputFile', 'sample.root')

options.parseArguments()


# Make the shortcuts to access some of the configuration options easily
runOnData = options.runOnData
elChan = (options.channels.find('e') != -1)
muChan = (options.channels.find('m') != -1)


# Provide a default global tag if user has not given any.  With data use
# the global tag for prompt reconstruction [1].  With simulation take
# the one recommended by JERC group [2].
# [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions?rev=575#Prompt_reconstruction_Global_Tag
# [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/JECDataMC?rev=119
if len(options.globalTag) == 0:
    if runOnData:
        options.globalTag = '80X_dataRun2_Prompt_v9'
    else:
        options.globalTag = '80X_mcRun2_asymptotic_2016_miniAODv2_v1'
    
    print 'WARNING: No global tag provided. Will use the default one (' + options.globalTag + ')'

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, options.globalTag)


# Parse jet selection
jetSelParsed = re.match(r'(\d+)j(\d+)', options.jetSel)
if jetSelParsed is None:
    print 'Cannot parse jet selection "' + options.jetSel + '". Aborted.'
    sys.exit(1)
 
minNumJets = int(jetSelParsed.group(1))
jetPtThreshold = int(jetSelParsed.group(2))

if minNumJets > 0:
    print 'Will select events with at least', minNumJets,
    print 'jets with pt >', jetPtThreshold, 'GeV/c.'


# Define the input files
process.source = cms.Source('PoolSource')

if len(options.inputFiles) > 0:
    process.source.fileNames = cms.untracked.vstring(options.inputFiles)
else:
    # Default input files for testing
    if runOnData:
        # from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValSingleMuMINIAOD
        # process.source.fileNames = filesRelValSingleMuMINIAOD
        process.source.fileNames = cms.untracked.vstring('/store/data/Run2016D/SingleMuon/MINIAOD/PromptReco-v2/000/276/315/00000/168C3DE5-F444-E611-A012-02163E014230.root')
    else:
        # from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValTTbarPileUpMINIAODSIM
        # process.source.fileNames = filesRelValTTbarPileUpMINIAODSIM
        process.source.fileNames = cms.untracked.vstring('/store/mc/RunIISpring16MiniAODv2/TT_TuneCUETP8M1_13TeV-powheg-pythia8/MINIAODSIM/PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14_ext3-v1/00000/0064B539-803A-E611-BDEA-002590D0B060.root')

# process.source.fileNames = cms.untracked.vstring('/store/relval/...')

# Set a specific event range here (useful for debugging)
# process.source.eventsToProcess = cms.untracked.VEventRange('1:5')

# Set the maximum number of events to process for a local run (it is overiden by CRAB)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))


# Create processing paths.  There is one path per each channel (electron
# or muon).
process.elPath = cms.Path()
process.muPath = cms.Path()


# A simple class to add modules to all the paths simultaneously
class PathManager:
    
    def __init__(self, *paths):
        self.paths = []
        for p in paths:
            self.paths.append(p)
    
    def append(self, *modules):
        for p in self.paths:
            for m in modules:
                p += m

paths = PathManager(process.elPath, process.muPath)


# Filter on properties of the first vertex
process.goodOfflinePrimaryVertices = cms.EDFilter('FirstVertexFilter',
    src = cms.InputTag('offlineSlimmedPrimaryVertices'),
    cut = cms.string('!isFake & ndof > 4. & abs(z) < 24. & position.rho < 2.')
)

paths.append(process.goodOfflinePrimaryVertices)


# Define basic reconstructed objects
from Analysis.PECTuples.ObjectsDefinitions_cff import (define_electrons, define_muons, define_jets,
    define_METs)

(eleQualityCuts, eleEmbeddedCutBasedIDLabels, eleCutBasedIDMaps, eleMVAIDMaps) = \
    define_electrons(process)
muQualityCuts = define_muons(process)
(recorrectedJetsLabel, jetQualityCuts, pileUpIDMap) = \
    define_jets(process, reapplyJEC=True, runOnData=runOnData)
define_METs(process, runOnData=runOnData)


# The loose event selection
process.countTightPatElectrons = cms.EDFilter('PATCandViewCountFilter',
    src = cms.InputTag('patElectronsForEventSelection'),
    minNumber = cms.uint32(1), maxNumber = cms.uint32(999)
)
process.countTightPatMuons = cms.EDFilter('PATCandViewCountFilter',
    src = cms.InputTag('patMuonsForEventSelection'),
    minNumber = cms.uint32(1), maxNumber = cms.uint32(999)
)
if elChan:
    process.elPath += process.countTightPatElectrons
if muChan:
    process.muPath += process.countTightPatMuons

if minNumJets > 0:
    process.countGoodJets = cms.EDFilter('SystAwareJetSelector',
        src = cms.InputTag('analysisPatJets'),
        jetTypeLabel = cms.string('AK4PFchs'),
        minPt = cms.double(jetPtThreshold),
        includeJERCVariations = cms.bool(not runOnData),
        genJets = cms.InputTag('slimmedGenJets'),
        rho = cms.InputTag('fixedGridRhoFastjetAll'),
        minNum = cms.uint32(minNumJets)
    )
    paths.append(process.countGoodJets)


# Apply event filters recommended for analyses involving MET
from Analysis.PECTuples.EventFilters_cff import apply_event_filters
apply_event_filters(
    process, paths, runOnData=runOnData,
    isPromptReco=options.isPromptReco
)


# Save decisions of selected triggers.  The lists are based on menu [1],
# which was used in the re-HLT campaign with RunIISpring16MiniAODv2.
# Events that are not accepted by any of the considered triggers are
# rejected since they cannot be used in an analysis.
# [1] /frozen/2016/25ns10e33/v2.1/HLT/V3
triggerNames = [
    # Single-lepton paths
    'Mu45_eta2p1', 'Mu50', 'Mu55',
    'IsoMu20', 'IsoTkMu20', 'IsoMu22', 'IsoTkMu22', 'IsoMu22_eta2p1', 'IsoTkMu22_eta2p1',
    'IsoMu24', 'IsoTkMu24', 'IsoMu27', 'IsoTkMu27',
    'Ele23_WPLoose_Gsf', 'Ele24_eta2p1_WPLoose_Gsf',
    'Ele25_WPTight_Gsf', 'Ele25_eta2p1_WPLoose_Gsf', 'Ele25_eta2p1_WPTight_Gsf',
    'Ele27_WPLoose_Gsf', 'Ele27_WPTight_Gsf',
    'Ele27_eta2p1_WPLoose_Gsf', 'Ele27_eta2p1_WPTight_Gsf',
    'Ele32_eta2p1_WPTight_Gsf', 'Ele35_WPLoose_Gsf',
    # Dilepton paths
    'Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL',
    'Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL',
    'Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ', 'Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ',
    'Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ',
    # Cross-triggers
    'Ele27_eta2p1_WPLoose_Gsf_HT200'
]

if runOnData:
    process.pecTrigger = cms.EDFilter('SlimTriggerResults',
        triggers = cms.vstring(triggerNames),
        filter = cms.bool(True),
        savePrescales = cms.bool(True),
        triggerBits = cms.InputTag('TriggerResults', processName=options.triggerProcessName),
        hltPrescales = cms.InputTag('patTrigger'),
        l1tPrescales = cms.InputTag('patTrigger', 'l1min')
    )
else:
    process.pecTrigger = cms.EDFilter('SlimTriggerResults',
        triggers = cms.vstring(triggerNames),
        filter = cms.bool(True),
        savePrescales = cms.bool(False),
        triggerBits = cms.InputTag('TriggerResults', processName=options.triggerProcessName)
    )

paths.append(process.pecTrigger)


# Save event ID and basic event content
process.pecEventID = cms.EDAnalyzer('PECEventID')

process.pecElectrons = cms.EDAnalyzer('PECElectrons',
    src = cms.InputTag('analysisPatElectrons'),
    rho = cms.InputTag('fixedGridRhoFastjetAll'),
    effAreas = cms.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt'),
    embeddedBoolIDs = cms.vstring(eleEmbeddedCutBasedIDLabels),
    boolIDMaps = cms.VInputTag(eleCutBasedIDMaps),
    contIDMaps = cms.VInputTag(eleMVAIDMaps),
    selection = eleQualityCuts
)

process.pecMuons = cms.EDAnalyzer('PECMuons',
    src = cms.InputTag('analysisPatMuons'),
    selection = muQualityCuts,
    primaryVertices = cms.InputTag('offlineSlimmedPrimaryVertices')
)

process.pecJetMET = cms.EDAnalyzer('PECJetMET',
    runOnData = cms.bool(runOnData),
    jets = cms.InputTag('analysisPatJets'),
    jetSelection = jetQualityCuts,
    contIDMaps = cms.VInputTag(pileUpIDMap),
    met = cms.InputTag('slimmedMETs', processName=process.name_())
)

process.pecPileUp = cms.EDAnalyzer('PECPileUp',
    primaryVertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
    rho = cms.InputTag('fixedGridRhoFastjetAll'),
    runOnData = cms.bool(runOnData),
    puInfo = cms.InputTag('slimmedAddPileupInfo')
)

paths.append(process.pecEventID, process.pecElectrons, process.pecMuons, process.pecJetMET,
    process.pecPileUp)


# Save global generator information
if not runOnData:
    process.pecGenerator = cms.EDAnalyzer('PECGenerator',
        generator = cms.InputTag('generator'),
        saveLHEWeightVars = cms.bool(options.saveLHEWeightVars),
        lheEventInfoProduct = cms.InputTag(options.labelLHEEventProduct)
    )
    paths.append(process.pecGenerator)


# Save information about the hard interaction and selected particles
if not runOnData and options.saveGenParticles:
    process.pecGenParticles = cms.EDAnalyzer('PECGenParticles',
        genParticles = cms.InputTag('prunedGenParticles'),
        saveExtraParticles = cms.vuint32(6, 23, 24, 25)
    )
    paths.append(process.pecGenParticles)


# Save information on generator-level jets and MET
if not runOnData and options.saveGenJets:
    process.pecGenJetMET = cms.EDAnalyzer('PECGenJetMET',
        jets = cms.InputTag('slimmedGenJets'),
        cut = cms.string('pt > 8.'),
        # ^The pt cut above is the same as in JME-13-005
        saveFlavourCounters = cms.bool(True),
        met = cms.InputTag('slimmedMETs', processName=process.name_())
    )
    paths.append(process.pecGenJetMET)


# If one of possible channels has not been requested by the user, clear
# the corresponding path
if not elChan:
    process.elPath = cms.Path()
if not muChan:
    process.muPath = cms.Path()


# The output file for the analyzers
postfix = '_' + string.join([random.choice(string.letters) for i in range(3)], '')

if options.outputFile.endswith('.root'):
    outputBaseName = options.outputFile[:-5] 
else:
    outputBaseName = options.outputFile

process.TFileService = cms.Service('TFileService',
    fileName = cms.string(outputBaseName + postfix + '.root'))
