#include "GenJetsInfo.h"

#include <DataFormats/Candidate/interface/Candidate.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <DataFormats/PatCandidates/interface/PackedGenParticle.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <FWCore/Framework/interface/MakerMacros.h>

#include <Math/GenVector/VectorUtil.h>


using namespace std;
using namespace edm;


GenJetsInfo::GenJetsInfo(edm::ParameterSet const &cfg):
    jetSelector(cfg.getParameter<string>("cut")),
    saveFlavourCounters(cfg.getParameter<bool>("saveFlavourCounters")),
    noDoubleCounting(cfg.getParameter<bool>("noDoubleCounting"))
{
    jetToken = consumes<View<reco::GenJet>>(cfg.getParameter<InputTag>("jets"));
}


void GenJetsInfo::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    // Documentation for descriptions of the configuration is available in [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideConfigurationValidationAndHelp
    
    edm::ParameterSetDescription desc;
    desc.add<InputTag>("jets", InputTag("slimmedGenJets"))->
     setComment("Collection of generator-level jets.");
    desc.add<string>("cut", "")->
     setComment("Selection to choose which jets should be stored.");
    desc.add<bool>("saveFlavourCounters", false)->
     setComment("Indicates if information on flavours of nearby partons should be stored.");
    desc.add<bool>("noDoubleCounting", true)->
     setComment("Indicates if same heavy-flavour hadron can be counted in several jets.");
    
    descriptions.add("genJets", desc);
}


void GenJetsInfo::beginJob()
{
    // Create the output tree
    tree = fs->make<TTree>("GenJets", "Properties of generator-level jets");
    
    storeJetsPointer = &storeJets;
    tree->Branch("jets", &storeJetsPointer);
}


void GenJetsInfo::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Read the collection of generator-level jets
    Handle<View<reco::GenJet>> jets;
    event.getByToken(jetToken, jets);
    
    
    // Collections of already encountered hadrons with b and c quarks. They are needed to prevent
    //accounting for same hadrons twice. The collections are global for all jets in the event;
    //therefore if a parton has been counted in a jet, it normally cannot be counted again even in a
    //different jet. Since jets are ordered in pt, harder jets have priority in getting the hadrons
    //assigned. However, if the noDoubleCounting flag is set to false, the collections are reset for
    //every jet, which turns the cleaning local, per-jet only
    vector<reco::Candidate const *> bHadFound, cHadFound;
    
    
    // Loop over the jets
    storeJets.clear();
    pec::GenJet storeJet;  // will reuse same object to fill the vector
    
    for (unsigned i = 0; i < jets->size(); ++i)
    {
        auto const &j = jets->at(i);
        storeJet.Reset();
        
        if (jetSelector(j))
        {
            // Save the jet four-momentum
            storeJet.SetPt(j.pt());
            storeJet.SetEta(j.eta());
            storeJet.SetPhi(j.phi());
            storeJet.SetM(j.mass());
            
        
            // Count hadrons with b and c quarks inside the jet
            if (saveFlavourCounters)
            {
                // Counters for hadrons with b and c quarks in the current jet
                unsigned bMult = 0, cMult = 0;
                
                
                // If there is no need to check for double counting with other jets, reset the
                //collections of encountered heavy-flavour hadrons. The cleaning will then be
                //restricted to the current jet only
                if (not noDoubleCounting)
                {
                    bHadFound.clear();
                    cHadFound.clear();
                }
                
                
                // Loop over constituents of the jet
                for (unsigned iConst = 0; iConst < j.numberOfSourceCandidatePtrs(); ++iConst)
                {
                    edm::Ptr<reco::Candidate> const &constituent = j.sourceCandidatePtr(iConst);
                    
                    // Not every status-1 GEN particle is saved in miniAOD and thus some of
                    //constituents might be missing. Skip such particles
                    if (constituent.isNull() or not constituent.isAvailable())
                        continue;
                    
                    
                    // The jet constituent is a stable particle. Check its parents among pruned GEN
                    //particles
                    
                    // Get the first mother. It is treated specially because it is of a different
                    //type (reco::GenParticle) than the constituent. Though it is downcasted to
                    //reco::Candidate for simplicity
                    reco::Candidate const *p =
                     dynamic_cast<pat::PackedGenParticle const *>(constituent.get())->mother(0);
                    
                    if (not p)
                        continue;
                    
                    
                    // To calculate the b/c-hadron multiplicities, we are only interested in
                    //hadron ancestors. They have status 2 and abs(pdgId) > 100. If this condition
                    //is not met, skip to the next jet constituent. A typical situation with miniAOD
                    //when it happens, is when a consituent is declared an immediate daughter of an
                    //initial proton
                    if (p->status() > 2 or abs(p->pdgId()) <= 100)
                        continue;
                    
                    
                    // Follow the ancestors until the oldest hadron is reached. On the stopping
                    //condition see the comment above
                    while (p->mother(0) and abs(p->mother(0)->pdgId()) > 100 and
                     p->mother(0)->status() <= 2)
                        p = p->mother(0);
                    
                    
                    // Check the type of the particle as in AN-2012/251
                    int const absPdgId = abs(p->pdgId());
                    
                    if ((absPdgId / 100) % 10 == 5 or (absPdgId / 1000) % 10 == 5)
                    {
                        // It is a hadron with b quark. Make sure it is a new one
                        if (find(bHadFound.begin(), bHadFound.end(), p) == bHadFound.end())
                        {
                            ++bMult;
                            bHadFound.emplace_back(p);
                        }
                    }
                    
                    if ((absPdgId / 100) % 10 == 4 or (absPdgId / 1000) % 10 == 4)
                        if (find(cHadFound.begin(), cHadFound.end(), p) == cHadFound.end())
                        {
                            ++cMult;
                            cHadFound.emplace_back(p);
                        }
                }
                
                
                storeJet.SetBottomMult(bMult);
                storeJet.SetCharmMult(cMult);
            }
            
            
            // Add the jet to the vector
            storeJets.emplace_back(storeJet);
        }
    }
    
    
    tree->Fill();
}


DEFINE_FWK_MODULE(GenJetsInfo);
