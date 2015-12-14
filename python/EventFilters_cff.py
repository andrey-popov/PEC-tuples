import FWCore.ParameterSet.Config as cms

def ApplyEventFilters(process, paths, runOnData = False,
                      goodVertices = 'goodOfflinePrimaryVertices'):
    """ 
        (Documentation is to be added.)
    """
    
    # All filters except for the selection on primary vertices are applied as described in [1].
    # The selection on primary vertices is tighter than needed for analyses with MET, and it is
    # applied in the main configuration
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=70
    
    # Run filters for HBHE noise
    process.load('CommonTools.RecoAlgos.HBHENoiseFilterResultProducer_cfi')
    process.HBHENoiseFilterResultProducer.minZeros = cms.int32(99999)
    process.HBHENoiseFilterResultProducer.IgnoreTS4TS5ifJetInLowBVRegion = cms.bool(False) 
    process.HBHENoiseFilterResultProducer.defaultDecision = \
        cms.string('HBHENoiseFilterResultRun2Loose')
    
    process.ApplyBaselineHBHENoiseFilter = cms.EDFilter('BooleanFlagFilter',
        inputLabel = cms.InputTag('HBHENoiseFilterResultProducer', 'HBHENoiseFilterResult'),
        reverseDecision = cms.bool(False))

    process.ApplyBaselineHBHEIsoNoiseFilter = cms.EDFilter('BooleanFlagFilter',
        inputLabel = cms.InputTag('HBHENoiseFilterResultProducer', 'HBHEIsoNoiseFilterResult'),
        reverseDecision = cms.bool(False))
    
    paths.append(process.HBHENoiseFilterResultProducer, process.ApplyBaselineHBHENoiseFilter,
        process.ApplyBaselineHBHEIsoNoiseFilter)
    
    
    # Apply filters whose decisions are stored in MiniAOD. The corresponding trigger results are
    # true for good events. It is thus required that all specified bits are true. Currently, only
    # the filter for the noise from bad superclusters in EE is applied
    process.applyEmulatedMETFilters = cms.EDFilter('HLTHighLevel',
        HLTPaths = cms.vstring('Flag_eeBadScFilter'),
        andOr = cms.bool(False),  # AND mode
        throw = cms.bool(True),
        TriggerResultsTag = cms.InputTag('TriggerResults', '', 'PAT'))
    
    paths.append(process.applyEmulatedMETFilters)
    