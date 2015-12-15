#include "EventIDFilter.h"

#include <FWCore/ParameterSet/interface/FileInPath.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <TFile.h>
#include <TTree.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <fstream>
#include <memory>
#include <list>
#include <utility>


using namespace edm;
using namespace std;


EventIDFilter::EventIDFilter(ParameterSet const &cfg):
    rejectKnownEvents(cfg.getParameter<bool>("rejectKnownEvents"))
{
    // Check the type of the input file with collection of event IDs and read it
    string const eventListFileName(cfg.getParameter<string>("eventListFile"));
    
    if (boost::ends_with(eventListFileName, ".txt"))
        ReadTextFile(eventListFileName);
    else if (boost::ends_with(eventListFileName, ".root"))
        ReadROOTFile(eventListFileName);
    else
    {
        Exception excp(errors::LogicError);
        excp << "Judging from the extension, format of the file \"" << eventListFileName <<
         "\" is not supported.\n";
        excp.raise();
    }
    
    
    // Sort the collection of event IDs read from the file. This is needed to improve the
    //performance of the lookup
    sort(knownEvents.begin(), knownEvents.end());
}


void EventIDFilter::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<string>("eventListFile")->
     setComment("Name of a text file containing a list of events.");
    desc.add<bool>("rejectKnownEvents", false)->
     setComment("Determines whether a known event is kept or rejected.");
    
    descriptions.add("eventIDFilter", desc);
}


bool EventIDFilter::filter(Event &event, EventSetup const &)
{
    // Check if ID of the current event is present in the collection. Profit from the fact that the
    //collection is sorted
    bool const eventKnown = binary_search(knownEvents.begin(), knownEvents.end(), event.id());
    
    
    return rejectKnownEvents xor eventKnown;
}


void EventIDFilter::ReadTextFile(string const &fileName)
{
    // Open the given text file with event IDs
    ifstream eventListFile(FileInPath(fileName).fullPath());
    
    if (not eventListFile.good())
    {
        Exception excp(errors::FileOpenError);
        excp << "Cannot open file \"" << fileName << "\".\n";
        excp.raise();
    }
    
    
    // Read IDs from the file. Lines are parsed with the help of a regular expression
    string line;
    boost::regex eventIDRegex("^(\\d+):(\\d+):(\\d+)$", boost::regex::extended);
    boost::smatch matchResults;
    
    while (true)
    {
        // Read the next line from the input file
        getline(eventListFile, line);
        
        if (eventListFile.eof() or line.length() == 0)
            break;
        
        
        // Extract the event ID from the line and add it to the collection
        if (not boost::regex_match(line, matchResults, eventIDRegex))
        {
            Exception excp(errors::LogicError);
            excp << "Failed to parse line\n  \"" << line << "\"\nof input file \"" << fileName <<
             "\". The file format seems to be wrong.\n";
            excp.raise();
        }
        
        unsigned long const run = stol(matchResults[1]);
        unsigned long const lumiSection = stol(matchResults[2]);
        unsigned long long const event = stoll(matchResults[3]);
        
        knownEvents.emplace_back(run, lumiSection, event);
    }
    
    
    eventListFile.close();
}


void EventIDFilter::ReadROOTFile(string const &fileName)
{
    // Open the ROOT file with event IDs
    unique_ptr<TFile> eventListFile(TFile::Open(FileInPath(fileName).fullPath().c_str()));
    
    if (eventListFile->IsZombie())
    {
        Exception excp(errors::FileOpenError);
        excp << "Cannot open file \"" << fileName << "\".\n";
        excp.raise();
    }
    
    
    // Get the tree with event IDs
    string const treeName("EventID");
    unique_ptr<TTree> eventListTree(dynamic_cast<TTree *>(eventListFile->Get(treeName.c_str())));
    
    if (not eventListTree)
    {
        Exception excp(errors::LogicError);
        excp << "Input file \"" << fileName << "\" does not follow expected format. Cannot find " <<
         "tree \"" << treeName << "\".\n";
        excp.raise();
    }
    
    
    // Make sure expected branches are present and contain data of proper types
    list<string> missingBranches, wrongTypeBranches;
    
    for (auto const &b: initializer_list<pair<string, string>>{make_pair("run"s, "i"s),
     make_pair("lumi"s, "i"), make_pair("event"s, "l"s)})
    {
        auto const &branchName = b.first;
        auto const &branchType = b.second;
        
        TBranch *branch = eventListTree->GetBranch(branchName.c_str());
        
        if (not branch)
            missingBranches.emplace_back(branchName);
        else
        {
            if (not ((branchName + "/" + branchType) == branch->GetTitle()))
                wrongTypeBranches.emplace_back(branchName);
        }
    }
    
    if (missingBranches.size() > 0)
    {
        Exception excp(errors::LogicError);
        auto bIt = missingBranches.begin();
        excp << "In the tree \"" << treeName << "\" in input file \"" << fileName <<
         "\", following branches are missing: \"" << *bIt << "\"";
        
        for (++bIt; bIt != missingBranches.end(); ++bIt)
            excp << ", \"" << *bIt << "\"";
        
        excp << ".\n";
        excp.raise();
    }
    
    if (wrongTypeBranches.size() > 0)
    {
        Exception excp(errors::LogicError);
        auto bIt = wrongTypeBranches.begin();
        excp << "In the tree \"" << treeName << "\" in input file \"" << fileName <<
         "\", following branches have wrong types: \"" << *bIt << "\"";
        
        for (++bIt; bIt != wrongTypeBranches.end(); ++bIt)
            excp << ", \"" << *bIt << "\"";
        
        excp << ".\n";
        excp.raise();
    }
    
    
    // Set up buffers to read the tree
    UInt_t run, lumiSection;
    ULong64_t event;
    
    eventListTree->SetBranchAddress("run", &run);
    eventListTree->SetBranchAddress("lumi", &lumiSection);
    eventListTree->SetBranchAddress("event", &event);
    
    
    // Finally, read event IDs from the tree
    unsigned long const nEntries = eventListTree->GetEntries();
    
    for (unsigned long ev = 0; ev < nEntries; ++ev)
    {
        eventListTree->GetEntry(ev);
        knownEvents.emplace_back(run, lumiSection, event);
    }
}


DEFINE_FWK_MODULE(EventIDFilter);
