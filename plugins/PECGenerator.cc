#include "PECGenerator.h"

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;
using namespace std;


PECGenerator::PECGenerator(ParameterSet const &cfg):
    saveLHEWeightVars(cfg.getParameter<bool>("saveLHEWeightVars"))
{
    // Register required input data
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<InputTag>("generator"));
    
    if (saveLHEWeightVars)
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
    desc.add<bool>("saveLHEWeightVars", true)->
     setComment("Indicates whether LHE-level variations of event weights should be stored.");
    desc.add<InputTag>("lheEventInfoProduct", InputTag("externalLHEProducer"))->
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
    
    Handle<LHEEventProduct> lheEventInfo;
    event.getByToken(lheEventInfoToken, lheEventInfo);
    
    
    // Process ID
    generatorInfo.SetProcessId(lheEventInfo->hepeup().IDPRUP);
    
    
    // Event weights
    generatorInfo.SetNominalWeight(generator->weight());
    
    if (saveLHEWeightVars)
    {
        // Alternative LHE weights will be rescaled by the ratio between the nominal weight above
        //and the nominal LHE weight, as instructed here [1]
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/LHEReaderCMSSW?rev=7#How_to_use_weights
        double const factor = generator->weight() / lheEventInfo->originalXWGTUP();
        
        
        // Save the alternative weights
        vector<gen::WeightsInfo> const &altWeights = lheEventInfo->weights();
        
        for (auto const &altWeight: altWeights)
            generatorInfo.AddAltWeight(altWeight.wgt * factor);
    }
        
    
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
