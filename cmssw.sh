#!/bin/bash

# Recent 80X release
release="CMSSW_8_0_28"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init

# Latest updates for MET for the legacy re-reconstruction [1]
# [1] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=66#Instructions_for_8_0_X_X_28_for
git cms-merge-topic -u cms-met:METRecipe_8020_for80Xintegration
