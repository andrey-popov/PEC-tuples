/**
 * \file PlainEventContent.h
 * \author Andrey Popov
 * 
 * The plugin defined in this module stores most of basic information in an event in a set of plain
 * ROOT trees. It addresses all the basic objects: charged leptons, jets, and MET. Apart from their
 * four-momenta, a number of other properties such as various ID criteria, isolation, b-tagging, and
 * other characteristics are saved. Although the most of them are defined in code of the plugin, the
 * can provide an arbitrary number of string-based selections, which are evaluated for each object
 * of an appropriate type and whose results are stored in dedicated boolean branches.
 * 
 * The plugin expects a vector of METs instead of a single input tag. It allows to store versions
 * with different corrections as well as systematical variations of MET.
 * 
 * When a simulated event is processed, the plugin stores true jet flavours, information on PDF,
 * particles from the hard interaction, true pile-up configuration, and other details.
 * 
 * The plugin does not store any information on trigger decision.
 */


#pragma once

#include <UserCode/SingleTop/interface/Electron.h>
#include <UserCode/SingleTop/interface/Muon.h>
#include <UserCode/SingleTop/interface/Jet.h>
#include <UserCode/SingleTop/interface/EventID.h>
#include <UserCode/SingleTop/interface/PileUpInfo.h>
#include <UserCode/SingleTop/interface/GeneratorInfo.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <string>
#include <vector>


/**
 * \class PlainEventContent
 * \brief Stores all important information about an event in a set of plain ROOT trees
 * 
 * Consult file's documentation for details.
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
    
    /**
     * \brief Destructor
     * 
     * Deletes arrays that keep results of addititonal selections.
     */
    ~PlainEventContent();
    
public:
    /// Creates output trees and assigns branches to them
    virtual void beginJob();
    
    /// A placeholder; does nothing
    virtual void endJob();
    
    /// Creates an object to access JEC uncertainties
    virtual void beginRun(edm::Run const &run, edm::EventSetup const &setup);
    
    /// Deletes the object to access JEC uncertainties
    virtual void endRun(edm::Run const &run, edm::EventSetup const &setup);
    
    /**
     * \brief Analyses current event
     * 
     * Writes all the relevant information into buffers associated with the output trees and fills
     * the trees.
     */
    virtual void analyze(edm::Event const &event, edm::EventSetup const &setup);
    
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
private:
    /// Tags to access collections of electrons, muons, and jets
    edm::InputTag const electronTag, muonTag, jetTag;
    
    /**
     * \brief Tags to access collections of MET
     * 
     * The plugin reads not a single MET but a vector of them. It allows to store MET with various
     * corrections as well as its systematical variations.
     */
    std::vector<edm::InputTag> const metTags;
    
    /// Minimal corrected transverse momentum to determine which jets are stored
    double const jetMinPt;
    
    /// Minimal raw transverse momentum to determine which jets are stored
    double const jetMinRawPt;
    
    /**
     * \brief String-based selection whose result is to be saved
     * 
     * These selections do not affect which objects are stored in the output files. Instead, each
     * string defines a selection that is evalueated and whose result is saved in a dedicated
     * boolean branch.
     */
    std::vector<std::string> const eleSelection, muSelection, jetSelection;
    
    /**
     * \brief Indicates whether an event is data or simulation
     * 
     * It is used to deduce if the plugin should read generator information or attempt to access
     * systematical uncertainties.
     */
    bool const runOnData;
    
    /**
     * \brief Tags to access generator information
     * 
     * They are ignored in case of real data.
     */
    edm::InputTag const generatorTag;
    
    /// Tag to access reconstructed primary vertices
    edm::InputTag const primaryVerticesTag;
    
    /// Pile-up information in simulation
    edm::InputTag const puSummaryTag;
    
    /// Rho (mean angular energy density)
    edm::InputTag const rhoTag;
    
    /**
     * \brief Input tags for jet pile-up ID
     * 
     * The vector might contain 0, 1, or 2 elements. If more than 2 elements are provided,
     * constructor throws an exception. If the vector is empty, no branch to store jet pile-up ID
     * is added to the output tree. If there are two elements, the first one should refer to the
     * cut-based ID, and the latter one specifies the MVA ID.
     */
    std::vector<edm::InputTag> jetPileUpIDTags;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /**
     * \brief Output tree
     * 
     * The tree aggregates all information stored by the plugin. Its structure differs between data
     * and simulation as in the latter case a branch with generator-level information is added.
     */
    TTree *outTree;
    
    /// Basic generator information to be stored in the output file
    pec::EventID eventId;
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
    pec::EventID *eventIdPointer;
    
    /**
     * \brief Trimmed electrons to be stored in the output file
     * 
     * Mass is always close to the PDG value and thus does not encode useful information. It is set
     * to zero to faciliate compression. Bit flags include conversion rejection, trigger-emulating
     * preselection required for the triggering MVA ID [1-2], and user-defined selections. Consult
     * the source code to find their indices.
     * [1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Training_of_the_MVA
     * [2] https://hypernews.cern.ch/HyperNews/CMS/get/egamma-elecid/72.html
     */
    std::vector<pec::Electron> storeElectrons;
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
    std::vector<pec::Electron> *storeElectronsPointer;
    
    /**
     * \brief Trimmed muons to be stored in the output file
     * 
     * Mass is always close to the PDG value and thus does not encode useful information. It is set
     * to zero to faciliate compression. Bit flags include the tight quality ID and user-defined
     * selections. Consult the source code to find their indices.
     */
    std::vector<pec::Muon> storeMuons;
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
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
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
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
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
    std::vector<pec::Candidate> *storeMETsPointer;
    
    
    /// Basic generator information to be stored in the output file
    pec::GeneratorInfo generatorInfo;
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
    pec::GeneratorInfo *generatorInfoPointer;
    
    
    /// Information on pile-up to be stored in the output file
    pec::PileUpInfo puInfo;
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
    pec::PileUpInfo *puInfoPointer;
};
