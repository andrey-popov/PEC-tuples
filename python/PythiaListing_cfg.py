import FWCore.ParameterSet.Config as cms
process = cms.Process('PythiaListing')

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

options.register(
    'fileName', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
    'Name of the input file'
)
options.register(
    'run', 1, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'Run number'
)
options.register(
    'event', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'Event number'
)

options.parseArguments()

if len(options.fileName) == 0:
    raise RuntimeError, 'No input file is provided'


process.source = cms.Source('PoolSource',
    fileNames = cms.untracked.vstring(options.fileName))

if (options.event != 0):
    process.source.eventsToProcess = cms.untracked.VEventRange(
        str(options.run) + ':' + str(options.event)
    )
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.printTree = cms.EDAnalyzer('ParticleListDrawer',
    maxEventsToPrint = cms.untracked.int32(1),
    printVertex = cms.untracked.bool(False),
    src = cms.InputTag('genParticles')
)

process.p = cms.Path(process.printTree)
