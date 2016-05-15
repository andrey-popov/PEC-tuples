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
    latter is applied in the main configuration (and in a slightly
    tigher version).
    
    Arguments:
        process: The process to which the filters are to be added.
        paths: A PathManager with all active paths.
        runOnData: Flag to distinguish processing of data and
            simulation.
        isPromptReco: Flag to distinguish PromptReco of data from
            ReReco.  Not used currently.
    
    Return value:
        None.
    
    [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=82#MiniAOD_805
    """
    
    # Decisions of all filters are stored in MiniAOD as trigger results,
    # which are true for good events.  It is sufficient to simply check
    # the corresponding bits.
    process.applyEmulatedMETFilters = cms.EDFilter('HLTHighLevel',
        HLTPaths = cms.vstring(
            'Flag_HBHENoiseFilter', 'Flag_HBHENoiseIsoFilter', 'Flag_CSCTightHalo2015Filter',
            'Flag_EcalDeadCellTriggerPrimitiveFilter', 'Flag_eeBadScFilter'),
        andOr = cms.bool(False),  # AND mode
        throw = cms.bool(True),
        TriggerResultsTag = cms.InputTag(
            'TriggerResults', '',
            'RECO' if runOnData else 'PAT'
        )
    )
    
    paths.append(process.applyEmulatedMETFilters)
    