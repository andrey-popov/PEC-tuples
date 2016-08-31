#!/bin/bash

# Release supported for MET
release="CMSSW_8_0_11"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init

# Updates for MET [1]
# [1] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=52#Instructions_for_8_0_X_X_11
git cms-merge-topic cms-met:metTool80X

# Additional MET filters [2]
# [2] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=99#How_to_run_the_Bad_Charged_Hadro
git cms-merge-topic -u cms-met:CMSSW_8_0_X-METFilterUpdate
