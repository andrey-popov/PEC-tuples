#!/bin/bash

# The latest release in the 76X series
release="CMSSW_7_6_4"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded

exit 0
# Code below has not been updated


# Bugfixes in CMSSW
git cms-merge-topic andrey-popov:fix-ShiftedJetProducerT
