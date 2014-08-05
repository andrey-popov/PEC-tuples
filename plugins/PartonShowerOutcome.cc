#include "PartonShowerOutcome.h"

#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <algorithm>


PartonShowerOutcome::PartonShowerOutcome(edm::ParameterSet const &cfg):
    absPdgIdToSave(cfg.getParameter<std::vector<int>>("absPdgId")),
    genParticlesSrc(cfg.getParameter<edm::InputTag>("genParticles"))
{}


void PartonShowerOutcome::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    // Documentation for descriptions of the configuration is available in [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideConfigurationValidationAndHelp
    
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("genParticles", edm::InputTag("genParticles"))->
     setComment("Name of collection of generator particles.");
    desc.add<std::vector<int>>("absPdgId", std::vector<int>{4, 5})->
     setComment("Absolute values of PDG ID of particles to be stored.");
    
    descriptions.add("heavyFlavours", desc);
}


void PartonShowerOutcome::beginJob()
{
    // Create the output tree
    outTree = fileService->make<TTree>("PartonShowerInfo",
     "Properties of selected particles from parton shower");
    
    storePartonsPointer = &storePartons;
    outTree->Branch("partons", &storePartonsPointer);
}


void PartonShowerOutcome::analyze(edm::Event const &event, edm::EventSetup const &eventSetup)
{
    // Read the generator particles in the current event
    edm::Handle<edm::View<reco::GenParticle>> genParticles;
    event.getByLabel(genParticlesSrc, genParticles);
    
    
    // Loop over the generator particles
    storePartons.clear();
    pec::ShowerParton storeParton;  // same object will be reused to fill the vector
    
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
        
        
        // An appropriate parton has been found. Save its properties
        storeParton.Reset();
        
        storeParton.SetPt(p.pt());
        storeParton.SetEta(p.eta());
        storeParton.SetPhi(p.phi());
        
        storeParton.SetPdgId(p.pdgId());
        storeParton.SetOrigin(DeduceOrigin(p));
        
        
        // The particle has been set up. Push it into the vector
        storePartons.push_back(storeParton);
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


pec::ShowerParton::Origin PartonShowerOutcome::DeduceOrigin(reco::Candidate const &particle)
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
        return pec::ShowerParton::Origin::Proton;
    
    // All ISR has a valence quark as its mother. A valence quark is an immediate daughter of a beam
    //particle and thus has no grand mothers
    if (p->mother(0)->numberOfMothers() == 0)
        return pec::ShowerParton::Origin::ISR;
    
    // All the rest is an immediate or indirect dauther of two sea partons picked for the initial
    //state of the hard process. It is classified as FSR
    return pec::ShowerParton::Origin::FSR;
}


DEFINE_FWK_MODULE(PartonShowerOutcome);
