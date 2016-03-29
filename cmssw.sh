#!/bin/bash

# The latest release in the 76X series
release="CMSSW_7_6_4"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded

exit 0
# Code below has not been updated


# Cut-based electron ID
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=27#Recipe_for_regular_users_for_7_4
git cms-merge-topic ikrav:egm_id_7.4.12_v1

# Bugfixes in CMSSW
git cms-merge-topic andrey-popov:fix-ShiftedJetProducerT
