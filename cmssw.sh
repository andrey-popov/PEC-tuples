#!/bin/bash

# Recent 80X release
release="CMSSW_8_0_26_patch1"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init

# Updates for MET [1]. They are contain updates for MET filters as well [2].
# [1] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=64#Instructions_for_8_0_X_X_26_patc
# [2] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=64#Important_Note
git cms-merge-topic -u cms-met:METRecipe_8020
git cms-merge-topic -u cms-met:METRecipe_80X_part2

# DeepCSV b-tagger [3]
# [3] https://twiki.cern.ch/twiki/bin/viewauth/CMS/DeepFlavour?rev=6
git cms-merge-topic -u mverzett:DeepFlavour-from-CMSSW_8_0_21
mkdir RecoBTag/DeepFlavour/data
cd !$
wget http://home.fnal.gov/~verzetti//DeepFlavour/training/DeepFlavourNoSL.json
cd $CMSSW_BASE/src
