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
    std::vector<edm::InputTag> const metTag;
    
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
    
    /// Determines of status-3 generator particles should be saved
    bool const saveHardInteraction;
    
    /**
     * \brief Tags to access generator information
     * 
     * They are ignored in case of real data.
     */
    edm::InputTag const generatorTag, genParticlesTag;
    
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
    
    
    /// Maximal size to allocate buffer arrays
    static unsigned const maxSize = 64;
    
    
    /// Tree to store event ID information
    TTree *eventIDTree;
    
    // Buffers for the event-ID tree
    ULong64_t runNumber, lumiSection, eventNumber;
    
    
    /**
     * \brief Tree to store all general information
     * 
     * It is responsible for the most of information that is written by the plugin. It includes
     * four-momenta of all objects, their identification, isolations, b-tagging, etc.
     */
    TTree *basicInfoTree;
    
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
     * to zero to faciliate compression.
     */
    std::vector<pec::Muon> storeMuons;
    
    /// ROOT needs a variable with a pointer to an object to store the object in a tree
    std::vector<pec::Muon> *storeMuonsPointer;
    
    
    UChar_t muSize;  // actual number of muons in an event
    Float_t muPt[maxSize];    // muon four-momenta
    Float_t muEta[maxSize];   //
    Float_t muPhi[maxSize];   //
    //Float_t muMass[maxSize];  // it equals (0.106 +- 0.03) GeV, can be assumed the PDG value
    Bool_t muCharge[maxSize];  // muon's charge (true for muon, false for anti-muon)
    Float_t muDB[maxSize];  // impact-parameter in the transverse plane
    Float_t muRelIso[maxSize];  // relative isolation
    Bool_t muQualityTight[maxSize];  // quality cuts to define tight muons
    Bool_t **muSelectionBits;  // results of the additional selection
    
    UChar_t jetSize;  // actual number of jets in an event
    Float_t jetRawPt[maxSize];    // raw jet four-momenta
    Float_t jetRawEta[maxSize];   //
    Float_t jetRawPhi[maxSize];   //
    Float_t jetRawMass[maxSize];  //
    
    Float_t jetTCHP[maxSize];  // b-tagging discriminators
    Float_t jetCSV[maxSize];   //
    Float_t jetSecVertexMass[maxSize];  // mass of the secondary vertex
    
    // Jet area
    Float_t jetArea[maxSize];
    
    // Electric charge of the jet
    //It simply copies the value returned by pat::Jet::jetCharge(), which is calculated as a sum
    //of electric charges of the jet's contituents weighted with their pt, as mentioned in [1].
    //Note, however, that other definitions are possible [2].
    //[1] https://hypernews.cern.ch/HyperNews/CMS/get/JetMET/1425.html
    //[2] http://arxiv.org/abs/1209.2421
    Float_t jetCharge[maxSize];
    
    // Jet pull angle (radians)
    //The pull vector is defined in [1], Eq. (3.7). The pull angle is an angle between this
    //vector and the rapidity axis
    //[1] http://arxiv.org/abs/1010.3698
    Float_t jetPullAngle[maxSize];
    
    // Jet pile-up ID [1]
    //A bit set to store decisions of two jet pile-up ID algorithms at maximum. First three bits
    //(0x1, 0x2, 0x4) are set to true if the jet passes loose, medium, or tight working point of
    //the first algorithm, respectively. The next three bits (0x8, 0x10, 0x20) are set in a similar
    //fashion if there is one more algorithm specified.
    //Normally, the first algorithm is cut-based while the second is MVA. See details in the
    //documentation for data member jetPileUpIDTags.
    //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID
    UChar_t jetPileUpID[maxSize];
    
    Bool_t **jetSelectionBits;  // results of the additional selection
    
    UChar_t metSize;  // number of different METs stored in the event
    Float_t metPt[maxSize];   // MET absolute value
    Float_t metPhi[maxSize];  // MET phi
    
    
    /**
     * \brief Tree to store generator information
     * 
     * The tree is written only in case the runOnData flag is set to false. Particles from the hard
     * interaction are stored if only the runHardInteraction flag is set to true in addition.
     */
    TTree *generatorTree;
    
    Short_t processID;  // process ID; e.g. different subprocesses in MadGraph get different IDs
    Float_t genWeight;  // generator weight of an event
    
    Char_t jetFlavour[maxSize];  // jet flavour according to the "algorithmic" definition
    Char_t jetGenPartonFlavour[maxSize];  // flavour of the parton matched to jet (0 if no match)
    //^ See here (*) for the motivation of using the both flavour definitions
    //(*) https://hypernews.cern.ch/HyperNews/CMS/get/b2g-selections/103.html
    
    // Shows if there is a generator-level jet matched. Definition from JME-13-005 is used
    Bool_t jetGenJetMatch[maxSize];
            
    Float_t pdfX1, pdfX2;  // momenta fraction carried by initial-state partons
    Float_t pdfQ;  // scale used to evaluate PDF
    Char_t pdfId1, pdfId2;  // ID of the initial-state partons; gluons are encoded by code 0
    
    // Information about the hard interaction (status-3 particles). The beam particles (the protons)
    //are skipped
    UChar_t hardPartSize;  // number of the saved particles
    Char_t hardPartPdgId[maxSize];  // their PDG ID
    Char_t hardPartFirstMother[maxSize], hardPartLastMother[maxSize];  // indices of mothers
    Float_t hardPartPt[maxSize];    // four-momenta of the particles
    Float_t hardPartEta[maxSize];   //
    Float_t hardPartPhi[maxSize];   //
    Float_t hardPartMass[maxSize];  //
    
    
    /**
     * \brief Tree to store pile-up information
     * 
     * Number of reconstructed primary vertices and rho (angular energy density) are saved for real
     * data. In case of simulation additional truth information is recorded.
     */
    TTree *puTree;
    
    UChar_t pvSize;  // number of primary vertices
    Float_t puRho;  // mean energy density
    Float_t puTrueNumInteractions;  // true mean number of pile-up interactions in the event
    UChar_t puSize;  // number of stored pile-up bunch crossings
    Char_t puBunchCrossing[maxSize];  // indices for the bunch crossings
    UChar_t puNumInteractions[maxSize];  // number of pile-up interactions in each crossing
};
