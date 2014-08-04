#include "HardInteractionInfo.h"

#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace std;
using namespace edm;


HardInteractionInfo::HardInteractionInfo(ParameterSet const &cfg):
    genParticlesTag(cfg.getParameter<InputTag>("genParticles"))
{}


void HardInteractionInfo::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("genParticles", InputTag("genParticles"))->
     setComment("Tag to access generator particles.");
    
    descriptions.add("hardInteraction", desc);
}


void HardInteractionInfo::beginJob()
{
    outTree = fileService->make<TTree>("HardInteraction",
     "Tree contrains generator-level particles from the hard interaction");
    
    storeParticlesPointer = &storeParticles;
    outTree->Branch("particles", &storeParticlesPointer);
}


void HardInteractionInfo::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Read the generator-level particles
    Handle<View<reco::GenParticle>> genParticles;
    event.getByLabel(genParticlesTag, genParticles);
    
    
    storeParticles.clear();
    pec::GenParticle storeParticle;  // will reuse same object to fill the vector
    
    // Will keep trace of already visited particles to search for mothers in there
    vector<reco::GenParticle const *> visitedParticles;
    
    for (unsigned i = 2; i < genParticles->size(); ++i)
    //^ The beam particles are skipped, which is why the index starts from 2
    {
        reco::GenParticle const &p = genParticles->at(i);
        storeParticle.Reset();  // reset the object as it is reused from the previous iteration
        
        if (p.status() != 3)  // all the status 3 particles are at the beginning of the listing
            break;
        
        
        unsigned const nMothers = p.numberOfMothers();
        
        if (nMothers > 0)
        {
            auto it = find(visitedParticles.begin(), visitedParticles.end(), p.mother(0));
            
            if (it != visitedParticles.end())
                storeParticle.SetFirstMotherIndex(it - visitedParticles.begin());
            
            if (nMothers > 1)
            {
                it = find(visitedParticles.begin(), visitedParticles.end(), p.mother(nMothers - 1));
                
                if (it != visitedParticles.end())
                    storeParticle.SetLastMotherIndex(it - visitedParticles.begin());
            }
            
        }
        
        storeParticle.SetPdgId(p.pdgId());
        storeParticle.SetPt(p.pt());
        storeParticle.SetEta(p.eta());
        storeParticle.SetPhi(p.phi());
        storeParticle.SetM(p.mass());
        
        
        visitedParticles.push_back(&genParticles->at(i));
        storeParticles.push_back(storeParticle);
    }
    
    
    // Save the event in the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(HardInteractionInfo);
