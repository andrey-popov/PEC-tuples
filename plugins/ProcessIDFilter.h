#pragma once

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>
#include <SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h>

#include <vector>


/**
 * \class ProcessIDFilter
 * \brief Performs filtering based on process ID
 * 
 * Process ID is read from either LHE or HepMC record, depending on which of the two supported
 * input tag parameters is provided. The other input tag must not be set. Accepted are events whose
 * process IDs are found in the provided list.
 */
class ProcessIDFilter: public edm::EDFilter
{
public:
    /// Constructor
    ProcessIDFilter(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Checks if the event stems from a process with allowed ID
    virtual bool filter(edm::Event &event, edm::EventSetup const &) override;
    
private:
    /// Token to access global generator information
    edm::EDGetTokenT<GenEventInfoProduct> generatorToken;
    
    /// Token to access LHE event record
    edm::EDGetTokenT<LHEEventProduct> lheEventInfoToken;
    
    /// Indicates whether process ID should be read from LHE or HepMC record
    bool useLHEInfo;
    
    /**
     * \brief Process IDs to be selected by this filter
     * 
     * The vector is ordered.
     */
    std::vector<int> allowedProcessIDs;
};
