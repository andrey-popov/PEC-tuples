#!/bin/bash

# Use the latest 53X release recommended by PAT. However switch to 5_3_7_patch5 is recommended for
# HCAL laser pollusion
# https://twiki.cern.ch/twiki/bin/view/CMS/PdmVKnowFeatures#HCAL_laser_events_in_prompt_2012
release="CMSSW_5_3_7_patch4"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded

# Use this interface to determine which tags are included in the release:
# https://cmstags.cern.ch/tc/#Releases


# The latest PAT release V08-09-51
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATReleaseNotes52X
addpkg DataFormats/PatCandidates V06-05-06-05
addpkg PhysicsTools/PatAlgos     V08-09-51
addpkg DataFormats/StdDictionaries V00-02-14
addpkg FWCore/GuiBrowsers V00-00-70

# Dependency-induced check-out
# https://hypernews.cern.ch/HyperNews/CMS/get/physTools/3021/1.html
cvs co -r $release PhysicsTools/PatUtils

# Latest tag for pixel-based luminosity
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/LumiCalc
addpkg RecoLuminosity/LumiDB   V04-02-07

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
cvs co -r $release JetMETCorrections/Type1MET/python
cvs co -r 1.6 JetMETCorrections/Type1MET/python/pfMETsysShiftCorrections_cfi.py

# The file with the jet resolution must be available locally (corresponds to CMS AN-2011/330)
#cvs co -r $release PhysicsTools/PatUtils/data/pfJetResolutionMCtoDataCorrLUT.root
#^ Aldready checked out

# MET filters
# https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFilters
cvs co -r V00-00-13 RecoMET/METFilters
cvs co -r V00-00-08 RecoMET/METAnalyzers
#cvs co -r V01-00-11-01 DPGAnalysis/Skims  # superceded by HCAL laser filer's dependencies
cvs co -r V00-11-17 DPGAnalysis/SiStripTools

# HCAL-laser-polluted event filter requires CMSSW_5_3_7_patch5. These are the relevant packages
# https://twiki.cern.ch/twiki/bin/view/CMS/PdmVKnowFeatures#HCAL_laser_events_in_prompt_2012
cvs co -r V01-00-03-10 EventFilter/HcalRawToDigi
cvs co -r V01-00-15 DPGAnalysis/Skims

# MVA electron ID in PAT and effective-area rho correction to the isolation
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections?rev=4#Electrons
# https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#MVA_based_Id_in_PAT
# https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
cvs co -r V00-00-31-EA02 -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools
cd EGamma/EGammaAnalysisTools/data
cat download.url | xargs wget
cd -


# WHAT IS BELOW THIS LINE HAS NOT BEEN CHECKED
#-------------------------------------------------------------------------------

# B-tagging
# https://hypernews.cern.ch/HyperNews/CMS/get/btag/879.html
#~addpkg RecoBTag/PerformanceDB  V01-03-14
#~addpkg CondFormats/PhysicsToolsObjects V01-03-12  # needed by the above package


# The latest PU reweighting
# https://twiki.cern.ch/twiki/bin/view/CMS/PileupMCReweightingUtilities#Reweighting_Using_PhysicsTools_U
#~addpkg PhysicsTools/Utilities  V08-03-17
