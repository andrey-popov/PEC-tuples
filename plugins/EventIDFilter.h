#pragma once

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>

#include <DataFormats/Provenance/interface/EventID.h>

#include <vector>
#include <string>


/**
 * \class EventIDFilter
 * \brief Performs event filtering based on given collection of event IDs
 * 
 * Depending on the configuration, keeps or rejects events whose IDs are found in the collection.
 * The collection is read from a text or a ROOT file. Their formats are described in the
 * documentation for methods ReadTextFile and ReadROOTFile.
 */
class EventIDFilter: public edm::EDFilter
{
public:
    /**
     * \brief Constructor
     * 
     * Reads the configuration. Reads the input file with event IDs, stores them in a vector, and
     * sorts it.
     */
    EventIDFilter(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Performs event filtering based on ID of the current event
    virtual bool filter(edm::Event &event, edm::EventSetup const &) override;
    
private:
    /**
     * \brief Reads a collection of event IDs from a text file
     * 
     * Event IDs must be stored in the form "run:lumi:event", one per line. No blank lines or
     * comments are allowed. If method fails to parse the file, it throws an exception.
     */
    void ReadTextFile(std::string const &fileName);
    
    /**
     * \brief Reads a collection of event IDs from a ROOT file
     * 
     * Event IDs must be stored in a tree called "EventID" in branches "run", "lumi", "event" of
     * types 'i', 'i', 'l' (as defined in TBranch) respectively. If these requirements are not
     * satisfied, the method throws an exception.
     */
    void ReadROOTFile(std::string const &fileName);
    
private:
    /**
     * \brief Collection of event IDs read from the input file
     * 
     * The vector will be sorted before the first event is checked. Use a vector instead of a set
     * because lookup in an ordered vector is at least as fast as in a set, while insertion is
     * much faster.
     */
    std::vector<edm::EventID> knownEvents;
    
    /// Determines if events present in the container should be kept or rejected
    bool rejectKnownEvents;
};
