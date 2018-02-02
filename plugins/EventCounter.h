#pragma once

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>
#include <SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h>
#include <SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TH1D.h>

#include <vector>


/**
 * \class SignedKahanSum
 * \brief Implements compensated summation for positive and negative numbers separately
 * 
 * This class computes a sum of the given sequence of numbers on the fly. It tries to compensate
 * for errors arising from the floating-point arithmetic using the Kahan summation algorithm [1].
 * Summation is done independently for positive and negative numbers in order to prevent a
 * catastrophic cancellation.
 * [1] https://en.wikipedia.org/wiki/Kahan_summation_algorithm
 */
class SignedKahanSum
{
public:
    /// Trivial constructor
    SignedKahanSum() noexcept;
    
public:
    /// Adds a new number to the sum
    void Fill(double x);
    
    /// Returns the current sum
    double GetSum() const;
    
private:
    /// Current sums of positive and negative numbers
    double posSum, negSum;
    
    /// Compensation values for the two sums, which are used in the Kahan algorithm
    double posCompensation, negCompensation;
};


/**
 * \class EventCounter
 * \brief A plugin to save number of processed events, mean generator-level weights, and,
 * optionally, pileup profile
 * 
 * This plugin stores the total number of processed events and the mean nominal generator-level
 * event weight. If configured to do so, it also saves mean values of each type of alternative
 * LHE-level weights. These quantities are stored in a trivial tree containing a single entry.
 * 
 * In addition, when an input tag with PileupSummaryInfo is provided in the configuration, the
 * plugin fills a histogram with pileup profile.
 * 
 * The plugin can only process simulated events. Normally it should be put in the execution path
 * before any filters.
 * 
 * Computation of mean weights is implemented with the help of the compensated summation algorithm
 * provided by class SignedKahanSum.
 */
class EventCounter: public edm::EDAnalyzer
{
public:
    /// Constructor
    EventCounter(edm::ParameterSet const &cfg);
    
public:
    /// Updates event counter and sums of event weights
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
    /// Creates histogram for pileup profile if needed
    virtual void beginJob() override;
    
    /// Saves output in a trivial tree
    virtual void endJob() override;
    
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    /// Token to access global generator information
    edm::EDGetTokenT<GenEventInfoProduct> generatorToken;
    
    /// Token to access per-event LHE weights
    edm::EDGetTokenT<LHEEventProduct> lheEventInfoToken;
    
    /// Flag requesting saving of mean LHE-level event weights
    bool saveAltLHEWeights;
    
    /**
     * \brief Token to access pileup information
     * 
     * Might not be initialized. In that case do not store pileup profile.
     */
    edm::EDGetTokenT<edm::View<PileupSummaryInfo>> puSummaryToken;
    
    /// Running number of processed events
    ULong64_t nEventProcessed;
    
    /// Sum of nominal weights of processed events
    SignedKahanSum sumNominalWeight;
    
    /**
     * \brief Sums of alternative weights, for each type of weight
     * 
     * This vector is reinitialized when the first event is processed.
     */
    std::vector<SignedKahanSum> sumAltWeightCollection;
    
    /**
     * \brief Non-owning pointer to a histogram with pileup profile
     * 
     * Only filled when requested through the plugin's configuration.
     */
    TH1D *pileupProfile;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
};
