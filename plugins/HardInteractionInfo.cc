#include "HardInteractionInfo.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Utilities/interface/EDMException.h>

#include <FWCore/Framework/interface/MakerMacros.h>

#include <map>

#define DEBUG


using namespace std;
using namespace edm;


HardInteractionInfo::ParticleWithMother::ParticleWithMother():
    particle(nullptr),
    mother(nullptr)
{}


HardInteractionInfo::ParticleWithMother::ParticleWithMother(reco::Candidate const *particle_,
 reco::Candidate const *mother_ /*= nullptr*/):
    particle(particle_),
    mother(mother_)
{}


reco::Candidate const *HardInteractionInfo::ParticleWithMother::operator->() const
{
    return particle;
}


bool HardInteractionInfo::ParticleWithMother::operator==(reco::Candidate const *rhs) const
{
    return (particle == rhs);
}


reco::Candidate const *HardInteractionInfo::ParticleWithMother::Get() const
{
    return particle;
}


void HardInteractionInfo::ParticleWithMother::ResetMother(reco::Candidate const *mother_)
{
    mother = mother_;
}


unsigned HardInteractionInfo::ParticleWithMother::NumberOfMothers() const
{
    if (mother)
        return 1;
    else
        return particle->numberOfMothers();
}


reco::Candidate const *HardInteractionInfo::ParticleWithMother::Mother(int index) const
{
    if (mother)
    {
        // Return the overriding mother
        return (index == 0 or index == -1) ? mother : nullptr;
    }
    else
    {
        int const nMothers = particle->numberOfMothers();
        
        
        // Check the range of the index
        if (index >= nMothers or index < -nMothers)
            return nullptr;
        
        
        // Reinterpret a negative index
        if (index < 0)
            index += nMothers;
        
        return particle->mother(index);
    }
}


HardInteractionInfo::HardInteractionInfo(ParameterSet const &cfg):
    desiredExtraPartIds({6, 23, 24, 25})  // hard-coded for the time being
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
    #ifdef DEBUG
    cout << "\033[1;34mEvent: " << event.id().run() << ":" << event.id().event() << "\033[0m\n\n";
    #endif
        
    
    // Clear vectors of particles to be stored
    bookedParticles.clear();
    storeParticles.clear();
    
    
    // Read the generator-level particles
    Handle<View<reco::GenParticle>> genParticles;
    event.getByToken(genParticlesToken, genParticles);
    
    
    // Loop over the source collection of GEN-level particles and identify particles from the final
    //state of the hard(est) interaction and oldest versions of desired additional particles. Both
    //groups of pointers are stored in sets, so there is no need to care about double counting
    set<reco::Candidate const *> meFinalState, extraPartRoots;
    
    for (reco::GenParticle const &p: *genParticles)
    {
        int const absPdgId = abs(p.pdgId());
        
        
        // Skip hadrons and artificial objects like string or clusters
        if (absPdgId > 80)
            continue;
        //^ This also rejects exotics like SUSY, technicolour, etc., but they are not used in the
        //analysis
        
        
        // Skip particles after hadronisation. The protection is mostly needed to exclude W from
        //tau decays in Pythia 6
        if (p.status() <= 2)
            continue;
        
        
        // Check if the particle is from the final state of the hard(est) interaction. It is
        //necessary that such particle has exactly two mothers (the initial state)
        if (p.numberOfMothers() == 2)
        {
            // In case of Pythia 8, need to check also that the status indicates that the particle
            //stems from the hardest subprocess (because particles with code 71 also might have two
            //mothers) [1]. For Pythia 6 the status should be 3. Both conditions can be combined as
            //done below
            //[1] http://home.thep.lu.se/~torbjorn/pythia81html/ParticleProperties.html
            if (p.status() == 3 or (p.status() > 20 and p.status() < 30))
                meFinalState.emplace(&p);
        }
        
        
        // Check if the current particle is of type the user wants to store
        if (desiredExtraPartIds.count(absPdgId) > 0)
        {
            // Find the oldest ancestor of the same type (in Pythia 8 same particle might enter the
            //history many times)
            reco::Candidate const *root = &p;
            
            while (root->numberOfMothers() > 0 and root->mother(0)->pdgId() == root->pdgId())
                root = root->mother(0);
            
            
            extraPartRoots.emplace(root);
        }
    }
    
    
    #ifdef DEBUG
    cout << "Final state:\n";
    
    for (auto const &p: meFinalState)
        cout << " PDG ID: " << p->pdgId() << ", status: " << p->status() << '\n';
    
    cout << "\nRoots of interesting particles:\n";
    
    for (auto const &p: extraPartRoots)
        cout << " PDG ID: " << p->pdgId() << ", status: " << p->status() << '\n';
    
    cout << endl;
    #endif
    
    
    // Identify particles from the initial state. They are mothers of the final state
    for (auto const &pMEFinal: meFinalState)
    {
        for (unsigned iMother = 0; iMother < pMEFinal->numberOfMothers(); ++iMother)
        {
            auto const *mother = pMEFinal->mother(iMother);
            
            // In miniAOD with Pythia 8 gluons from the initial state are not stored. As a result,
            //one or both of the incoming protons are set as mothers of the final state. Do not
            //store them
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
    
    
    // Book particles from the final state
    for (auto const &p: meFinalState)
        BookParticle(p);
    
    
    // Book additional particles requested by the user
    for (auto const &root: extraPartRoots)
    {
        // The oldest ancestor (the "root") found before
        BookParticle(root);
        
        
        // Move along descendants of the root until the youngest descendant of the same type is
        //found. It then decays to other particles
        reco::Candidate const *decay = root;
        
        while (true)
        {
            reco::Candidate const *daughterSamePdgId = nullptr;
            
            for (unsigned iDaughter = 0; iDaughter < decay->numberOfDaughters(); ++iDaughter)
                if (decay->daughter(iDaughter)->pdgId() == decay->pdgId() and
                 decay->daughter(iDaughter)->status() > 2)
                //^ The second part of the condition is needed for Pythia 6. It has decays like
                //W[3] -> e[3] v[3] W[2], W[2] -> W[2], W[2] -> nothing, where the number in
                //brackets is the status
                {
                    daughterSamePdgId = decay->daughter(iDaughter);
                    break;
                }
            
            if (not daughterSamePdgId)
                break;
            else
                decay = daughterSamePdgId;
        }
        
        
        // Book decay products of the youngest descendant
        for (unsigned iDaughter = 0; iDaughter < decay->numberOfDaughters(); ++iDaughter)
        {
            reco::Candidate const *d = decay->daughter(iDaughter);
            
            
            // Skip hadrons (seen this happening in Pythia 6) and artificial objects
            if (abs(d->pdgId()) > 80)
                continue;
            
            
            BookParticle(d, root);
        }
    }
    
    
    // Put all booked particles into the storage vector
    for (auto const &p: bookedParticles)
    {
        pec::GenParticle storeParticle;
        
        // Fill PDG ID and four-momentum. Indices of mothers will be set later
        storeParticle.SetPdgId(p->pdgId());
        storeParticle.SetPt(p->pt());
        storeParticle.SetEta(p->eta());
        storeParticle.SetPhi(p->phi());
        storeParticle.SetM(p->mass());
        
        
        // Add the new particle to the storage vector
        storeParticles.emplace_back(storeParticle);
    }
    
    
    // Set mothers of particles in the storage vector. The mothers are identified with their indices
    //in the vector. In order to be able to find these indices efficiently, build a mapping from
    //particle pointers in bookedParticles to their indices in the same vector
    map<reco::Candidate const *, unsigned> particleToIndex;
    
    for (unsigned iPart = 0; iPart < bookedParticles.size(); ++iPart)
        particleToIndex[bookedParticles.at(iPart).Get()] = iPart;
    
    
    // Use the map to set up indices of mothers. Note that particles in vectors bookedParticles and
    //storeParticles are ordered identically
    for (unsigned iPart = 0; iPart < bookedParticles.size(); ++iPart)
    {
        auto const &p = bookedParticles.at(iPart);
        
        // Set first mother (if any)
        if (p.NumberOfMothers() > 0)
        {
            auto res = particleToIndex.find(p.Mother(0));
            
            if (res != particleToIndex.end())
                storeParticles.at(iPart).SetFirstMotherIndex(res->second);
        }
        
        // Set last mother (if more than one)
        if (p.NumberOfMothers() > 1)
        {
            auto res = particleToIndex.find(p.Mother(-1));
            
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
    
    
    // Everything is done. Save the event in the output tree
    outTree->Fill();
}


bool HardInteractionInfo::BookParticle(reco::Candidate const *p,
 reco::Candidate const *mother /*= nullptr*/)
{
    // Check if the given particle has already been booked for storing
    auto res = find(bookedParticles.begin(), bookedParticles.end(), p);
    
    if (res != bookedParticles.end())
    //^ The particle is already known
    {
        // Update the mother. It is needed because same particle could be booked twice: as a root
        //and as a decay product (consider a W from decay of a top quark, when the user requests to
        //store both t and W). In this case the particle should be stored with the mother specified
        //when the particle was booked as a decay product, not the real mother
        if (mother)
            res->ResetMother(mother);
        
        return false;
    }
    else
    {
        bookedParticles.emplace_back(p, mother);
        return true;
    }
}


DEFINE_FWK_MODULE(HardInteractionInfo);
