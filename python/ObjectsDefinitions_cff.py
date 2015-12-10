""" The module contains definitions of physical objects including required adjustments to the
    reconstruction process. Functions defined here must be called after usePF2PAT, name of the
    modules are hard-coded.
    """


import FWCore.ParameterSet.Config as cms


def DefineElectrons(process):
    """ This function adjusts electrons. The user is expected to use the following products only:
        
        1. analysisPatElectrons: loose non-isolated electrons to be saved in tuples.
        
        2. eleIDMaps: input tags to access maps of cut-based electron IDs.
        
        3. eleQualityCuts: vector of quality cuts to be applied to the above collection.
        
        4. patElectronsForEventSelection: collection of electrons that pass basic kinematical cuts;
        to be used for the event selection.
    """
    
    # Collection of electrons that will be stored in tuples
    process.analysisPatElectrons = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('slimmedElectrons'),
        cut = cms.string('pt > 20. & abs(eta) < 2.5'))
    
    
    # Calculate IDs for analysis electrons. The code is adapted from [1]
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=27#Recipe_for_regular_users_for_7_4
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import switchOnVIDElectronIdProducer, \
     setupAllVIDIdsInModule, setupVIDElectronSelection, DataFormat
    switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
    process.egmGsfElectronIDs.physicsObjectSrc = 'analysisPatElectrons'
    setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.' + \
     'cutBasedElectronID_Spring15_25ns_V1_cff', setupVIDElectronSelection)
    
    
    # Define labels of electron IDs to be saved
    eleIDLabelPrefix = 'egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-'
    eleIDMaps = [
        cms.InputTag(eleIDLabelPrefix + 'veto'), cms.InputTag(eleIDLabelPrefix + 'loose'),
        cms.InputTag(eleIDLabelPrefix + 'medium'), cms.InputTag(eleIDLabelPrefix + 'tight')]
    
    
    # Additional selections to be evaluated
    eleQualityCuts = cms.vstring(
        '(abs(superCluster.eta) < 1.4442 | abs(superCluster.eta) > 1.5660)')
    
    
    # Define electrons to be used for event selection at the Grid level. They are subjected to
    # tighter kinematical cuts
    process.patElectronsForEventSelection = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('analysisPatElectrons'),
        cut = cms.string('pt > 27. & abs(eta) < 2.1'))
    
    
    # Return values
    return eleQualityCuts, eleIDMaps



def DefineMuons(process):
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
    
    
    # Specify additional selection cuts to be evaluated. They have been migrated into the source
    # code of plugins, and the list is empty
    muQualityCuts = cms.vstring()
    
    
    # A collection to be used for an event selection at the Grid level. It applies for tighter
    # kinematical cuts to muons but allows a muon to be non-isolated or poorly identified
    process.patMuonsForEventSelection = cms.EDFilter('PATMuonSelector',
        src = cms.InputTag('analysisPatMuons'),
        cut = cms.string('pt > 20. & abs(eta) < 2.1'))
    
    
    # Return values
    return muQualityCuts


def DefineJets(process, reapplyJEC = False, runOnData = False):
    """ Reapplies JEC and applies quality selection to jets. User is expected to exploit the
        following collections:
        
        analysisPatJets: Corrected jets subjected to the recommended filtering on quality.
    """
    
    # Reapply JEC if requested. The corrections are read from the current global tag
    if reapplyJEC:
        # from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetCorrFactorsUpdated
        # from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetsUpdated
        process.load('PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff')
        
        process.patJetCorrFactorsReapplyJEC = process.patJetCorrFactorsUpdated.clone(
            src = cms.InputTag('slimmedJets'),
            levels = ['L1FastJet', 'L2Relative', 'L3Absolute'],
            payload = 'AK4PFchs')
        
        if runOnData:
            process.patJetCorrFactorsReapplyJEC.levels.append('L2L3Residual')
        
        process.recorrectedSlimmedJets = process.patJetsUpdated.clone(
            jetSource = cms.InputTag('slimmedJets'),
            jetCorrFactorsSource = cms.VInputTag(cms.InputTag('patJetCorrFactorsReapplyJEC')))
    
    
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
        src = (cms.InputTag('recorrectedSlimmedJets') if reapplyJEC else \
         cms.InputTag('slimmedJets')),
        cut = cms.string('pt > 5. & abs(eta) < 4.7 & ' + jetQualityCut))
    
    
    # When running over simulation, produce jet collections with varied systematic uncertainties.
    # They will be used to perform the loose event selection, taking the uncertainty into account
    if not runOnData:
        process.analysisPatJetsScaleUp = cms.EDProducer('ShiftedPATJetProducer',
            src = cms.InputTag('analysisPatJets'),
            jetCorrPayloadName = cms.string('AK4PFchs'),
            jetCorrUncertaintyTag = cms.string('Uncertainty'),
            addResidualJES = cms.bool(False),
            shiftBy = cms.double(+1.))
        
        process.analysisPatJetsScaleDown = process.analysisPatJetsScaleUp.clone(shiftBy = -1.)
