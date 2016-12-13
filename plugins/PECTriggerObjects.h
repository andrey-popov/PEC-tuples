#pragma once

#include <Analysis/PECTuples/interface/Candidate.h>

#include <DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <vector>
#include <string>


/**
 * \class PECTriggerObjects
 * \brief An EDM plugin to save trigger objects accepted by selected filters
 * 
 * For each selected HLT filter stores a vector of trigger objects that pass it. Tree branches are
 * named after the filters, trigger objects are stored as instances of pec::Candidate.
 */
class PECTriggerObjects: public edm::EDAnalyzer
{
private:
    /// Auxiliary structure to aggregate information about an HLT filter
    struct FilterBuffer
    {
        /// Constructor from a filter name
        FilterBuffer(std::string const &name);
        
        /// Name of the filter
        std::string name;
        
        /// Corresponding trigger objects
        std::vector<pec::Candidate> objects;
        
        /**
         * \brief Pointer to the vector of objects
         * 
         * ROOT needs it to store the vector in a tree.
         */
        std::vector<pec::Candidate> *objectsPointer;
    };
    
public:
    /// Constructor from a configuration
    PECTriggerObjects(edm::ParameterSet const &cfg);
    
public:
    /// Fills the output tree for each event
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
    /// Creates the output tree
    virtual void beginJob() override;
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    /// Token to access trigger objects
    edm::EDGetTokenT<edm::View<pat::TriggerObjectStandAlone>> triggerObjectsToken;
    
    /// Buffers to store trigger objects that pass selected filters
    std::vector<FilterBuffer> buffers;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /// Output tree
    TTree *outTree;
};
