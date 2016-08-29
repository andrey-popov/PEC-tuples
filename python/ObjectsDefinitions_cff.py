"""Defines reconstructed physics objects.

The module is exploited by cmsRun configuration MiniAOD_cfg.py.  It
exports several functions define_* that define different reconstructed
objects.
"""

import FWCore.ParameterSet.Config as cms


def define_electrons(process):
    """Define reconstructed electrons.
    
    Configure reconstructed electrons to be used in an analysis,
    together with the relevant identification requirements.
    
    Arguments:
        process: The process to which relevant electron producers are
            added.
    
    Return value:
        Return a tuple with the following elements:
        eleQualityCuts: List of string-based quality selections whose
            decisions are to be saved.
        eleEmbeddedCutBasedIDLabels: Labels of boolean electron IDs
            embedded in pat::Electron whose decisions are to be saved.
        eleCutBasedIDMaps: Tags to access maps with boolean electron IDs
            whose decisions are to be saved.
        eleMVAIDMaps: Tags to access maps with continuous electron IDs
            whose decisions are to be saved.
    
    In addition to constructing the return values, add producers that
    create the following collections of electrons:
        analysisPatElectrons: Collection of loosely identified
            non-isolated electrons to be saved.
        patElectronsForEventSelection: Collection of loosely identified
            non-isolated electrons that pass a realistically tight
            kinematical selection.  Exploited in the loose event
            selection applied in the main configuration.
    """
    
    # Collection of electrons that will be stored in tuples
    process.analysisPatElectrons = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('slimmedElectrons'),
        cut = cms.string('pt > 20. & (abs(eta) < 2.5 | abs(superCluster.eta) < 2.5)')
    )
    
    
    # Labels to access embedded cut-based ID
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=31
    eleEmbeddedCutBasedIDLabels = ['cutBasedElectronID-Spring15-25ns-V1-standalone-' + p
        for p in ['veto', 'loose', 'medium', 'tight']]
    
    
    # Decisions of triggering MVA ID are not stored in MiniAOD76Xv2 and
    # should be calculated
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentificationRun2?rev=26
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import (switchOnVIDElectronIdProducer,
        setupAllVIDIdsInModule, setupVIDElectronSelection, DataFormat)
    switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
    
    for idModule in ['mvaElectronID_Spring15_25ns_Trig_V1_cff']:
        setupAllVIDIdsInModule(
            process,
            'RecoEgamma.ElectronIdentification.Identification.' +
            idModule, setupVIDElectronSelection
        )
    
    process.electronMVAValueMapProducer.srcMiniAOD = 'analysisPatElectrons'
    
    
    # Labels of maps with electron ID.  No maps are needed for the
    # cut-based ID since its decisions are embedded in pat::Electron.
    eleCutBasedIDMaps = []
    eleMVAIDMaps = [
        'electronMVAValueMapProducer:ElectronMVAEstimatorRun2Spring15Trig25nsV1Values'
    ]
    
    
    # Additional selections to be evaluated
    eleQualityCuts = cms.vstring(
        # EE-EB gap
        '(abs(superCluster.eta) < 1.4442 | abs(superCluster.eta) > 1.5660)',
        # Trigger-emulating preselection [1], referenced from [2]
        # [1] https://hypernews.cern.ch/HyperNews/CMS/get/egamma/1645/2/1/1.html
        # [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentificationRun2?rev=26#Triggering_electron_MVA_details
        'pt > 15. & \
         ((abs(superCluster.eta) < 1.4442 & full5x5_sigmaIetaIeta < 0.012 & hcalOverEcal < 0.9 & \
          ecalPFClusterIso / pt < 0.37 & hcalPFClusterIso / pt < 0.25 & dr03TkSumPt / pt < 0.18 & \
          abs(deltaEtaSuperClusterTrackAtVtx) < 0.0095 & \
          abs(deltaPhiSuperClusterTrackAtVtx) < 0.065) | \
         (abs(superCluster.eta) > 1.5660 & full5x5_sigmaIetaIeta < 0.033 & hcalOverEcal < 0.09 & \
          ecalPFClusterIso / pt < 0.45 & hcalPFClusterIso / pt < 0.28 & dr03TkSumPt / pt < 0.18))'
    )
    
    
    # Define electrons to be used for the loose event selection in the
    # main configuration.  Tighter kinematical cuts are applied to them.
    process.patElectronsForEventSelection = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('analysisPatElectrons'),
        cut = cms.string('pt > 23. & (abs(eta) < 2.5 | abs(superCluster.eta) < 2.5)')
    )
    
    
    # Return values
    return eleQualityCuts, eleEmbeddedCutBasedIDLabels, eleCutBasedIDMaps, eleMVAIDMaps



def define_muons(process):
    """Define reconstructed muons.
    
    Configure reconstructed muons to be used in an analysis, together
    with the relevant identification requirement.
    
    Arguments:
        process: The process to which relevant muon producers are added.
    
    Return value:
        A list of string-based quality selections whose decisions are to
            be saved.
    
    Also add producers that create the following collections of muons:
        analysisPatMuons: Collection of loosely identified non-isolated
            muons to be saved.
        patMuonsForEventSelection: Collection of loosely identified
            non-isolated muons that pass a realistically tight
            kinematical selection.  Exploited in the loose event
            selection applied in the main configuration.
    """
    
    # Define a collection of muons to be used in the analysis.  It
    # includes also non-isolated muons.
    process.analysisPatMuons = cms.EDFilter('PATMuonSelector',
        src = cms.InputTag('slimmedMuons'),
        cut = cms.string('pt > 10. & abs(eta) < 2.5')
    )
    
    
    # Specify additional selection cuts to be evaluated.  The list is
    # actually empty as the selection has been moved to the C++ code.
    muQualityCuts = cms.vstring()
    
    
    # A collection to be used for the loose event selection in the main
    # configuration.  Realistically tight kinematical cuts are applied.
    process.patMuonsForEventSelection = cms.EDFilter('PATMuonSelector',
        src = cms.InputTag('analysisPatMuons'),
        cut = cms.string('pt > 18. & abs(eta) < 2.5')
    )
    
    
    # Return values
    return muQualityCuts


def define_photons(process):
    """Define reconstructed photons.
    
    Configure reconstructed photons to be used in an analysis.
    
    Arguments:
        process: The process to which relevant photon producers are
            added.
    
    Return value:
        Return a tuple with the following elements:
        phoQualityCuts: List of string-based quality selections whose
            decisions are to be saved.
        phoCutBasedIDMaps: Tags to access maps with boolean photon IDs
            whose decisions are to be saved.
    
    In addition to constructing the return values, add producers that
    create the following collections of photons:
        analysisPatPhotons: Collection of loosely identified
            non-isolated photons to be saved.
    """
    
    # Collection of photons satisfying a basic kinematic selection
    process.analysisPatPhotons = cms.EDFilter('PATPhotonSelector',
        src = cms.InputTag('slimmedPhotons'),
        cut = cms.string('pt > 20. & abs(eta) < 2.5')
    )
    
    
    # Decisions of cut-based identification algorithm [1]
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedPhotonIdentificationRun2?rev=28
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import (switchOnVIDPhotonIdProducer,
        setupAllVIDIdsInModule, setupVIDPhotonSelection, DataFormat)
    switchOnVIDPhotonIdProducer(process, DataFormat.MiniAOD)
    
    for idModule in ['cutBasedPhotonID_Spring15_25ns_V1_cff']:
        setupAllVIDIdsInModule(
            process,
            'RecoEgamma.PhotonIdentification.Identification.' +
            idModule, setupVIDPhotonSelection
        )
    
    process.photonIDValueMapProducer.srcMiniAOD = 'analysisPatPhotons'
    process.egmPhotonIDs.physicsObjectSrc = 'analysisPatPhotons'
    
    
    # Labels of maps with photon ID
    phoCutBasedIDMaps = ['egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-' + p
        for p in ['loose', 'medium', 'tight']]
    
    
    # Information about geometry is needed to evaluate the ID
    process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
    
    
    # Additional selections to be evaluated
    phoQualityCuts = cms.vstring(
        # EE-EB gap
        '(abs(superCluster.eta) < 1.4442 | abs(superCluster.eta) > 1.5660)'
    )
    
    return phoQualityCuts, phoCutBasedIDMaps



def define_jets(process, reapplyJEC=False, runOnData=False):
    """Define reconstructed jets.
    
    Configure reconstructed jets to be used in an analysis.  In
    particular, reapply JEC if requested.
    
    Arguments:
        process: The process to which relevant jet producers are added.
        reapplyJEC: Flag determining if JEC are to be reapplied or not.
        runOnData: Flag to distinguish processing of data and
            simulation.
    
    Return value:
        Return a tuple with the following elements:
        recorrectedJetsLabel: Label to access collection of jets with
            up-to-date JEC (reapplied or not, depending on the
            configuration) and no kinamatical or quality selection.
        jetQualityCuts: List of string-based quality selections whose
            decisions are to be saved.
        pileUpIDMap: Name of a map that contains real-valued pile-up ID
            decisions.
    
    Create the following jet collections:
        analysisPatJets: Jets with up-to-date JEC and a loose quality
            selection to be used in an analysis.
    """
    
    # Reapply JEC if requested [1].  The corrections are read from the
    # current global tag.
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=134#CorrPatJets
    if reapplyJEC:
        jecLevels = ['L1FastJet', 'L2Relative', 'L3Absolute']
        if runOnData:
            jecLevels.append('L2L3Residual')
        
        from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
        updateJetCollection(
            process, labelName = 'UpdatedJEC',
            jetSource = cms.InputTag('slimmedJets'),
            jetCorrections = ('AK4PFchs', cms.vstring(jecLevels), 'None')
        )
    
    recorrectedJetsLabel = ('updatedPatJetsUpdatedJEC' if reapplyJEC else 'slimmedJets')
    
    
    # Define analysis-level jets.  The produced collection will contain
    # all jets that have a chance to pass the given pt threshold thanks
    # to JEC and JER variations.
    process.analysisPatJets = cms.EDFilter('JERCJetSelector',
        src = cms.InputTag(recorrectedJetsLabel),
        jetTypeLabel = cms.string('AK4PFchs'),
        preselection = cms.string('abs(eta) < 5.'),
        minPt = cms.double(20.),
        includeJERCVariations = cms.bool(not runOnData),
        genJets = cms.InputTag('slimmedGenJets'),
        rho = cms.InputTag('fixedGridRhoFastjetAll')
    )
    
    
    # Jet ID [1].  Accessors to energy fractions in pat::Jet take into
    # account JEC, and thus there is no need to unapply the corrections.
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/JetID?rev=94#Recommendations_for_13_TeV_data
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
        'abs(eta) > 3. & neutralMultiplicity > 10 & neutralEmEnergyFraction < 0.90'
    )
    
    # Specify additional selection to be evaluated
    jetQualityCuts = cms.vstring(jetLooseID)
    
    
    # Set up pile-up jet ID.  The IDs are now included as userFloats,
    # but in MiniAOD80Xv1 they have been produced with 76X training
    # (while ones in MiniAOD80Xv2 will be up-to-date [1]).  For the
    # moment, recompute the IDs.
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID?rev=27#Information_for_13_TeV_data_anal
    process.load('RecoJets.JetProducers.PileupJetID_cfi')
    process.pileupJetIdCustomized = process.pileupJetId.clone(
        jets = cms.InputTag('analysisPatJets'),
        inputIsCorrected = True,
        applyJec = False,
        vertexes = cms.InputTag('offlineSlimmedPrimaryVertices')
    )
    pileUpIDMap = 'pileupJetIdCustomized:fullDiscriminant'
    
    
    return recorrectedJetsLabel, jetQualityCuts, pileUpIDMap


def define_METs(process, runOnData=False):
    """Define reconstructed MET.
    
    Configure recalculation of corrected MET and its systematic
    uncertainties.  Only type-1 corrections are applied.  Due to
    limitations of MET tools, uncertainties are calculated even when
    running over data, while this is not needed.  Moreover, they include
    uncertainties corresponding to variations in energies of leptons,
    taus, and photons, although these variations are not considered in
    targeted analyses.
    
    There have been many problems with the MET PAT tool and related
    CMSSW plugins.  Although some of them are fixed here, there might be
    others.  Thus, MET should be used ith a great causion.
    
    Arguments:
        process: The process to which relevant MET producers are added.
        runOnData: Flag to distinguish processing of data and
            simulation.
    
    Return value:
        None.
    
    Among other things, add to the process producer slimmedMETs, which
    overrides the namesake collection from MiniAOD.  User must use this
    new collection.
    """
    
    # Recalculate MET corrections.  Some poor documentation is
    # available in [1-2].
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=46#Instructions_for_8_0_X_X_5
    # [2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools?rev=60#MET_Systematics_Tools
    from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import \
        runMetCorAndUncFromMiniAOD
    runMetCorAndUncFromMiniAOD(
        process,
        isData=runOnData,
        # electronColl='', muonColl='', photonColl='', tauColl='',
        postfix=''
    )
    # ^Use default collections of leptons, taus, and photons.  Could
    # have switched off calculation of the corresponding variations of
    # MET by setting collection names to '', but PATMETSlimmer requires
    # these variations [1].
    # [1] https://github.com/cms-sw/cmssw/blob/CMSSW_8_0_8/PhysicsTools/PatAlgos/plugins/PATMETSlimmer.cc#L80-L95
