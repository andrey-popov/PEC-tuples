#include "EventCounter.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <TTree.h>


SignedKahanSum::SignedKahanSum() noexcept:
    posSum(0.), negSum(0.),
    posCompensation(0.), negCompensation(0.)
{}


void SignedKahanSum::Fill(double x)
{
    if (x >= 0.)
    {
        double const xCompensated = x - posCompensation;
        double const sum = posSum + xCompensated;
        posCompensation = (sum - posSum) - xCompensated;
        posSum = sum;
    }
    else
    {
        // Run the standard Kahan algorithm with the inverted input
        x = -x;
        
        double const xCompensated = x - negCompensation;
        double const sum = negSum + xCompensated;
        negCompensation = (sum - negSum) - xCompensated;
        negSum = sum;
    }
}


double SignedKahanSum::GetSum() const
{
    // Since there might be a catastrophic cancellation between the positive and negative sums,
    //take into account also the correction from the compensations
    return (posSum - negSum) - (posCompensation - negCompensation);
}



EventCounter::EventCounter(edm::ParameterSet const &cfg):
    saveAltLHEWeights(cfg.getParameter<bool>("saveAltLHEWeights")),
    nEventProcessed(0)
{
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<edm::InputTag>("generator"));
    
    if (saveAltLHEWeights)
    {
        lheEventInfoToken =
          consumes<LHEEventProduct>(cfg.getParameter<edm::InputTag>("lheEventProduct"));
    }
}


void EventCounter::analyze(edm::Event const &event, edm::EventSetup const &)
{
    // Update event counter
    ++nEventProcessed;
    
    
    // Update the sum of nominal event weights
    edm::Handle<GenEventInfoProduct> generator;
    event.getByToken(generatorToken, generator);
    
    sumNominalWeight.Fill(generator->weight());
    
    
    // Update sums of alternative event weights if requested
    if (saveAltLHEWeights)
    {
        edm::Handle<LHEEventProduct> lheEventInfo;
        event.getByToken(lheEventInfoToken, lheEventInfo);
        
        std::vector<gen::WeightsInfo> const &altWeights = lheEventInfo->weights();
        
        
        // If this is the first event being processed, create summators for the alternative weights
        if (sumAltWeightCollection.empty())
            sumAltWeightCollection.resize(altWeights.size());
        
        
        // Add alternative weights to the corresponding sums rescaling them with the ratio between
        //the nominal weight read above and the nominal LHE weight, as prescribed in [1]
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/LHEReaderCMSSW?rev=7#How_to_use_weights
        double const factor = generator->weight() / lheEventInfo->originalXWGTUP();
        
        for (unsigned i = 0; i < altWeights.size(); ++i)
            sumAltWeightCollection[i].Fill(altWeights[i].wgt * factor);
    }
}


void EventCounter::endJob()
{
    TTree *tree = fileService->make<TTree>("EventCounts", "Event counts and weights");
    
    
    tree->Branch("NumProcessed", &nEventProcessed);
    
    Float_t bfMeanNominalWeight = sumNominalWeight.GetSum() / nEventProcessed;
    tree->Branch("MeanNominalWeight", &bfMeanNominalWeight);
    
    
    std::vector<Float_t> bfMeanAltWeightCollection;
    
    if (saveAltLHEWeights)
    {
        for (auto const &summator: sumAltWeightCollection)
            bfMeanAltWeightCollection.emplace_back(summator.GetSum() / nEventProcessed);
        
        tree->Branch("MeanAltWeights", &bfMeanAltWeightCollection);
    }
    
    
    tree->Fill();
}


void EventCounter::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("generator", edm::InputTag("generator"))->
      setComment("Tag to access GenEventInfoProduct.");
    desc.add<bool>("saveAltLHEWeights", true)->
      setComment("Requests saving mean values of alternative LHE-level weights.");
    desc.add<edm::InputTag>("lheEventProduct", edm::InputTag("externalLHEProducer"))->
      setComment("Tag to access LHEEventProduct. Ignored if saveAltLHEWeights is False.");
    
    descriptions.add("eventCounter", desc);
}


DEFINE_FWK_MODULE(EventCounter);
