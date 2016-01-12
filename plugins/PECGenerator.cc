#include "PECGenerator.h"

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;
using namespace std;


PECGenerator::PECGenerator(ParameterSet const &cfg):
    externalLHE(cfg.getParameter<bool>("externalLHE"))
{
    // Register required input data
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<InputTag>("generator"));
    
    if (externalLHE)
    {
        lheEventInfoToken =
         consumes<LHEEventProduct>(cfg.getParameter<InputTag>("lheEventInfoProduct"));
    }
}


void PECGenerator::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("generator", InputTag("generator"))->
     setComment("Generator-level event information.");
    desc.add<bool>("externalLHE", true)->
     setComment("Indicates whether an external LHE generator was used.");
    desc.add<InputTag>("lheEventInfoProduct")->
     setComment("Tag to access per-event LHE information. Ignored if externalLHE is False.");
    
    descriptions.add("generator", desc);
}


void PECGenerator::beginJob()
{
    outTree = fileService->make<TTree>("Generator", "Global generator-level properties");
    
    generatorInfoPointer = &generatorInfo;
    outTree->Branch("generator", &generatorInfoPointer);
}


void PECGenerator::analyze(Event const &event, EventSetup const &)
{
    // Reset the buffer from the previous event
    generatorInfo.Reset();
    
    
    // Read generator information for the current event
    Handle<GenEventInfoProduct> generator;
    event.getByToken(generatorToken, generator);
    
    
    // Process ID
    generatorInfo.SetProcessId(generator->signalProcessID());
    
    
    // Event weights
    if (externalLHE)
    {
        // Read the LHE information
        Handle<LHEEventProduct> lheEventInfo;
        event.getByToken(lheEventInfoToken, lheEventInfo);
        
        
        // Read the weights
        generatorInfo.SetNominalWeight(lheEventInfo->originalXWGTUP());
        vector<gen::WeightsInfo> const &altWeights = lheEventInfo->weights();
        
        for (auto const &altWeight: altWeights)
            generatorInfo.AddAltWeight(altWeight.wgt);
    }
    else
        generatorInfo.SetNominalWeight(generator->weight());
    
    // At the moment event weights are only applied at the LHE level, and thus the nominal weight
    //should be the same in the both cases. However, this might change in future
        
    
    // PDF information
    GenEventInfoProduct::PDF const *pdf = generator->pdf();
    
    if (pdf)
    {
        generatorInfo.SetPdfXs(pdf->x.first, pdf->x.second);
        generatorInfo.SetPdfIds(pdf->id.first, pdf->id.second);
        generatorInfo.SetPdfQScale(pdf->scalePDF);
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECGenerator);
