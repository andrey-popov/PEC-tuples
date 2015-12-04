#pragma once

#include <Analysis/PECTuples/interface/Muon.h>
#include <Analysis/PECTuples/interface/Jet.h>
#include <Analysis/PECTuples/interface/PileUpInfo.h>
#include <Analysis/PECTuples/interface/GeneratorInfo.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/PatCandidates/interface/Electron.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/PatCandidates/interface/Jet.h>
#include <DataFormats/PatCandidates/interface/MET.h>
#include <DataFormats/VertexReco/interface/Vertex.h>
#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>
#include <SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \class PlainEventContent
 * \brief This CMSSW plugin saves events in a ROOT file using a very slim format
 * 
 * The plugin stores most of basic objects: muons, electrons, jets, MET. It saves their
 * four-momenta, isolation, b-tagging discriminators, various IDs, etc. Most of the properties are
 * defined in the source code, but the user can provide a number of arbitrary string-based selection
 * criteria, whose results are evaluated and saved.
 * 
 * Sctructure of the output file differs between data and simulation, with some additional branches
 * added in the latter case.
 * 
 * Read documentation for data members, especially, storeElectrons, storeMuons, storeJets, and
 * storeMETs, for further information.
 */
class PlainEventContent: public edm::EDAnalyzer
{
public:
    /**
     * \brief Constructor
     * 
     * Initialises input tags, selections, and flags according to a given configuration. Allocates
     * arrays to keep results of additional selections (as the number of these selections is known
     * at run-time only).
     */
    PlainEventContent(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates output trees and assigns branches to them
    virtual void beginJob();
    
    /**
     * \brief Analyses current event
     * 
     * Writes all the relevant information into buffers associated with the output trees and fills
     * the trees.
     */
    virtual void analyze(edm::Event const &event, edm::EventSetup const &setup);
    
private:
    /// Collection of muons
    edm::EDGetTokenT<edm::View<pat::Muon>> muonToken;
    
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
     * \brief String-based selections for muons
     * 
     * See comments for eleSelectors.
     */
    std::vector<StringCutObjectSelector<pat::Muon>> muSelectors;
    
    /**
     * \brief String-based selections for jets
     * 
     * See comments for eleSelectors.
     */
    std::vector<StringCutObjectSelector<pat::Jet>> jetSelectors;
    
    /**
     * \brief Indicates whether an event is data or simulation
     * 
     * It is used to deduce if the plugin should read generator information.
     */
    bool const runOnData;
    
    /**
     * \brief Generator information
     * 
     * Ignored in data.
     */
    edm::EDGetTokenT<GenEventInfoProduct> generatorToken;
    
    /// Collection of reconstructed primary vertices
    edm::EDGetTokenT<reco::VertexCollection> primaryVerticesToken;
    
    /**
     * \brief Pile-up information in simulation
     * 
     * Ignored in data.
     */
    edm::EDGetTokenT<edm::View<PileupSummaryInfo>> puSummaryToken;
    
    /// Rho (mean angular pt density)
    edm::EDGetTokenT<double> rhoToken;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /**
     * \brief Output tree
     * 
     * The tree aggregates all information stored by the plugin. Its structure differs between data
     * and simulation as in the latter case a branch with generator-level information is added.
     */
    TTree *outTree;
    
    /**
     * \brief Trimmed muons to be stored in the output file
     * 
     * Mass is always close to the PDG value and thus does not encode useful information. It is set
     * to zero to faciliate compression. Bit flags include the tight quality ID and user-defined
     * selections. Consult the source code to find their indices.
     */
    std::vector<pec::Muon> storeMuons;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Muon> *storeMuonsPointer;
    
    /**
     * \brief Trimmed jets to the stored in the output file
     * 
     * The four-momenta stored are uncorrected. In case of soft jets some properties might be set to
     * zero as they are not needed and this would allow a better compression in the output file. Bit
     * flags show if the jet is matched to a generator-level jet (always set to false in real data)
     * and include user-defined selections. Consult the source code to find the indices.
     */
    std::vector<pec::Jet> storeJets;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Jet> *storeJetsPointer;
        
    /**
     * \brief METs to be stored in the output file
     * 
     * Includes all METs whose input tags were provided to the plugin in the didecated parameter.
     * Usually, these are METs with different corrections and/or systematical variations. MET is
     * stored as an instance of pec::Candidate, but pseudorapidity and mass are set to zeros, which
     * allows them to be compressed efficiently.
     */
    std::vector<pec::Candidate> storeMETs;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::Candidate> *storeMETsPointer;
    
    
    /// Basic generator information to be stored in the output file
    pec::GeneratorInfo generatorInfo;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    pec::GeneratorInfo *generatorInfoPointer;
    
    
    /// Information on pile-up to be stored in the output file
    pec::PileUpInfo puInfo;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    pec::PileUpInfo *puInfoPointer;
};
