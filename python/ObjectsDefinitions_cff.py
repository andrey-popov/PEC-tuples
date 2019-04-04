"""Definitions of reconstructed physics objects.

The module is exploited by cmsRun configuration MiniAOD_cfg.py.  It
exports several functions define_* that define different reconstructed
objects.
"""

import FWCore.ParameterSet.Config as cms


def setup_egamma_preconditions(process, task, period):
    """Perform common setup for electrons and photons.

    This function must be called before calls to define_electrons and/or
    define_photons.
    
    Arguments:
        process: The process to which relevant electron producers are
            added.
        task: Task to which non-standard producers are attached.
        period: Data-taking period, '2016' or '2017'.
    """
    
    # Follow instructions from [1]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2?rev=14
    from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq

    if period == '2016':
        setupEgammaPostRecoSeq(
            process, runEnergyCorrections=False, era='2016-Legacy'
        )
    elif period == '2017':
        setupEgammaPostRecoSeq(process, era='2017-Nov17ReReco')


    # Add producers and filters from a sequence created as a side effect
    # to the task to allow for the unscheduled execution
    for module_name in process.egammaPostRecoSeq.moduleNames():
        module = getattr(process, module_name)

        if (isinstance(module, cms.EDProducer)
                or isinstance(module, cms.EDAnalyzer)):
            task.add(module)


def define_electrons(process, task, period):
    """Define reconstructed electrons.
    
    Configure reconstructed electrons to be used in an analysis,
    together with the relevant identification requirements.  Function
    setup_egamma_preconditions must be called before this one.
    
    Arguments:
        process: The process to which relevant electron producers are
            added.
        task: Task to which non-standard producers are attached.
        period: Data-taking period, '2016' or '2017'.
    
    Return value:
        Return a tuple with the following elements:
        quality_cuts: List of string-based quality selections whose
            decisions are to be saved.
        embedded_cut_based_id_labels: Labels of boolean electron IDs
            embedded in pat::Electron whose decisions are to be saved.
        embedded_mva_id_labels: Labels of continuous electron IDs
            embedded in pat::Electron whose decitions are to be saved.
        cut_based_id_maps: Tags to access maps with boolean electron IDs
            whose decisions are to be saved.
        mva_id_maps: Tags to access maps with continuous electron IDs
            whose decisions are to be saved.
    
    In addition to constructing the return values, add producers that
    create the following collections of electrons:
        analysisPatElectrons: Collection of loosely identified
            non-isolated electrons to be saved.
        patElectronsForEventSelection: Collection of loosely identified
            non-isolated electrons that pass a realistically tight
            kinematic selection.  Exploited in the loose event selection
            applied in the main configuration.
    """
    
    # Collection of electrons that will be stored in tuples
    process.analysisPatElectrons = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('slimmedElectrons'),
        cut = cms.string('pt > 15. & (abs(eta) < 2.5 | abs(superCluster.eta) < 2.5)')
    )
    
    # Labels to access embedded ID, as recommended in [1]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations?rev=12#Electrons_IDs
    if period == '2016':
        embedded_cut_based_id_labels = [
            'cutBasedElectronID-Summer16-80X-V1-' + wp
            for wp in ['veto', 'loose', 'medium', 'tight']
        ]
        embedded_mva_id_labels = [
            'ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Values'
        ]
    elif period == '2017':
        embedded_cut_based_id_labels = [
            'cutBasedElectronID-Fall17-94X-V2-' + wp
            for wp in ['veto', 'loose', 'medium', 'tight']
        ]
        embedded_mva_id_labels = [
            'ElectronMVAEstimatorRun2Fall17IsoV1Values'
        ]
    
    
    # Labels of maps with electron ID (nothing at the moment)
    cut_based_id_maps = []
    mva_id_maps = []
    
    
    # Additional selections to be evaluated (nothing at the moment)
    quality_cuts = cms.vstring()
    
    
    # Define electrons to be used for the loose event selection in the
    # main configuration.  Tighter kinematical cuts are applied to them.
    process.patElectronsForEventSelection = cms.EDFilter('PATElectronSelector',
        src = cms.InputTag('analysisPatElectrons'),
        cut = cms.string('pt > 23. & (abs(eta) < 2.5 | abs(superCluster.eta) < 2.5)')
    )
    
    
    # Add producers to the task
    task.add(
        process.analysisPatElectrons, process.patElectronsForEventSelection,
    )
    
    
    return (quality_cuts, embedded_cut_based_id_labels, embedded_mva_id_labels,
            cut_based_id_maps, mva_id_maps)



def define_muons(process, task):
    """Define reconstructed muons.
    
    Configure reconstructed muons to be used in an analysis, together
    with the relevant identification requirement.
    
    Arguments:
        process: The process to which relevant muon producers are added.
        task: Task to which non-standard producers are attached.
    
    Return value:
        A list of string-based quality selections whose decisions are to
            be saved.
    
    Also add producers that create the following collections of muons:
        analysisPatMuons: Collection of loosely identified non-isolated
            muons to be saved.
        patMuonsForEventSelection: Collection of loosely identified
            non-isolated muons that pass a realistically tight
            kinematic selection.  Exploited in the loose event selection
            applied in the main configuration.
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
    # configuration.  Realistically tight kinematic cuts are applied.
    process.patMuonsForEventSelection = cms.EDFilter('PATMuonSelector',
        src = cms.InputTag('analysisPatMuons'),
        cut = cms.string('pt > 20. & abs(eta) < 2.5')
    )
    
    
    # Add producers to the task
    task.add(process.analysisPatMuons, process.patMuonsForEventSelection)
    
    
    return muQualityCuts


def define_photons(process, task):
    """Define reconstructed photons.
    
    Configure reconstructed photons to be used in an analysis.  Function
    setup_egamma_preconditions must be called before this one.
    
    Arguments:
        process: The process to which relevant photon producers are
            added.
        task: Task to which non-standard producers are attached.
    
    Return value:
        Return a tuple with the following elements:
        quality_cuts: List of string-based quality selections whose
            decisions are to be saved.
        embedded_cut_based_id_labels: Labels of embedded cut-based IDs
            whose decisions are to be saved.
        cut_based_id_maps: Tags to access maps with boolean photon IDs
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
    
    
    # Labels to access embedded ID, as recommended in [1].  They are
    # suitable for both 2016 and 2017
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations?rev=12#Photon_IDs
    embedded_cut_based_id_labels = [
        'cutBasedPhotonID-Fall17-94X-V2-' + wp
        for wp in ['veto', 'loose', 'medium', 'tight']
    ]


    # Labels of maps with photon ID (nothing at the moment)
    cut_based_id_maps = []
    
    
    # Additional selections to be evaluated
    quality_cuts = cms.vstring(
        # EE-EB gap
        '(abs(superCluster.eta) < 1.4442 | abs(superCluster.eta) > 1.5660)'
    )
    
    
    # Add producers to the task
    task.add(process.analysisPatPhotons)
    
    
    return quality_cuts, embedded_cut_based_id_labels, cut_based_id_maps



def define_jets(process, task, reapplyJEC=False, runOnData=False):
    """Define reconstructed jets.
    
    Configure reconstructed jets to be used in an analysis.  In
    particular, reapply JEC if requested.
    
    Arguments:
        process: The process to which relevant jet producers are added.
        task: Task to which non-standard producers are attached.
        reapplyJEC: Flag determining if JEC are to be reapplied or not.
        runOnData: Flag to distinguish processing of data and
            simulation.
    
    Return value:
        Return a tuple with the following elements:
        recorrectedJetsLabel: Label to access collection of jets with
            up-to-date JEC (reapplied or not, depending on the
            configuration) and no kinematic or quality selection.
        jetQualityCuts: List of string-based quality selections whose
            decisions are to be saved.
    
    Create the following jet collections:
        analysisPatJets: Jets with up-to-date JEC and a loose quality
            selection to be used in an analysis.
    """
    
    # Reapply JEC [1] if requested.  The corrections are read from the
    # global tag.
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=139#CorrPatJets
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
        
        recorrectedJetsLabel = 'updatedPatJetsUpdatedJEC'
    
    else:
        recorrectedJetsLabel = 'slimmedJets'
    
    
    # Define analysis-level jets.  The produced collection will contain
    # all jets that have a chance to pass the given pt threshold thanks
    # to JEC and JER variations.
    process.analysisPatJets = cms.EDFilter('JERCJetSelector',
        src = cms.InputTag(recorrectedJetsLabel),
        jetTypeLabel = cms.string('AK4PFchs'),
        preselection = cms.string(''),
        minPt = cms.double(15.),
        includeJERCVariations = cms.bool(not runOnData),
        genJets = cms.InputTag('slimmedGenJets'),
        rho = cms.InputTag('fixedGridRhoFastjetAll')
    )
    
    
    # Additional selection to be evaluated (empty at the moment)
    jetQualityCuts = cms.vstring()
    
    
    # Add (non-standard) producers to the task
    task.add(process.analysisPatJets)
    
    
    return recorrectedJetsLabel, jetQualityCuts


def define_METs(process, task, runOnData=False):
    """Define reconstructed MET.
    
    Configure recalculation of corrected MET and its systematic
    uncertainties.  Only type-1 corrections are applied.  Due to
    limitations of MET tools, uncertainties are calculated even when
    running over data, while this is not needed.  Moreover, they include
    uncertainties corresponding to variations in energies of leptons,
    taus, and photons, although these variations are not considered in
    targeted analyses.
    
    Arguments:
        process: The process to which relevant MET producers are added.
        task: Task to which non-standard producers are attached.
            Not used currently.
        runOnData: Flag to distinguish processing of data and
            simulation.
    
    Return value:
        InputTag that defines MET collection to be used.
    """
    
    # Recalculate MET corrections [1]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=64#Instructions_for_8_0_X_X_26_patc
    from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import \
        runMetCorAndUncFromMiniAOD
    runMetCorAndUncFromMiniAOD(process, isData=runOnData, postfix='')
    
    metTag = cms.InputTag('slimmedMETs', processName=process.name_())
    return metTag
