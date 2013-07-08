#!/bin/bash

# Use the latest release recommended for 2012 data
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATRecipes#CMSSW_5_3_X_pro2012
release="CMSSW_5_3_11"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded

# Use this interface to determine which tags are included in the release if needed:
# https://cmstags.cern.ch/tc/#Releases


# Tags for the latest PAT release V08-09-62. Tags recommended for tau reconstruction are ignored
#as tau leptons are not used in the analysis
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATReleaseNotes52X
addpkg DataFormats/PatCandidates V06-05-06-12
addpkg PhysicsTools/PatAlgos     V08-09-62
addpkg PhysicsTools/PatUtils
#^ The last command checks a package out of release. It must be recompiled due to dependencies of
#other packages. See an example in
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
# https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Recipe_for_53X
addpkg RecoEgamma/EgammaTools       V09-00-01
addpkg EgammaAnalysis/ElectronTools SE_PhotonIsoProducer_MovedIn
cvs co -r SE_PhotonIsoProducer_MovedOut -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools

cd EgammaAnalysis/ElectronTools/data/
cat download.url | xargs wget
cd -




# WHAT IS BELOW THIS LINE HAS NOT BEEN CHECKED
#-------------------------------------------------------------------------------

# MET type-I and type-0 corrections
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMetAnalysis#Type_I_0_with_PAT
#addpkg CommonTools/ParticleFlow V00-03-16  # same version in the release
#addpkg CommonTools/RecoAlgos V00-03-23  # newer version in the release
#addpkg CommonTools/RecoUtils V00-00-13  # newer version in the release
#addpkg PhysicsTools/PatAlgos V08-09-23  # newer version checked out for PAT
#addpkg PhysicsTools/PatUtils V03-09-23  # newer version in the release
#addpkg DataFormats/ParticleFlowCandidate V15-03-03  # newer version in the release
#addpkg DataFormats/TrackReco V10-02-02  # same version in the release
#addpkg DataFormats/VertexReco V02-00-04  # same version in the release
#addpkg DataFormats/StdDictionaries V00-02-14  # same version checked out for PAT

# The latest data file for MET phi modulation (Run2012 A+B+C). However one shall check out the
# whole python subdirectory as the python configuration fragments will be searched for locally
#cvs co -r $release JetMETCorrections/Type1MET/python
#cvs co -r 1.6 JetMETCorrections/Type1MET/python/pfMETsysShiftCorrections_cfi.py

# The file with the jet resolution must be available locally (corresponds to CMS AN-2011/330)
#cvs co -r $release PhysicsTools/PatUtils/data/pfJetResolutionMCtoDataCorrLUT.root
#^ Aldready checked out

# MET filters
# https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters
#cvs co -r V00-00-13 RecoMET/METFilters
#cvs co -r V00-00-08 RecoMET/METAnalyzers
#cvs co -r V01-00-11-01 DPGAnalysis/Skims  # superceded by HCAL laser filer's dependencies
#cvs co -r V00-11-17 DPGAnalysis/SiStripTools

# HCAL-laser-polluted event filter requires CMSSW_5_3_7_patch5. These are the relevant packages
# https://twiki.cern.ch/twiki/bin/view/CMS/PdmVKnowFeatures#HCAL_laser_events_in_prompt_2012
#cvs co -r V01-00-03-10 EventFilter/HcalRawToDigi
#cvs co -r V01-00-15 DPGAnalysis/Skims


# B-tagging
# https://hypernews.cern.ch/HyperNews/CMS/get/btag/879.html
#~addpkg RecoBTag/PerformanceDB  V01-03-14
#~addpkg CondFormats/PhysicsToolsObjects V01-03-12  # needed by the above package


# The latest PU reweighting
# https://twiki.cern.ch/twiki/bin/view/CMS/PileupMCReweightingUtilities#Reweighting_Using_PhysicsTools_U
#~addpkg PhysicsTools/Utilities  V08-03-17
