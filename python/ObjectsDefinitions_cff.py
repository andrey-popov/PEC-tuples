""" The module contains definitions of physical objects including required adjustments to the
    reconstruction process. Functions defined here must be called after usePF2PAT, name of the
    modules are hard-coded.
    """


# Metadata
__author__ = 'Andrey Popov'
__email__ = 'Andrey.Popov@cern.ch'


import FWCore.ParameterSet.Config as cms


def DefineElectrons(process, PFRecoSequence, runOnData):
    """ This function adjusts electron reconstruction. Among all the fields being added to the
        process, the user is expected to use the following only:
        
        1. nonIsolatedLoosePatElectrons: maximally loose collection of electrons to be saved in the
        tuples.
        
        2. eleQualityCuts: vector of quality cuts to be applied to the above collection.
        
        3. patElectronsForEventSelection: collection to be exploited for an event selection, 
        contains all the electrons passing a simple pure-kinematic selection.
        
        4. selectedPatElectrons: electrons passing loose cuts in isolation, ID, and kinematics; to
        be used for the MET uncertainty tool.
    """
    
    # Define a module to produce a value map with rho correction of electron isolation. The
    # configuration fragment is copied from [1] because it is not included in the current tag of
    # UserCode/EGamma/EGammaAnalysisTools. General outline of configuration is inspired by [2].
    # [1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/EGamma/EGammaAnalysisTools/python/electronIsolatorFromEffectiveArea_cfi.py?hideattic=0&revision=1.1.2.2&view=markup
    # [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections?rev=4#Electrons
    # 
    # In both real data and simulation an effective area derived from real data (2012 HCP dataset)
    # is applied. Possible difference between data and simulation is belived to be small [3-4]
    # [3] https://hypernews.cern.ch/HyperNews/CMS/get/top/1607.html
    # [4] https://hypernews.cern.ch/HyperNews/CMS/get/egamma/1263/1/2/1.html
    process.elPFIsoValueEA03 = cms.EDFilter('ElectronIsolatorFromEffectiveArea',
        gsfElectrons = cms.InputTag('gsfElectrons'),
        pfElectrons = cms.InputTag('pfSelectedElectrons'),
        rhoIso = cms.InputTag('kt6PFJets', 'rho'),
        EffectiveAreaType = cms.string('kEleGammaAndNeutralHadronIso03'),
        EffectiveAreaTarget = cms.string('kEleEAData2012'))
    
    
    # Change the isolation cone used in pfIsolatedElectrons to 0.3, as recommended in [1] and [2].
    # The parameter for the delta-beta correction is initialized with the map for the rho correction
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/EgammaCutBasedIdentification?rev=17#Particle_Flow_Isolation
    # [2] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
    process.pfIsolatedElectrons.isolationValueMapsCharged = cms.VInputTag(
        cms.InputTag('elPFIsoValueCharged03PFId'))
    process.pfIsolatedElectrons.deltaBetaIsolationValueMap = cms.InputTag('elPFIsoValuePU03PFId')
    process.pfIsolatedElectrons.isolationValueMapsNeutral = cms.VInputTag(
        cms.InputTag('elPFIsoValueNeutral03PFId'), cms.InputTag('elPFIsoValueGamma03PFId'))
    process.pfIsolatedElectrons.deltaBetaIsolationValueMap = cms.InputTag('elPFIsoValueEA03')
    
    PFRecoSequence.replace(process.pfIsolatedElectrons,
     process.elPFIsoValueEA03 * process.pfIsolatedElectrons)
    
    
    # Adjust parameters for the rho correction [1]. The cut on the isolation value is set in
    # accordance with [2]
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections?rev=4#Electrons
    # [2] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Veto
    process.pfIsolatedElectrons.doDeltaBetaCorrection = True
    process.pfIsolatedElectrons.deltaBetaFactor = -1.
    process.pfIsolatedElectrons.isolationCut = 0.15
    
    
    # Load electron MVA ID modules. See an example in [1], which is referenced from [2]
    # [1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/EgammaAnalysis/ElectronTools/test/patTuple_electronId_cfg.py?view=markup&pathrev=SE_PhotonIsoProducer_MovedIn
    # [2] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification?rev=45#Recipe_for_53X
    process.load('EgammaAnalysis.ElectronTools.electronIdMVAProducer_cfi')
    
    # Insert a module to filter electrons based on their ID. See [1] as an example
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections?rev=4#Electrons
    process.pfIdentifiedElectrons = cms.EDFilter('ElectronIDPFCandidateSelector',
        recoGsfElectrons = cms.InputTag('gsfElectrons'),
        electronIdMap = cms.InputTag('mvaTrigV0'),
        electronIdCut = cms.double(0.),
        src = cms.InputTag('pfIsolatedElectrons'))
    
    
    # Apply remaining cuts that define veto electrons as required in [1]. It is implemented via an
    # additional module and not in pfSelectedElectrons, becase all the isolation maps are associated
    # with the latter collection, and they will be needed also for a looser electron selection
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Veto
    process.pfElectronsForTopProjection = process.pfSelectedElectrons.clone(
        src = 'pfIdentifiedElectrons',
        cut = 'pt > 20. & abs(eta) < 2.5')
    process.pfNoElectron.topCollection = 'pfElectronsForTopProjection'
    
    PFRecoSequence.replace(process.pfIsolatedElectrons,
     process.pfIsolatedElectrons * process.mvaTrigV0 * process.pfIdentifiedElectrons *
     process.pfElectronsForTopProjection)
    
    
    
    # Collection pfElectronsForTopProjection, which contains isolated and identified electrons
    # passing basic kinematical cuts, is used in the top projections. It should also be
    # provided to the MET uncertainty tool (in case of MC simulated data); however, the latter
    # expects PAT electrons. Since they cannot be constructed from pfElectronsForTopProjection
    # collection (isolation ValueMap issues), they should be subjected to an additional selection.
    
    # Set an accessor for the MVA ID [1-2]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
    # [2] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/EGamma/EGammaAnalysisTools/test/patTuple_electronId_cfg.py?revision=1.2&view=markup&pathrev=V00-00-16
    process.patElectrons.electronIDSources = cms.PSet(mvaTrigV0 = cms.InputTag('mvaTrigV0'))
    
    # Change the size of the isolation cone for PAT electrons
    from PhysicsTools.PatAlgos.tools.pfTools import adaptPFIsoElectrons
    adaptPFIsoElectrons(process, process.patElectrons, '', '03')
    
    # Insert the effective-area isolation correction as a user isolation
    process.patElectrons.isolationValues.user = cms.VInputTag(cms.InputTag('elPFIsoValueEA03'))
    
    # Selection to mimic the pfElectronsForTopProjection collection
    process.selectedPatElectrons.cut = '(' + process.pfElectronsForTopProjection.cut.value() + \
     ') & electronID("mvaTrigV0") > 0.'
    
    
    
    # Although the "good" electrons are a subset of the patElectrons collection defined above, it is
    # usefull to save all the electrons in the event (especially for the QCD studies). Duplicate the
    # patElectrons module to perform it
    process.nonIsolatedLoosePatElectrons = process.patElectrons.clone(
        pfElectronSource = 'pfSelectedElectrons')
    
    PFRecoSequence.replace(process.patElectrons, process.patElectrons +
     process.nonIsolatedLoosePatElectrons)
    
    
    
    # The above electron collection will be stored in the produced tuples. It is also needed to save
    # the results of some quality criteria evaluation. Such parameters as pt, eta, isolation,
    # transverse impact-parameter, MVA ID value, and conversion flag are stored in the tuples,
    # whereas other criteria are not. Instead they are encoded in the following selection strings
    eleQualityCuts = cms.vstring(
        '(abs(superCluster.eta) < 1.4442 | abs(superCluster.eta) > 1.5660)')
    
    
    
    # Finally, a collection for the event selection is needed. It is based on pure kinematical
    # properties only (the electron is allowed to be non-isolated or be poorly identified). Note
    # that it is recommended [1] to use momentum of the associated GSF electron
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/B2GRefEventSel#Isolation_and_Corrections_to_Iso
    process.patElectronsForEventSelection = process.selectedPatElectrons.clone(
        src = 'nonIsolatedLoosePatElectrons',
        cut = 'ecalDrivenMomentum.pt > 30. & (ecalDrivenMomentum.eta < 2.5)')
    
    PFRecoSequence.replace(process.nonIsolatedLoosePatElectrons,
     process.nonIsolatedLoosePatElectrons + process.patElectronsForEventSelection)
    
    
    # Return values
    return eleQualityCuts



def DefineMuons(process, PFRecoSequence, runOnData):
    """ This function adjusts muon reconstruction. The following collections and variables are
        expected to be used by the user:
        
        1. nonIsolatedLoosePatMuons: maximally loose collection of muons to be stored in the tuples.
        
        2. muQualityCuts: vector of quality cuts to be applied to the above collection.
        
        3. patMuonsForEventSelection: collection of loose-quality muons that pass basic kinematical
        requirements; to be used for an event selection.
        
        4. selectedPatMuons: loosely identified and isolated muons, which are expected by the MET
        uncertainty tool.
    """
    
    # Enable delta-beta correction for the muon isolation and set the recommended cut [1]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#Muon_Isolation
    process.pfIsolatedMuons.doDeltaBetaCorrection = True
    process.pfIsolatedMuons.deltaBetaFactor = -0.5
    process.pfIsolatedMuons.isolationCut = 0.2
    
    
    # Apply remaining cuts that are recommended for loose/veto muons [1]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Muons
    process.pfMuonsForTopProjection = process.pfSelectedMuons.clone(
        src = 'pfIsolatedMuons',
        cut = 'pt > 10. & abs(eta) < 2.5')
    process.pfNoMuon.topCollection = 'pfMuonsForTopProjection'
    
    PFRecoSequence.replace(process.pfIsolatedMuons,
     process.pfIsolatedMuons * process.pfMuonsForTopProjection)
    
    
    # Collection pfMuonsForTopProjection is used for the top projection and contains (loosely)
    # isolated muons satisfying some loose identification criteria. A collection that refers to the
    # same physics objects is expected to be fed into the MET uncertainty tool, but the latter works
    # with PAT muons and not with PF candidates. For this reason selection adopted for
    # pfMuonsForTopProjection is reimplemented for selectedPatMuons. In addition, identification
    # requirements from [1], which cannot be applied to PF candidates, are checked. The first one,
    # pat::Muon::isPFMuon is expected to return true always since PFBRECO is used, and it is
    # provided for completeness only
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#Loose_Muon
    process.selectedPatMuons.cut = '(' + process.pfMuonsForTopProjection.cut.value() + \
     ') & isPFMuon & (isGlobalMuon | isTrackerMuon)'
    
    
    # The "good" muons are contained in the above collection, but it is advantageous to save all the
    # muons in the event, including the loose ones, in order to allow QCD studies. The task is
    # performed with a duplicate of patMuons module
    process.nonIsolatedLooseMuonMatch = process.muonMatch.clone(
        src = 'pfSelectedMuons')
    process.nonIsolatedLoosePatMuons = process.patMuons.clone(
        pfMuonSource = 'pfSelectedMuons',
        genParticleMatch = 'nonIsolatedLooseMuonMatch')
    
    PFRecoSequence.replace(process.patMuons, process.patMuons +
     process.nonIsolatedLooseMuonMatch * process.nonIsolatedLoosePatMuons)
    if runOnData:
        PFRecoSequence.remove(process.nonIsolatedLooseMuonMatch)
    
    
    # The above collection of muons is saved in the tuples. It is needed to store informaion about
    # quality criteria the muons meet. A part of it is encoded in dedicated variables such as pt,
    # eta, isolation, or impact-parameter. The others are included in the selection strings
    # below. They follow recommendations in [1]
    # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#The2012Data
    muQualityCuts = cms.vstring(
        # loose muons for veto
        'isPFMuon & (isGlobalMuon | isTrackerMuon)',
        # tight muons
        'isPFMuon & isGlobalMuon & globalTrack.normalizedChi2 < 10 & '\
        'globalTrack.hitPattern.numberOfValidMuonHits > 0 & numberOfMatchedStations > 1 & '\
        'innerTrack.hitPattern.numberOfValidPixelHits > 0 & '\
        'track.hitPattern.trackerLayersWithMeasurement > 5')
    
    
    # Finally, a collection for an event selection is needed. It is based on pure kinematical
    # properties only (the muon is allowed to be non-isolated or be poorly identified)
    process.patMuonsForEventSelection = process.selectedPatMuons.clone(
        src = 'nonIsolatedLoosePatMuons',
        cut = 'pt > 24. & abs(eta) < 2.1')
    
    PFRecoSequence.replace(process.nonIsolatedLoosePatMuons,
     process.nonIsolatedLoosePatMuons + process.patMuonsForEventSelection)
    
    
    # Return values
    return muQualityCuts


def DefineJets(process, paths, runOnData):
    """ Adjusts the jet reconstruction. The function must be called after the MET uncertainty tool.
        The user is expected to operate with the following collections:
        
        1. analysisPatJets: jets subjected to the recommended quality selection; to be used in the
        analysis.
        
        2. patJetsForEventSelection: a hard subset of the above collection needed to perform the
        event selection.
        
        3. selectedPatJets: jets from PFBRECO embedded in pat::Jet class; to be used with the MET
        uncertainty tool.
    """
    
    # Jet identification criteria as recommended in (*)
    # (*) https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID
    jetQualityCut = 'numberOfDaughters > 1 & neutralHadronEnergyFraction < 0.99 & '\
     'neutralEmEnergyFraction < 0.99 & (abs(eta) < 2.4 & chargedEmEnergyFraction < 0.99 & '\
     'chargedHadronEnergyFraction > 0. & chargedMultiplicity > 0 | abs(eta) >= 2.4)'
    
    
    # The collection selectedPatJets encodes the jets reconstructed in the PFBRECO framework; they
    # are used in the MET uncertainty tool
    
    
    # Jets considered in the analysis are subjected to an additional selection
    process.analysisPatJets = process.selectedPatJets.clone(
        src = 'selectedPatJets' if runOnData else 'smearedPatJets',
        cut = 'pt > 10. & abs(eta) < 4.7 & (' + jetQualityCut + ')')
    
    
    # Jets used in the event selection
    process.patJetsForEventSelection = process.analysisPatJets.clone(
        src = 'analysisPatJets',
        cut = 'pt > 40.')
    
    
    paths.append(process.selectedPatJets, process.analysisPatJets, process.patJetsForEventSelection)
    
    
    # Finally, switch on the tag infos. It is needed to access the secondary vertex (*)
    # (*) https://hypernews.cern.ch/HyperNews/CMS/get/physTools/2714/2/1/1.html
    process.patJets.addTagInfos = True


def DefineMETs(process, paths, runOnData, jecLevel):
    """ The function adjusts MET reconstruction. The following corrections are included: type-I
        (switched on by PF2PAT function), type-0, and phi-modulation correction.
    """
    
    METCollections = []
    #^ MET collections to store. The first one will be raw PF MET, the second one will include the
    # type-I and type-0 corrections as well as the MET phi-modulation correction. Type-I correction
    # is performed with selectedPatJets collection
    process.load('JetMETCorrections.Type1MET.pfMETsysShiftCorrections_cfi')

    if runOnData:
        METCollections.extend(['patPFMet', 'patMETs'])
        
        # Include the type-0 MET correction. The code is inspired by [1]
        # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis#Type_I_II_0_with_PF2PAT
        process.patType1CorrectedPFMet.srcType1Corrections.append(
            cms.InputTag('patPFMETtype0Corr'))
        
        # Correct for MET phi modulation. The code is inspired by the implementation [1] of the
        # runMEtUncertainties tool and [2]
        # [1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/PhysicsTools/PatUtils/python/tools/metUncertaintyTools.py?revision=1.25&view=markup
        # [2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis#MET_x_y_Shift_Correction_for_mod
        process.pfMEtSysShiftCorr.parameter = process.pfMEtSysShiftCorrParameters_2012runABCDvsNvtx_data
        process.patType1CorrectedPFMet.srcType1Corrections.append(
            cms.InputTag('pfMEtSysShiftCorr'))
        
        # There is some mismatch between the python configurations and CMSSW plugins in the current
        # setup (*), (**). This is corrected below
        # (*) http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/JetMETCorrections/Type1MET/plugins/SysShiftMETcorrInputProducer.cc?revision=1.2&view=markup
        # (**) http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/JetMETCorrections/Type1MET/plugins/SysShiftMETcorrInputProducer.cc?revision=1.3&view=markup
        process.pfMEtSysShiftCorr.src = process.pfMEtSysShiftCorr.srcMEt
        process.pfMEtSysShiftCorr.parameter = process.pfMEtSysShiftCorr.parameter[0]
        
        process.patPF2PATSequence.replace(process.patType1CorrectedPFMet,
         process.pfMEtSysShiftCorrSequence * process.patType1CorrectedPFMet)
    
    else:  # in case of MC the runMEtUncertainties tool takes care of the corrections
        METCollections.extend(['patMETs', 'patType1CorrectedPFMet'])
        
        # Produce the corrected MET and perform systematical shifts [1]
        # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools#METSysTools
        from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
        runMEtUncertainties(process,
            electronCollection = 'selectedPatElectrons', muonCollection = 'selectedPatMuons',
            tauCollection = '', photonCollection = '', jetCollection = 'selectedPatJets',
            jetCorrLabel = jecLevel,
            makeType1corrPFMEt = True, makeType1p2corrPFMEt = False,
            doApplyType0corr = True, doApplySysShiftCorr = True,
            sysShiftCorrParameter = process.pfMEtSysShiftCorrParameters_2012runABCDvsNvtx_mc[0],
            #dRjetCleaning = -1,  # this parameter is never used by the function
            addToPatDefaultSequence = False, outputModule = '')
        
        # Switch off the lepton-jet cleaning
        del(process.patJetsNotOverlappingWithLeptonsForMEtUncertainty.checkOverlaps.electrons)
        del(process.patJetsNotOverlappingWithLeptonsForMEtUncertainty.checkOverlaps.muons)
        
        # Add systematical variations
        METCollections.extend(['patType1CorrectedPFMetJetEnUp', 'patType1CorrectedPFMetJetEnDown',
            'patType1CorrectedPFMetJetResUp', 'patType1CorrectedPFMetJetResDown',
            'patType1CorrectedPFMetUnclusteredEnUp', 'patType1CorrectedPFMetUnclusteredEnDown',
            'patType1CorrectedPFMetElectronEnUp', 'patType1CorrectedPFMetElectronEnDown',
            'patType1CorrectedPFMetMuonEnUp', 'patType1CorrectedPFMetMuonEnDown'])
        
        # Update files with individual JEC uncertainty sources [1] and correct name of subtotal
        # uncertainty as it is outdated in the default configuraion. There are two files with jet
        # energy corrections and uncertainties: for data and for simulation; the difference
        # originate from L1FastJet corrections [2]
        # [1] https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources?rev=17#2012_JEC
        # [2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=115#JetEnCor2012Summer13
        for moduleName in process.metUncertaintySequence.moduleNames():
            module = getattr(process, moduleName)
            if ['jetCorrUncertaintyTag', 'jetCorrInputFileName'] in dir(module):
                module.jetCorrUncertaintyTag = 'SubTotalMC'
                module.jetCorrInputFileName = cms.FileInPath(
                 'UserCode/SingleTop/data/Summer13_V4_MC_Uncertainty_AK5PFchs.txt')
        
        # Correct for the mismatch between the python configurations and CMSSW plugins (similar to the
        # case of the real data above)
        process.pfMEtSysShiftCorr.src = process.pfMEtSysShiftCorr.srcMEt
        
        paths.append(process.metUncertaintySequence)
    
    # Return the list of produced collections
    return METCollections

