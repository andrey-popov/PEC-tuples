#include "ProcessIDFilter.h"

#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/Exception.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <algorithm>


ProcessIDFilter::ProcessIDFilter(edm::ParameterSet const &cfg):
    allowedProcessIDs(cfg.getParameter<std::vector<int>>("processIDs"))
{
    // Make sure the vector with process IDs is sorted to allow using binary search
    std::sort(allowedProcessIDs.begin(), allowedProcessIDs.end());
    
    
    // Set up tokens to read process ID
    edm::InputTag const generatorTag = cfg.getParameter<edm::InputTag>("generator");
    edm::InputTag const lheEventInfoTag = cfg.getParameter<edm::InputTag>("lheEventProduct");
    
    if (generatorTag.label().empty() == lheEventInfoTag.label().empty())
    {
        cms::Exception excp("Configuration");
        excp << "Input tag for either \"generator\" or \"lheEventProduct\" must be provided.";
        excp.raise();
    }
    
    if (not generatorTag.label().empty())
    {
        useLHEInfo = false;
        generatorToken = consumes<GenEventInfoProduct>(generatorTag);
    }
    
    if (not lheEventInfoTag.label().empty())
    {
        useLHEInfo = true;
        lheEventInfoToken = consumes<LHEEventProduct>(lheEventInfoTag);
    }
}


void ProcessIDFilter::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("generator", edm::InputTag())->
      setComment("Tag to access GenEventInfoProduct or an empty value (\"\").");
    desc.add<edm::InputTag>("lheEventProduct", edm::InputTag())->
      setComment("Tag to access LHEEventProduct or an empty value (\"\").");
    desc.add<std::vector<int>>("processIDs")->
      setComment("Process IDs to select.");
    
    descriptions.add("processIDFilter", desc);
}


bool ProcessIDFilter::filter(edm::Event &event, edm::EventSetup const &)
{
    int processID;
    
    if (useLHEInfo)
    {
        edm::Handle<LHEEventProduct> lheEventInfo;
        event.getByToken(lheEventInfoToken, lheEventInfo);
        processID = lheEventInfo->hepeup().IDPRUP;
    }
    else
    {
        edm::Handle<GenEventInfoProduct> generator;
        event.getByToken(generatorToken, generator);
        processID = generator->signalProcessID();
    }
    
    return std::binary_search(allowedProcessIDs.begin(), allowedProcessIDs.end(), processID);
}


DEFINE_FWK_MODULE(ProcessIDFilter);
