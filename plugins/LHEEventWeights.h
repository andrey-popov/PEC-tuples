#pragma once

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h>
#include <SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h>
#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>
#include <utility>


/**
 * \class LHEEventWeights
 * \brief This plugin reads LHE event weights and extracts their descriptions
 * 
 * The plugin reads the LHE header and reports the list of computed alternative weights, including
 * their IDs and brief descriptions provided in the header. If requested, it computes average values
 * of all weights in the current job. The output is either printed to the standard output or
 * directed to text files, depending on the configuration. User can also configure the plugin to
 * store weights in all events in a ROOT file.
 */
class LHEEventWeights: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * Reads parameters from the configuration.
     */
    LHEEventWeights(edm::ParameterSet const &cfg);
    
    /// Destructor
    ~LHEEventWeights();
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Stores weights and updates their mean values (if requested)
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
    /**
     * \brief Prints out description of alternative weights as provided in the LHE header
     * 
     * It would be more natural to read the LHE header in beginRun, but this cannot be done because
     * of technical limitations, see e.g. here [1].
     * [1] https://hypernews.cern.ch/HyperNews/CMS/get/physTools/3437.html
     */
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
    
    /// Sets up the tree to store event weights
    void SetupWeightTree(unsigned nAltWeights);
    
private:
    /// Token to access per-run LHE information
    edm::EDGetTokenT<LHERunInfoProduct> lheRunInfoToken;
    
    /// Token to access per-event weights
    edm::EDGetTokenT<LHEEventProduct> lheEventInfoToken;
    
    /**
     * \brief Token to access global generator information
     * 
     * Only read if flag rescaleLHEWeights is set to true.
     */
    edm::EDGetTokenT<GenEventInfoProduct> generatorToken;
    
    /**
     * \brief Tag of the LHE header with information about weights
     * 
     * This string is used to identify a particular "header" among those provided by
     * LHERunInfoProduct.
     */
    std::string weightsHeaderTag;
    
    /**
     * \brief Indicates whether varied LHE weights should be rescaled to the weight given by
     * GenEventInfoProduct
     * 
     * Such rescaling is included in the instruction in [1].
     * [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/LHEReaderCMSSW?rev=7#How_to_use_weights
     */
    bool rescaleLHEWeights;
    
    /// Indicates whether mean values of all weights should be calculated
    bool computeMeanWeights;
    
    /// Indicates whether event weights should be stored in a ROOT tree
    bool storeWeights;
    
    /// Indicates if the output should be directed to files instead of standard output
    bool printToFiles;
    
    
    /// Buffer to keep (possibly rescaled) alternative LHE weights
    std::vector<double> altWeights;
    
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
     * \brief Actual number of alternative weights
     * 
     * Since this variable will be used as the length of arrays, ROOT only allows it to be of type
     * Int_t.
     */
    Int_t bfNumAltWeights;
    
    /**
     * \brief Alternative weights
     * 
     * Pointer to a dynamically allocated array. The array is owned by this.
     */
    Float_t *bfAltWeights;
};
