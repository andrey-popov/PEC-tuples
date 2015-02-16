#pragma once

#include <UserCode/SingleTop/interface/GenParticle.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/HepMCCandidate/interface/GenParticle.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <vector>


/**
 * \class HardInteractionInfo
 * \author Andrey Popov
 * \brief Stores particles from the hard interaction in a ROOT file
 * 
 * Particles from the hard interaction are identified with their status, which is hard-coded to the
 * value used in Pythia 6.
 */
class HardInteractionInfo: public edm::EDAnalyzer
{
public:
    /// Constructor
    HardInteractionInfo(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates the output tree
    virtual void beginJob();
    
    /// Reads the event and stores the relevant information in the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &setup);
    
private:
    /// Collection of generator-level particles
    edm::EDGetTokenT<edm::View<reco::GenParticle>> genParticlesToken;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /// Tree to be written in the output ROOT file
    TTree *outTree;
    
    /// Trimmed generator-level particles to be stored in the output file
    std::vector<pec::GenParticle> storeParticles;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::GenParticle> *storeParticlesPointer;
};
