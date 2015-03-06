#include "HardInteractionInfo.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Utilities/interface/EDMException.h>

#include <FWCore/Framework/interface/MakerMacros.h>


using namespace std;
using namespace edm;


HardInteractionInfo::HardInteractionInfo(ParameterSet const &cfg):
    generator(Generator::Pythia8),  // hard-coded for the time being
    addPartToSave({6, 23, 24, 25})  // hard-coded for the time being
{
    genParticlesToken =
     consumes<View<reco::GenParticle>>(cfg.getParameter<InputTag>("genParticles"));
}


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
    // Clear vectors of particles to be stored
    bookedParticles.clear();
    storeParticles.clear();
    
    
    // Read the generator-level particles
    Handle<View<reco::GenParticle>> genParticles;
    event.getByToken(genParticlesToken, genParticles);
    
    
    // Final state of the matrix element. These are particles that have two mothers and are not
    //hadrons
    set<reco::Candidate const *> meFinalState;
    
    set<reco::Candidate const *> addPartToSaveRoots;
    
    
    for (reco::GenParticle const &p: *genParticles)
    {
        int const absPdgId = abs(p.pdgId());
        
        
        // Skip hadrons and PDG ID reserved for MC internals (like strings)
        if (absPdgId > 80)
            continue;
        //^ This also rejects exotics like SUSY, technicolour, etc., but they are not used in the
        //analysis
        
        
        // Check if the particle is from the final state of the hard(est) interaction. It is
        //necessary that such particle has two mothers (the initial state)
        if (p.numberOfMothers() == 2)
        {
            // In case of Pythia 8, need to check also that the status indicates that the particle
            //stems from the hardest subprocess (because particles with code 71 also might have two
            //mothers). See [1] about the status codes
            //[1] http://home.thep.lu.se/~torbjorn/pythia81html/ParticleProperties.html
            if (generator == Generator::Pythia6 /* (no additional requirements for Pythia 6) */ or
             (generator == Generator::Pythia8 and p.status() >= 21 and p.status() <= 29))
                meFinalState.emplace(&p);
        }
        
        
        if (addPartToSave.count(absPdgId) > 0)
        {
            reco::Candidate const *root = &p;
            
            while (root->numberOfMothers() > 0 and root->mother(0)->pdgId() == root->pdgId())
                root = root->mother(0);
            
            addPartToSaveRoots.emplace(root);
        }
    }
        
    
    // Fill the initial state
    for (auto const &pMEFinal: meFinalState)
    {
        for (unsigned iMother = 0; iMother < pMEFinal->numberOfMothers(); ++iMother)
        {
            auto const *mother = pMEFinal->mother(iMother);
            
            // Do not save the initial protons
            if (abs(mother->pdgId()) == 2212)
                continue;
            
            BookParticle(mother);
        }
    }
    
    
    for (auto const &p: meFinalState)
        BookParticle(p);
    
    
    for (auto const &root: addPartToSaveRoots)
    {
        BookParticle(root);
        
        reco::Candidate const *decay = root;
        
        while (true)
        {
            reco::Candidate const *daughterSamePdgId = nullptr;
            
            for (unsigned iDaughter = 0; iDaughter < decay->numberOfDaughters(); ++iDaughter)
                if (decay->daughter(iDaughter)->pdgId() == decay->pdgId())
                {
                    daughterSamePdgId = decay->daughter(iDaughter);
                    break;
                }
            
            if (not daughterSamePdgId)
                break;
            else
                decay = daughterSamePdgId;
        }
        
        for (unsigned iDaughter = 0; iDaughter < decay->numberOfDaughters(); ++iDaughter)
        {
            auto const *d = decay->daughter(iDaughter);
            BookParticle(d);
            
            //TODO: Need to specify root as mother here
        }
    }
    
    
    //TODO: Fill mothers
    
    
    // Save the event in the output tree
    outTree->Fill();
}


bool HardInteractionInfo::BookParticle(reco::Candidate const *p)
{
    // Check if the given particle has already been booked for storing
    if (find(bookedParticles.begin(), bookedParticles.end(), p) != bookedParticles.end())
        return false;
    
    bookedParticles.emplace_back(p);
    
    pec::GenParticle BookParticle;
    BookParticle.SetPdgId(p->pdgId());
    BookParticle.SetPt(p->pt());
    BookParticle.SetEta(p->eta());
    BookParticle.SetPhi(p->phi());
    BookParticle.SetM(p->mass());
    
    //TODO: Mothers?
    
    storeParticles.emplace_back(BookParticle);
    
    return true;
}


DEFINE_FWK_MODULE(HardInteractionInfo);
