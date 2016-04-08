#include "PECJetMET.h"

#include <CondFormats/JetMETObjects/interface/JetCorrectorParameters.h>
#include <JetMETCorrections/Objects/interface/JetCorrectionsRecord.h>

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <TMath.h>
#include <Math/GenVector/VectorUtil.h>


using namespace edm;
using namespace std;


PECJetMET::PECJetMET(edm::ParameterSet const &cfg):
    jecPayloadLabel(cfg.getParameter<string>("jecPayload")),
    jetMinPt(cfg.getParameter<double>("jetMinPt")),
    jetMinRawPt(cfg.getParameter<double>("jetMinRawPt")),
    runOnData(cfg.getParameter<bool>("runOnData")),
    rawJetMomentaOnly(cfg.getParameter<bool>("rawJetMomentaOnly"))
{
    // Register required input data
    jetToken = consumes<edm::View<pat::Jet>>(cfg.getParameter<InputTag>("jets"));
    metToken = consumes<edm::View<pat::MET>>(cfg.getParameter<InputTag>("met"));
    
    for (InputTag const &tag: cfg.getParameter<vector<InputTag>>("contIDMaps"))
        contIDMapTokens.emplace_back(consumes<ValueMap<float>>(tag));
    
    
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
    desc.add<string>("jecPayload")->setComment("Label of applied JEC payload.");
    desc.add<vector<string>>("jetSelection", vector<string>(0))->
     setComment("User-defined selections for jets whose results will be stored in the output "
     "tree.");
    desc.add<vector<InputTag>>("contIDMaps", vector<InputTag>(0))->
     setComment("Maps with real-valued ID decisions to be stored.");
    desc.add<double>("jetMinPt", 20.)->
     setComment("Jets with corrected pt above this threshold will be stored in the output tree.");
    desc.add<double>("jetMinRawPt", 999.)->
     setComment("Jets with raw pt above this threshold will be stored in the output tree.");
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
}


void PECJetMET::beginRun(Run const &, EventSetup const &setup)
{
    // Construct the object to obtain JEC uncertainty [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=124#JetCorUncertainties
    ESHandle<JetCorrectorParametersCollection> jecParametersCollection;
    setup.get<JetCorrectionsRecord>().get(jecPayloadLabel, jecParametersCollection); 
    
    JetCorrectorParameters const &jecParameters = (*jecParametersCollection)["Uncertainty"];
    jecUncProvider.reset(new JetCorrectionUncertainty(jecParameters));
}


void PECJetMET::analyze(Event const &event, EventSetup const &setup)
{
    // Read the jet collection
    Handle<View<pat::Jet>> srcJets;
    event.getByToken(jetToken, srcJets);
    
    
    // Read maps with real-valued jet ID
    vector<Handle<ValueMap<float>>> contIDMaps(contIDMapTokens.size());
    
    for (unsigned i = 0; i < contIDMapTokens.size(); ++i)
        event.getByToken(contIDMapTokens.at(i), contIDMaps.at(i));
    
    
    // Loop through the collection and store relevant properties of jets
    storeJets.clear();
    pec::Jet storeJet;  // will reuse this object to fill the vector
    
    for (unsigned int i = 0; i < srcJets->size(); ++i)
    {
        pat::Jet const &j = srcJets->at(i);
        storeJet.Reset();
        
        
        // Will check if the current jet satisfies the provided selection on transverse momentum.
        //Jets just below the threshold might pass it as a result of a fluctuation in JEC.
        //Check this possibility
        double jetPtUpFluctuationFactor = 1.;
        double curJECUncertainty = 0.;
        
        if (not runOnData)
        {
            // Find JEC uncertainty for the current jet [1]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=124#JetCorUncertainties
            jecUncProvider->setJetEta(j.eta());
            jecUncProvider->setJetPt(j.pt());
            curJECUncertainty = jecUncProvider->getUncertainty(true);
            
            
            // Update the factor for upwards fluctuation
            jetPtUpFluctuationFactor = 1. + fabs(curJECUncertainty);
        }
        
        
        // Perform filtering on transverse momentum and save properties of the current jet
        reco::Candidate::LorentzVector const &rawP4 = j.correctedP4("Uncorrected");
        
        if (j.pt() * jetPtUpFluctuationFactor > jetMinPt or rawP4.pt() > jetMinRawPt)
        {
            storeJet.SetPt(rawP4.pt());
            storeJet.SetEta(rawP4.eta());
            storeJet.SetPhi(rawP4.phi());
            storeJet.SetM(rawP4.mass());
            
            if (not rawJetMomentaOnly)
            {
                storeJet.SetJECFactor(1. / j.jecFactor("Uncorrected"));
                //^ Raw momentum is stored, and it will need to be corrected back to current level
                
                if (not runOnData)
                    storeJet.SetJECUncertainty(curJECUncertainty);
            }
            
            storeJet.SetArea(j.jetArea());
            storeJet.SetCharge(j.jetCharge());
            storeJet.SetBTagCSV(j.bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags"));
            storeJet.SetBTagCMVA(j.bDiscriminator("pfCombinedMVAV2BJetTags"));
            
            // Mass of the secondary vertex is available as userFloat [1]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2015?rev=92#Jets
            storeJet.SetSecVertexMass(j.userFloat("vtxMass"));
            
            
            // Save the pile-up ID if available. It should be the first ID in the dedicated
            //collection of maps
            if (contIDMaps.size() > 0)
            {
                Ptr<pat::Jet> const jetPtr(srcJets, i);
                storeJet.SetPileUpID((*contIDMaps.at(0))[jetPtr]);
            }
            
            
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
                
                storeJet.SetBit(0, (j.genJet() and j.genJet()->pt() > 8. and
                 ROOT::Math::VectorUtil::DeltaR(j.p4(), j.genJet()->p4()) < 0.25));
                //^ The matching is performed according to the definition from JME-13-005. By
                //default, PAT uses a looser definition
            }
            
            
            // User-difined selectors if any. The first bit has already been used for the match with
            //generator-level jet
            for (unsigned i = 0; i < jetSelectors.size(); ++i)
                storeJet.SetBit(i + 1, jetSelectors[i](j));
            
            
            // The jet is set up. Add it to the vector
            storeJets.emplace_back(storeJet);
        }
    }
    
    
    // Read MET
    Handle<View<pat::MET>> metHandle;
    event.getByToken(metToken, metHandle);
    pat::MET const &met = metHandle->front();
    
    storeMETs.clear();
    pec::Candidate storeMET;
    //^ Will reuse this object to fill the vector of METs
    
    // Nominal MET (type-I corrected)
    storeMET.Reset();
    storeMET.SetPt(met.shiftedPt(pat::MET::NoShift, pat::MET::Type1));
    storeMET.SetPhi(met.shiftedPhi(pat::MET::NoShift, pat::MET::Type1));
    storeMETs.emplace_back(storeMET);
    
    // Raw MET
    storeMET.Reset();
    storeMET.SetPt(met.shiftedPt(pat::MET::NoShift, pat::MET::Raw));
    storeMET.SetPhi(met.shiftedPhi(pat::MET::NoShift, pat::MET::Raw));
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
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECJetMET);
