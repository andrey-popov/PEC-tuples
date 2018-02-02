#include "PECTriggerObjects.h"

#include <FWCore/Framework/interface/MakerMacros.h>


PECTriggerObjects::FilterBuffer::FilterBuffer(std::string const &name_):
    name(name_),
    objectsPointer(&objects)
{}


PECTriggerObjects::PECTriggerObjects(edm::ParameterSet const &cfg)
{
    triggerObjectsToken = consumes<edm::View<pat::TriggerObjectStandAlone>>(
      cfg.getParameter<edm::InputTag>("triggerObjects"));
    triggerResToken =
      consumes<edm::TriggerResults>(cfg.getParameter<edm::InputTag>("triggerResults"));
    
    
    auto const &filterNames = cfg.getParameter<std::vector<std::string>>("filters");
    buffers.reserve(filterNames.size());
    //^ This is important in order to set up pointers in FilterBuffer properly
    
    for (auto const filterName: filterNames)
        buffers.emplace_back(filterName);
}


void PECTriggerObjects::analyze(edm::Event const &event, edm::EventSetup const &)
{
    for (auto &buffer: buffers)
        buffer.objects.clear();
    
    
    edm::Handle<edm::View<pat::TriggerObjectStandAlone>> triggerObjects;
    event.getByToken(triggerObjectsToken, triggerObjects);
    
    edm::Handle<edm::TriggerResults> triggerRes;
    event.getByToken(triggerResToken, triggerRes);
    
    for (auto const &obj: *triggerObjects)
    {
        const_cast<pat::TriggerObjectStandAlone &>(obj).unpackFilterLabels(event, *triggerRes);
        
        pec::Candidate cand;
        cand.SetPt(obj.pt());
        cand.SetEta(obj.eta());
        cand.SetPhi(obj.phi());
        cand.SetM(obj.mass());
        
        for (auto &buffer: buffers)
        {
            if (obj.hasFilterLabel(buffer.name))
                buffer.objects.emplace_back(cand);
        }
    }
    
    outTree->Fill();
}


void PECTriggerObjects::beginJob()
{
    outTree = fileService->make<TTree>("TriggerObjects", "Trigger objects by filters");
    
    for (auto &buffer: buffers)
        outTree->Branch(buffer.name.c_str(), &buffer.objectsPointer);
}


void PECTriggerObjects::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("triggerResults", edm::InputTag("TriggerResults"))->
      setComment("Trigger results.");
    desc.add<edm::InputTag>("triggerObjects")->setComment("PAT trigger objects.");
    desc.add<std::vector<std::string>>("filters")->setComment("Filters to be stored.");
    descriptions.add("triggerObjects", desc);
}


DEFINE_FWK_MODULE(PECTriggerObjects);
