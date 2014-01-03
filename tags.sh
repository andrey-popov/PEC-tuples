#!/bin/bash

# Use the latest release recommended for 2012 data
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATRecipes?rev=302#CMSSW_5_3_X_pro2012
release="CMSSW_5_3_11"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded

# Use this interface to determine which tags are included in the release if needed (go to "Release
# management" section):
# https://cmstags.cern.ch/tc/#Releases


# Tags for the latest PAT release V08-09-62. Tags recommended for tau reconstruction are ignored
# as tau leptons are not used in the analysis
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATReleaseNotes52X?rev=148#V08_09_62
addpkg DataFormats/PatCandidates V06-05-06-12
addpkg PhysicsTools/PatAlgos     V08-09-62
addpkg PhysicsTools/PatUtils
#^ The last command checks a package out of release. It must be recompiled due to dependencies of
# other packages. See an example in
# https://hypernews.cern.ch/HyperNews/CMS/get/physTools/3021/1.html

# Latest b-tagging (recommended in the PAT recipe cited above)
addpkg RecoBTag/ImpactParameter V01-04-09-01
addpkg RecoBTag/SecondaryVertex V01-10-06
addpkg RecoBTag/SoftLepton      V05-09-11
addpkg RecoBTau/JetTagComputer  V02-03-02
addpkg RecoBTag/Configuration   V00-07-05

# Latest EGM isolation (recommended in the PAT recipe cited above)
addpkg RecoParticleFlow/PFProducer V15-02-06


# Latest tag for pixel-based luminosity
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/LumiCalc
addpkg RecoLuminosity/LumiDB   V04-02-08


# Recommendations for electron MVA ID
# https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification?rev=45#Recipe_for_53X
addpkg RecoEgamma/EgammaTools       V09-00-01
addpkg EgammaAnalysis/ElectronTools SE_PhotonIsoProducer_MovedIn
cvs co -r SE_PhotonIsoProducer_MovedOut -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools

cd EgammaAnalysis/ElectronTools/data/
cat download.url | xargs wget
cd -


# MET tags
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMETRecipe53X?rev=14#CMSSW_5_3_11
addpkg RecoMET/METAnalyzers V00-00-08
addpkg DataFormats/METReco V03-03-11-01
addpkg JetMETCorrections/Type1MET V04-06-09-02


# Latest MET phi-modulation corrections. The enclosing subpackage has been checked out as a part of
# MET tags. If it were not the case (i.e. if the release contained the recommended version of the
# subpackage), it would not be sufficient to check out the configuration fragment with corrections
# only but the whole python directory. The reason is that modules imported in this fragment are
# searched for locally.
#cvs co -r $release JetMETCorrections/Type1MET/python
cvs co -r 1.7 JetMETCorrections/Type1MET/python/pfMETsysShiftCorrections_cfi.py


# MET filters
# https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters?rev=61#A_Central_Filter_Package_RecoMET
#addpkg RecoMET/METFilters       V00-00-13-01  # same version in the release
#addpkg RecoMET/METAnalyzers     V00-00-08  # already check out as a part of MET tags
#addpkg CommonTools/RecoAlgos    V00-03-23  # a newer version in the release
#addpkg DPGAnalysis/Skims        V01-00-11-01  # a newer version in the release
addpkg DPGAnalysis/SiStripTools V00-11-17
#addpkg DataFormats/TrackerCommon V00-00-08  # same version in the release
#addpkg RecoLocalTracker/SubCollectionProducers V01-09-05  # same version in the release


# Correct an exception thrown when JEC are calculated for jet with too large pt or |eta|. See details in [1]
# [1] https://github.com/andrey-popov/single-top/issues/1
addpkg CondFormats/JetMETObjects
#^ A version from the release is checked out
sed -i 's/throw cms::Exception("SimpleJetCorrectionUncertainty")<<" bin variables out of range";/\
return 1.f;/1' CondFormats/JetMETObjects/src/SimpleJetCorrectionUncertainty.cc
