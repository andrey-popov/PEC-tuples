#pragma once

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <CommonTools/UtilAlgos/interface/TFileService.h>
#include <DataFormats/Common/interface/TriggerResults.h>
#include <FWCore/ServiceRegistry/interface/Service.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \class EventFlags
 * \brief Stores boolean values of selected flags
 * 
 * This plugin reads values of selected flags from a TriggerResults objects and saves them in a
 * TTree. Selected flags are given as a vector of strings of the format FlagName:BranchName, where
 * FlagName is the name of the flag in TriggerResults and BranchName is the desired name for the
 * TTree branch to store this flag; if the colon is not found in the string, the string is used as
 * both FlagName and BranchName.
 */
class EventFlags: public edm::EDAnalyzer
{
private:
    /// Auxiliary structure to aggregate information about a flag
    struct FlagInfo
    {
        /**
         * \brief Constructor from a name pair
         * 
         * The argument is either a single name, which is then used as both flag name and branch
         * name, or a string that combines the two, separating them with a colon.
         */
        FlagInfo(std::string const &name);
        
        /// Original name of the flag in TriggerResults
        std::string flagName;
        
        /// Name for the corresponding branch in the output tree
        std::string branchName;
        
        /// Index of the flag in TriggerResults
        unsigned index;
        
        /// Buffer for the tree to write flag decision in each event
        Bool_t decision;
    };
    
public:
    /// Constructor
    EventFlags(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    /// Fills the output tree with flags for the current event
    virtual void analyze(edm::Event const &event, edm::EventSetup const &eventSetup) override;
    
    /// Creates the output tree
    virtual void beginJob() override;
    
private:
    /// Token to access precomputed flags
    edm::EDGetTokenT<edm::TriggerResults> flagToken;
    
    /// Names and indices of selected flags
    std::vector<FlagInfo> flagInfos;
    
    /**
     * \brief Flag showing whether indices to access flags have been set up
     * 
     * It is assumed that the indices do not change throughout the whole job.
     */
    bool indicesSetup;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief The output tree
     * 
     * The tree is managed by the fileService object.
     */
    TTree *tree;
};
