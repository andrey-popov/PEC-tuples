""" The module contains definitions of physical objects including required adjustments to the
    reconstruction process. Functions defined here must be called after usePF2PAT, name of the
    modules are hard-coded.
    """


import FWCore.ParameterSet.Config as cms


def DefineElectrons(process):
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
        cut = cms.string('pt > 22. & abs(eta) < 2.5'))
    
    
    # Return values
    return eleQualityCuts, eleEmbeddedCutBasedIDLabels, eleCutBasedIDMaps, eleMVAIDMaps



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
        cut = cms.string('pt > 17. & abs(eta) < 2.5'))
    
    
    # Return values
    return muQualityCuts


def DefineJets(process, reapplyJEC = False, runOnData = False):
    """ 
        Adjusts jets. In particular, it reapplies JEC. User is expected to exploit the following
        products only:
        
        analysisPatJets: Corrected jets to be used in the analysis.
        
        recorrectedJetsLabel: Label of collection of jets with up-to-date JEC and no selection.
        
        jetQualityCuts: Vector of quality cuts to be applied to the above collection.
    """
    
    # Reapply JEC if requested [1]. The corrections are read from the current global tag
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=124#CorrPatJets
    if reapplyJEC:
        from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import \
            patJetCorrFactorsUpdated, patJetsUpdated
        
        process.jetCorrectionFactorsReapplyJEC = patJetCorrFactorsUpdated.clone(
            src = cms.InputTag('slimmedJets'),
            levels = ['L1FastJet', 'L2Relative', 'L3Absolute'],
            payload = 'AK4PFchs')
        
        if runOnData:
            process.jetCorrectionFactorsReapplyJEC.levels.append('L2L3Residual')
        
        process.recorrectedSlimmedJets = patJetsUpdated.clone(
            jetSource = cms.InputTag('slimmedJets'),
            jetCorrFactorsSource = cms.VInputTag(cms.InputTag('jetCorrectionFactorsReapplyJEC')))
    
    recorrectedJetsLabel = ('recorrectedSlimmedJets' if reapplyJEC else 'slimmedJets')
    
    
    # Define analysis-level jets by applying a very loose selection
    process.analysisPatJets = cms.EDFilter('PATJetSelector',
        src = cms.InputTag(recorrectedJetsLabel),
        cut = cms.string('pt > 10. & abs(eta) < 5.'))
    
    
    # Jet ID [1]. Accessors to energy fractions in pat::Jet take into account JEC, and thus there is
    # no need to unapply the corrections
    jetLooseID = (
        # Common block of requirements for |eta| < 3
        'abs(eta) <= 3. & (chargedMultiplicity + neutralMultiplicity) > 1 & ' +
         'neutralHadronEnergyFraction < 0.99 & neutralEmEnergyFraction < 0.99 & ' +
            # Additional requirements for |eta| < 2.4
            '(chargedHadronEnergyFraction > 0. & chargedMultiplicity > 0 & ' +
             'chargedEmEnergyFraction < 0.99' +
            ' | ' +
            # There are no additional requirements for 2.4 < |eta| < 3.
            'abs(eta) >= 2.4)' +
        ' | ' +
        # Requirements for the HF region
        'abs(eta) > 3. & neutralMultiplicity > 10 & neutralEmEnergyFraction < 0.90')
    
    # Specify additional selection to be evaluated
    jetQualityCuts = cms.vstring(jetLooseID)
    
    
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
    
    
    return recorrectedJetsLabel, jetQualityCuts


def DefineMETs(process, runOnData = False, jetCollection = ''):
    """
        Configures recalculation of corrected MET and its systematic uncertainties. Name of the jet
        collection exploited in the procedure is provided as an argument of the function. Only
        type-1 corrections are applied to MET. Due to limitations of MET tools, uncertainties are
        calculated even when runnning over data. Moreover, the uncertainties include ones
        corresponding to variations in energies of leptons, taus, and photons, although they are not
        used in the analysis.
        
        Uncertainties corresponding to JER are computed using outdated parameters. A recipe for
        13 TeV data is still under development.
        
        There have been many problems with the MET PAT tool and related CMSSW plugins. Although some
        of them are fixed here, there might be others. Thus, MET should be used with a great
        causion.
        
        The user should use the following products only:
        
        slimmedMETs: Recomputed corrected MET with uncertainties. Overrides a namesake collection
            in MiniAOD.
    """
    
    # Recalculate MET corrections. A very poor documentation is available in [1]. There is a
    # relevant discussion in hypernews
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools?rev=60#MET_Systematics_Tools
    # [2] https://hypernews.cern.ch/HyperNews/CMS/get/met/437.html?inline=-1
    from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import \
     runMetCorAndUncFromMiniAOD
    runMetCorAndUncFromMiniAOD(
        process,
        metType = 'PF',
        isData = runOnData,
        repro74X = False,
        # electronColl = '', muonColl = '', photonColl = '', tauColl = '',
        jetColl = jetCollection,
        jetCollUnskimmed = jetCollection,
        pfCandColl = 'packedPFCandidates',
        jecUncFile = 'Analysis/PECTuples/data/Summer15_25nsV6_MC_Uncertainty_AK4PFchs.txt',
        postfix = '')
    #^ Keyword argument repro74X in the above configuration is not documented. It should be set to
    # True when running over a MiniAOD dataset produced in a 7_4_X release with X <= 12.
    # Use default collections of leptons, taus, and photons. Could have switched off calculation of
    # the corresponding variations of MET by setting collection names to '', but PATMETSlimmer
    # requires these variations [1].
    # [1] https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15_patch1/PhysicsTools/PatAlgos/plugins/PATMETSlimmer.cc#L80-L95
    
    
    # Wrong correction level specified in the default configuration [1]
    # [1] https://hypernews.cern.ch/HyperNews/CMS/get/met/437/1/1/1.html
    process.metcalo.correctionLevel = 'rawCalo'
    
    
    # Update the type of JEC uncertainties used (total instead of 'SubTotalMC') as recommended
    # in [1-2]
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETRun2Corrections?rev=35#type_1_PF_MET_recommended
    # [2] https://hypernews.cern.ch/HyperNews/CMS/get/met/425/1/1/1/1/1.html
    for module in ['shiftedPatJetEnUp', 'shiftedPatJetEnDown']:
        getattr(process, module).jetCorrUncertaintyTag = ''
    
    
    # Drop corrections for phi modulation since they are not recommended at the moment [1]
    # [1] https://hypernews.cern.ch/HyperNews/CMS/get/met/422/1/1/1.html
    del(process.slimmedMETs.tXYUncForRaw)
    del(process.slimmedMETs.tXYUncForT1)
