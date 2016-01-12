"""
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

options.register('inputFile', '', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Name of the input file')
options.register('storeWeights', False, VarParsing.multiplicity.singleton,
    VarParsing.varType.bool, 'Enables storing of event weights in a ROOT tree.')
options.register('outputName', 'eventWeights', VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 'Name of the output ROOT file.')

options.parseArguments()


# Specify the input file
if len(options.inputFile) == 0:
    raise RuntimeError, 'No input file is provided'

process.source = cms.Source('PoolSource',
    fileNames = cms.untracked.vstring(options.inputFile))

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))

if not options.storeWeights:
    # There is no need to read any events
    process.maxEvents.input = 1


# Service to handle the output file
if options.storeWeights:
    postfix = '_' + string.join([random.choice(string.letters) for i in range(3)], '')

    process.TFileService = cms.Service('TFileService',
        fileName = cms.string(options.outputName + postfix + '.root'))


# The plugin to read and store weights
process.lheEventWeights = cms.EDAnalyzer('LHEEventWeights',
    lheRunInfoProduct = cms.InputTag('externalLHEProducer'),
    lheEventInfoProduct = cms.InputTag('externalLHEProducer'),
    weightsHeaderTag = cms.string('initrwgt'),
    computeMeanWeights = cms.bool(True),
    storeWeights = cms.bool(options.storeWeights),
    printToFiles = cms.bool(True))

process.p = cms.Path(process.lheEventWeights)
