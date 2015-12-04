#include "PECEventID.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;


PECEventID::PECEventID(ParameterSet const &)
{}


void PECEventID::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    descriptions.add("eventID", desc);
    //^ The configuration is trivial
}


void PECEventID::beginJob()
{
    outTree = fileService->make<TTree>("EventID", "Event ID");
    
    eventIdPointer = &eventId;
    outTree->Branch("eventId", &eventIdPointer);
}


void PECEventID::analyze(edm::Event const &event, edm::EventSetup const &)
{
    // Reset the buffer from the previous event
    eventId.Reset();
    
    
    eventId.SetRunNumber(event.id().run());
    eventId.SetEventNumber(event.id().event());
    eventId.SetLumiSectionNumber(event.luminosityBlock());
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECEventID);
