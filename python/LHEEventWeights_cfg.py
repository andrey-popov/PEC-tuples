"""Configuration for cmsRun to inspect generator-level event weights.

When an extenral LHE generator is used, several event weights can be
evaluated for each event, and the nominal weight can differ from unity.
This configuration checks what weights are available and calculates
average values (per job) of weight of each type. If requested, it can
also store all weights in a ROOT file.

LHE weights are not reported directly but instead they are rescaled by
the ratio of nominal weights as available from GenEventInfoProduct and
LHEEventProduct.  This ratio might or might not be unit.

Behaviour can be controlled with a number of command-line options (see
their list in the code).
"""

import random, string


# Create a process
import FWCore.ParameterSet.Config as cms
process = cms.Process('LHEEventWeights')


# Enable MessageLogger and reduce its verbosity
process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100


# Parse command-line options
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

options.register(
    'storeWeights', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
    'Enables storing of event weights in a ROOT tree.'
)
options.register(
    'labelLHEInfoProduct', 'externalLHEProducer', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Label to access LHEEventProduct'
)

# Override defaults for automatically defined options
options.setType('outputFile', VarParsing.varType.string)
options.setDefault('outputFile', 'eventWeights.root')

options.parseArguments()


# Specify the input file
if len(options.inputFiles) == 0:
    raise RuntimeError, 'No input file is provided'

process.source = cms.Source('PoolSource',
    fileNames = cms.untracked.vstring(options.inputFiles)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))


# Service to handle the output file
if options.storeWeights:
    postfix = '_' + string.join([random.choice(string.letters) for i in range(3)], '')
    
    if options.outputFile.endswith('.root'):
        outputBaseName = options.outputFile[:-5] 
    else:
        outputBaseName = options.outputFile
    
    process.TFileService = cms.Service('TFileService',
        fileName = cms.string(outputBaseName + postfix + '.root'))


# The plugin to read and store weights
process.lheEventWeights = cms.EDAnalyzer('LHEEventWeights',
    lheRunInfoProduct = cms.InputTag(options.labelLHEInfoProduct),
    lheEventInfoProduct = cms.InputTag(options.labelLHEInfoProduct),
    generator = cms.InputTag('generator'),
    weightsHeaderTag = cms.string('initrwgt'),
    rescaleLHEWeights = cms.bool(True),
    computeMeanWeights = cms.bool(True),
    storeWeights = cms.bool(options.storeWeights),
    printToFiles = cms.bool(True)
)

process.p = cms.Path(process.lheEventWeights)
