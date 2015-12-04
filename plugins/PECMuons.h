#pragma once

#include <Analysis/PECTuples/interface/Muon.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <vector>


/**
 * \class PECMuons
 * \brief Stores muons
 * 
 * The plugin stores basic properties of muons in the given collection. It saves their four-momenta,
 * isolation, quality flags, etc. The mass in the four-momentum is always set to zero
 * to facilitate file compression. Bit flags of stored objects include the flag for tight muon
 * according to the official definition and results of custom selections specifed by the user.
 */
class PECMuons: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * Reads the configuration of the plugin and create string-based selectors.
     */
    PECMuons(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output tree
    virtual void beginJob() override;
    
    /**
     * \brief Analyses current event
     * 
     * Copies muons into the buffers, evaluates string-based selections, fills the output tree.
     */
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
private:
    /// Source collection of muons
    edm::EDGetTokenT<edm::View<pat::Muon>> muonToken;
    
    /**
     * \brief String-based selections for muons
     * 
     * These selections do not affect which muons are stored in the output files. Instead, each
     * string defines a selection that is evaluated and whose result is saved in the bit field of
     * the CandidateWithID class.
     * 
     * Details on implementation are documented in [1].
     * [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePhysicsCutParser
     */
    std::vector<StringCutObjectSelector<pat::Muon>> muSelectors;
    
    /// Collection of reconstructed primary vertices
    edm::EDGetTokenT<reco::VertexCollection> primaryVerticesToken;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /// Output tree
    TTree *outTree;
    
    /// Buffer to store muons
    std::vector<pec::Muon> storeMuons;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Muon> *storeMuonsPointer;
};
