#pragma once

#include <Analysis/PECTuples/interface/Electron.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/PatCandidates/interface/Electron.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \class PECElectrons
 * \brief Stores electrons
 * 
 * The plugin stores basic properties of electrons in the given collection. It saves their
 * four-momenta, isolation, quality flags, etc. The mass in the four-momentum is always set to zero
 * to facilitate file compression. Bit field inherited from CandidateWithID includes decision of a
 * conversion rejection algorithm and results of custom selections specifed by the user.
 * 
 * The plugin can store various IDs in a flexible way. It can store a variable number of boolean and
 * real-valued decisions embedded in pat::Electron, accessing them via labels provided in the
 * configuration. In addition, it can include boolean and real-valued decisions provided in the form
 * of value maps. All IDs are optional.
 */
class PECElectrons: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * Reads the configuration of the plugin and create string-based selectors.
     */
    PECElectrons(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output tree
    virtual void beginJob() override;
    
    /**
     * \brief Analyses current event
     * 
     * Copies electrons into the buffers, evaluates string-based selections, fills the output tree.
     */
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
private:
    /// Source collection of electrons
    edm::EDGetTokenT<edm::View<pat::Electron>> electronToken;
    
    /// Names of embedded boolean IDs to be saved
    std::vector<std::string> embeddedBoolIDLabels;
    
    /// Maps with additional boolean IDs
    std::vector<edm::EDGetTokenT<edm::ValueMap<bool>>> boolIDMapTokens;
    
    /// Names of embedded real-valued IDs to be saved
    std::vector<std::string> embeddedContIDLabels;
    
    /// Maps additional real-valued IDs
    std::vector<edm::EDGetTokenT<edm::ValueMap<float>>> contIDMapTokens;
    
    /**
     * \brief String-based selections
     * 
     * These selections do not affect which electrons are stored in the output files. Instead, each
     * string defines a selection that is evaluated and whose result is saved in the bit field of
     * the CandidateWithID class.
     * 
     * Details on implementation are documented in [1].
     * [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePhysicsCutParser
     */
    std::vector<StringCutObjectSelector<pat::Electron>> eleSelectors;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /// Output tree
    TTree *outTree;
    
    /// Buffer to store electrons
    std::vector<pec::Electron> storeElectrons;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Electron> *storeElectronsPointer;
};
