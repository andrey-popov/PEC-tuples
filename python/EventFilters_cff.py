"""Configures rejection of anomalous events.

The module is exploited by cmsRun configuration MiniAOD_cfg.py.  It
exports function apply_event_filters, which configures the process
accordingly.
"""

import FWCore.ParameterSet.Config as cms

def apply_event_filters(
    process, paths, period, runOnData=False, processName='PAT'
):
    """Configure the process to reject anomalous events.
    
    Apply filters to reject anomalous or otherwise problematic events as
    recommended for analyses involving MET.  Follow standard
    instructions except for the filtering on primary vertices.  The
    latter is applied in the main configuration (and in a slightly
    tighter version).
    
    Arguments:
        process: The process to which the filters are to be added.
        paths: A PathManager with all active paths.
        period: Data-taking period, '2016' or '2017'.
        runOnData: Flag to distinguish processing of data and
            simulation.
        processName: Name of the process in which stored filter
            decisions were computed.
    
    Return value:
        None.
    """
    
    # Decisions of most filters are stored in MiniAOD as trigger
    # results, which are true for good events.  It is sufficient to
    # simply check the corresponding bits.  This list is appropriate for
    # both 2016 and 2017 [1].
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=136#Analysis_Recommendations_for_ana
    precomputedFilters = [
        'Flag_globalSuperTightHalo2016Filter', 'Flag_HBHENoiseFilter',
        'Flag_HBHENoiseIsoFilter', 'Flag_EcalDeadCellTriggerPrimitiveFilter',
        'Flag_BadPFMuonFilter'
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


    # An additional filter needs to be applied for 2017 and 2018 [1]
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=136#How_to_run_ecal_BadCalibReducedM
    if period == '2017':
        process.load('RecoMET.METFilters.ecalBadCalibFilter_cfi')
        bad_det_ecal_list = cms.vuint32([
            872439604,872422825,872420274,872423218,
            872423215,872416066,872435036,872439336,
            872420273,872436907,872420147,872439731,
            872436657,872420397,872439732,872439339,
            872439603,872422436,872439861,872437051,
            872437052,872420649,872422436,872421950,
            872437185,872422564,872421566,872421695,
            872421955,872421567,872437184,872421951,
            872421694,872437056,872437057,872437313
        ])
        
        process.ecalBadCalibReducedMINIAODFilter = cms.EDFilter(
            'EcalBadCalibFilter',
            EcalRecHitSource = cms.InputTag('reducedEgamma:reducedEERecHits'),
            ecalMinEt = cms.double(50.),
            baddetEcal = bad_det_ecal_list,
            taggingMode = cms.bool(False),
            debug = cms.bool(False)
        )

        paths.append(process.ecalBadCalibReducedMINIAODFilter)

