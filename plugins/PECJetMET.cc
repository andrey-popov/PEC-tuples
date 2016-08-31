#include "PECJetMET.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <FWCore/Utilities/interface/Exception.h>

#include <TMath.h>
#include <TVector2.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>


using namespace edm;
using namespace std;


PECJetMET::PECJetMET(edm::ParameterSet const &cfg):
    runOnData(cfg.getParameter<bool>("runOnData")),
    rawJetMomentaOnly(cfg.getParameter<bool>("rawJetMomentaOnly"))
{
    // Register required input data
    jetToken = consumes<edm::View<pat::Jet>>(cfg.getParameter<InputTag>("jets"));
    metToken = consumes<edm::View<pat::MET>>(cfg.getParameter<InputTag>("met"));
    
    for (InputTag const &tag: cfg.getParameter<vector<InputTag>>("contIDMaps"))
        contIDMapTokens.emplace_back(consumes<ValueMap<float>>(tag));
    
    
    // Currently plugin does not read any information from the ID maps. Throw an exception if any
    //are actually given.
    if (contIDMapTokens.size() > 0)
    {
        cms::Exception excp("Configuration");
        excp << "Currenly module ignores all continuous ID maps.";
        excp.raise();
    }
    
    
    // Construct string-based selectors
    for (string const &selection: cfg.getParameter<vector<string>>("jetSelection"))
        jetSelectors.emplace_back(selection);
}


void PECJetMET::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<bool>("runOnData")->
     setComment("Indicates whether data or simulation is being processed.");
    desc.add<InputTag>("jets")->setComment("Collection of jets.");
    desc.add<vector<string>>("jetSelection", vector<string>(0))->
     setComment("User-defined selections for jets whose results will be stored in the output "
     "tree.");
    desc.add<vector<InputTag>>("contIDMaps", vector<InputTag>(0))->
     setComment("Maps with real-valued ID decisions to be stored.");
    desc.add<bool>("rawJetMomentaOnly", false)->
     setComment("Requests that only raw jet momenta are saved but not their corrections.");
    desc.add<InputTag>("met")->setComment("MET.");
    
    descriptions.add("jetMET", desc);
}


void PECJetMET::beginJob()
{
    outTree = fileService->make<TTree>("JetMET", "Properties of reconstructed jets and MET");
    
    
    storeJetsPointer = &storeJets;
    outTree->Branch("jets", &storeJetsPointer);
    
    storeMETsPointer = &storeMETs;
    outTree->Branch("METs", &storeMETsPointer);
    
    storeUncorrMETsPointer = &storeUncorrMETs;
    outTree->Branch("uncorrMETs", &storeUncorrMETsPointer);
    
    outTree->Branch("METSignificance", &storeMETSignificance);
}


void PECJetMET::analyze(Event const &event, EventSetup const &)
{
    // Read the jet collection
    Handle<View<pat::Jet>> srcJets;
    event.getByToken(jetToken, srcJets);
    
    
    // Read maps with real-valued jet ID. They are however not used currently.
    vector<Handle<ValueMap<float>>> contIDMaps(contIDMapTokens.size());
    
    for (unsigned i = 0; i < contIDMapTokens.size(); ++i)
        event.getByToken(contIDMapTokens.at(i), contIDMaps.at(i));
    
    
    // A part of the T1 MET correction evaluated only with stored jets. It will be used to compute
    //partly uncorrected MET.
    TVector2 metT1Corr;
    
    
    // Loop through the collection and store relevant properties of jets
    storeJets.clear();
    pec::Jet storeJet;  // will reuse this object to fill the vector
    
    for (unsigned int i = 0; i < srcJets->size(); ++i)
    {
        pat::Jet const &j = srcJets->at(i);
        storeJet.Reset();
        
        
        reco::Candidate::LorentzVector const &rawP4 = j.correctedP4("Uncorrected");
        
        storeJet.SetPt(rawP4.pt());
        storeJet.SetEta(rawP4.eta());
        storeJet.SetPhi(rawP4.phi());
        storeJet.SetM(rawP4.mass());
        
        if (not rawJetMomentaOnly)
        {
            if (runOnData)
            {
                storeJet.SetCorrFactor(1. / j.jecFactor("Uncorrected"));
                //^ Here jecFactor("Uncorrected") returns the factor to get raw momentum starting
                //from the corrected one. Since in fact the raw momentum is stored, the factor is
                //inverted
            }
            else
            {
                double const jerFactorNominal = j.userFloat("jerFactorNominal");
                double const jerFactorUp = j.userFloat("jerFactorUp");
                double const jerFactorDown = j.userFloat("jerFactorDown");
                
                storeJet.SetCorrFactor(1. / j.jecFactor("Uncorrected") * jerFactorNominal);
                //^ See the comment for real data concerning the inverted JEC factor
                storeJet.SetJECUncertainty(j.userFloat("jecUncertainty"));
                
                // For JER the variation is not necessarily symmetric. Save the largest
                //variation. Information about the sign of the variation is preserved, and the
                //stored uncertainty is negative if "up" variation of JER decreases the smearing
                //factor
                if (std::abs(jerFactorUp - jerFactorNominal) >
                  std::abs(jerFactorDown - jerFactorNominal))
                    storeJet.SetJERUncertainty(jerFactorUp / jerFactorNominal - 1.);
                else
                    storeJet.SetJERUncertainty(1. - jerFactorDown / jerFactorNominal);
            }
        }
        
        
        storeJet.SetArea(j.jetArea());
        storeJet.SetCharge(j.jetCharge());
        storeJet.SetBTag(pec::Jet::BTagAlgo::CSV,
          j.bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags"));
        storeJet.SetBTag(pec::Jet::BTagAlgo::CMVA, j.bDiscriminator("pfCombinedMVAV2BJetTags"));
        storeJet.SetCTag(pec::Jet::CTagAlgo::CvsB, j.bDiscriminator("pfCombinedCvsBJetTags"));
        storeJet.SetCTag(pec::Jet::CTagAlgo::CvsL, j.bDiscriminator("pfCombinedCvsLJetTags"));
        
        // Mass of the secondary vertex is available as userFloat [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2015?rev=92#Jets
        storeJet.SetSecVertexMass(j.userFloat("vtxMass"));
        
        // Save pileup ID
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID?rev=29#Information_for_13_TeV_data_anal
        storeJet.SetPileUpID(j.userFloat("pileupJetId:fullDiscriminant"));
        
        
        // Calculate the jet pull angle
        double const y = rawP4.Rapidity();
        double const phi = rawP4.phi();
        //^ It is fine to use uncorrected jet momentum since JEC does not affect the direction
        double pullY = 0., pullPhi = 0.;  // projections of the pull vector (unnormalised)
        
        // Loop over constituents of the jet
        for (unsigned iDaughter = 0; iDaughter < j.numberOfDaughters(); ++iDaughter)
        {
            reco::Candidate const *p = j.daughter(iDaughter);
            //^ Actually jet constituents are of type pat::PackedCandidate, but here only their
            //four-momenta are needed, so I do not upcast them
            
            double dPhi = p->phi() - phi;
            
            if (dPhi < -TMath::Pi())
                dPhi = 2 * TMath::Pi() + dPhi;
            else if (dPhi > TMath::Pi())
                dPhi = -2 * TMath::Pi() + dPhi;
            
            double const r = hypot(p->rapidity() - y, dPhi);
            pullY += p->pt() * r * (p->rapidity() - y);
            pullPhi += p->pt() * r * dPhi;
        }
        //^ The pull vector should be normalised by the jet's pt, but since I'm interested in
        //the polar angle only, it is not necessary
        
        storeJet.SetPullAngle(atan2(pullPhi, pullY));
        

        if (not runOnData)
        // These are variables is from the generator tree, but it's more convenient to
        //calculate it here
        {
            storeJet.SetFlavour(j.hadronFlavour());
            storeJet.SetBit(0, bool(j.userInt("hasGenMatch")));
        }
        
        
        // User-difined selectors if any. The first bit has already been used for the match with
        //generator-level jet
        for (unsigned i = 0; i < jetSelectors.size(); ++i)
            storeJet.SetBit(i + 1, jetSelectors[i](j));
        
        
        // The jet is set up. Add it to the vector
        storeJets.emplace_back(storeJet);
        
        
        // Update the partial T1 MET correction
        auto const deltaT1JetP4 = -(j.p4() - j.correctedP4("L1FastJet"));
        metT1Corr += TVector2(deltaT1JetP4.Px(), deltaT1JetP4.Py());
    }
    
    
    // Read MET
    Handle<View<pat::MET>> metHandle;
    event.getByToken(metToken, metHandle);
    pat::MET const &met = metHandle->front();
    
    storeMETSignificance = met.metSignificance();
    
    storeMETs.clear();
    pec::Candidate storeMET;
    //^ Will reuse this object to fill the vector of METs
    
    // Nominal MET (type-I corrected)
    storeMET.Reset();
    storeMET.SetPt(met.shiftedPt(pat::MET::NoShift, pat::MET::Type1));
    storeMET.SetPhi(met.shiftedPhi(pat::MET::NoShift, pat::MET::Type1));
    storeMETs.emplace_back(storeMET);
    
    // An empty entry is put for backward compatibility
    storeMET.Reset();
    storeMETs.emplace_back(storeMET);
    
    
    // Save MET with systematical variations
    if (not runOnData)
    {
        using Var = pat::MET::METUncertainty;
        
        for (Var const &var: {Var::JetEnUp, Var::JetEnDown, Var::JetResUp, Var::JetResDown,
         Var::UnclusteredEnUp, Var::UnclusteredEnDown})
        {
            storeMET.Reset();
            storeMET.SetPt(met.shiftedPt(var, pat::MET::Type1));
            storeMET.SetPhi(met.shiftedPhi(var, pat::MET::Type1));
            storeMETs.emplace_back(storeMET);
        }
    }
    
    
    // Save variants of uncorrected MET
    storeUncorrMETs.clear();
    
    // Raw MET
    storeMET.Reset();
    storeMET.SetPt(met.shiftedPt(pat::MET::NoShift, pat::MET::Raw));
    storeMET.SetPhi(met.shiftedPhi(pat::MET::NoShift, pat::MET::Raw));
    storeUncorrMETs.emplace_back(storeMET);
    
    // MET with partly undone T1 correction
    TVector2 const metUncorrT1(met.shiftedPx(pat::MET::NoShift, pat::MET::Type1) - metT1Corr.Px(),
      met.shiftedPy(pat::MET::NoShift, pat::MET::Type1) - metT1Corr.Py());
    storeMET.Reset();
    storeMET.SetPt(metUncorrT1.Mod());
    storeMET.SetPhi(metUncorrT1.Phi());
    storeUncorrMETs.emplace_back(storeMET);
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECJetMET);
