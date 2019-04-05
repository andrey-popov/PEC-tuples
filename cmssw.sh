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

# Weights to emulated L1 ECAL pre-firing. Adjust the official recipe a bit using a patch.
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1ECALPrefiringWeightRecipe?rev=9#Get_the_code_and_compile
git cms-merge-topic lathomas:L1Prefiring_9_4_9

patch -p0 < prefiring.patch
mv L1Prefiring/EventWeightProducer/files L1Prefiring/EventWeightProducer/data

scram b -j 4

