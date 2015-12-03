#!/bin/bash

# The latest release in the 74X series
release="CMSSW_7_4_15_patch1"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


# Cut-based electron ID
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=27#Recipe_for_regular_users_for_7_4
git cms-merge-topic ikrav:egm_id_7.4.12_v1
