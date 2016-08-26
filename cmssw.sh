#!/bin/bash

# The latest release in the 80X series
release="CMSSW_8_0_18"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded
