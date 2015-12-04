#include "PECMuons.h"

#include <DataFormats/VertexReco/interface/Vertex.h>

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;
using namespace std;


PECMuons::PECMuons(ParameterSet const &cfg)
{
    // Register required input data
    muonToken = consumes<View<pat::Muon>>(cfg.getParameter<InputTag>("src"));
    primaryVerticesToken =
     consumes<reco::VertexCollection>(cfg.getParameter<InputTag>("primaryVertices"));
    
    
    // Construct string-based selectors
    for (string const &selection: cfg.getParameter<vector<string>>("selection"))
        muSelectors.emplace_back(selection);
}


void PECMuons::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("src")->setComment("Source collection of muons.");
    desc.add<vector<string>>("selection", vector<string>(0))->
     setComment("User-defined selections for muons whose results will be stored in the ouput "
     "tree.");
    desc.add<InputTag>("primaryVertices")->
     setComment("Collection of reconstructed primary vertices.");
    
    descriptions.add("eventContent", desc);
}


void PECMuons::beginJob()
{
    outTree = fileService->make<TTree>("Muons", "Properties of selected muons");
    
    storeMuonsPointer = &storeMuons;
    outTree->Branch("muons", &storeMuonsPointer);
}


void PECMuons::analyze(Event const &event, EventSetup const &)
{
    // First read primary vertices
    Handle<reco::VertexCollection> vertices;
    event.getByToken(primaryVerticesToken, vertices);
    
    if (vertices->size() == 0)
    {
        Exception excp(errors::LogicError);
        excp << "Event contains zero good primary vertices.\n";
        excp.raise();
    }
    
    
    // Read the muon collection
    Handle<View<pat::Muon>> srcMuons;
    event.getByToken(muonToken, srcMuons);
    
    
    // Loop through the collection and store relevant properties of muons
    storeMuons.clear();
    pec::Muon storeMuon;  // will reuse this object to fill the vector
    
    for (unsigned i = 0; i < srcMuons->size(); ++i)
    {
        pat::Muon const &mu = srcMuons->at(i);
        storeMuon.Reset();
        
        
        // Set four-momentum. Mass is ignored
        storeMuon.SetPt(mu.pt());
        storeMuon.SetEta(mu.eta());
        storeMuon.SetPhi(mu.phi());
        
        storeMuon.SetCharge(mu.charge());
        storeMuon.SetDB(mu.dB());
        
        // Relative isolation with delta-beta correction
        storeMuon.SetRelIso((mu.chargedHadronIso() + max(mu.neutralHadronIso() + mu.photonIso() -
         0.5 * mu.puChargedHadronIso(), 0.)) / mu.pt());
        
        // Tight muons are defined according to [1]. Note it does not imply selection on isolation
        //or kinematics
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2?rev=22#Tight_Muon
        storeMuon.SetBit(0, mu.isTightMuon(vertices->front()));
        
        // Evaluate user-defined selectors if any
        for (unsigned i = 0; i < muSelectors.size(); ++i)
            storeMuon.SetBit(1 + i, muSelectors[i](mu));
        
        
        // The muon is set up. Add it to the vector
        storeMuons.emplace_back(storeMuon);
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECMuons);
