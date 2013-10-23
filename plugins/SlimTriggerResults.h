/**
 * \file SlimTriggerResults.h
 * \author Andrey Popov
 * 
 * The module defines an EDM plugin to store information about selected trigger paths.
 */

#pragma once

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <HLTrigger/HLTcore/interface/HLTConfigProvider.h>
#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <map>


/**
 * \struct TriggerState
 * \brief An auxiliary structure to represent a state of a trigger path in the current event
 * 
 * It contains a fully-qualified name of the trigger in the current menu, which can change over time
 * (due to the change in version number). The structure also host several buffers, which are used to
 * fill the output tree in the SlimTriggerResults class.
 */
struct TriggerState
{
    /// Default constructor
    TriggerState();
    
    /// A fully-qualified name of the trigger
    std::string fullName;
    
    /// A buffer to indicate whether the trigger was run in the current event
    Bool_t wasRun;
    
    /// A buffer to indicate whether the trigger has fired in the current event
    Bool_t accept;
    
    /// A buffer to store the prescale of the trigger in the current luminosity block
    Int_t prescale;
};


/**
 * \class SlimTriggerResults
 * \brief The class defines an EDM plugin to save information about selected trigger paths
 * 
 * Details of the interface are described in the file's documentation section.
 */
class SlimTriggerResults: public edm::EDFilter
{
public:
    /// Constructor from a configuration
    SlimTriggerResults(edm::ParameterSet const &cfg);
    
public:
    /// Creates the output tree
    void beginJob();
    
    /// Updates the information on the trigger menu
    bool beginRun(edm::Run &run, edm::EventSetup const &setup);
    
    /// Fills the output tree for each event
    bool filter(edm::Event &event, edm::EventSetup const &setup);
    
private:
    /// Strips the "HLT_" prefix and version postfix from a trigger name
    static std::string GetTriggerBasename(std::string const &name);
    
private:
    /**
     * \brief Map from trigger basenames to associated state structures
     * 
     * The "HLT_" prefix and the "_v*" postfix are stripped off of the trigger names. The map must
     * not be modified after the beginJob method has run since it provides the output tree pointers
     * to the map's elements
     */
    std::map<std::string, TriggerState> triggers;
    
    /// Name of the process that evaluated the trigger menu (usually it is "HTL")
    std::string const triggerProcessName;
    
    /// Specifies whether the plugins is to reject events that do not fire any of selected triggers
    bool const filterOn;
    
    /// An object to access information about each trigger
    HLTConfigProvider hltConfigProvider;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief The output tree
     * 
     * The tree is managed by the fileService object and will be deleted by its descructor.
     */
    TTree *triggerTree;
};
