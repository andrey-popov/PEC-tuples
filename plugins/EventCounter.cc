#include "EventCounter.h"

#include <DataFormats/Common/interface/View.h>
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
    lheWeightIndices(cfg.getParameter<std::vector<int>>("saveAltLHEWeights")),
    psWeightIndices(cfg.getParameter<std::vector<int>>("saveAltPSWeights")),
    nEventProcessed(0)
{
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<edm::InputTag>("generator"));
    
    if (not lheWeightIndices.Empty())
    {
        lheEventInfoToken =
          consumes<LHEEventProduct>(cfg.getParameter<edm::InputTag>("lheEventProduct"));
    }
    
    
    if (cfg.exists("puInfo"))
        puSummaryToken = consumes<edm::View<PileupSummaryInfo>>(
          cfg.getParameter<edm::InputTag>("puInfo"));
}


void EventCounter::analyze(edm::Event const &event, edm::EventSetup const &)
{
    // Update event counter
    ++nEventProcessed;
    
    
    // Update the sum of nominal event weights
    edm::Handle<GenEventInfoProduct> generator;
    event.getByToken(generatorToken, generator);
    
    sumNominalWeight.Fill(generator->weight());
    
    
    // Update sums of alternative LHE event weights if requested
    if (not lheWeightIndices.Empty())
    {
        edm::Handle<LHEEventProduct> lheEventInfo;
        event.getByToken(lheEventInfoToken, lheEventInfo);
        
        std::vector<gen::WeightsInfo> const &altWeights = lheEventInfo->weights();
        
        
        // If this is the first event being processed, create summators for the alternative weights
        if (sumAltLheWeightCollection.empty())
            sumAltLheWeightCollection.resize(
              lheWeightIndices.NumberIndices(0, altWeights.size() - 1));
        
        
        // Add alternative weights to the corresponding sums rescaling them with the ratio between
        //the nominal weight read above and the nominal LHE weight, as prescribed in [1]
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/LHEReaderCMSSW?rev=7#How_to_use_weights
        double const factor = generator->weight() / lheEventInfo->originalXWGTUP();
        
        unsigned writeIndex = 0;

        for (int readIndex: lheWeightIndices.GetIndices(0, altWeights.size() - 1))
        {
            double const weight = altWeights[readIndex].wgt * factor;
            sumAltLheWeightCollection[writeIndex].Fill(weight);
            ++writeIndex;
        }
    }


    // Update sums of alternative PS event weights if requested and if they are available
    std::vector<double> const &psWeights = generator->weights();

    if (not psWeightIndices.Empty() and psWeights.size() > 1)
    {
        // If this is the first event being processed, create summators for the alternative weights
        if (sumAltPsWeightCollection.empty())
            sumAltPsWeightCollection.resize(
              psWeightIndices.NumberIndices(0, psWeights.size() - 1));
        
        
        unsigned writeIndex = 0;

        for (int readIndex: psWeightIndices.GetIndices(0, psWeights.size() - 1))
        {
            sumAltPsWeightCollection[writeIndex].Fill(psWeights[readIndex]);
            ++writeIndex;
        }
    }
    
    
    // Fill pileup profile if requested
    if (not puSummaryToken.isUninitialized())
    {
        edm::Handle<edm::View<PileupSummaryInfo>> puSummary;
        event.getByToken(puSummaryToken, puSummary);
        
        pileupProfile->Fill(puSummary->front().getTrueNumInteractions());
    }
}


void EventCounter::beginJob()
{
    if (not puSummaryToken.isUninitialized())
        pileupProfile = fileService->make<TH1D>("PileupProfile", "Pileup profile", 1000, 0., 100.);
}


void EventCounter::endJob()
{
    TTree *tree = fileService->make<TTree>("EventCounts", "Event counts and weights");
    
    
    tree->Branch("NumProcessed", &nEventProcessed);
    
    Float_t bfMeanNominalWeight = sumNominalWeight.GetSum() / nEventProcessed;
    tree->Branch("MeanNominalWeight", &bfMeanNominalWeight);
    
    
    std::vector<Float_t> bfMeanAltLheWeightCollection;
    
    if (not lheWeightIndices.Empty())
    {
        for (auto const &summator: sumAltLheWeightCollection)
            bfMeanAltLheWeightCollection.emplace_back(summator.GetSum() / nEventProcessed);
        
        tree->Branch("MeanAltLheWeights", &bfMeanAltLheWeightCollection);
    }


    std::vector<Float_t> bfMeanAltPsWeightCollection;
    
    if (not psWeightIndices.Empty())
    {
        for (auto const &summator: sumAltPsWeightCollection)
            bfMeanAltPsWeightCollection.emplace_back(summator.GetSum() / nEventProcessed);
        
        tree->Branch("MeanAltPsWeights", &bfMeanAltPsWeightCollection);
    }
    
    
    tree->Fill();
}


void EventCounter::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("generator", edm::InputTag("generator"))->
      setComment("Tag to access GenEventInfoProduct.");
    desc.add<std::vector<int>>("saveAltLHEWeights", std::vector<int>())->
      setComment("Intervals of indices of alternative LHE-level weights to be stored. "
        "Parsed using class IndexIntervals.");
    desc.add<edm::InputTag>("lheEventProduct", edm::InputTag("externalLHEProducer"))->
      setComment("Tag to access LHEEventProduct. Ignored if saveAltLHEWeights is False.");
    desc.add<std::vector<int>>("saveAltPSWeights", std::vector<int>())->
      setComment("Intervals of indices of alternative PS weights to be stored. "
        "Parsed using class IndexIntervals.");
    desc.addOptional<edm::InputTag>("puInfo")->
      setComment("Pileup summary. Providing this requests storing of pileup profile.");
    
    descriptions.add("eventCounter", desc);
}


DEFINE_FWK_MODULE(EventCounter);
