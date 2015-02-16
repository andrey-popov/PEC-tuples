#include <UserCode/SingleTop/plugins/PATCandViewCountMultiFilter.h>

#include <FWCore/Framework/interface/MakerMacros.h>




PATCandViewCountMultiFilter::PATCandViewCountMultiFilter(edm::ParameterSet const &cfg):
    selection(cfg.getParameter<std::string>("cut")),
    minNumber(cfg.getParameter<unsigned>("minNumber")),
    maxNumber(cfg.getParameter<unsigned>("maxNumber"))
{
    for (edm::InputTag const &tag: cfg.getParameter<std::vector<edm::InputTag>>("src"))
        sourceTokens.emplace_back(consumes<edm::View<reco::Candidate>>(tag));
}


bool PATCandViewCountMultiFilter::filter(edm::Event &event, edm::EventSetup const &eventSetup)
{
    // Loop over the input collections
    for (auto const &sourceToken: sourceTokens)
    {
        // Get the collection
        edm::Handle<edm::View<reco::Candidate>> collection;
        event.getByToken(sourceToken, collection);
        
        
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
