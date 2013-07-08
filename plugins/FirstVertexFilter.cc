/**
 * @author Andrey.Popov@cern.ch
 *
 * Description: see the header file.
 */

#include <UserCode/SingleTop/interface/FirstVertexFilter.h>

#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <string>
#include <memory>


FirstVertexFilter::FirstVertexFilter(const edm::ParameterSet &cfg):
    src(cfg.getParameter<edm::InputTag>("src")),
    cut(cfg.getParameter<std::string>("cut"))
{
    produces<reco::VertexCollection>();
}


FirstVertexFilter::~FirstVertexFilter()
{}


void FirstVertexFilter::beginJob()
{}


void FirstVertexFilter::endJob()
{}


bool FirstVertexFilter::filter(edm::Event &iEvent, const edm::EventSetup &iSetup)
{
    edm::Handle<reco::VertexCollection> vertices;
    iEvent.getByLabel(src, vertices);
    
    // Details on string-based selectors can be found in SWGuidePhysicsCutParser
    StringCutObjectSelector<reco::Vertex> selector(cut);
    std::auto_ptr<reco::VertexCollection> selectedVertices(new reco::VertexCollection);
    
    for (reco::VertexCollection::const_iterator v = vertices->begin(); v != vertices->end(); ++v)
        if (selector(*v))
            selectedVertices->push_back(*v);
    
    iEvent.put(selectedVertices);
    
    return selector(vertices->front());
}


DEFINE_FWK_MODULE(FirstVertexFilter);
