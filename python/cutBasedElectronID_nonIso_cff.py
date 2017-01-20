# This configuration fragment defines cut-based electron ID without
# requirements on isolation.  The file is a modified version of [1].  It
# was adjusted loosely following instructions in [2].  This basically
# means renaming the working points, resetting all isolation cuts to
# 999, and updating MD5 hashes.
# [1] RecoEgamma/ElectronIdentification/python/Identification/cutBasedElectronID_Summer16_80X_V1_cff.py
# [2] https://twiki.cern.ch/twiki/bin/view/CMS/TopEGM?rev=58#Cut_based_Electron_Identificatio


from PhysicsTools.SelectorUtils.centralIDRegistry import central_id_registry

# Common functions and classes for ID definition are imported here:
from RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_tools import *


# Veto working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-veto"
WP_Veto_EB = EleWorkingPoint_V3(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0115  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00749 , # dEtaInSeedCut
    dPhiInCut                      = 0.228   , # dPhiInCut
    hOverECut                      = 0.356   , # hOverECut
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.299   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 2          # missingHitsCut
    )

WP_Veto_EE = EleWorkingPoint_V3(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0370  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00895 , # dEtaInSeedCut
    dPhiInCut                      = 0.213   , # dPhiInCut
    hOverECut                      = 0.211   , # hOverECut
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.150   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 3          # missingHitsCut
    )

# Loose working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-loose"
WP_Loose_EB = EleWorkingPoint_V3(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0110  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00477 , # dEtaInSeedCut
    dPhiInCut                      = 0.222   , # dPhiInCut
    hOverECut                      = 0.298   , # hOverECut
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.241   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

WP_Loose_EE = EleWorkingPoint_V3(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0314  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00868 , # dEtaInSeedCut
    dPhiInCut                      = 0.213   , # dPhiInCut
    hOverECut                      = 0.101   , # hOverECut
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.140   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1         # missingHitsCut
    )

# Medium working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-medium"
WP_Medium_EB = EleWorkingPoint_V3(
    idName                         = idName , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.00998, # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00311, # dEtaInSeedCut
    dPhiInCut                      = 0.103  , # dPhiInCut
    hOverECut                      = 0.253  , # hOverECut
    relCombIsolationWithEALowPtCut = 999.   , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.   , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.134  , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

WP_Medium_EE = EleWorkingPoint_V3(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0298  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00609 , # dEtaInSeedCut
    dPhiInCut                      = 0.0450  , # dPhiInCut
    hOverECut                      = 0.0878  , # hOverECut
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.130   , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

# Tight working point Barrel and Endcap
idName = "cutBasedElectronID-nonIso-tight"
WP_Tight_EB = EleWorkingPoint_V3(
    idName                         = idName    , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.00998   , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00308   , # dEtaInSeedCut
    dPhiInCut                      = 0.0816    , # dPhiInCut
    hOverECut                      = 0.0414    , # hOverECut
    relCombIsolationWithEALowPtCut = 999.      , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.      , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0129    , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

WP_Tight_EE = EleWorkingPoint_V3(
    idName                         = idName  , # idName
    full5x5_sigmaIEtaIEtaCut       = 0.0292  , # full5x5_sigmaIEtaIEtaCut
    dEtaInSeedCut                  = 0.00605 , # dEtaInSeedCut
    dPhiInCut                      = 0.0394  , # dPhiInCut
    hOverECut                      = 0.0641  , # hOverECut
    relCombIsolationWithEALowPtCut = 999.    , # relCombIsolationWithEALowPtCut
    relCombIsolationWithEAHighPtCut= 999.    , # relCombIsolationWithEAHighPtCut
    absEInverseMinusPInverseCut    = 0.0129 , # absEInverseMinusPInverseCut
    # conversion veto cut needs no parameters, so not mentioned
    missingHitsCut                 = 1          # missingHitsCut
    )

# Second, define what effective areas to use for pile-up correction
isoInputs = IsolationCutInputs_V2(
    # phoIsolationEffAreas
    "RecoEgamma/ElectronIdentification/data/Summer16/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_80X.txt"
)


#
# Set up VID configuration for all cuts and working points
#

cutBasedElectronID_nonIso_veto = configureVIDCutBasedEleID_V3(WP_Veto_EB, WP_Veto_EE, isoInputs)
cutBasedElectronID_nonIso_loose = configureVIDCutBasedEleID_V3(WP_Loose_EB, WP_Loose_EE, isoInputs)
cutBasedElectronID_nonIso_medium = configureVIDCutBasedEleID_V3(WP_Medium_EB, WP_Medium_EE, isoInputs)
cutBasedElectronID_nonIso_tight = configureVIDCutBasedEleID_V3(WP_Tight_EB, WP_Tight_EE, isoInputs)


# The MD5 sum numbers below reflect the exact set of cut variables
# and values above. If anything changes, one has to 
# 1) comment out the lines below about the registry, 
# 2) run "calculateMD5 <this file name> <one of the VID config names just above>
# 3) update the MD5 sum strings below and uncomment the lines again.
#

central_id_registry.register(cutBasedElectronID_nonIso_veto.idName,
                             'ec6b8b290acb73eb90b24c566fe0e2c8')
central_id_registry.register(cutBasedElectronID_nonIso_loose.idName,
                             '1977e321dd2689c990df48e6462b440c')
central_id_registry.register(cutBasedElectronID_nonIso_medium.idName,
                             'a1601c42d076f69aeedc8c0fad9df769')
central_id_registry.register(cutBasedElectronID_nonIso_tight.idName,
                             '2b8e7e709a52208cd1153a07e2e5f6b1')


### for now until we have a database...
cutBasedElectronID_nonIso_veto.isPOGApproved = cms.untracked.bool(False)
cutBasedElectronID_nonIso_loose.isPOGApproved = cms.untracked.bool(False)
cutBasedElectronID_nonIso_medium.isPOGApproved = cms.untracked.bool(False)
cutBasedElectronID_nonIso_tight.isPOGApproved = cms.untracked.bool(False)
