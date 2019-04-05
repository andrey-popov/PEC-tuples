#include "EventWeights.h"

#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/Utilities/interface/Exception.h>


EventWeights::EventWeights(edm::ParameterSet const &cfg):
    outTree(nullptr)
{
    auto const tags = cfg.getParameter<std::vector<edm::InputTag>>("sources");

    for (auto const &tag: tags)
        weightInfos.emplace_back(consumes<decltype(weightInfos)::value_type::value_type>(tag));


    auto const branchNames = cfg.getParameter<std::vector<std::string>>("storeNames");

    if (branchNames.empty())
    {
        for (unsigned i = 0; i < weightInfos.size(); ++i)
        {
            auto &branchName = weightInfos[i].branchName;
            branchName = tags[i].label();

            if (not tags[i].instance().empty())
                branchName += "_" + tags[i].instance();
        }
    }
    else
    {
        if (branchNames.size() != tags.size())
        {
            cms::Exception excp("Configuration");
            excp << "Number of given input tags (" << tags.size() <<
              ") does not match the number " << "of names for branches (" <<
              branchNames.size() << ").";
            excp.raise();
        }

        for (unsigned i = 0; i < weightInfos.size(); ++i)
            weightInfos[i].branchName = branchNames[i];
    }
}


void EventWeights::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<std::vector<edm::InputTag>>("sources")->setComment("Plugins that produce weights.");
    desc.add<std::vector<std::string>>("storeNames", std::vector<std::string>())->
      setComment("(Optional) names for output branches.");
    
    descriptions.add("eventWeights", desc);
}


void EventWeights::analyze(edm::Event const &event, edm::EventSetup const &)
{
    for (auto &weightInfo: weightInfos)
        weightInfo.Read(event);

    outTree->Fill();
}


void EventWeights::beginJob()
{
    outTree = fileService->make<TTree>("EventWeights", "Additional event weights");

    for (auto &weightInfo: weightInfos)
        outTree->Branch(weightInfo.branchName.c_str(), &weightInfo.value);
}


DEFINE_FWK_MODULE(EventWeights);

