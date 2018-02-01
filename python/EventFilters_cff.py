"""Configures rejection of anomalous events.

The module is exploited by cmsRun configuration MiniAOD_cfg.py.  It
exports function apply_event_filters, which configures the process
accordingly.
"""

import FWCore.ParameterSet.Config as cms

def apply_event_filters(process, paths, runOnData=False, isPromptReco=False, processName='PAT'):
    """Configure the process to reject anomalous events.
    
    Apply filters to reject anomalous or otherwise problematic events as
    recommended for analyses involving MET.  Follow instructions
    provided in [1] except for the filtering on primary vertices.  The
    latter is applied in the main configuration (and in a slightly
    tighter version).
    
    Arguments:
        process: The process to which the filters are to be added.
        paths: A PathManager with all active paths.
        runOnData: Flag to distinguish processing of data and
            simulation.
        isPromptReco: Flag to distinguish PromptReco of data from
            ReReco.  Not used currently.
        processName: Name of the process in which stored filter
            decisions were computed.
    
    Return value:
        None.
    
    [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=115#Moriond_2018
    """
    
    # Decisions of most filters are stored in MiniAOD as trigger
    # results, which are true for good events.  It is sufficient to
    # simply check the corresponding bits.
    precomputedFilters = [
        'Flag_globalTightHalo2016Filter', 'Flag_HBHENoiseFilter', 'Flag_HBHENoiseIsoFilter',
        'Flag_EcalDeadCellTriggerPrimitiveFilter', 'Flag_BadPFMuonFilter',
        'Flag_BadChargedCandidateFilter', 'Flag_ecalBadCalibFilter'
    ]
    
    if runOnData:
        precomputedFilters.append('Flag_eeBadScFilter')
    
    process.applyEmulatedMETFilters = cms.EDFilter('HLTHighLevel',
        HLTPaths = cms.vstring(precomputedFilters),
        andOr = cms.bool(False),  # AND mode
        throw = cms.bool(True),
        TriggerResultsTag = cms.InputTag('TriggerResults', '', processName)
    )
    
    paths.append(process.applyEmulatedMETFilters)
