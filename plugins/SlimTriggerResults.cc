#include <UserCode/SingleTop/plugins/SlimTriggerResults.h>

#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Common/interface/TriggerResultsByName.h>
#include <DataFormats/Common/interface/HLTPathStatus.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <boost/algorithm/string/predicate.hpp>

#include <vector>


using namespace std;


TriggerState::TriggerState():
    wasRun(false),
    accept(false),
    prescale(0)
{}


SlimTriggerResults::SlimTriggerResults(edm::ParameterSet const &cfg):
    triggerProcessName(cfg.getParameter<string>("triggerProcessName")),
    filterOn(cfg.getParameter<bool>("filter"))
{
    // Loop over the names of triggers the user has provided
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
        triggerTree->Branch((t.first + "__prescale").c_str(), &t.second.prescale);
    }
}


bool SlimTriggerResults::beginRun(edm::Run &run, edm::EventSetup const &setup)
{
    bool menuChanged = false;
    
    if (hltConfigProvider.init(run, setup, triggerProcessName, menuChanged))
    {
        // Check if the trigger menu has changed. Note it can happen only with a new run
        if (menuChanged)
        {
            // Reset all the trigger buffers
            for (auto &t: triggers)
            {
                t.second.fullName = "";
                t.second.wasRun = false;
                t.second.accept = false;
                t.second.prescale = 0;
            }
            
            // Loop over names of all triggers in the menu
            for (string const &actualName: hltConfigProvider.triggerNames())
            {
                // Update the full trigger name in the associated TriggerState object if the
                //current trigger's name is among the names of selected triggers
                auto res = triggers.find(GetTriggerBasename(actualName));
                
                if (res != triggers.end())
                    res->second.fullName = actualName;
            }
        }
    }
    else
    {
        edm::Exception excp(edm::errors::Unknown);
        excp << "HLTConfigProvider::init terminated with an error\n";
        excp.raise();
    }
    
    
    return true;
}


bool SlimTriggerResults::filter(edm::Event &event, edm::EventSetup const &setup)
{
    // Read the trigger results for the current event
    edm::TriggerResultsByName resultsByName(event.triggerResultsByName(triggerProcessName));
    
    
    bool result = false;  // will contain logical OR of all the selected triggers
    
    
    // Fill buffers for all the selected triggers
    for (auto &t: triggers)
    {
        // Continue to the next trigger if the current one is not in the current menu
        if (t.second.fullName.length() == 0)
            continue;
        
        
        // Update state of the current trigger
        auto const &pathStatus = resultsByName[t.second.fullName];
        t.second.wasRun = pathStatus.wasrun();
        t.second.accept = pathStatus.accept();
        t.second.prescale = hltConfigProvider.prescaleValue(event, setup, t.second.fullName);
        
        
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
    desc.add<string>("triggerProcessName", "HLT")->
     setComment("Name of the process in which trigger decisions were evaluated.");
    
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


DEFINE_FWK_MODULE(SlimTriggerResults);
