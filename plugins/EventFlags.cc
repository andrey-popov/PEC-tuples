#include "EventFlags.h"

#include <FWCore/Common/interface/TriggerNames.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/Utilities/interface/EDMException.h>


EventFlags::FlagInfo::FlagInfo(std::string const &name):
    index(-1),
    decision(false)
{
    auto const delimPos = name.find(':');
    
    if (delimPos == std::string::npos)
    {
        flagName = name;
        branchName = name;
    }
    else
    {
        flagName = name.substr(0, delimPos);
        branchName = name.substr(delimPos + 1);
    }
}


EventFlags::EventFlags(edm::ParameterSet const &cfg):
    indicesSetup(false),
    tree(nullptr)
{
    flagToken = consumes<edm::TriggerResults>(cfg.getParameter<edm::InputTag>("src"));
    
    
    auto const &flagStrings = cfg.getParameter<std::vector<std::string>>("flags");
    flagInfos.reserve(flagStrings.size());
    
    for (auto const &name: flagStrings)
        flagInfos.emplace_back(name);
}


void EventFlags::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src")->setComment("TriggerResults object with evaluated flags.");
    desc.add<std::vector<std::string>>("flags")->setComment("Flags to store.");
    
    descriptions.add("eventFlags", desc);
}


void EventFlags::analyze(edm::Event const &event, edm::EventSetup const &eventSetup)
{
    // Read flags for the current event. If this is the first event, find indices correspoinding
    //to the selected flags.
    edm::Handle<edm::TriggerResults> flags;
    event.getByToken(flagToken, flags);
    
    if (not indicesSetup)
    {
        edm::TriggerNames const &flagNames = event.triggerNames(*flags);
        unsigned const nFlags = flagNames.size();
        
        for (unsigned i = 0; i < flagInfos.size(); ++i)
        {
            unsigned const index = flagNames.triggerIndex(flagInfos[i].flagName);
            
            if (index == nFlags)
            {
                edm::Exception excp(edm::errors::LogicError);
                excp << "Flag \"" << flagInfos[i].flagName << "\" is not found.";
                excp.raise();
            }
            else
                flagInfos[i].index = i;
        }
        
        indicesSetup = true;
    }
    
    
    // Read and store flag values
    for (auto &flagInfo: flagInfos)
        flagInfo.decision = flags->accept(flagInfo.index);
    
    tree->Fill();
}


void EventFlags::beginJob()
{
    tree = fileService->make<TTree>("EventFlags", "Selected flags");
    
    for (auto &info: flagInfos)
        tree->Branch(info.branchName.c_str(), &info.decision);
}


DEFINE_FWK_MODULE(EventFlags);
