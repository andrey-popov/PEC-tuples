# This configuration fragment defines cut-based electron ID without
# requirements on isolation.  The file is a modified version of [1].  It
# was adjusted loosely following instructions in [2].  This basically
# means renaming the working points, resetting all isolation cuts to
# 999, and updating MD5 hashes.
# [1] https://github.com/lsoffi/cmssw/blob/CMSSW_9_4_0_pre3_TnP/RecoEgamma/ElectronIdentification/python/Identification/cutBasedElectronID_Fall17_94X_V1_cff.py
# [2] https://twiki.cern.ch/twiki/bin/view/CMS/TopEGM?rev=58#Cut_based_Electron_Identificatio


from PhysicsTools.SelectorUtils.centralIDRegistry import central_id_registry
import FWCore.ParameterSet.Config as cms

# Common functions and classes for ID definition are imported here:
from RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_tools \
    import (EleWorkingPoint_V4, IsolationCutInputs_V2, configureVIDCutBasedEleID_V4)


# Veto working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-veto"
WP_Veto_EB = EleWorkingPoint_V4(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0128  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00523 , # dEtaInSeedCut
    dPhiInCut                      = 0.159   , # dPhiInCut
    hOverECut_C0                   = 0.05    , # hOverECut
    hOverECut_CE                   = 1.12    ,
    hOverECut_Cr                   = 0.0368  ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.193   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 2          # missingHitsCut
    )

WP_Veto_EE = EleWorkingPoint_V4(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0445  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00984 , # dEtaInSeedCut
    dPhiInCut                      = 0.157   , # dPhiInCut
    hOverECut_C0                   = 0.05    , # hOverECut
    hOverECut_CE                   = 0.5     ,
    hOverECut_Cr                   = 0.201   ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0962   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 3          # missingHitsCut
)

# Loose working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-loose"
WP_Loose_EB = EleWorkingPoint_V4(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0105  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00387 , # dEtaInSeedCut
    dPhiInCut                      = 0.0716   , # dPhiInCut
    hOverECut_C0                   = 0.05    , # hOverECut
    hOverECut_CE                   = 1.12    ,
    hOverECut_Cr                   = 0.0368  ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.129   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

WP_Loose_EE = EleWorkingPoint_V4(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0356  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.0072 , # dEtaInSeedCut
    dPhiInCut                      = 0.147   , # dPhiInCut
    hOverECut_C0                   = 0.0414  , # hOverECut
    hOverECut_CE                   = 0.5     ,
    hOverECut_Cr                   = 0.201   ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0875   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1         # missingHitsCut
)

# Medium working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-medium"
WP_Medium_EB = EleWorkingPoint_V4(
    idName                         = idName , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0105, # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00365, # dEtaInSeedCut
    dPhiInCut                      = 0.0588  , # dPhiInCut
    hOverECut_C0                   = 0.026   , # hOverECut
    hOverECut_CE                   = 1.12    ,
    hOverECut_Cr                   = 0.0368  ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0327  , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

WP_Medium_EE = EleWorkingPoint_V4(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0309  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00625 , # dEtaInSeedCut
    dPhiInCut                      = 0.0355  , # dPhiInCut
    hOverECut_C0                   = 0.026    , # hOverECut
    hOverECut_CE                   = 0.5     ,
    hOverECut_Cr                   = 0.201   ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0335   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
)

# Tight working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-tight"
WP_Tight_EB = EleWorkingPoint_V4(
    idName                         = idName    , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0104   , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00353   , # dEtaInSeedCut
    dPhiInCut                      = 0.0499    , # dPhiInCut
    hOverECut_C0                   = 0.026   , # hOverECut
    hOverECut_CE                   = 1.12    ,
    hOverECut_Cr                   = 0.0368  ,
    relCombIsolationWithEALowPtCut = 999.      , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.      , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0278    , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

WP_Tight_EE = EleWorkingPoint_V4(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0305  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00567 , # dEtaInSeedCut
    dPhiInCut                      = 0.0165  , # dPhiInCut
    hOverECut_C0                   = 0.026   , # hOverECut
    hOverECut_CE                   = 0.5     ,
    hOverECut_Cr                   = 0.201   ,
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 999.    , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
)

# Second, define what effective areas to use for pile-up correction
isoInputs = IsolationCutInputs_V2(
    # phoIsolationEffAreas
    "RecoEgamma/ElectronIdentification/data/Fall17/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_92X.txt"
)


#
# Set up VID configuration for all cuts and working points
#

cutBasedElectronID_nonIso_veto = configureVIDCutBasedEleID_V4(WP_Veto_EB, WP_Veto_EE, isoInputs)
cutBasedElectronID_nonIso_loose = configureVIDCutBasedEleID_V4(WP_Loose_EB, WP_Loose_EE, isoInputs)
cutBasedElectronID_nonIso_medium = configureVIDCutBasedEleID_V4(WP_Medium_EB, WP_Medium_EE, isoInputs)
cutBasedElectronID_nonIso_tight = configureVIDCutBasedEleID_V4(WP_Tight_EB, WP_Tight_EE, isoInputs)


# The MD5 sum numbers below reflect the exact set of cut variables
# and values above. If anything changes, one has to 
# 1) comment out the lines below about the registry, 
# 2) run "calculateMD5 <this file name> <one of the VID config names just above>
# 3) update the MD5 sum strings below and uncomment the lines again.
#

central_id_registry.register(cutBasedElectronID_nonIso_veto.idName,
                             'a7cdbf9d82f51ba7ee7976e0359c2b39')
central_id_registry.register(cutBasedElectronID_nonIso_loose.idName,
                             'a23aa7d202e607ae05f69a40c7a25d1e')
central_id_registry.register(cutBasedElectronID_nonIso_medium.idName,
                             '238d090ca2b0dde7af8976fa81fb343f')
central_id_registry.register(cutBasedElectronID_nonIso_tight.idName,
                             '2223037d42a507c2d45404332df87e5c')


### for now until we have a database...
cutBasedElectronID_nonIso_veto.isPOGApproved = cms.untracked.bool(False)
cutBasedElectronID_nonIso_loose.isPOGApproved = cms.untracked.bool(False)
cutBasedElectronID_nonIso_medium.isPOGApproved = cms.untracked.bool(False)
cutBasedElectronID_nonIso_tight.isPOGApproved = cms.untracked.bool(False)
