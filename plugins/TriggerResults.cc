#include "TriggerResults.h"

#include <FWCore/Common/interface/TriggerResultsByName.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <TObjString.h>



TriggerResults::TriggerResults(const edm::ParameterSet &cfg):
    triggerProcessName(cfg.exists("triggerProcessName") ?
     cfg.getParameter<string>("triggerProcessName") : "HLT"),
    whiteList(cfg.exists("whiteList") ?
     cfg.getParameter<vector<string> >("whiteList") : vector<string>()),
    blackList(cfg.exists("blackList") ?
     cfg.getParameter<vector<string> >("blackList") : vector<string>()),
    orMode(cfg.exists("orMode") ? cfg.getParameter<bool>("orMode") : true),
    filterOn(cfg.exists("filter") ? cfg.getParameter<bool>("filter") : true),
    dumperOn(cfg.exists("dumper") ? cfg.getParameter<bool>("dumper") : false),
    whiteListRegex(whiteList.size()),
    blackListRegex(blackList.size()),
    names("TObjString"),
    namesPointer(&names)
{
    // Create the regular expressions
    for (unsigned i = 0; i < whiteList.size(); ++i)
        whiteListRegex[i].assign(whiteList[i]);
    
    for (unsigned i = 0; i < blackList.size(); ++i)
        blackListRegex[i].assign(blackList[i]);
}


TriggerResults::~TriggerResults()
{}


void TriggerResults::beginJob()
{
    // Create the tree and add the branches if the dumper mode is on
    if (dumperOn)
    {
        triggerInfoTree = fs->make<TTree>("TriggerInfo",
         "Tree to store the trigger information");
        
        triggerInfoTree->Branch("size", &size);
        triggerInfoTree->Branch("names", "TClonesArray", &namesPointer, 32000, 0);
        //triggerInfoTree->Branch("wasRun", wasRun, "wasRun[size]/O");
        triggerInfoTree->Branch("hasFired", hasFired, "hasFired[size]/O");
        triggerInfoTree->Branch("prescale", prescale, "prescale[size]/I");
    }
}


void TriggerResults::endJob()
{}


bool TriggerResults::beginRun(edm::Run &run, const edm::EventSetup &eventSetup)
{
    // Initialize the HLTConfigProvider and check the HLT configuration
    bool changed = false;
    
    if (hltConfigProvider.init(run, eventSetup, triggerProcessName, changed))
    {
        if (changed)
        // The trigger tables has changed. Rebuild the list of the matched triggers
        {
            // Names of all triggers in the table
            vector<string> triggerNames(hltConfigProvider.triggerNames());
            
            // Reset the vector of selected triggers
            selectedTriggers.clear();
            
            // Loop through the trigger names
            for (vector<string>::const_iterator n = triggerNames.begin();
             n != triggerNames.end(); ++n)
            {
                bool foundInWhiteList = matchTriggerName(*n, whiteListRegex);
                bool foundInBlackList = matchTriggerName(*n, blackListRegex);
                
                if ((!orMode  &&  foundInWhiteList  &&  !foundInBlackList)  ||
                 (orMode  &&  (foundInWhiteList  ||  !foundInBlackList)))
                    selectedTriggers.push_back(*n);
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


bool TriggerResults::endRun(edm::Run &run, const edm::EventSetup &eventSetup)
{
    return true;
}


bool TriggerResults::filter(edm::Event &event, const edm::EventSetup &eventSetup)
{
    // Read the trigger results from the event
    edm::TriggerResultsByName resultsByName =
     event.triggerResultsByName(triggerProcessName.c_str());
    
    bool result = false;
    size = 0;
    
    // Loop through the names of the selected triggers
    for (vector<string>::const_iterator n = selectedTriggers.begin();
     n != selectedTriggers.end(); ++n)
    {
        // Get the information about the trigger
        bool run = resultsByName.wasrun(*n);
        bool fired = resultsByName.accept(*n);
        
        // Question the decision for filtering
        if (filterOn  &&  run  &&  fired)
        {
            result = true;
            
            // If we do not fill the ROOT tree, that's all we need
            if (!dumperOn)
                return result;
        }
        
        if (dumperOn)
        {
            new (names[size]) TObjString(n->c_str());
            
            if (size < MAX_LEN)
            {
                //wasRun[size] = run;
                hasFired[size] = fired;
                prescale[size] = hltConfigProvider.prescaleValue(event, eventSetup, *n);
                
                ++size;
            }
        }
    }
    
    
    // Fill the tree. If filtering is on the tree is filled for the survided events only
    if (dumperOn  &&  (!filterOn  ||  result))
        triggerInfoTree->Fill();
    
    // Reset the TClonesArray
    names.Clear("C");
    
    
    // Return the filter decision
    return (filterOn) ? result : true;
}


bool TriggerResults::matchTriggerName(const string &name, const vector<boost::regex> &patterns)
{
    // Treat the empty pattern vector in a special way: it matches everything
    if (patterns.size() == 0)
        return true;
    
    // Check all the patterns
    for (vector<boost::regex>::const_iterator r = patterns.begin(); r != patterns.end(); ++r)
        if (regex_search(name, *r))
            return true;
    
    
    return false;  // if we reached this, no match was found
}


DEFINE_FWK_MODULE(TriggerResults);
