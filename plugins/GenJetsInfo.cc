#include <UserCode/SingleTop/plugins/GenJetsInfo.h>

#include <DataFormats/Candidate/interface/Candidate.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <Math/GenVector/VectorUtil.h>


// Define the static member
unsigned const GenJetsInfo::maxSize;


GenJetsInfo::GenJetsInfo(edm::ParameterSet const &cfg):
    jetSrc(cfg.getParameter<edm::InputTag>("jets")),
    jetCut(cfg.getParameter<std::string>("cut")),
    genParticleSrc(cfg.getParameter<edm::InputTag>("genParticles")),
    readGenParticles(genParticleSrc.label().length() > 0)
{}


void GenJetsInfo::beginJob()
{
    // Create the output tree
    tree = fs->make<TTree>("GenJets", "Properties of generator-level jets");
    
    // Assign branches of the output tree
    tree->Branch("jetSize", &jetSize);
    
    tree->Branch("jetPt", jetPt, "jetPt[jetSize]/F");
    tree->Branch("jetEta", jetEta, "jetEta[jetSize]/F");
    tree->Branch("jetPhi", jetPhi, "jetPhi[jetSize]/F");
    tree->Branch("jetMass", jetMass, "jetMass[jetSize]/F");
    
    tree->Branch("bMult", bMult, "bMult[jetSize]/I");
    tree->Branch("cMult", cMult, "cMult[jetSize]/I");
}


void GenJetsInfo::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Read the collection of generator-level jets
    edm::Handle<edm::View<reco::Candidate>> jets;
    event.getByLabel(jetSrc, jets);
    
    // Read the collection of generator-level particles
    edm::Handle<edm::View<reco::GenParticle>> particles;
    
    if (readGenParticles)
        event.getByLabel(genParticleSrc, particles);
    
    
    // Construct the jet selector
    StringCutObjectSelector<reco::Candidate> jetSelector(jetCut);
    
    
    // Loop over the jets
    jetSize = 0;
    
    for (unsigned i = 0; i < jets->size() and jetSize < int(maxSize); ++i)
    {
        auto const &j = jets->at(i);
        
        if (jetSelector(j))
        {
            // Save the jet four-momentum
            jetPt[jetSize] = j.pt();
            jetEta[jetSize] = j.eta();
            jetPhi[jetSize] = j.phi();
            jetMass[jetSize] = j.mass();
            
        
            // Loop over the generator-level particles
            if (readGenParticles)
            {
                bMult[jetSize] = cMult[jetSize] = 0;
                
                for (unsigned iParticle = 0; iParticle < particles->size(); ++iParticle)
                {
                    auto const &p = particles->at(iParticle);
                    
                    if (p.status() != 2)
                        continue;
                    
                    if (ROOT::Math::VectorUtil::DeltaR(j.p4(), p.p4()) > 0.5)
                        continue;
                    
                    int const absPdgId = abs(p.pdgId());
                    
                    if (absPdgId == 5)
                        ++bMult[jetSize];
                    else if (absPdgId == 4)
                        ++cMult[jetSize];
                }
            }
            
            
            // Update the jet counter
            ++jetSize;
        }
    }
    
    
    tree->Fill();
}


DEFINE_FWK_MODULE(GenJetsInfo);