#include "PECGenerator.h"

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;
using namespace std;


PECGenerator::PECGenerator(ParameterSet const &cfg)
{
    // Register required input data
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<InputTag>("generator"));
}


void PECGenerator::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("generator", InputTag("generator"))->
     setComment("Generator-level event information.");
    
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
    
    
    Handle<GenEventInfoProduct> generator;
    event.getByToken(generatorToken, generator);
    
    
    generatorInfo.SetProcessId(generator->signalProcessID());
    
    for (double const &weight: generator->weights())
        generatorInfo.AddWeight(weight);
    
    
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
