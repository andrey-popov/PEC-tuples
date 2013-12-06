#include <UserCode/SingleTop/plugins/PartonShowerOutcome.h>

#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <algorithm>


// Static const data member
int const PartonShowerOutcome::maxSize;


PartonShowerOutcome::PartonShowerOutcome(edm::ParameterSet const &cfg):
    absPdgIdToSave(cfg.getParameter<std::vector<int>>("absPdgId")),
    genParticlesSrc(cfg.getParameter<edm::InputTag>("genParticles"))
{}


void PartonShowerOutcome::beginJob()
{
    // Create the output tree
    outTree = fileService->make<TTree>("PartonShowerInfo",
     "Properties of selected particles from parton shower");
    
    // Assign branches to the tree
    outTree->Branch("psSize", &bfSize, "psSize/b");
    outTree->Branch("psPdgId", bfPdgId, "psPdgId[psSize]/S");
    outTree->Branch("psOrigin", bfOrigin, "psOrigin[psSize]/b");
    outTree->Branch("psPt", bfPt, "psPt[psSize]/F");
    outTree->Branch("psEta", bfEta, "psEta[psSize]/F");
    outTree->Branch("psPhi", bfPhi, "psPhi[psSize]/F");
}


void PartonShowerOutcome::analyze(edm::Event const &event, edm::EventSetup const &eventSetup)
{
    // Read the generator particles in the current event
    edm::Handle<edm::View<reco::GenParticle>> genParticles;
    event.getByLabel(genParticlesSrc, genParticles);
    
    
    // Loop over the generator particles
    int nParticles = 0;
    
    for (reco::GenParticle const &p: *genParticles)
    {
        // Consider only particles with status 2
        if (p.status() != 2)
            continue;
        
        
        // Filter on PDG ID
        if (std::find(absPdgIdToSave.begin(), absPdgIdToSave.end(), abs(p.pdgId())) ==
         absPdgIdToSave.end())
            continue;
        
        // Keep only particles in the final state of parton shower. Their daughters are either
        //stable particles (status 1) or strings on clusters. The latter is generalised to daugthers
        //with any special PDG ID codes (from 81 to 100)
        bool isPSFinal = false;
        
        for (unsigned i = 0; i < p.numberOfDaughters(); ++i)
        {
            reco::Candidate const *daughter = p.daughter(i);
            
            if (daughter->status() == 1 or (daughter->pdgId() >= 81 and daughter->pdgId() <= 100))
            {
                isPSFinal = true;
                break;
            }
        }
        
        if (not isPSFinal)
            continue;
        
        
        
        // Fill arrays with information about the current particle
        bfPdgId[nParticles] = p.pdgId();
        bfPt[nParticles] = p.pt();
        bfEta[nParticles] = p.eta();
        bfPhi[nParticles] = p.phi();
        
        // Deduce the origin of the particle
        bfOrigin[nParticles] = int(DeduceOrigin(p));
        
        
        // Finally, increase the counter and break the loop if maximal allowed number of particles
        //is reached
        ++nParticles;
        
        if (nParticles == maxSize)
            break;
    }
    
    
    // Specify the total number of particles to be written and fill the output tree
    bfSize = nParticles;
    outTree->Fill();
}


PartonShowerOutcome::ParticleOrigin PartonShowerOutcome::DeduceOrigin(
 reco::Candidate const &particle)
{
    // Check mothers recursively until the first particle with status 3 is found
    reco::Candidate const *p = &particle;
    
    while (p->status() != 3)
    {
        if (p->numberOfMothers() == 0)
        {
            edm::Exception excp(edm::errors::LogicError);
            excp << "A particle has no mothers, which is not expected.";
            excp.raise();
        }
        
        p = p->mother(0);
    }
    
    
    // Now it is possible to classify the particle depending on its origin
    // If the given particle is an immediate daughter of a beam particle, then its first mother with
    //status 3 (the beam particle) has no mothers
    if (p->numberOfMothers() == 0)
        return ParticleOrigin::Proton;
    
    // All ISR has a valence quark as its mother. A valence quark is an immediate daughter of a beam
    //particle and thus has no grand mothers
    if (p->mother(0)->numberOfMothers() == 0)
        return ParticleOrigin::ISR;
    
    // All the rest is an immediate or indirect dauther of two sea partons picked for the initial
    //state of the hard process. It is classified as FSR
    return ParticleOrigin::FSR;
}


DEFINE_FWK_MODULE(PartonShowerOutcome);
