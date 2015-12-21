"""
"""

# Create a process
import FWCore.ParameterSet.Config as cms
process = cms.Process('LHEEventWeights')


# Parse command-line options
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

options.register('inputFile', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Name of the input file')

options.parseArguments()

if len(options.inputFile) == 0:
    raise RuntimeError, 'No input file is provided'


# Specify the input file
process.source = cms.Source('PoolSource',
    fileNames = cms.untracked.vstring(options.inputFile))

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))


process.lheEventWeights = cms.EDAnalyzer('LHEEventWeights',
    lheRunInfoProduct = cms.InputTag('externalLHEProducer'))

process.p = cms.Path(process.lheEventWeights)
