#!/bin/bash

# Recent 94X release
release="CMSSW_9_4_3"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


git cms-init

# Electron ID [1-2]
# [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=53#Recipe_for_regular_users_for_92X
# [2] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2?rev=38#Recommended_MVA_Recipe_for_regul
git cms-merge-topic lsoffi:CMSSW_9_4_0_pre3_TnP
git cms-merge-topic guitargeek:ElectronID_MVA2017_940pre3

scram b -j 10

# Weights for electron MVA ID [1]
# [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2?rev=53#Recipe_for_regular_users_for_92X
cd $CMSSW_BASE/external/$SCRAM_ARCH
git clone https://github.com/lsoffi/RecoEgamma-ElectronIdentification.git data/RecoEgamma/ElectronIdentification/data
cd data/RecoEgamma/ElectronIdentification/data
git checkout CMSSW_9_4_0_pre3_TnP

cd $CMSSW_BASE/src
