# Metadata
__author__ = 'Andrey Popov'
__email__ = 'Andrey.Popov@cern.ch'

import FWCore.ParameterSet.Config as cms

# Function to set the filters up in accordance with the process
def ApplyEventFilters(process, goodVertices = 'goodOfflinePrimaryVertices', runOnFastSim = False,
    run53XFilters = True):
    """ The function initialises a number of filters to reject anomalous events. It packs them into
        sequence 'eventFilterSequence' which is added to the process (but the user has to insert it
        into the appropriate paths). Some of the filters depend on the PAT collections, therefore
        the prepared sequence must be added after the PAT one. All the filters are applied to both
        real data and MC simulation on identical basis.
        
        The arguments:
        
        process: An instance of cms.Process which the filters are added to.
        
        goodVertices: The collection of the selected ("good") primary vertices.
        
        runOnFastSim: Indicates whether the code processes a dataset produced with FastSimulation.
            Some of the filters cannot be evaluated in this case and are switched off.
        
        run53XFilters: Indicated whether to run the filters which require information stored in 53X
            only but not in 52X. Since the RelVals are reconstructed in 52X, these filters ought to
            be turned off in test runs.
        """
    eventFiltersSequence = cms.Sequence()
    
    
    # Scraping events
    # https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCollisionsDataAnalysis#Recipes_to_get_started
    process.scrapingFilter = cms.EDFilter('FilterOutScraping',
        applyfilter = cms.untracked.bool(True),
        debugOn = cms.untracked.bool(False),
        numtrack = cms.untracked.uint32(10),
        thresh = cms.untracked.double(0.25))
    
    eventFiltersSequence += process.scrapingFilter
    
    
    # CSC beam halo
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#CSC_Beam_Halo_Filter
    if not runOnFastSim:
        process.load('RecoMET.METAnalyzers.CSCHaloFilter_cfi')
        eventFiltersSequence += process.CSCTightHaloFilter
    
    
    # HBHE noise filter. MET TWiki states (*) that requirements for noise isolation are needed despite
    # HBHE noise TWiki switches them off (**)
    # (*) https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#HBHE_Noise_Filter
    # (**) https://twiki.cern.ch/twiki/bin/view/CMS/HBHEAnomalousSignals2011
    if not runOnFastSim:
        process.load('CommonTools.RecoAlgos.HBHENoiseFilter_cfi')
        eventFiltersSequence += process.HBHENoiseFilter
    
    
    # HCAL laser events
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#HCAL_laser_events
    # https://twiki.cern.ch/twiki/bin/view/CMS/PdmVKnowFeatures#HCAL_laser_events_in_prompt_2012
    process.load("EventFilter.HcalRawToDigi.hcallasereventfilter2012_cff")
    
    eventFiltersSequence += process.hcallLaserEvent2012Filter
    
    
    # ECAL dead cell filter
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#ECAL_dead_cell_filter
    process.load('RecoMET.METFilters.EcalDeadCellTriggerPrimitiveFilter_cfi')
    process.EcalDeadCellTriggerPrimitiveFilter.tpDigiCollection = cms.InputTag('ecalTPSkimNA')
    
    process.load('RecoMET.METFilters.EcalDeadCellBoundaryEnergyFilter_cfi')
    process.EcalDeadCellBoundaryEnergyFilter.taggingMode = cms.bool(False)
    process.EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyDeadCellsEB=cms.untracked.double(10)
    process.EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyDeadCellsEE=cms.untracked.double(10)
    process.EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyGapEB=cms.untracked.double(100)
    process.EcalDeadCellBoundaryEnergyFilter.cutBoundEnergyGapEE=cms.untracked.double(100)
    process.EcalDeadCellBoundaryEnergyFilter.enableGap=cms.untracked.bool(False)
    process.EcalDeadCellBoundaryEnergyFilter.limitDeadCellToChannelStatusEB = cms.vint32(12,14)
    process.EcalDeadCellBoundaryEnergyFilter.limitDeadCellToChannelStatusEE = cms.vint32(12,14)
    
    eventFiltersSequence += process.EcalDeadCellTriggerPrimitiveFilter
    # EcalDeadCellBoundaryEnergyFilter is configured but not actually used yet
    
    
    # Tracking failure filter 
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#Tracking_failure_filter
    process.load('RecoMET.METFilters.trackingFailureFilter_cfi')
    process.trackingFailureFilter.VertexSource = goodVertices
    eventFiltersSequence += process.trackingFailureFilter
    
    
    # Bad EE supercrystal filter
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#Bad_EE_Supercrystal_filter_added
    process.load('RecoMET.METFilters.eeBadScFilter_cfi')
    eventFiltersSequence += process.eeBadScFilter
    
    
    # Large laser correction calibration filter
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#EB_or_EE_Xtals_with_large_laser
    process.load('RecoMET.METFilters.ecalLaserCorrFilter_cfi')
    eventFiltersSequence += process.ecalLaserCorrFilter
    
    
    # Tracking POG filters
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters#Tracking_odd_events_filters_trac
    # https://twiki.cern.ch/twiki/bin/view/CMS/TrackingPOGFilters#Filters
    if not runOnFastSim and run53XFilters:
        process.load('RecoMET.METFilters.trackingPOGFilters_cff')
        eventFiltersSequence += process.trkPOGFilters
    
    
    
    # Add the sequence containing all the filters to the process
    process.eventFiltersSequence = eventFiltersSequence
