#!/bin/bash

# The latest release in the 80X series
release="CMSSW_8_0_8"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init

# Fixes not provided centrally
git remote add private-updates https://github.com/andrey-popov/cmssw.git
git fetch private-updates
git cms-addpkg PhysicsTools/PatUtils
git cherry-pick fcb419ef5d4c2e2b7c1410f4d9d8621bf641d524  # From branch fix-ShiftedJetProducerT
