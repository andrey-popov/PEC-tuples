#pragma once

#include <Analysis/PECTuples/interface/GenJet.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/JetReco/interface/GenJet.h>
#include <DataFormats/PatCandidates/interface/MET.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <vector>


/**
 * \class PECGenJetMET
 * \brief A CMSSW plugin to save generator-level jets and MET
 * 
 * Saves generator-level jets in a ROOT file. In the default configuration the plugin stores only
 * their four momenta. If the flag saveFlavourCounters is set to true, it saves additionally the
 * numbers of hadrons with b or c quarks among ancestors of jet's constituents (as it was done in
 * AN-2012/251). In the default configuration each hadron is counted only once; it the case of
 * ambiguity (when its decay products are shared among several jets), it is assigned to the harder
 * jet. This behaviour can be switched off by setting the flag noDoubleCounting to false, which is
 * usually not recommended however [1].
 * [1] https://github.com/andrey-popov/single-top/issues/49
 * 
 * In an optional input tag for reconstructed (sic!) MET is provided, the corresponding
 * generator-level MET is also stored.
 */
class PECGenJetMET: public edm::EDAnalyzer
{
public:
    /// Constructor from a configuration fragment
    PECGenJetMET(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates the output tree and assigns it branches
    void beginJob() override;
    
    /// Fills the output tree with generator-level jets
    void analyze(edm::Event const &event, edm::EventSetup const &setup) override;
    
private:
    /// Collection of generator-level jets
    edm::EDGetTokenT<edm::View<reco::GenJet>> jetToken;
    
    /// MET
    edm::EDGetTokenT<edm::View<pat::MET>> metToken;
    
    /**
     * \brief Selector to filter jets
     * 
     * If the string is empty, all jets are saved.
     */
    StringCutObjectSelector<reco::Candidate> const jetSelector;
    
    /// Indicates whether the plugin should store information on flavours of jet constituents
    bool const saveFlavourCounters;
    
    /**
     * \brief Indicates if same hadron can be counted in more than one jet
     * 
     * The flag is ignored if saveFlavourCounters is false.
     */
    bool const noDoubleCounting;
    
    /// Indicates whether an input tag for MET is provided in the configuration
    bool metGiven;
    
    
    /// A service to write to ROOT files
    edm::Service<TFileService> fs;
    
    
    /// The output tree (owned by the TFile service)
    TTree *tree;
    
    /**
     * \brief Trimmed generator-level jets to be stored in the output file
     * 
     * Depending on the flag saveFlavourCounters, the jets will or will not contain information on
     * the numbers of B and C hadrons inside them.
     */
    std::vector<pec::GenJet> storeJets;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::GenJet> *storeJetsPointer;
    
    /**
     * \brief Buffer to store MET
     * 
     * Although only a single generator-level MET is stored in each event, a vector is used for the
     * sake of uniformity with the PECJetMET plugin. MET is  stored as an instance of
     * pec::Candidate, but pseudorapidity and mass are set to zeros, which allows them to be
     * compressed efficiently. The buffer is filled if only an tag to access MET is provided in the
     * configuration.
     */
    std::vector<pec::Candidate> storeMETs;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Candidate> *storeMETsPointer;
};
