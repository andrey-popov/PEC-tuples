#!/bin/bash

# Recent 94X release
release="CMSSW_9_4_13_patch13"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init --upstream-only

# Electron and photon ID [1]
# https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2?rev=14#2017_MiniAOD_V2
git cms-merge-topic cms-egamma:EgammaPostRecoTools

scram b -j 4

