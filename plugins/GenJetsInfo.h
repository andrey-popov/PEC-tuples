/**
 * \file GenJetsInfo.h
 * \author Andrey Popov
 * 
 * The module defines a plugin to save generator-level jets into a plain ROOT tuple. In the default
 * configuration is stores only jet four momenta. If flag saveFlavourCounters is set to true, the
 * plugin saves additionally the number of status-2 b and c partons in the vicinity of the jet.
 * 
 * Usage example:
 *   process.genJets = cms.EDAnalyzer('GenJetsInfo',
 *       jets = cms.InputTag('ak5GenJets'),
 *       cut = cms.string('pt > 8.'),
 *       saveFlavourCounters = cms.bool(True),
 *       genParticles = cms.InputTag('genParticles'))
 * An empty cut (default one) means that all jets will be saved.
 */


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


/// A class to save generator-level jets
class GenJetsInfo: public edm::EDAnalyzer
{
public:
    /// Constructor from a configuration fragment
    GenJetsInfo(edm::ParameterSet const &cfg);

public:
    /// Creates the output tree and assigns it branches
    void beginJob();
    
    /// Fills the output tree with generator-level jets
    void analyze(edm::Event const &event, edm::EventSetup const &setup);
    
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    /// Input tag to identify the collection of generator-level jets
    edm::InputTag const jetSrc;
    
    /// String defining a selection for the jets
    std::string const jetCut;
    
    /// Indicates whether the plugin should store information on flavours of nearby partons
    bool const saveFlavourCounters;
    
    /**
     * \brief Input tag to identify the collection of generator-level particles
     * 
     * It is not used if saveFlavourCounters is set to false.
     */
    edm::InputTag const genParticleSrc;
    
    /// A service to write to ROOT files
    edm::Service<TFileService> fs;
    
    /// The output tree (owned by the TFile service)
    TTree *tree;
    
    /// Maximal size of buffer arrays used to preallocate them
    static unsigned const maxSize = 64;
    
    // Output buffers
    UChar_t jetSize;
    
    Float_t jetPt[maxSize];
    Float_t jetEta[maxSize];
    Float_t jetPhi[maxSize];
    Float_t jetMass[maxSize];
    
    // Number of b and c quark with status 2 in cone 0.5 around the jet
    UChar_t bMult[maxSize], cMult[maxSize];
};
