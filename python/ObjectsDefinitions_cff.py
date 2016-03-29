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
        cut = cms.string('pt > 20. & abs(eta) < 2.5')
    )
    
    
    # Labels to access embedded cut-based ID
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=27
    eleEmbeddedCutBasedIDLabels = ['cutBasedElectronID-Spring15-25ns-V1-standalone-' + p
        for p in ['veto', 'loose', 'medium', 'tight']]
    
    
    # Decisions of triggering MVA ID are not stored in MiniAOD2015v2 and
    # should be calculated
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentificationRun2?rev=23
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
        # [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentificationRun2?rev=23#Recipes_for_7_4_12_Spring15_MVA
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
        cut = cms.string('pt > 22. & abs(eta) < 2.5')
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
        cut = cms.string('pt > 17. & abs(eta) < 2.5')
    )
    
    
    # Return values
    return muQualityCuts


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
    
    Create the following jet collections:
        analysisPatJets: Jets with up-to-date JEC and a loose quality
            selection to be used in an analysis.
        analysisPatJetsScaleUp/Down: Collections of jets with varied JEC
            systematic uncertainties.  To be used in the loose event
            selection in the main configuration.
    """
    
    # Reapply JEC if requested [1].  The corrections are read from the
    # current global tag.
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=133#CorrPatJets
    if reapplyJEC:
        from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import (updatedPatJetCorrFactors,
            updatedPatJets)
        
        process.jetCorrectionFactorsReapplyJEC = updatedPatJetCorrFactors.clone(
            src = cms.InputTag('slimmedJets'),
            levels = ['L1FastJet', 'L2Relative', 'L3Absolute'],
            payload = 'AK4PFchs'
        )
        
        if runOnData:
            process.jetCorrectionFactorsReapplyJEC.levels.append('L2L3Residual')
        
        process.recorrectedSlimmedJets = updatedPatJets.clone(
            jetSource = cms.InputTag('slimmedJets'),
            jetCorrFactorsSource = cms.VInputTag(cms.InputTag('jetCorrectionFactorsReapplyJEC'))
        )
    
    recorrectedJetsLabel = ('recorrectedSlimmedJets' if reapplyJEC else 'slimmedJets')
    
    
    # Define analysis-level jets by applying a very loose selection
    process.analysisPatJets = cms.EDFilter('PATJetSelector',
        src = cms.InputTag(recorrectedJetsLabel),
        cut = cms.string('pt > 10. & abs(eta) < 5.')
    )
    
    
    # Jet ID [1].  Accessors to energy fractions in pat::Jet take into
    # account JEC, and thus there is no need to unapply the corrections
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/JetID?rev=93#Recommendations_for_13_TeV_data
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
    
    
    # When running over simulation, produce jet collections with varied
    # systematic uncertainties.  They will be used to perform the loose
    # event selection, taking the uncertainty into account
    if not runOnData:
        process.analysisPatJetsScaleUp = cms.EDProducer('ShiftedPATJetProducer',
            src = cms.InputTag('analysisPatJets'),
            jetCorrPayloadName = cms.string('AK4PFchs'),
            jetCorrUncertaintyTag = cms.string('Uncertainty'),
            addResidualJES = cms.bool(False),
            shiftBy = cms.double(+1.)
        )
        
        process.analysisPatJetsScaleDown = process.analysisPatJetsScaleUp.clone(shiftBy = -1.)
    
    
    return recorrectedJetsLabel, jetQualityCuts


def define_METs(process, runOnData=False, jetCollection=''):
    """Define reconstructed MET.
    
    Configure recalculation of corrected MET and its systematic
    uncertainties.  Only type-1 corrections are applied.  Due to
    limitations of MET tools, uncertainties are calculated even when
    running over data, while this is not needed.  Moreover, they include
    uncertainties corresponding to variations in energies of leptons,
    taus, and photons, although these variations are not considered in
    targeted analyses.
    
    Uncertainties corresponding to JER are computed using outdated
    parameters.  A recipe for 13 TeV data is still under development.
    
    There have been many problems with the MET PAT tool and related
    CMSSW plugins.  Although some of them are fixed here, there might be
    others.  Thus, MET should be used ith a great causion.
    
    Arguments:
        process: The process to which relevant MET producers are added.
        runOnData: Flag to distinguish processing of data and
            simulation.
        jetCollection: Name of jet collection to be exploited for type-1
            corrections.  Should be the collection with up-to-date JEC.
    
    Return value:
        None.
    
    Among other things, add to the process producer slimmedMETs, which
    overrides the namesake collection from MiniAOD.  User must use this
    new collection.
    """
    
    # Recalculate MET corrections.  Some poor documentation is
    # available in [1].  There is a relevant discussion in hypernews.
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools?rev=60#MET_Systematics_Tools
    # [2] https://hypernews.cern.ch/HyperNews/CMS/get/met/437.html?inline=-1
    from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import \
        runMetCorAndUncFromMiniAOD
    runMetCorAndUncFromMiniAOD(
        process,
        metType='PF',
        isData=runOnData,
        repro74X=False,
        # electronColl='', muonColl='', photonColl='', tauColl='',
        jetColl=jetCollection,
        jetCollUnskimmed=jetCollection,
        pfCandColl='packedPFCandidates',
        jecUncFile='Analysis/PECTuples/data/Summer15_25nsV6_MC_Uncertainty_AK4PFchs.txt',
        postfix=''
    )
    # ^Keyword argument repro74X in the above configuration is not
    # documented.  It should be set to True when running over a MiniAOD
    # dataset produced in a 7_4_X release with X <= 12.  Use default
    # collections of leptons, taus, and photons.  Could have switched
    # off calculation of the corresponding variations of MET by setting
    # collection names to '', but PATMETSlimmer requires these
    # variations [1].
    # [1] https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15_patch1/PhysicsTools/PatAlgos/plugins/PATMETSlimmer.cc#L80-L95
    
    
    # Wrong correction level specified in the default configuration [1]
    # [1] https://hypernews.cern.ch/HyperNews/CMS/get/met/437/1/1/1.html
    process.metcalo.correctionLevel = 'rawCalo'
    
    
    # Update the type of JEC uncertainties used (total instead of
    # 'SubTotalMC') as recommended in [1-2]
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETRun2Corrections?rev=35#type_1_PF_MET_recommended
    # [2] https://hypernews.cern.ch/HyperNews/CMS/get/met/425/1/1/1/1/1.html
    for module in ['shiftedPatJetEnUp', 'shiftedPatJetEnDown']:
        getattr(process, module).jetCorrUncertaintyTag = ''
    
    
    # Drop corrections for phi modulation since they are not recommended
    # at the moment [1]
    # [1] https://hypernews.cern.ch/HyperNews/CMS/get/met/422/1/1/1.html
    del(process.slimmedMETs.tXYUncForRaw)
    del(process.slimmedMETs.tXYUncForT1)
