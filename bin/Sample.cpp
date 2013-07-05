#include "Sample.hpp"
 
#include <iostream>
#include <stdexcept>


using namespace std;


Sample::Sample(string const &fileName_):
    fileName(fileName_),
    srcFile(nullptr), srcTree(nullptr)
{}


Sample::~Sample()
{
    delete srcTree;
    delete srcFile;
}


void Sample::Open()
{
    if (srcFile not_eq nullptr)
        throw runtime_error("Sample::Open: Trying to open the file for the second time.");
    
    
    // Open the source file
    srcFile = TFile::Open(fileName.c_str());
    
    // Get the required trees from the file and assign the buffers to read the branches. If several
    //trees are to be read, add them with srcTree->AddFriend
    srcTree = dynamic_cast<TTree *>(srcFile->Get("eventContent/PUInfo"));
    srcTree->SetBranchAddress("PUTrueNumInteractions", &PUTrueNumInteractions);
    
    // Set the event counters
    curEvent = 0;
    nEvents = srcTree->GetEntries();
}


bool Sample::NextEvent() const
{
    if (curEvent == nEvents)  // no more events in the file
        return false;
    
    
    srcTree->GetEntry(curEvent);
    ++curEvent;
    return true;
}


string Sample::GetShortName() const
{
    size_t const idxLastSlash = fileName.rfind('/');
    size_t const idxLastPeriod = fileName.rfind('.');
    
    return fileName.substr(idxLastSlash + 1, idxLastPeriod - idxLastSlash - 1);
    //^ If works if there are no slash in the file name, but it will produce an incorrect result if
    //the period is not found
}


string const &Sample::GetFileName() const
{
    return fileName;
}


Float_t Sample::GetTrueNumPUInteractions() const
{
    return PUTrueNumInteractions;
}