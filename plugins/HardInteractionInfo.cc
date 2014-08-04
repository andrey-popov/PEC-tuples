#include "HardInteractionInfo.h"

#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace std;
using namespace edm;


// A static data member
unsigned const HardInteractionInfo::maxSize;


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
    
    outTree->Branch("hardPartSize", &hardPartSize);
    outTree->Branch("hardPartPdgId", hardPartPdgId, "hardPartPdgId[hardPartSize]/B");
    outTree->Branch("hardPartFirstMother", hardPartFirstMother,
     "hardPartFirstMother[hardPartSize]/B");
    outTree->Branch("hardPartLastMother", hardPartLastMother,
     "hardPartLastMother[hardPartSize]/B");
    outTree->Branch("hardPartPt", hardPartPt, "hardPartPt[hardPartSize]/F");
    outTree->Branch("hardPartEta", hardPartEta, "hardPartEta[hardPartSize]/F");
    outTree->Branch("hardPartPhi", hardPartPhi, "hardPartPhi[hardPartSize]/F");
    outTree->Branch("hardPartMass", hardPartMass, "hardPartMass[hardPartSize]/F");
}


void HardInteractionInfo::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Read the generator-level particles
    Handle<View<reco::GenParticle>> genParticles;
    event.getByLabel(genParticlesTag, genParticles);
    
    hardPartSize = 0;
    vector<reco::GenParticle const *> visitedParticles;
    
    for (unsigned i = 2; i < genParticles->size(); ++i)  // skip the protons
    {
        reco::GenParticle const &p = genParticles->at(i);
        
        if (p.status() != 3)  // all the status 3 particles are at the beginning of listing
            break;
        
        int const nMothers = p.numberOfMothers();
        
        if (nMothers > 0)
        {
            auto it = find(visitedParticles.begin(), visitedParticles.end(), p.mother(0));
            hardPartFirstMother[hardPartSize] = (it != visitedParticles.end()) ?
             it - visitedParticles.begin() : -1;
            //^ The indices correspond to the position in the stored arrays, not genParticles
            
            if (nMothers > 1)
            {
                it = find(visitedParticles.begin(), visitedParticles.end(),
                 p.mother(nMothers - 1));
                hardPartLastMother[hardPartSize] = (it != visitedParticles.end()) ?
                 it - visitedParticles.begin() : -1;
            }
            else
                hardPartLastMother[hardPartSize] = hardPartFirstMother[hardPartSize];
            
        }
        else
            hardPartFirstMother[hardPartSize] = hardPartLastMother[hardPartSize] = -1;
        
        hardPartPdgId[hardPartSize] = p.pdgId();
        hardPartPt[hardPartSize] = p.pt();
        hardPartEta[hardPartSize] = p.eta();
        hardPartPhi[hardPartSize] = p.phi();
        hardPartMass[hardPartSize] = p.mass();
        
        
        ++hardPartSize;
        visitedParticles.push_back(&genParticles->at(i));
    }
    
    
    // Save the event in the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(HardInteractionInfo);
