#include "PECPileUp.h"

#include <DataFormats/VertexReco/interface/Vertex.h>

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;
using namespace std;


PECPileUp::PECPileUp(ParameterSet const &cfg):
    runOnData(cfg.getParameter<bool>("runOnData"))
{
    // Register required input data
    primaryVerticesToken =
     consumes<reco::VertexCollection>(cfg.getParameter<InputTag>("primaryVertices"));
    puSummaryToken = consumes<View<PileupSummaryInfo>>(cfg.getParameter<InputTag>("puInfo"));
    rhoToken = consumes<double>(cfg.getParameter<InputTag>("rho"));
    rhoCentralToken = consumes<double>(cfg.getParameter<InputTag>("rhoCentral"));
}


void PECPileUp::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("primaryVertices")->
     setComment("Collection of reconstructed primary vertices.");
    desc.add<InputTag>("rho", InputTag("fixedGridRhoFastjetAll"))->
     setComment("Rho (mean angular pt density).");
    desc.add<InputTag>("rhoCentral", InputTag("fixedGridRhoFastjetCentral"))->
     setComment("Rho in the central region.");
    desc.add<bool>("runOnData")->
     setComment("Indicates whether data or simulation is being processed.");
    desc.add<InputTag>("puInfo", InputTag("addPileupInfo"))->
     setComment("Pile-up information as simulated. If runOnData is true, this parameter is "
     "ignored.");
    
    descriptions.add("pileUp", desc);
}


void PECPileUp::beginJob()
{
    outTree = fileService->make<TTree>("PileUp", "Information about pile-up");
    
    puInfoPointer = &puInfo;
    outTree->Branch("puInfo", &puInfoPointer);
}


void PECPileUp::analyze(Event const &event, EventSetup const &)
{
    // Reset the buffer from the previous event
    puInfo.Reset();
    
    
    // Save the number of primary vertices
    Handle<reco::VertexCollection> vertices;
    event.getByToken(primaryVerticesToken, vertices);
    
    if (vertices->size() == 0)
    {
        Exception excp(errors::LogicError);
        excp << "Event contains zero good primary vertices.\n";
        excp.raise();
    }
    
    puInfo.SetNumPV(vertices->size());
    
    
    // Save rho
    Handle<double> rho, rhoCentral;
    
    event.getByToken(rhoToken, rho);
    puInfo.SetRho(*rho);
    
    event.getByToken(rhoCentralToken, rhoCentral);
    puInfo.SetRhoCentral(*rhoCentral);
    
    
    // Save pile-up information as simulated
    if (not runOnData)
    {
        Handle<View<PileupSummaryInfo>> puSummary;
        event.getByToken(puSummaryToken, puSummary);
        
        puInfo.SetTrueNumPU(puSummary->front().getTrueNumInteractions());
        //^ The "true" number of interactions is same for all bunch crossings
        
        for (unsigned i = 0; i < puSummary->size(); ++i)
            if (puSummary->at(i).getBunchCrossing() == 0)
            {
                puInfo.SetInTimePU(puSummary->at(i).getPU_NumInteractions());
                break;
            }
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECPileUp);
