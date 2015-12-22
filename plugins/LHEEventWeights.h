#pragma once

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h>
#include <SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>
#include <utility>


/**
 * \class LHEEventWeights
 * \brief 
 * 
 * 
 */
class LHEEventWeights: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * 
     */
    LHEEventWeights(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates the output tree if requested
    virtual void beginJob() override;
    
    /// 
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
    /// 
    virtual void endRun(edm::Run const &run, edm::EventSetup const &) override;
    
    /// Prints out mean values of the nominal and alternative weights
    virtual void endJob() override;
    
private:
    /**
     * \brief Sets up running means of nominal and alternative weights while processing first event
     * 
     * IDs of alternative weights are read from the given vector.
     */
    void SetupWeightMeans(std::vector<gen::WeightsInfo> const &altWeights);
    
private:
    /// Token to access per-run LHE information
    edm::EDGetTokenT<LHERunInfoProduct> lheRunInfoToken;
    
    /// Token to access per-event weights
    edm::EDGetTokenT<LHEEventProduct> lheEventInfoToken;
    
    /**
     * \brief Tag of the LHE header with information about weights
     * 
     * This string is used to identify a particular "header" among those provided by
     * LHERunInfoProduct.
     */
    std::string weightsHeaderTag;
    
    /// Indicates whether mean values of all weights should be calculated
    bool computeMeanWeights;
    
    /// Indicates whether event weights should be stored in a ROOT tree
    bool storeWeights;
    
    /// Indicates if the output should be directed to files instead of standard output
    bool printToFiles;
    
    
    /**
     * \brief Running means of nominal and alternative weights
     * 
     * Pairs stored in this vector consist of text IDs of weights and their current mean values,
     * which are updated with every new event. The first element of the vector corresponds to the
     * nominal weight. It is followed by pairs for alternative weights, keeping their ordering.
     */
    std::vector<std::pair<std::string, double>> meanWeights;
    
    /**
     * \brief Total number of events processed
     * 
     * This counter is needed to update mean values of weights.
     */
    unsigned long long nEventsProcessed;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief Output tree
     * 
     * Owned by the file service.
     */
    TTree *outTree;
    
    /// Output buffer to store nominal weight
    Float_t bfNominalWeight;
    
    /**
     * \brief Maximal supported number of alternative weights
     * 
     * Used to preallocate arrays.
     */
    static unsigned const maxNumAltWeights = 512;
    
    /**
     * \brief Actual number of alternative weights
     * 
     * Since this variable will be used as the length of arrays, ROOT only allows it to be of type
     * Int_t.
     */
    Int_t bfNumAltWeights;
    
    /// Alternative weights
    Float_t bfAltWeights[maxNumAltWeights];
};
