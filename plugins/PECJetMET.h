#pragma once

#include <Analysis/PECTuples/interface/Jet.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/PatCandidates/interface/Jet.h>
#include <DataFormats/PatCandidates/interface/MET.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \class PECJetMET
 * \brief Stores reconstructed jets and MET
 * 
 * The plugin stores basic properties of jets (four-momenta, b-tagging discriminators, IDs, etc.)
 * and MET. Bit flags indicate the presence of a generator-level jet nearby and include decisions of
 * user-defined selectors. Fields with generator-level information are not filled when processing
 * data.
 */
class PECJetMET: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * Reads the configuration of the plugin and create string-based selectors.
     */
    PECJetMET(edm::ParameterSet const &cfg);
    
public:
    /// Verifies configuration of the plugin
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output tree
    virtual void beginJob() override;
    
    /**
     * \brief Analyses current event
     * 
     * Copies jets and MET into the buffers, evaluates string-based selections for jets, fills the
     * output tree.
     */
    virtual void analyze(edm::Event const &event, edm::EventSetup const &setup) override;
    
private:
    /// Collection of jets
    edm::EDGetTokenT<edm::View<pat::Jet>> jetToken;
    
    /// MET
    edm::EDGetTokenT<edm::View<pat::MET>> metToken;
    
    /// Minimal corrected transverse momentum to determine which jets are stored
    double const jetMinPt;
    
    /// Minimal raw transverse momentum to determine which jets are stored
    double const jetMinRawPt;
    
    /// A flag that indicates if corrected or raw momenta should be stored for jets
    bool const saveCorrectedJetMomenta;
    
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
    std::vector<StringCutObjectSelector<pat::Jet>> jetSelectors;
    
    /**
     * \brief Indicates whether an event is data or simulation
     * 
     * Determines if the plugin should read generator-level information for jets, such as flavour.
     */
    bool const runOnData;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /// Output tree
    TTree *outTree;
    
    /**
     * \brief Buffer to store jets
     * 
     * Bit flags in the objects show if the jet is matched to a generator-level jet (always set to
     * false in real data) and include user-defined selections. Generator-level properties are not
     * filled when running on data.
     */
    std::vector<pec::Jet> storeJets;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Jet> *storeJetsPointer;
        
    /**
     * \brief Buffer to store MET
     * 
     * Includes the nominal given MET as well as various its corrections and/or systematical
     * variations. Cosult the source code to find the corresponding indices. MET is  stored as an
     * instance of pec::Candidate, but pseudorapidity and mass are set to zeros, which allows them
     * to be compressed efficiently.
     */
    std::vector<pec::Candidate> storeMETs;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Candidate> *storeMETsPointer;
};
