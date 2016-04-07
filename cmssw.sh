#!/bin/bash

# The latest release in the 76X series
release="CMSSW_7_6_4"
scramv1 project CMSSW $release  # cmsrel alias expanded

cd $release/src
eval `scramv1 runtime -sh`  # this is cmsenv alias expanded


# Bugfixes in CMSSW
git cms-init
git remote add private-updates https://github.com/andrey-popov/cmssw.git
git fetch private-updates
git cms-addpkg PhysicsTools/PatUtils
git cherry-pick fcb419ef5d4c2e2b7c1410f4d9d8621bf641d524  # From branch fix-ShiftedJetProducerT


# Update pile-up jet ID.  Recipe is based on instructions in [1], but instead of switching to a
# dedicated branch, commits that are believed to be releavant, are cherry-picked [2].
# [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID?rev=25#Information_for_13_TeV_data_anal
# [2] https://hypernews.cern.ch/HyperNews/CMS/get/jet-algorithms/383/3.html
git cms-addpkg RecoJets/JetProducers
git remote add pu-jet-id https://github.com/jbrands/cmssw.git
git fetch pu-jet-id
git cherry-pick 14b0ec7a968a9ec69952c172d72fc669b2df1143^..3aace641ab0531604ab0db50801b9ae8156d069a
cd RecoJets/JetProducers/data/
wget https://github.com/jbrands/RecoJets-JetProducers/raw/3dad903ed25d025f68be94d6f781ca957d6f86ac/pileupJetId_76x_Eta0to2p5_BDT.weights.xml.gz
wget https://github.com/jbrands/RecoJets-JetProducers/raw/3dad903ed25d025f68be94d6f781ca957d6f86ac/pileupJetId_76x_Eta2p5to2p75_BDT.weights.xml.gz
wget https://github.com/jbrands/RecoJets-JetProducers/raw/3dad903ed25d025f68be94d6f781ca957d6f86ac/pileupJetId_76x_Eta2p75to3_BDT.weights.xml.gz
wget https://github.com/jbrands/RecoJets-JetProducers/raw/3dad903ed25d025f68be94d6f781ca957d6f86ac/pileupJetId_76x_Eta3to5_BDT.weights.xml.gz
cd ../../..


# Fix recomputation of MET
# [3] https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=41#Instructions_for_7_6_X_Recommend
git cms-merge-topic cms-met:metTool76X
