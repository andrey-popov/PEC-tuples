#include <UserCode/SingleTop/plugins/PATCandViewCountMultiFilter.h>

#include <FWCore/Framework/interface/MakerMacros.h>




PATCandViewCountMultiFilter::PATCandViewCountMultiFilter(edm::ParameterSet const &cfg):
    sources(cfg.getParameter<std::vector<edm::InputTag>>("src")),
    selection(cfg.getParameter<std::string>("cut")),
    minNumber(cfg.getParameter<unsigned>("minNumber")),
    maxNumber(cfg.getParameter<unsigned>("maxNumber"))
{}


bool PATCandViewCountMultiFilter::filter(edm::Event &event, edm::EventSetup const &eventSetup)
{
    // Loop over the input collections
    for (edm::InputTag const &source: sources)
    {
        // Get the collection
        edm::Handle<edm::View<reco::Candidate>> collection;
        event.getByLabel(source, collection);
        
        
        // Loop over the collection and count how many candidates pass the selection
        unsigned nPassed = 0;
        
        for (auto const &candidate: *collection)
            if (selection(candidate))
                ++nPassed;
        
        
        // Check the number of selected candidates
        if (nPassed >= minNumber and nPassed <= maxNumber)
            return true;
    }
    
    
    // If the workflow reaches this point, no collection contains a suitable number of good
    //candidates
    return false;
}


void PATCandViewCountMultiFilter::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    
    desc.add<std::vector<edm::InputTag>>("src")->
     setComment("Input collections to be checked.");
    desc.add<std::string>("cut", "")->
     setComment("Selection to be applied to candidates.");
    desc.add<unsigned>("minNumber", 0)->
     setComment("Minimal allowed number of candidates that pass the selection.");
    desc.add<unsigned>("maxNumber", 9999)->
     setComment("Minimal allowed number of candidates that pass the selection.");
    
    descriptions.add("patCandViewCountMultiFilter", desc);
}


DEFINE_FWK_MODULE(PATCandViewCountMultiFilter);
