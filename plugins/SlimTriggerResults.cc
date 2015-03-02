#include "SlimTriggerResults.h"

#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Common/interface/TriggerResultsByName.h>
#include <DataFormats/Common/interface/HLTPathStatus.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <boost/algorithm/string/predicate.hpp>

#include <vector>


using namespace std;


TriggerState::TriggerState():
    inMenu(false),
    index(0),
    wasRun(false),
    accept(false),
    prescale(0)
{}


SlimTriggerResults::SlimTriggerResults(edm::ParameterSet const &cfg):
    filterOn(cfg.getParameter<bool>("filter")),
    savePrescales(cfg.getParameter<bool>("savePrescales"))
{
    // Push trigger names provided by the user into a map
    auto const &triggerNames = cfg.getParameter<vector<string>>("triggers");
    
    for (auto const &name: triggerNames)
    {
        string const basename = GetTriggerBasename(name);
        
        
        // A sanity check
        if (basename.length() == 0)
        {
            edm::Exception excp(edm::errors::LogicError);
            excp << "The trigger name \"" << name << "\" does not seem as a valid one.\n";
            excp.raise();
        }
        
        
        // Finally, insert the basename in the map
        triggers[basename];
    }
    
    
    // Tokens to read trigger details
    triggerBitsToken =
     consumes<edm::TriggerResults>(cfg.getParameter<edm::InputTag>("triggerBits"));
    triggerPrescalesToken =
     consumes<pat::PackedTriggerPrescales>(cfg.getParameter<edm::InputTag>("triggerPrescales"));

}


void SlimTriggerResults::beginJob()
{
    // Create the output tree
    triggerTree = fileService->make<TTree>("TriggerInfo", "States of selected triggers");
    
    // Assign branches to it
    for (auto &t: triggers)
    {
        triggerTree->Branch((t.first + "__wasRun").c_str(), &t.second.wasRun);
        triggerTree->Branch((t.first + "__accept").c_str(), &t.second.accept);
        
        if (savePrescales)
            triggerTree->Branch((t.first + "__prescale").c_str(), &t.second.prescale);
    }
}


bool SlimTriggerResults::filter(edm::Event &event, edm::EventSetup const &setup)
{
    // Read trigger decisions for the current event
    edm::Handle<edm::TriggerResults> triggerBits;
    event.getByToken(triggerBitsToken, triggerBits);
    
    
    // Check if the trigger configuration has changed and update trigger indices if needed
    if (triggerBits->parameterSetID() != prevTriggerParameterSetID)
    {
        UpdateMenu(event.triggerNames(*triggerBits));
        prevTriggerParameterSetID = triggerBits->parameterSetID();
    }
    
    
    // Read prescales if needed
    edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;
    
    if (savePrescales)
        event.getByToken(triggerPrescalesToken, triggerPrescales);
    
    
    // Overall filter result that will be a logical OR of all selected triggers
    bool result = true;
    
    
    
    // Fill buffers for all selected triggers
    for (auto &t: triggers)
    {
        // Continue to the next trigger if the current one is not in the current menu
        if (not t.second.inMenu)
            continue;
        
        
        // Update state of the current trigger
        t.second.wasRun = triggerBits->wasrun(t.second.index);
        t.second.accept = triggerBits->accept(t.second.index);
        
        if (savePrescales)
            t.second.prescale = triggerPrescales->getPrescaleForIndex(t.second.index);
        
        
        if (t.second.wasRun and t.second.accept)
            result = true;
    }
    
    
    // Fill the output tree if the event is accepted
    if (result or not filterOn)
        triggerTree->Fill();
    
    
    return (filterOn) ? result : true;
}


void SlimTriggerResults::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    // Documentation for descriptions of the configuration is available in [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideConfigurationValidationAndHelp
    
    edm::ParameterSetDescription desc;
    desc.add<vector<string>>("triggers")->
     setComment("Names of triggers whose results are to be saved.");
    desc.add<bool>("filter", false)->
     setComment("Indicates if an event that does not fire any of the requested triggers should be "
     "rejected.");
    desc.add<bool>("savePrescales", true)->
     setComment("Specifies whether trigger prescales should be saved.");
    desc.add<edm::InputTag>("triggerBits", edm::InputTag("TriggerResults"))->
     setComment("Trigger decisions.");
    desc.add<edm::InputTag>("triggerPrescales", edm::InputTag("patTrigger"))->
     setComment("Packed trigger prescales.");
    
    descriptions.add("triggerInfo", desc);
}


string SlimTriggerResults::GetTriggerBasename(string const &name)
{
    // The name might (or might not) contain a prefix "HLT_" and/or a postfix with version
    //number of the form "_v\d+", "_v*", or "_v". They are stripped off if found
    string basename(name);
    
    // First, the prefix
    if (boost::starts_with(basename, "HLT_"))
        basename = basename.substr(4);
    
    // Now check the postfix
    if (boost::ends_with(basename, "_v*"))
        basename = basename.substr(0, basename.length() - 3);
    else if (boost::ends_with(basename, "_v"))
        basename = basename.substr(0, basename.length() - 2);
    else
    {
        // Maybe, the full version was specified
        int pos = basename.length() - 1;
        
        while (pos >= 0 and basename[pos] >= '0' and basename[pos] <= '9')
            --pos;
        
        if (pos >= 2 and basename[pos] == 'v' and basename[pos - 1] == '_')
            basename = basename.substr(0, pos - 1);
    }
    
    
    return basename;
}


void SlimTriggerResults::UpdateMenu(edm::TriggerNames const &triggerNames)
{
    // Reset all trigger buffers
    for (auto &t: triggers)
    {
        t.second.inMenu = false;
        t.second.index = 0;
        t.second.wasRun = false;
        t.second.accept = false;
        t.second.prescale = 0;
    }
    
    
    // Loop over names of all triggers in the menu
    for (unsigned i = 0; i < triggerNames.size(); ++i)
    {
        // Update the full trigger name in the associated TriggerState object if the
        //current trigger's name is among the names of selected triggers
        auto res = triggers.find(GetTriggerBasename(triggerNames.triggerName(i)));
        
        if (res != triggers.end())
        {
            res->second.inMenu = true;
            res->second.index = i;
        }
    }
}


DEFINE_FWK_MODULE(SlimTriggerResults);
