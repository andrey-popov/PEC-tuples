#include <UserCode/SingleTop/plugins/GenJetsInfo.h>

#include <DataFormats/Candidate/interface/Candidate.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <DataFormats/JetReco/interface/GenJet.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <Math/GenVector/VectorUtil.h>


using namespace std;
using namespace edm;


// Define the static member
unsigned const GenJetsInfo::maxSize;


GenJetsInfo::GenJetsInfo(edm::ParameterSet const &cfg):
    jetSrc(cfg.getParameter<InputTag>("jets")),
    jetCut(cfg.getParameter<string>("cut")),
    saveFlavourCounters(cfg.getParameter<bool>("saveFlavourCounters"))
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
    
    if (saveFlavourCounters)
        tree->Branch("bcMult", bcMult, "bcMult[jetSize]/b");
}


void GenJetsInfo::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Read the collection of generator-level jets
    Handle<View<reco::GenJet>> jets;
    event.getByLabel(jetSrc, jets);
    
    
    // Construct the jet selector
    StringCutObjectSelector<reco::Candidate> jetSelector(jetCut);
    
    
    // Collections of already encountered hadrons with b and c quarks. Needed to prevent double
    //counting
    vector<reco::Candidate const *> bHadFound, cHadFound;
    
    
    // Loop over the jets
    jetSize = 0;
    
    for (unsigned i = 0; i < jets->size() and jetSize < maxSize; ++i)
    {
        auto const &j = jets->at(i);
        
        if (jetSelector(j))
        {
            // Save the jet four-momentum
            jetPt[jetSize] = j.pt();
            jetEta[jetSize] = j.eta();
            jetPhi[jetSize] = j.phi();
            jetMass[jetSize] = j.mass();
            
        
            // Count hadrons with b and c quarks inside the jet
            if (saveFlavourCounters)
            {
                bHadFound.clear();
                cHadFound.clear();
                
                
                // Loop over the constituents
                for (unsigned iConst = 0; iConst < j.getGenConstituents().size(); ++iConst)
                {
                    // Jets are clustered from stable particles. Find the first hadron ancestor,
                    //which was created in the hadronisation
                    reco::Candidate const *p = j.getGenConstituent(iConst);
                    
                    while ((p->mother(0)->pdgId() > 100 or p->mother(0)->pdgId() < 81) and
                     p->mother(0)->status() <= 2)
                    //^ PDG ID 81-100 are reserved for MC internals. E.g. 92 is a string in Pythia
                        p = p->mother(0);
                    
                    
                    // Check the type of the particle as in AN-2012/251
                    if ((abs(p->pdgId()) / 100) % 10 == 5 or (abs(p->pdgId()) / 1000) % 10 == 5)
                    {
                        // It is a hadron with b quark. Make sure it is a new one
                        if (find(bHadFound.begin(), bHadFound.end(), p) == bHadFound.end())
                            bHadFound.push_back(p);
                    }
                    
                    if ((abs(p->pdgId()) / 100) % 10 == 4 or (abs(p->pdgId()) / 1000) % 10 == 4)
                        if (find(cHadFound.begin(), cHadFound.end(), p) == cHadFound.end())
                            cHadFound.push_back(p);
                }
                
                
                bcMult[jetSize] = min<int>(bHadFound.size(), 15) * 16 +
                 min<int>(cHadFound.size(), 15);
            }
            
            
            // Update the jet counter
            ++jetSize;
        }
    }
    
    
    tree->Fill();
}


void GenJetsInfo::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    // Documentation for descriptions of the configuration is available in [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideConfigurationValidationAndHelp
    
    edm::ParameterSetDescription desc;
    desc.add<InputTag>("jets", InputTag("ak5GenJets"))->
     setComment("Collection of generator-level jets.");
    desc.add<string>("cut", "")->
     setComment("Selection to choose which jets should be stored.");
    desc.add<bool>("saveFlavourCounters", false)->
     setComment("Indicates if information on flavours of nearby partons should be stored.");
    
    descriptions.add("genJets", desc);
}


DEFINE_FWK_MODULE(GenJetsInfo);