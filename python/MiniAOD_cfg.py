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
performed, relevant reconstructed objects as well as some generator-
level properties are saved in a ROOT file with the help of a set of
dedicated EDAnalyzers.  The job does not produce any EDM output.

Behaviour can be controlled using a number of command-line options (see
their list in the code below).
"""

import random
import re
import string

import FWCore.ParameterSet.Config as cms


# Create a process.  The era is chosen according to what was used for
# the RunIIFall17MiniAOD campaign, and it is used to choose b-tagging
# configuration [1].
# [1] https://hypernews.cern.ch/HyperNews/CMS/get/btag/1483/1.html
from Configuration.StandardSequences.Eras import eras
process = cms.Process('Analysis', eras.Run2_2017)


# Enable MessageLogger and reduce its verbosity
process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1000


# Ask to print a summary in the log
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)


# Parse command-line options.  In addition to the options defined below,
# use several standard ones: inputFiles, outputFile, maxEvents.
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')

options.register(
    'globalTag', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Global tag to be used'
)
# Leptonic channels to be processed.  Here 'e' and 'm' stand for
# electron and muon respectively.
options.register(
    'channels', 'em', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Leptonic channels to process'
)
options.register(
    'jetSel', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Selection on jet pt and b-tagging discriminators'
)
options.register(
    'processIDs', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Comma-separated list of process IDs to select in simulation'
)
options.register(
    'runOnData', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Indicates whether the job processes data or simulation'
)
options.register(
    'period', '2017', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Data-taking period'
)
# options.register(
#     'isPromptReco', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
#     'In case of data, distinguishes PromptReco and ReReco. Ignored for simulation'
# )
options.register(
    'disableTriggerFilter', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Switch off filtering on selected triggers'
)
options.register(
    'triggerProcessName', 'HLT', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Name of the process that evaluated trigger decisions'
)
options.register(
    'saveAltLHEWeights', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Save alternative LHE-level event weights'
)
options.register(
    'labelLHEEventProduct', 'externalLHEProducer', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Label to access LHEEventProduct'
)
options.register(
    'saveGenParticles', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Save information about the hard(est) interaction and certain particles'
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


# Check provided data-taking period.  At the moment only '2017' is
# supported.
if options.period not in ['2017']:
    raise RuntimeError('Data-taking period "{}" is not supported.'.format(options.period))


# Make shortcuts to access some of the configuration options easily
runOnData = options.runOnData
elChan = (options.channels.find('e') != -1)
muChan = (options.channels.find('m') != -1)


# Provide a default global tag if user has not given any.  Chosen
# according to [1].  They include outdated JEC Summer16_23Sep2016V4.
# [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Global_Tags_for_2017_Nov_re_reco?rev=615
if not options.globalTag:
    if runOnData:
        options.globalTag = '94X_dataRun2_ReReco_EOY17_v2'
    else:
        options.globalTag = '94X_mc2017_realistic_v10'
    
    print 'WARNING: No global tag provided. Will use the default one: {}.'.format(
        options.globalTag
    )

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, options.globalTag)


# Define the input files
process.source = cms.Source('PoolSource',
    inputCommands = cms.untracked.vstring('keep *', 'drop LHERunInfoProduct_*_*_*')
)

if len(options.inputFiles) > 0:
    process.source.fileNames = cms.untracked.vstring(options.inputFiles)
else:
    # Default input files for testing
    if runOnData:
        # A file from era 2017F, which contains lumi sections from
        # multiple certified runs, mostly 305064 and 305377.
        process.source.fileNames = cms.untracked.vstring('/store/data/Run2017F/SingleMuon/MINIAOD/17Nov2017-v1/010000/547EF7C6-A2EC-E711-88E3-0CC47A7C356A.root')
    else:
        process.source.fileNames = cms.untracked.vstring('/store/mc/RunIIFall17MiniAOD/TTToSemiLeptonic_TuneCP5_PSweights_13TeV-powheg-pythia8/MINIAODSIM/94X_mc2017_realistic_v10-v1/50000/081541BD-E8DF-E711-AD98-02163E00C6E3.root')

# process.source.fileNames = cms.untracked.vstring('/store/relval/...')

# Set a specific event range here (useful for debugging)
# process.source.eventsToProcess = cms.untracked.VEventRange('1:5')

# Set the maximum number of events to process for a local run (it is overiden by CRAB)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))


# Override JER factors from global tags with newer versions.  The
# snippet is adapted from [1].
# [1] https://github.com/cms-met/cmssw/blob/8b17ab5d8b28236e2d2215449f074cceccc4f132/PhysicsTools/PatAlgos/test/corMETFromMiniAOD.py
process.jerDB = cms.ESSource('PoolDBESSource',
    connect = cms.string('sqlite_fip:Analysis/PECTuples/data/Spring16_25nsV10_MC.db'),
    toGet = cms.VPSet(
        cms.PSet(
            record = cms.string('JetResolutionRcd'),
            tag = cms.string('JR_Spring16_25nsV10_MC_PtResolution_AK4PFchs'),
            label = cms.untracked.string('AK4PFchs_pt')
        ),
        cms.PSet(
            record = cms.string('JetResolutionRcd'),
            tag = cms.string('JR_Spring16_25nsV10_MC_PhiResolution_AK4PFchs'),
            label = cms.untracked.string('AK4PFchs_phi')
        ),
        cms.PSet(
            record = cms.string('JetResolutionScaleFactorRcd'),
            tag = cms.string('JR_Spring16_25nsV10_MC_SF_AK4PFchs'),
            label = cms.untracked.string('AK4PFchs')
        ),
    )
)
process.jerDBPreference = cms.ESPrefer('PoolDBESSource', 'jerDB')


# Set up random-number service.  CRAB overwrites the seeds
# automatically.
process.RandomNumberGeneratorService = cms.Service('RandomNumberGeneratorService',
    analysisPatJets = cms.PSet(
        initialSeed = cms.untracked.uint32(372),
        engineName = cms.untracked.string('TRandom3')
    ),
    jetsForEventSelection = cms.PSet(
        initialSeed = cms.untracked.uint32(3631),
        engineName = cms.untracked.string('TRandom3')
    )
)


# Information about geometry and magnetic field is needed to run DeepCSV
# b-tagging.  Geometry is also needed to evaluate electron ID.
process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')


# Create processing paths.  There is one path per each channel (electron
# or muon).
process.elPath = cms.Path()
process.muPath = cms.Path()

from Analysis.PECTuples.Utils_cff import PathManager
paths = PathManager(process.elPath, process.muPath)


# Apply filtering on process IDs
if not runOnData and options.processIDs:
    process.processIDFilter = cms.EDFilter('ProcessIDFilter',
        lheEventProduct = cms.InputTag(options.labelLHEEventProduct),
        processIDs = cms.vint32([int(i) for i in options.processIDs.split(',')])
    )
    paths.append(process.processIDFilter)


# Include an event counter before any selection is applied.  It is only
# needed for simulation.
if not runOnData:
    process.eventCounter = cms.EDAnalyzer('EventCounter',
        generator = cms.InputTag('generator'),
        saveAltLHEWeights = cms.bool(options.saveAltLHEWeights),
        lheEventProduct = cms.InputTag(options.labelLHEEventProduct)
    )
    paths.append(process.eventCounter)


# Filter on properties of the first vertex
process.goodOfflinePrimaryVertices = cms.EDFilter('FirstVertexFilter',
    src = cms.InputTag('offlineSlimmedPrimaryVertices'),
    cut = cms.string('!isFake & ndof > 4. & abs(z) < 24. & position.rho < 2.')
)

paths.append(process.goodOfflinePrimaryVertices)


# Define basic reconstructed objects.  Non-standard producers are
# attached to a task.
from Analysis.PECTuples.ObjectsDefinitions_cff import (
    define_electrons, define_muons, define_jets, define_METs
)
process.analysisTask = cms.Task()

eleQualityCuts, eleEmbeddedCutBasedIDLabels, eleCutBasedIDMaps, eleMVAIDMaps = \
    define_electrons(process, process.analysisTask)
muQualityCuts = define_muons(process, process.analysisTask)
recorrectedJetsLabel, jetQualityCuts = define_jets(
    process, process.analysisTask, reapplyJEC=False, runOnData=runOnData
)
metTag = cms.InputTag('slimmedMETs')


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

if options.jetSel:
    from Analysis.PECTuples.Utils_cff import add_jet_selection
    add_jet_selection(options.jetSel, process, paths, runOnData)


# Apply event filters recommended for analyses involving MET
from Analysis.PECTuples.EventFilters_cff import apply_event_filters
apply_event_filters(
    process, paths, runOnData=runOnData,
    processName='RECO' if runOnData else 'PAT'
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
        filter = cms.bool(not options.disableTriggerFilter),
        savePrescales = cms.bool(True),
        triggerBits = cms.InputTag('TriggerResults', processName=options.triggerProcessName),
        hltPrescales = cms.InputTag('patTrigger'),
        l1tPrescales = cms.InputTag('patTrigger', 'l1min')
    )
else:
    process.pecTrigger = cms.EDFilter('SlimTriggerResults',
        triggers = cms.vstring(triggerNames),
        filter = cms.bool(not options.disableTriggerFilter),
        savePrescales = cms.bool(False),
        triggerBits = cms.InputTag('TriggerResults', processName=options.triggerProcessName)
    )

paths.append(process.pecTrigger)


# Save event ID and basic event content
process.pecEventID = cms.EDAnalyzer('PECEventID')

effAreasTemplate = 'RecoEgamma/ElectronIdentification/data/{}/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_{}.txt'

if options.period == '2017':
    effAreas = effAreasTemplate.format('Fall17', '92X')
elif options.period == '2016':
    effAreas = effAreasTemplate.format('Summer16', '80X')
else:
    effAreas = ''

process.pecElectrons = cms.EDAnalyzer('PECElectrons',
    src = cms.InputTag('analysisPatElectrons'),
    rho = cms.InputTag('fixedGridRhoFastjetAll'),
    effAreas = cms.FileInPath(effAreas),
    primaryVertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
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
    met = metTag
    # metCorrToUndo = cms.VInputTag(cms.InputTag('patPFMetT1T2Corr', 'type1'))
)

process.pecPileUp = cms.EDAnalyzer('PECPileUp',
    primaryVertices = cms.InputTag('goodOfflinePrimaryVertices'),
    rho = cms.InputTag('fixedGridRhoFastjetAll'),
    rhoCentral = cms.InputTag('fixedGridRhoFastjetCentral'),
    runOnData = cms.bool(runOnData),
    puInfo = cms.InputTag('slimmedAddPileupInfo')
)

paths.append(process.pecEventID, process.pecElectrons, process.pecMuons, process.pecJetMET,
    process.pecPileUp)


# Save global generator information
if not runOnData:
    process.pecGenerator = cms.EDAnalyzer('PECGenerator',
        generator = cms.InputTag('generator'),
        saveAltLHEWeights = cms.bool(options.saveAltLHEWeights),
        lheEventProduct = cms.InputTag(options.labelLHEEventProduct)
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
        met = metTag
    )
    paths.append(process.pecGenJetMET)


# Associate with the paths the analysis-specific task and the task
# filled by PAT tools automatically
paths.associate(process.analysisTask)

from PhysicsTools.PatAlgos.tools.helpers import getPatAlgosToolsTask
paths.associate(getPatAlgosToolsTask(process))


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
