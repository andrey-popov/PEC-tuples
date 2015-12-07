#pragma once

#include <Analysis/PECTuples/interface/GeneratorInfo.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \class PECGenerator
 * \brief Stores global generator-level information
 * 
 * Saves generator-level weights, PDF information, etc. This plugin must be only run on simulation.
 */
class PECGenerator: public edm::EDAnalyzer
{
public:
    /// Constructor
    PECGenerator(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output tree
    virtual void beginJob() override;
    
    /// Writes global generator information into the buffer and fills the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
private:
    /// Token to access global generator information
    edm::EDGetTokenT<GenEventInfoProduct> generatorToken;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /// Output tree
    TTree *outTree;
    

    /// Buffer to store generator information
    pec::GeneratorInfo generatorInfo;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    pec::GeneratorInfo *generatorInfoPointer;
};
