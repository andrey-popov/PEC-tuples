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
 * In case of jets, additional string-based selections can be used to prefilter them. Selection
 * jetCut determines which jets are written in the trees. If a jet fails it but passes another
 * selection specified in softJetCut parameter, it is added to an aggregate, and the summed four-
 * momentum and Ht of the aggregate are stored in a tree. However, this integral information is
 * written if only saveIntegralSoftJets flag is activated. Jets provided to the plugin are expected
 * to be smeared for nominal JER in case of simulated events.
 * 
 * The plugin expects a vector of METs instead of a single input tag. It allows to store versions
 * with different corrections as well as systematical variations of MET.
 * 
 * When a simulated event is processed, variations of jet momenta for JEC and JER uncertainties are
 * written. In addition, the plugin stores true jet flavours, information on PDF, particles from
 * the hard interaction, true pile-up configuration, and other details.
 * 
 * The plugin does not store any information on trigger decision.
 */


#pragma once

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/FileInPath.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h>

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
    
private:
    /// Tags to access collections of electrons, muons, and jets
    edm::InputTag const eleSrc, muSrc, jetSrc;
    
    /**
     * \brief Tags to access collections of MET
     * 
     * The plugin reads not a single MET but a vector of them. It allows to store MET with various
     * corrections as well as its systematical variations.
     */
    std::vector<edm::InputTag> const metSrc;
    
    /// String-based selection to determine which jets are stored
    std::string const jetCut;
    
    /**
     * \brief String-based selection to define "soft jets"
     * 
     * The "soft" jets jets are not stored on their own. Instead, several integral characteristics
     * such as total four-momentum or total Ht are evaluated with this jets and saved along with
     * information which is necessary to reproduces their systematical variations for JEC and JER
     * uncertainties. A jet is checked against this selection if only it fails jetCut. In addition,
     * these characteristics are evaluated and saved if only flag saveIntegralSoftJets is set to
     * true.
     */
    std::string const softJetCut;
    
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
    
    /// Determines if integral properties for soft jets should be saved
    bool const saveIntegralSoftJets;
    
    /**
     * \brief Tags to access generator information
     * 
     * They are ignored in case of real data.
     */
    edm::InputTag const generatorSrc, genParticlesSrc;
    
    /// Tag to access reconstructed primary vertices
    edm::InputTag const primaryVerticesSrc;
    
    /// Pile-up information in simulation
    edm::InputTag const puSummarySrc;
    
    /// Rho (mean angular energy density)
    edm::InputTag const rhoSrc;
    
    /**
     * \brief Jet collections shifted for JER systematics
     * 
     * Must contain exactly two tags: the first one refers to the up variation, the latter one
     * refers to the down variation. This data member is ignored in case of real data.
     */
    std::vector<edm::InputTag> jerSystJetsSrc;
    
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    
    /// An object to accesss JEC uncertainty
    JetCorrectionUncertainty *jecUncProvider;
    
    
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
    
    UChar_t eleSize;  // actual number of electrons in an event
    Float_t elePt[maxSize];    // electron four-momenta
    Float_t eleEta[maxSize];   //
    Float_t elePhi[maxSize];   //
    //Float_t eleMass[maxSize];  // it equals (0 +- 0.03) GeV, can be assumed the PDG value
    Bool_t eleCharge[maxSize];  // electron's charge (true for electron, false for positron)
    Float_t eleDB[maxSize];  // impact-parameter in the transverse plane
    Float_t eleRelIso[maxSize];  // relative isolation
    
    // Trigger-emulating preselection required for triggering MVA ID [1-2]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Training_of_the_MVA
    //[2] https://hypernews.cern.ch/HyperNews/CMS/get/egamma-elecid/72.html
    Bool_t eleTriggerPreselection[maxSize];
    
    // Electron MVA ID [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
    Float_t eleMVAID[maxSize];
    
    // Old cut-based electron ID [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMS/SimpleCutBasedEleID
    UChar_t eleIDSimple70cIso[maxSize];
    
    Bool_t elePassConversion[maxSize];  // conversion veto (true for good electrons)
    Bool_t **eleSelectionBits;  // results of the additional selection
    
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
    Float_t jetPt[maxSize];    // jet corrected four-momenta
    Float_t jetEta[maxSize];   //
    Float_t jetPhi[maxSize];   //
    Float_t jetMass[maxSize];  //
    Float_t jecUncertainty[maxSize];  // JEC uncertainty
    
    // JER systematics. The components of 4-momentum are scaled simultaneously (*). Therefore,
    //phi and eta components are not affected and equals ones of nominal jets
    //(*) http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h?view=markup, function produce()
    Float_t jetPtJERUp[maxSize];
    Float_t jetMassJERUp[maxSize];
    Float_t jetPtJERDown[maxSize];
    Float_t jetMassJERDown[maxSize];
    
    Float_t jetTCHP[maxSize];  // b-tagging discriminators
    Float_t jetCSV[maxSize];   //
    Float_t jetSecVertexMass[maxSize];  // mass of the secondary vertex
    
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
    
    Bool_t **jetSelectionBits;  // results of the additional selection
    
    UChar_t metSize;  // number of different METs stored in the event
    Float_t metPt[maxSize];   // MET absolute value
    Float_t metPhi[maxSize];  // MET phi
    
    
    /**
     * \brief Tree to store integral properties of soft jets
     * 
     * Consult documentation for the softJetCut data member for definition of the soft jets. This
     * tree is written if only the flag saveIntegralSoftJets is set to true. Systematical variations
     * are stored only when simulation is processed.
     */
    TTree *integralPropTree;
    
    // Summed four-momentum of the soft jets and their Ht
    Float_t softJetPt;
    Float_t softJetEta;
    Float_t softJetPhi;
    Float_t softJetMass;
    Float_t softJetHt;
    
    // JEC uncertainties for the soft jets. This is a weighted sum unc_i * p4_i, where i indexes the
    //jets. In addition, sum unc_i * pt_i is saved to allow use of Ht
    Float_t softJetPtJECUnc;
    Float_t softJetEtaJECUnc;
    Float_t softJetPhiJECUnc;
    Float_t softJetMassJECUnc;
    Float_t softJetHtJECUnc;
    
    // JER systematics for the soft jets
    Float_t softJetPtJERUp;
    Float_t softJetEtaJERUp;
    Float_t softJetPhiJERUp;
    Float_t softJetMassJERUp;
    Float_t softJetHtJERUp;
    
    Float_t softJetPtJERDown;
    Float_t softJetEtaJERDown;
    Float_t softJetPhiJERDown;
    Float_t softJetMassJERDown;
    Float_t softJetHtJERDown;
    
    
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
