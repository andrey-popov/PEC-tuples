#include "EventIDFilter.h"

#include <FWCore/ParameterSet/interface/FileInPath.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>


using namespace edm;
using namespace std;


EventIDFilter::EventIDFilter(ParameterSet const &cfg):
    rejectKnownEvents(cfg.getParameter<bool>("rejectKnownEvents"))
{
    // Check the type of the input file with collection of event IDs and read it
    string const eventListFileName(cfg.getParameter<string>("eventListFile"));
    
    if (boost::ends_with(eventListFileName, ".txt"))
        ReadTextFile(eventListFileName);
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
    
    
    // Read IDs from the file
    string line;
    istringstream ist;
    
    while (true)
    {
        getline(eventListFile, line);
        
        if (eventListFile.eof() or line.length() == 0)
            break;
        
        
        ist.clear();
        ist.str(line);  // should contain three numbers separated by semicolon
        
        unsigned long run, lumiSection, event;
        ist >> run;
        ist.ignore();
        ist >> lumiSection;
        ist.ignore();
        ist >> event;
        
        
        knownEvents.emplace_back(run, lumiSection, event);
    }
    
    
    eventListFile.close();
}


DEFINE_FWK_MODULE(EventIDFilter);
