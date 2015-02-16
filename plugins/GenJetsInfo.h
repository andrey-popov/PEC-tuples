#pragma once

#include <UserCode/SingleTop/interface/GenJet.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <DataFormats/JetReco/interface/GenJet.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <vector>


/**
 * \class GenJetsInfo
 * \author Andrey Popov
 * \brief A CMSSW plugin to save generator-level jets
 * 
 * Saves generator-level jets in a ROOT file. In the default configuration the plugin stores only
 * their four momenta. If the flag saveFlavourCounters is set to true, it saves additionally the
 * numbers of hadrons with b or c quarks among ancestors of jet's constituents (as was done in
 * AN-2012/251).
 */
class GenJetsInfo: public edm::EDAnalyzer
{
public:
    /// Constructor from a configuration fragment
    GenJetsInfo(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates the output tree and assigns it branches
    void beginJob();
    
    /// Fills the output tree with generator-level jets
    void analyze(edm::Event const &event, edm::EventSetup const &setup);
    
private:
    /// Collection of generator-level jets
    edm::EDGetTokenT<edm::View<reco::GenJet>> jetToken;
    
    /**
     * \brief String defining a selection for the jets
     * 
     * If the string is empty, all jets are saved.
     */
    std::string const jetCut;
    
    /// Indicates whether the plugin should store information on flavours of jet constituents
    bool const saveFlavourCounters;
    
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
};
