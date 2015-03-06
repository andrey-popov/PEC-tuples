#include "HardInteractionInfo.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Utilities/interface/EDMException.h>

#include <FWCore/Framework/interface/MakerMacros.h>

#include <map>

#define DEBUG


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
    bookedParticlesOverwrittenMothers.clear();
    storeParticles.clear();
    
    
    // Read the generator-level particles
    Handle<View<reco::GenParticle>> genParticles;
    event.getByToken(genParticlesToken, genParticles);
    
    
    #ifdef DEBUG
    cout << "\033[1;34mEvent: " << event.id().run() << ":" << event.id().event() << "\033[0m\n\n";
    #endif
        
    
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
    
    
    #ifdef DEBUG
    cout << "Final state:\n";
    
    for (auto const &p: meFinalState)
        cout << " PDG ID: " << p->pdgId() << ", status: " << p->status() << '\n';
    
    cout << "\nRoots of interesting particles:\n";
    
    for (auto const &p: addPartToSaveRoots)
        cout << " PDG ID: " << p->pdgId() << ", status: " << p->status() << '\n';
    
    cout << endl;
    #endif
    
    
    
    
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
    
    
    #ifdef DEBUG
    cout << "Initial state:\n";
    
    for (auto const &p: bookedParticles)
        cout << " PDG ID: " << p->pdgId() << ", status: " << p->status() << '\n';
    
    cout << endl;
    #endif
    
    
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
            BookParticle(decay->daughter(iDaughter), root);
    }
    
    
    // Set indices of mothers of booked particles
    map<reco::Candidate const *, unsigned> particleToIndex;
    
    for (unsigned iPart = 0; iPart < bookedParticles.size(); ++iPart)
        particleToIndex[bookedParticles.at(iPart)] = iPart;
    
    for (unsigned iPart = 0; iPart < bookedParticles.size(); ++iPart)
    {
        auto mother = bookedParticlesOverwrittenMothers.at(iPart);
        
        if (not mother and bookedParticles.at(iPart)->numberOfMothers() > 0)
            mother = bookedParticles.at(iPart)->mother(0);
        
        
        auto res = particleToIndex.find(mother);
        
        if (res != particleToIndex.end())
            storeParticles.at(iPart).SetFirstMotherIndex(res->second);
        
        
        if (not bookedParticlesOverwrittenMothers.at(iPart) and
         bookedParticles.at(iPart)->numberOfMothers() > 1)
        {
            res = particleToIndex.find(
             bookedParticles.at(iPart)->mother(bookedParticles.at(iPart)->numberOfMothers() - 1));
            
            if (res != particleToIndex.end())
                storeParticles.at(iPart).SetLastMotherIndex(res->second);
        }
    }
    
    
    #ifdef DEBUG
    cout << "All particles that will be stored:\n";
    
    for (unsigned iPart = 0; iPart < storeParticles.size(); ++iPart)
    {
        auto const &p = storeParticles.at(iPart);
        
        cout << " #" << iPart << ": PDG ID: " << p.PdgId() << ", mothers: " <<
         p.FirstMotherIndex() << ", " << p.LastMotherIndex() << endl;
    }
    
    cout << "\n\n";
    #endif
    
    
    // Save the event in the output tree
    outTree->Fill();
}


bool HardInteractionInfo::BookParticle(reco::Candidate const *p,
 reco::Candidate const *overwriteMother /*= nullptr*/)
{
    // Check if the given particle has already been booked for storing
    if (find(bookedParticles.begin(), bookedParticles.end(), p) != bookedParticles.end())
        return false;
    
    bookedParticles.emplace_back(p);
    bookedParticlesOverwrittenMothers.emplace_back(overwriteMother);
    
    pec::GenParticle BookParticle;
    BookParticle.SetPdgId(p->pdgId());
    BookParticle.SetPt(p->pt());
    BookParticle.SetEta(p->eta());
    BookParticle.SetPhi(p->phi());
    BookParticle.SetM(p->mass());
    
    // Mothers are filled later
    
    storeParticles.emplace_back(BookParticle);
    
    return true;
}


DEFINE_FWK_MODULE(HardInteractionInfo);
