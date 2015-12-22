#include "LHEEventWeights.h"

#include "LHEEventWeights.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <iostream>
#include <iomanip>


using namespace edm;
using namespace std;


unsigned const LHEEventWeights::maxNumAltWeights;


LHEEventWeights::LHEEventWeights(ParameterSet const &cfg):
    weightsHeaderTag(cfg.getParameter<string>("weightsHeaderTag")),
    computeMeanWeights(cfg.getParameter<bool>("computeMeanWeights")),
    storeWeights(cfg.getParameter<bool>("storeWeights")),
    nEventsProcessed(0)
{
    // Register required input data
    lheRunInfoToken =
     consumes<LHERunInfoProduct, edm::InRun>(cfg.getParameter<InputTag>("lheRunInfoProduct"));
    //^ See here [1] about reading data from a run
    //[1] https://hypernews.cern.ch/HyperNews/CMS/get/edmFramework/3583/1.html
    lheEventInfoToken =
     consumes<LHEEventProduct>(cfg.getParameter<InputTag>("lheEventInfoProduct"));
}


void LHEEventWeights::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("lheRunInfoProduct")->setComment("Tag to access per-run LHE information.");
    desc.add<string>("weightsHeaderTag", "initrwgt")->
     setComment("Tag to identify LHE header with description of event weights.");
    desc.add<InputTag>("lheEventInfoProduct")->
     setComment("Tag to access per-event LHE information.");
    desc.add<bool>("computeMeanWeights", true)->
     setComment("Indicates whether mean values of all weights should be computed.");
    desc.add<bool>("storeWeights", false)->
     setComment("Indicates whether event weights should be stored in a ROOT tree.");
    
    descriptions.add("lheEventWeights", desc);
}


void LHEEventWeights::beginJob()
{
    if (storeWeights)
    {
        outTree = fileService->make<TTree>("EventWeights", "Generator-level event weights");
        
        outTree->Branch("nominalWeight", &bfNominalWeight);
        outTree->Branch("numAltWeights", &bfNumAltWeights);
        outTree->Branch("altWeights", bfAltWeights, "altWeights[numAltWeights]/F");
    }
}


void LHEEventWeights::analyze(Event const &event, EventSetup const &)
{
    // Read LHE information for the current event
    Handle<LHEEventProduct> lheEventInfo;
    event.getByToken(lheEventInfoToken, lheEventInfo);
    
    
    // The nominal weight
    double const nominalWeight = lheEventInfo->originalXWGTUP();
    
    // Vector of alternative weights (e.g. systematic variations)
    vector<gen::WeightsInfo> const &altWeights = lheEventInfo->weights();
    
    
    // Perform initialization when processing the first event
    if (nEventsProcessed == 0)
    {
        if (computeMeanWeights)
            SetupWeightMeans(altWeights);
    }
    
    
    // Update means if requested
    if (computeMeanWeights)
    {
        // Use online algorithm described here [1]
        //[1] https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
        meanWeights.front().second +=
         (nominalWeight - meanWeights.front().second) / (nEventsProcessed + 1);
        
        for (unsigned i = 0; i < altWeights.size(); ++i)
        {
            meanWeights.at(i + 1).second +=
             (altWeights.at(i).wgt - meanWeights.at(i + 1).second) / (nEventsProcessed + 1);
        }
    }
    
    
    // Fill the output tree if requested
    if (storeWeights)
    {
        bfNominalWeight = nominalWeight;
        bfNumAltWeights = altWeights.size();
        
        for (unsigned i = 0; i < altWeights.size() and i < maxNumAltWeights; ++i)
            bfAltWeights[i] = altWeights.at(i).wgt;
        
        
        outTree->Fill();
    }
    
    
    // Update event counter
    ++nEventsProcessed;
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


void LHEEventWeights::endJob()
{
    cout << "Mean values of event weights:\n index   ID   mean\n\n";
    cout.precision(10);
    cout << "   -   nominal   " << meanWeights.front().second << "\n\n";
    
    for (unsigned i = 1; i < meanWeights.size(); ++i)
    {
        auto const &w = meanWeights.at(i);
        cout << " " << setw(3) << i - 1 << "   " << w.first << "   " << w.second << '\n';
    }
    
    cout << endl;
}


void LHEEventWeights::SetupWeightMeans(vector<gen::WeightsInfo> const &altWeights)
{
    meanWeights.reserve(1 + altWeights.size());
    
    
    // Set text IDs for all weights and set their means to zeros
    meanWeights.emplace_back("nominal", 0.);
    
    for (auto const &w: altWeights)
        meanWeights.emplace_back(w.id, 0.);
}


DEFINE_FWK_MODULE(LHEEventWeights);
