#!/bin/bash

# The latest release in the 80X series
release="CMSSW_8_0_18"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init

# Additional MET filters [1]
# [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=99#How_to_run_the_Bad_Charged_Hadro
git cms-merge-topic -u cms-met:CMSSW_8_0_X-METFilterUpdate
