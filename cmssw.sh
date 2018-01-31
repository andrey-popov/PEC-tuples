#!/bin/bash

# Recent 94X release
release="CMSSW_9_4_3"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init
