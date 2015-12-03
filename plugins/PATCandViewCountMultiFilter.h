/**
 * \file PATCandViewCountMultiFilter.h
 * 
 * Defines a plugin that extends PATCandViewCountFilter to a case of multiple input collections.
 * For each collection it counts candidates that pass a user-defined selection. If the number of
 * selected candidates in at least one of the collections matches the desired range, the event is
 * accepted.
 */

#pragma once

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <DataFormats/Candidate/interface/Candidate.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <vector>


/**
 * \class PATCandViewCountMultiFilter
 * \brief A EDFilter to check if a desired number of objects passing a selection is found in at
 * least one of the input collections
 * 
 * Consult the file's documentation section for details.
 */
class PATCandViewCountMultiFilter: public edm::EDFilter
{
public:
    /// Constructor
    PATCandViewCountMultiFilter(edm::ParameterSet const &cfg);
    
public:
    /// Evaluates decision of the plugin
    virtual bool filter(edm::Event &event, edm::EventSetup const &eventSetup);
    
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    /**
     * \brief Input collections
     * 
     * They are expected to contain instances of a class convertable to reco::Candidate.
     */
    std::vector<edm::EDGetTokenT<edm::View<reco::Candidate>>> sourceTokens;
    
    /// Desired selection to filter candidates
    StringCutObjectSelector<reco::Candidate> const selection;
    
    /**
     * \brief Allowed range of numbers of candidates that pass the selection
     * 
     * The boundaries are included.
     */
    unsigned minNumber, maxNumber;
};
