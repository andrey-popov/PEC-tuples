""" The module contains definitions of physical objects including required adjustments to the
    reconstruction process. Functions defined here must be called after usePF2PAT, name of the
    modules are hard-coded.
    """


# Metadata
__author__ = 'Andrey Popov'


import FWCore.ParameterSet.Config as cms


def DefineElectrons(process, paths):
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
    
    paths.append(process.analysisPatElectrons)
    
    
    # Calculate IDs for analysis electrons. The code is taken from the example in [1]; it looks
    # quite strange and really badly written
    # [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=13#Recipe_for_regular_users_for_min
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupAllVIDIdsInModule, \
     setupVIDElectronSelection
    process.load('RecoEgamma.ElectronIdentification.egmGsfElectronIDs_cfi')
    process.egmGsfElectronIDs.physicsObjectSrc = 'analysisPatElectrons'
    setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.' + \
     'cutBasedElectronID_PHYS14_PU20bx25_V1_miniAOD_cff', setupVIDElectronSelection)
    
    paths.append(process.egmGsfElectronIDs)
    
    
    # Define labels of electron IDs to be saved
    eleIDLabelPrefix = 'egmGsfElectronIDs:cutBasedElectronID-PHYS14-PU20bx25-V1-miniAOD-standalone-'
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
    
    paths.append(process.patElectronsForEventSelection)
    
    
    # Return values
    return eleQualityCuts, eleIDMaps



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
        
        # Insert missing modules into the sequence
        process.patPF2PATSequence.replace(process.patType1CorrectedPFMet,
            process.type0PFMEtCorrection + process.patPFMETtype0Corr + \
            process.pfMEtSysShiftCorrSequence + process.patType1CorrectedPFMet)
    
    else:  # in case of MC the runMEtUncertainties tool takes care of the corrections
        METCollections.extend(['patMETs', 'patType1CorrectedPFMet'])
        
        # Produce the corrected MET and perform systematical shifts [1]
        # [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATTools#METSysTools
        from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
        runMEtUncertainties(process,
            electronCollection = 'selectedPatElectrons', muonCollection = 'selectedPatMuons',
            tauCollection = '', photonCollection = '', jetCollection = 'cleanPatJets',
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
        
        # Correct for the mismatch between the python configurations and CMSSW plugins (similar to
        # the case of the real data above)
        process.pfMEtSysShiftCorr.src = process.pfMEtSysShiftCorr.srcMEt
        
        # Insert modules to perform type-0 and phi-modulation corrections into the sequence (for
        # some reason MET uncertainty tool does not do it automatically)
        process.metUncertaintySequence.replace(process.patType1CorrectedPFMet,
            process.type0PFMEtCorrection + process.patPFMETtype0Corr + process.pfMEtSysShiftCorr + \
            process.patType1CorrectedPFMet)
        #^ Some collections are created several times under different names (good primary vertices,
        # for example), but it should not impose a significant overhead
        
        paths.append(process.metUncertaintySequence)
    
    # Return the list of produced collections
    return METCollections

