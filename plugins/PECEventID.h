#pragma once

#include <Analysis/PECTuples/interface/EventID.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>


/**
 * \class PECEventID
 * \brief Stores event ID (run, luminosity block, and event number)
 */
class PECEventID: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * The plugin has a trivial configuration, and thus the constructor does not read it.
     */
    PECEventID(edm::ParameterSet const &);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output tree
    virtual void beginJob() override;
    
    /// Writes ID of the current event in the buffer and fills the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
private:
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /// Output tree
    TTree *outTree;
    
    /// Buffer to store event ID
    pec::EventID eventId;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    pec::EventID *eventIdPointer;
};
