""" The module contains definitions of physical objects including required adjustments to the
    reconstruction process. Functions defined here must be called after usePF2PAT, name of the
    modules are hard-coded.
    """


import FWCore.ParameterSet.Config as cms


def DefineElectrons(process, paths):
    """ 
        This function adjusts electrons. The user is expected to use only the following collections
        and objects:
        
        analysisPatElectrons: Collection of loose non-isolated electrons to be saved in tuples.
        
        patElectronsForEventSelection: Collection of electrons that pass basic kinematical cuts.
        To be used in the loose event selection.
        
        eleEmbeddedCutBasedIDLabels: Labels of embedded boolean electron IDs to be stored in tuples.
        
        eleCutBasedIDMaps: Input tags to access maps of boolean electron IDs to be stored in tuples.
        
        eleMVAIDMaps: Input tags to access real-valued electron IDs to be stored in tuples.
        
        eleQualityCuts: Vector of quality cuts whose decisions are to be stured in tuples.
    """
    
    # Collection of electrons that will be stored in tuples
    process.analysisPatElectrons = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('slimmedElectrons'),
        cut = cms.string('pt > 20. & abs(eta) < 2.5'))
    
    paths.append(process.analysisPatElectrons)
    
    
    # Labels to access embedded cut-based ID
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=27
    eleEmbeddedCutBasedIDLabels = ['cutBasedElectronID-Spring15-25ns-V1-standalone-' + p \
     for p in ['veto', 'loose', 'medium', 'tight']]
    
    
    # Decisions of triggering MVA ID are not stored in MiniAOD2015v2 and should be calculated
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentificationRun2?rev=23
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import switchOnVIDElectronIdProducer, \
     setupAllVIDIdsInModule, setupVIDElectronSelection, DataFormat
    switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
    
    for idModule in ['mvaElectronID_Spring15_25ns_Trig_V1_cff']:
        setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.' + \
         idModule, setupVIDElectronSelection)
    
    process.electronMVAValueMapProducer.srcMiniAOD = 'analysisPatElectrons'
    paths.append(process.electronMVAValueMapProducer)
    
    
    # Labels of maps with electron ID. No maps are needed for the cut-based ID since its decisions
    # are saved in pat::Electron
    eleCutBasedIDMaps = []
    eleMVAIDMaps = [
        'electronMVAValueMapProducer:ElectronMVAEstimatorRun2Spring15Trig25nsV1Values']
    
    
    # Additional selections to be evaluated
    eleQualityCuts = cms.vstring(
        # EE-EB gap
        '(abs(superCluster.eta) < 1.4442 | abs(superCluster.eta) > 1.5660)',
        # Trigger-emulating preselection [1], referenced from [2]
        # [1] https://hypernews.cern.ch/HyperNews/CMS/get/egamma/1645/2/1/1.html
        # [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentificationRun2?rev=23#Recipes_for_7_4_12_Spring15_MVA
        'pt > 15. & \
         ((abs(superCluster.eta) < 1.4442 & full5x5_sigmaIetaIeta < 0.012 & hcalOverEcal < 0.9 & \
          ecalPFClusterIso / pt < 0.37 & hcalPFClusterIso / pt < 0.25 & dr03TkSumPt / pt < 0.18 & \
          abs(deltaEtaSuperClusterTrackAtVtx) < 0.0095 & \
          abs(deltaPhiSuperClusterTrackAtVtx) < 0.065) | \
         (abs(superCluster.eta) > 1.5660 & full5x5_sigmaIetaIeta < 0.033 & hcalOverEcal < 0.09 & \
          ecalPFClusterIso / pt < 0.45 & hcalPFClusterIso / pt < 0.28 & dr03TkSumPt / pt < 0.18))')
    
    
    # Define electrons to be used for event selection at the Grid level. They are subjected to
    # tighter kinematical cuts
    process.patElectronsForEventSelection = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('analysisPatElectrons'),
        cut = cms.string('pt > 27. & abs(eta) < 2.1'))
    
    paths.append(process.patElectronsForEventSelection)
    
    
    # Return values
    return eleQualityCuts, eleEmbeddedCutBasedIDLabels, eleCutBasedIDMaps, eleMVAIDMaps



def DefineMuons(process, paths):
    """ This function adjusts muons. The following collections and variables are expected to be
        used by the user:
        
        1. analysisPatMuons: collection of loose non-isolated muons to be stored in tuples.
        
        2. muQualityCuts: vector of quality cuts to be applied to the above collection.
        
        3. patMuonsForEventSelection: collection of loose non-isolated muons that pass basic
        kinematical requirements; to be used for an event selection.
    """
    
    # Define a collection of muons to be used in the analysis. These muons might be non-isolated
    process.analysisPatMuons = cms.EDFilter('PATMuonSelector',
        src = cms.InputTag('slimmedMuons'),
        cut = cms.string('pt > 10. & abs(eta) < 2.5'))
    
    paths.append(process.analysisPatMuons)
    
    
    # Specify additional selection cuts to be evaluated. They have been migrated into the source
    # code of plugins, and the list is empty
    muQualityCuts = cms.vstring()
    
    
    # A collection to be used for an event selection at the Grid level. It applies for tighter
    # kinematical cuts to muons but allows a muon to be non-isolated or poorly identified
    process.patMuonsForEventSelection = cms.EDFilter('PATMuonSelector',
        src = cms.InputTag('analysisPatMuons'),
        cut = cms.string('pt > 20. & abs(eta) < 2.1'))
    
    paths.append(process.patMuonsForEventSelection)
    
    
    # Return values
    return muQualityCuts


def DefineJets(process, paths):
    """ Applies quality selection to jets. The user is expected to operate with the following
        collections:
        
        1. analysisPatJets: jets subjected to recommended quality selection; to be used in the
        analysis.
    """
    
    # Set jet identification criteria as recommended in [1-2]. The fraction of neutral-hadron and
    # HF-hadron energy is defined below differently from the formula in [2]. However, the formula
    # is written for uncorrected jets, while JEC-corrected ones are used below. One can rescale the
    # jet energy in the formula, but the expression below yields the same result. All accessors to
    # energy fractions from PAT jets account for the effect of JEC
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID
    # [2] https://hypernews.cern.ch/HyperNews/CMS/get/JetMET/1429.html
    jetQualityCut = 'numberOfDaughters > 1 & '\
     '(neutralHadronEnergyFraction + HFHadronEnergyFraction) < 0.99 & '\
     'neutralEmEnergyFraction < 0.99 & (abs(eta) < 2.4 & chargedEmEnergyFraction < 0.99 & '\
     'chargedHadronEnergyFraction > 0. & chargedMultiplicity > 0 | abs(eta) >= 2.4)'
    
    # Select jets that pass the above ID and some kinematical cuts
    process.analysisPatJets = cms.EDFilter('PATJetSelector',
        src = cms.InputTag('slimmedJets'),
        cut = cms.string('pt > 5. & abs(eta) < 4.7 & ' + jetQualityCut))
    
    
    paths.append(process.analysisPatJets)
