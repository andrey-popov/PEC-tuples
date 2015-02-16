#include "FirstVertexFilter.h"

#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <FWCore/Framework/interface/MakerMacros.h>

#include <string>
#include <memory>


using namespace std;


FirstVertexFilter::FirstVertexFilter(const edm::ParameterSet &cfg):
    cut(cfg.getParameter<std::string>("cut"))
{
    verticesToken = consumes<reco::VertexCollection>(cfg.getParameter<edm::InputTag>("src"));
    
    produces<reco::VertexCollection>();
}


void FirstVertexFilter::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src")->setComment("Source collection of vertices.");
    desc.add<string>("cut")->setComment("Selection to apply to the vertices.");
    
    descriptions.add("firstVertexFilter", desc);
}


bool FirstVertexFilter::filter(edm::Event &event, const edm::EventSetup &eventSetup)
{
    edm::Handle<reco::VertexCollection> vertices;
    event.getByToken(verticesToken, vertices);
    
    // Details on string-based selectors can be found in SWGuidePhysicsCutParser
    StringCutObjectSelector<reco::Vertex> selector(cut);
    std::unique_ptr<reco::VertexCollection> selectedVertices(new reco::VertexCollection);
    
    for (reco::VertexCollection::const_iterator v = vertices->begin(); v != vertices->end(); ++v)
        if (selector(*v))
            selectedVertices->push_back(*v);
    
    event.put(move(selectedVertices));
    
    return selector(vertices->front());
}


DEFINE_FWK_MODULE(FirstVertexFilter);
