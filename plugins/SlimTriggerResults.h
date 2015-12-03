#pragma once

#include <DataFormats/Common/interface/TriggerResults.h>
#include <FWCore/Common/interface/TriggerNames.h>
#include <DataFormats/PatCandidates/interface/PackedTriggerPrescales.h>

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

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
    
    /**
     * \brief Indicates if the trigger is present in the current menu
     * 
     * It might happen that some of triggers requested by user are missing in some of menus.
     */
    bool inMenu;
    
    /// Index of the trigger in the (current) menu
    unsigned index;
    
    /**
     * \brief A buffer to indicate whether the trigger was run in the current event
     * 
     * It is read directly by the output tree.
     */
    Bool_t wasRun;
    
    /**
     * \brief A buffer to indicate whether the trigger has fired in the current event
     * 
     * It is read directly by the output tree.
     */
    Bool_t accept;
    
    /**
     * \brief A buffer to store the prescale of the trigger in the current luminosity block
     * 
     * It is read directly by the output tree.
     */
    UInt_t prescale;
};


/**
 * \class SlimTriggerResults
 * \brief An EDM plugin to save information about selected trigger paths
 * 
 * The module defines an EDM plugin to store information about selected trigger paths. The user
 * must provide a list of triggers in which (s)he is interested. Prefix "HLT_" and postfix with
 * version number in a trigger name might be omitted; HLT_Mu15_v7, HLT_Mu15_v, HLT_Mu15, Mu15_v7,
 * Mu15 all are valid inputs and refer to the same trigger (different trigger versions are not
 * distinguished). No wildcards are allowed.
 * 
 * Results are stored in a plain ROOT tree, which contains three branches for each requested
 * trigger: a boolean indicating if the trigger was executed in the current event, a boolean showing
 * if the current event was accepted by the trigger, and an integer with the trigger prescale.
 * 
 * The plugin can be configured in such a way that it rejects an event if it is not accepted by any
 * of the selected triggers. The default behaviour is to reject no events.
 */
class SlimTriggerResults: public edm::EDFilter
{
public:
    /// Constructor from a configuration
    SlimTriggerResults(edm::ParameterSet const &cfg);
    
public:
    /// Creates the output tree
    virtual void beginJob() override;
    
    /// Fills the output tree for each event
    virtual bool filter(edm::Event &event, edm::EventSetup const &setup) override;
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    /// Strips the "HLT_" prefix and version postfix from a trigger name
    static std::string GetTriggerBasename(std::string const &name);
    
    /**
     * \brief Updates indices of selected triggers in the menu
     * 
     * Should be called when the trigger menu changes as it might invalidate trigger indices. It is
     * the only method that reads trigger names.
     */
    void UpdateMenu(edm::TriggerNames const &triggerNames);
    
private:
    /**
     * \brief Map from trigger basenames to associated state structures
     * 
     * The "HLT_" prefix and the "_v*" postfix are stripped off of the trigger names. The map must
     * not be modified after the beginJob method has run since it provides the output tree pointers
     * to the map's elements
     */
    std::map<std::string, TriggerState> triggers;
    
    /// Token to access trigger decisions
    edm::EDGetTokenT<edm::TriggerResults> triggerBitsToken;
    
    /// Token to access trigger prescales
    edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescalesToken;
    
    /// Specifies whether the plugin is to reject events that do not fire any of selected triggers
    bool const filterOn;
    
    /// Specifies whether prescale column should be saved
    bool const savePrescales;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief The output tree
     * 
     * The tree is managed by the fileService object and will be deleted by its descructor.
     */
    TTree *triggerTree;
    
    /**
     * \brief ID of the previous trigger configuration
     * 
     * It is used to discover updates in the trigger menu.
     */
    edm::ParameterSetID prevTriggerParameterSetID;
};
