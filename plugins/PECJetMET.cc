#include "PECJetMET.h"

#include <CondFormats/JetMETObjects/interface/JetCorrectorParameters.h>
#include <JetMETCorrections/Modules/interface/JetResolution.h>
#include <JetMETCorrections/Objects/interface/JetCorrectionsRecord.h>

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <TMath.h>
#include <Math/GenVector/VectorUtil.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>


using namespace edm;
using namespace std;


PECJetMET::PECJetMET(edm::ParameterSet const &cfg):
    jetType(cfg.getParameter<string>("jetType")),
    jetMinPt(cfg.getParameter<double>("jetMinPt")),
    jetMinRawPt(cfg.getParameter<double>("jetMinRawPt")),
    runOnData(cfg.getParameter<bool>("runOnData")),
    rawJetMomentaOnly(cfg.getParameter<bool>("rawJetMomentaOnly")),
    rGen(0)
{
    // Register required input data
    jetToken = consumes<edm::View<pat::Jet>>(cfg.getParameter<InputTag>("jets"));
    metToken = consumes<edm::View<pat::MET>>(cfg.getParameter<InputTag>("met"));
    rhoToken = consumes<double>(cfg.getParameter<InputTag>("rho"));
    
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
    desc.add<string>("jetType")->setComment("Jet type for JES and JER corrections.");
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
    desc.add<InputTag>("rho", InputTag("fixedGridRhoFastjetAll"))->
     setComment("Rho (mean angular pt density).");
    
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
    setup.get<JetCorrectionsRecord>().get(jetType, jecParametersCollection); 
    
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
    
    
    // Read rho, which is used in JER smearing
    Handle<double> rho;
    event.getByToken(rhoToken, rho);
    
    
    // Objects that provide jet energy resolution and its scale factors
    JME::JetResolution jerProvider(JME::JetResolution::get(setup, jetType + "_pt"));
    JME::JetResolutionScaleFactor jerSFProvider(JME::JetResolutionScaleFactor::get(setup, jetType));
    
    
    // Loop through the collection and store relevant properties of jets
    storeJets.clear();
    pec::Jet storeJet;  // will reuse this object to fill the vector
    
    for (unsigned int i = 0; i < srcJets->size(); ++i)
    {
        pat::Jet const &j = srcJets->at(i);
        storeJet.Reset();
        
        
        // Check if there is a matching generator-level jet. Use half of the jet radius (i.e. 0.2)
        //as the maximal allowed separation, as done for JER smearing here [1]. By default, PAT
        //performs a looser matching requiring dR < 0.4 (see references in [2]). The flag will be
        //used to perform JER smearing, and it will also be stored in pec::Jet.
        //[1] https://github.com/cms-sw/cmssw/blob/CMSSW_8_0_3/PhysicsTools/PatUtils/python/patPFMETCorrections_cff.py#L132
        //[2] https://github.com/andrey-popov/PEC-tuples/issues/81
        bool matchedGenJetFound = false;
        
        if (j.genJet() and ROOT::Math::VectorUtil::DeltaR2(j.p4(), j.genJet()->p4()) < 0.2 * 0.2)
            matchedGenJetFound = true;
        
        
        // Will check if the current jet satisfies the provided selection on transverse momentum.
        //Jets just below the threshold might pass it as a result of a fluctuation in JEC.
        //Check this possibility
        double jetPtUpFluctuationFactor = 1.;
        double curJECUncertainty = 0.;
        double jerFactorNominal = 1., jerFactorUp = 1., jerFactorDown = 1.;
        
        if (not runOnData)
        {
            // Find JEC uncertainty for the current jet [1]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=124#JetCorUncertainties
            jecUncProvider->setJetEta(j.eta());
            jecUncProvider->setJetPt(j.pt());
            curJECUncertainty = jecUncProvider->getUncertainty(true);
            
            
            // Obtain JER data-to-simulation scale factors
            double const jerSFNominal =
              jerSFProvider.getScaleFactor({{JME::Binning::JetEta, j.eta()}}, Variation::NOMINAL);
            double const jerSFUp =
              jerSFProvider.getScaleFactor({{JME::Binning::JetEta, j.eta()}}, Variation::UP);
            double const jerSFDown =
              jerSFProvider.getScaleFactor({{JME::Binning::JetEta, j.eta()}}, Variation::DOWN);
            
            
            // Compute JER factors to rescale jet momentum. The formulas are taken from [1] and [2]
            //for the case of present and missing generator-level match
            //[1] https://github.com/cms-sw/cmssw/blob/CMSSW_8_0_3/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L237
            //[2] https://github.com/cms-sw/cmssw/blob/CMSSW_8_0_3/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L244
            if (matchedGenJetFound)
            {
                double const energyFactor = (j.energy() - j.genJet()->energy()) / j.energy();
                
                jerFactorNominal = 1. + (jerSFNominal - 1.) * energyFactor;
                jerFactorUp = 1. + (jerSFUp - 1.) * energyFactor;
                jerFactorDown = 1. + (jerSFDown - 1.) * energyFactor;
            }
            else
            {
                double const ptResolution =
                  jerProvider.getResolution({{JME::Binning::JetPt, j.pt()},
                  {JME::Binning::JetEta, j.eta()}, {JME::Binning::Rho, *rho}});
                
                
                // A shift in jet pt is randomly sampled according to the resolution in
                //simulation and then scaled based on the data-to-simulation scale factors. It is
                //important that the sampling is only done once and then reused for the
                //systematical variations. Otherwise the variations would also include an effect
                //due to resampling and not just because of the shift in the scale factor
                double const mcShift = rGen.Gaus(0., ptResolution);
                
                jerFactorNominal = 1. + mcShift *
                  std::sqrt(std::max(std::pow(jerSFNominal, 2) - 1., 0.)) / j.pt();
                jerFactorUp = 1. + mcShift *
                  std::sqrt(std::max(std::pow(jerSFUp, 2) - 1., 0.)) / j.pt();
                jerFactorDown = 1. + mcShift *
                  std::sqrt(std::max(std::pow(jerSFDown, 2) - 1., 0.)) / j.pt();
            }
            
            
            // Update the factor for upwards fluctuation
            jetPtUpFluctuationFactor = std::max({1. + fabs(curJECUncertainty), jerFactorNominal,
              jerFactorUp, jerFactorDown});
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
                storeJet.SetCorrFactor(1. / j.jecFactor("Uncorrected") * jerFactorNominal);
                //^ Here jecFactor("Uncorrected") returns the factor to get raw momentum starting
                //from the corrected one. Since in fact the raw momentum is stored, the factor is
                //inverted
                
                if (not runOnData)
                {
                    storeJet.SetJECUncertainty(curJECUncertainty);
                    
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
                
                storeJet.SetBit(0, (matchedGenJetFound and j.genJet()->pt() > 8.));
                //^ The check of angular distance has already been performed. Tighten the matching
                //condition with a requirement on pt as in JME-13-005
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
