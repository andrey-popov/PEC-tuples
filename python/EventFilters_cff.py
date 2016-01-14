"""Configures rejection of anomalous events.

The module is exploited by cmsRun configuration MiniAOD_cfg.py.  It
exports function apply_event_filters, which configures the process
accordingly.
"""

import FWCore.ParameterSet.Config as cms

def apply_event_filters(process, paths, runOnData=False, isPromptReco=False):
    """Configure the process to reject anomalous events.
    
    Apply filters to reject anomalous or otherwise problematic events as
    recommended for analyses involving MET.  Follow instructions
    provided in [1] except for the filtering on primary vertices.  The
    latter is applied in the main configuration (and in slightly tigher
    version).
    
    Arguments:
        process: The process to which the filters are to be added.
        paths: A PathManager with all active paths.
        runOnData: Flag to distinguish processing of data and
            simulation.
        isPromptReco: Flag to distinguish PromptReco of data from
            ReReco. This affects the name of a process that stored
            decisions of some filters as trigger bits.
    
    Return value:
        None.
    
    [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=70
    """
    
    # Run filters for HBHE noise
    process.load('CommonTools.RecoAlgos.HBHENoiseFilterResultProducer_cfi')
    process.HBHENoiseFilterResultProducer.minZeros = cms.int32(99999)
    process.HBHENoiseFilterResultProducer.IgnoreTS4TS5ifJetInLowBVRegion = cms.bool(False) 
    process.HBHENoiseFilterResultProducer.defaultDecision = \
        cms.string('HBHENoiseFilterResultRun2Loose')
    
    process.ApplyBaselineHBHENoiseFilter = cms.EDFilter('BooleanFlagFilter',
        inputLabel = cms.InputTag('HBHENoiseFilterResultProducer', 'HBHENoiseFilterResult'),
        reverseDecision = cms.bool(False)
    )

    process.ApplyBaselineHBHEIsoNoiseFilter = cms.EDFilter('BooleanFlagFilter',
        inputLabel = cms.InputTag('HBHENoiseFilterResultProducer', 'HBHEIsoNoiseFilterResult'),
        reverseDecision = cms.bool(False)
    )
    
    paths.append(
        process.ApplyBaselineHBHENoiseFilter,
        process.ApplyBaselineHBHEIsoNoiseFilter
    )
    
    
    # Apply filters whose decisions are stored in MiniAOD.  The
    # corresponding trigger results are true for good events.  It is
    # thus required that all specified bits are true.  Currently, only
    # the filter for the noise from bad superclusters in EE is applied.
    process.applyEmulatedMETFilters = cms.EDFilter('HLTHighLevel',
        HLTPaths = cms.vstring('Flag_eeBadScFilter'),
        andOr = cms.bool(False),  # AND mode
        throw = cms.bool(True),
        TriggerResultsTag = cms.InputTag(
            'TriggerResults', '',
            'RECO' if runOnData and isPromptReco else 'PAT'
        )
    )
    
    paths.append(process.applyEmulatedMETFilters)
    
    
    # Applied filters based on lists of event IDs.  The lists are
    # available for data only.
    if runOnData:
        # Reject few more events with noise from a bad SC in EE.  They
        # have not been flagged by trigger bits saved in MiniAOD
        process.newEEBadSC = cms.EDFilter('EventIDFilter',
            eventListFile = cms.FileInPath('Analysis/PECTuples/data/ecalscn1043093_Dec01.txt'),
            rejectKnownEvents = cms.bool(True)
        )
        
        paths.append(process.newEEBadSC)
        
        
        # Reject events flagged by the updated CSC beam halo filter.
        # Use ROOT file with the list of event IDs instead of a text one
        # because of the large number of events.
        process.cscBeamHalo = cms.EDFilter('EventIDFilter',
            eventListFile = cms.FileInPath('Analysis/PECTuples/data/csc2015_Dec01.root'),
            rejectKnownEvents = cms.bool(True)
        )
        
        paths.append(process.cscBeamHalo)
    