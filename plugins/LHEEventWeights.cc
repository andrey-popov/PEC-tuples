#include "LHEEventWeights.h"

#include "LHEEventWeights.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <iostream>


using namespace edm;
using namespace std;


LHEEventWeights::LHEEventWeights(ParameterSet const &cfg):
    weightsHeaderTag(cfg.getParameter<string>("weightsHeaderTag"))
{
    // Register required input data
    lheRunInfoToken =
     consumes<LHERunInfoProduct, edm::InRun>(cfg.getParameter<InputTag>("lheRunInfoProduct"));
    //^ See here [1] about reading data from a run
    //[1] https://hypernews.cern.ch/HyperNews/CMS/get/edmFramework/3583/1.html
}


void LHEEventWeights::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("lheRunInfoProduct")->setComment("Tag to access per-run LHE information.");
    desc.add<string>("weightsHeaderTag", "initrwgt")->
     setComment("Tag to identify LHE header with description of event weights.");
    
    descriptions.add("lheEventWeights", desc);
}


void LHEEventWeights::beginJob()
{
    // outTree = fileService->make<TTree>("EventID", "Event ID");
    
    // eventIdPointer = &eventId;
    // outTree->Branch("eventId", &eventIdPointer);
}


void LHEEventWeights::endRun(Run const &run, EventSetup const &)
{
    // Read LHE header
    Handle<LHERunInfoProduct> lheRunInfo;
    run.getByLabel("externalLHEProducer", lheRunInfo);
    
    
    // The header is split in LHERunInfoProduct into several blocks also called "headers". Loop over
    //them and find the one that contains descriptions of event weights
    for (auto header = lheRunInfo->headers_begin(); header != lheRunInfo->headers_end(); ++header)
    {
        // Skip all "headers" except for the sought-for one
        if (header->tag() != weightsHeaderTag)
            continue;
        
        // Print out the header
        for (auto const &l: header->lines())
            cout << l;
    }
}


void LHEEventWeights::analyze(Event const &event, EventSetup const &)
{}


DEFINE_FWK_MODULE(LHEEventWeights);
