#pragma once

#include <Analysis/PECTuples/interface/PileUpInfo.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/VertexReco/interface/VertexFwd.h>
#include <SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <vector>


/**
 * \class PECPileUp
 * \brief Stores information related to pile-up
 * 
 * Main properties are the number of primary vertices and the density rho. In case of simulation,
 * the number of additional pp collisions is also stored.
 */
class PECPileUp: public edm::EDAnalyzer
{
public:
    /// Constructor
    PECPileUp(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output tree
    virtual void beginJob() override;
    
    /// Writes information about pile-up into buffers and fills the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &) override;
    
private:
    /// Collection of reconstructed primary vertices
    edm::EDGetTokenT<reco::VertexCollection> primaryVerticesToken;
    
    /// Rho (mean angular pt density)
    edm::EDGetTokenT<double> rhoToken;
    
    /// Indicates whether an event is data or simulation
    bool const runOnData;
    
    /**
     * \brief Pile-up information in simulation
     * 
     * Not read in case of data.
     */
    edm::EDGetTokenT<edm::View<PileupSummaryInfo>> puSummaryToken;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /// Output tree
    TTree *outTree;
    
    /**
     * \brief Buffer to store pile-up information
     * 
     * In this object, fields that correspond to simulation truth are not filled when running over
     * data.
     */
    pec::PileUpInfo puInfo;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    pec::PileUpInfo *puInfoPointer;
};
