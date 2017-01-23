#!/bin/bash

# Recent 80X release
release="CMSSW_8_0_24"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


# Initial build.  Needed to create $CMSSW_BASE/external area, which is used by the recipe for
# electron MVA ID.
scram b


git cms-init

# Updates for MET [1]
# [1] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=57#Instructions_for_8_0_X_X_20_for
git cms-merge-topic cms-met:METRecipe_8020

# Additional MET filters [2]
# [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=103#How_to_run_the_Bad_Charged_Hadro
git cms-merge-topic -u cms-met:fromCMSSW_8_0_20_postICHEPfilter

# Updated cut-based electron ID [3]
# [3] https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2?rev=41#Recipe_for_regular_users_for_8_0
git cms-merge-topic ikrav:egm_id_80X_v2


# Updated general-purpose MVA-based ID [4].  Note that code needs to be compiled for the directory
# $CMSSW_BASE/external to be created.  When running jobs in CRAB one needs to set
# config.JobType.sendExternalFolder = True.
# [4] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2?rev=30#Recipes_for_regular_users_common
cd $CMSSW_BASE/external/$SCRAM_ARCH/
git clone https://github.com/ikrav/RecoEgamma-ElectronIdentification.git data/RecoEgamma/ElectronIdentification/data
cd data/RecoEgamma/ElectronIdentification/data
git checkout egm_id_80X_v1
cd $CMSSW_BASE/src


# DeepCSV b-tagger [5]
# [5] https://twiki.cern.ch/twiki/bin/viewauth/CMS/DeepFlavour?rev=6
git cms-merge-topic -u mverzett:DeepFlavour-from-CMSSW_8_0_21
mkdir RecoBTag/DeepFlavour/data
cd !$
wget http://home.fnal.gov/~verzetti//DeepFlavour/training/DeepFlavourNoSL.json
cd $CMSSW_BASE/src
