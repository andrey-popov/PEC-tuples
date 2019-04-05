#pragma once

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \brief A plugin to store event weights from EDM event
 *
 * The configuration must provide a vector of input tags that identify the weights to be stored
 * (which must be of type double). Names for the corresponding branches in the output tree can also
 * be provided. If not, they are constructed from the input tags.
 */
class EventWeights: public edm::EDAnalyzer
{
private:
    /// Auxiliary class to aggregate details about a single weight
    template <typename T>
    struct WeightInfo
    {
        using value_type = T;

        WeightInfo(edm::EDGetTokenT<T> &&token);

        /// Read the value of the weight from the given event
        void Read(edm::Event const &event);

        /// Token to read the weight from the event
        edm::EDGetTokenT<T> token;

        /// Name for the branch in which this weight will be stored
        std::string branchName;

        /// Buffer to read the value of the weight into
        T value;
    };

public:
    EventWeights(edm::ParameterSet const &cfg);

public:
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    void analyze(edm::Event const &event, edm::EventSetup const &) override;
    void beginJob() override;

private:
    /// Details about weights to be saved
    std::vector<WeightInfo<double>> weightInfos;
    
    /// An object to handle output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief Output tree
     *
     * Managed by the fileService object.
     */
    TTree *outTree;
};


template <typename T>
EventWeights::WeightInfo<T>::WeightInfo(edm::EDGetTokenT<T> &&token_):
    token(token_)
{}


template <typename T>
void EventWeights::WeightInfo<T>::Read(edm::Event const &event)
{
    edm::Handle<double> handle;
    event.getByToken(token, handle);
    value = *handle;
}

