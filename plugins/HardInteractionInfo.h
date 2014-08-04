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


/**
 * \class HardInteractionInfo
 * \author Andrey Popov
 * \brief Stores particles from the hard interaction in a ROOT file
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
    /// A tag to access generator-level particles
    edm::InputTag const genParticlesTag;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /// Maximal size to allocate buffer arrays
    static unsigned const maxSize = 64;
    
    /// Tree to be written in the output ROOT file
    TTree *outTree;
    
    // Information about the hard interaction (status-3 particles). The beam particles (the protons)
    //are skipped
    UChar_t hardPartSize;  // number of the saved particles
    Char_t hardPartPdgId[maxSize];  // their PDG ID
    Char_t hardPartFirstMother[maxSize], hardPartLastMother[maxSize];  // indices of mothers
    Float_t hardPartPt[maxSize];    // four-momenta of the particles
    Float_t hardPartEta[maxSize];   //
    Float_t hardPartPhi[maxSize];   //
    Float_t hardPartMass[maxSize];  //
};
