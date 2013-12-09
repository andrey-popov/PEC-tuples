/**
 * \author Andrey Popov
 *
 * The plugin saves all the necessary information from the event to a set of flat ROOT trees. It
 * takes all the basic objects: charged leptons, jets, METs. User can specify arbitrary string-based
 * selection for the leptons and jets; it is not used to filter the collection but it is evaluated
 * and the result saved to the tuples as the array of boolean values. Additionally two types of
 * filtering selection for the jets are provided. The parameter 'jetCut' defines which jets are
 * saved to the file. If a jet fails this selection but passes 'softJetCut' it is accounted for in
 * the integral soft jet characteristics of the event. If a jet fails the both filtering selections
 * it is completely ignored. The user is allowed to specify an arbitrary number of alternative METs
 * which is useful to store the MET systematics.
 * 
 * When running on simulation the input jets are supposed to be smeared properly in order to
 * reproduce the jet resolution in data. The plugin takes two additional jet collections
 * corresponding to the JER systematic variation (they are not read for the data). JER variations
 * as well as the JEC uncertainties splitted by stat. independent sources are saved for the
 * simulation.
 * 
 * The basic generator information is saved when available. It includes processID, PDF info, jet
 * flavours, PU information. When requested by 'saveFinalMEState' flag the PDG ID of the particles
 * in the final state of ME are saved. It was tested to provide the reasonable results for POWHEG
 * and MG.
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


using edm::InputTag;
using std::string;
using std::vector;


#define MAX_LEN 128  // maximum size used to allocate the arrays


class PlainEventContent: public edm::EDAnalyzer
{
    public:
        PlainEventContent(edm::ParameterSet const &cfg);
        ~PlainEventContent();
    
    private:
        void beginJob();
        void endJob();
        void beginRun(edm::Run const &run, edm::EventSetup const &setup);
        void endRun(edm::Run const &run, edm::EventSetup const &setup);
        void analyze(edm::Event const &event, edm::EventSetup const &setup);
        
        // The source collections
        InputTag const eleSrc, muSrc, jetSrc;
        vector<InputTag> const metSrc;
        // String-based selection of the jets to be saved in the tuples
        string const jetCut;
        // String-based selection of the jets to be treated as "soft"
        string const softJetCut;
        // String-based selection which result is stored with the objects (not used for filtering)
        vector<string> const eleSelection, muSelection, jetSelection;
        bool const runOnData;    // indicated whether generator info is availible
        bool const saveHardInteraction;  // whether to save info on status-3 particles
        
        /// Determines if integral properties for soft jets should be saved
        bool const saveIntegralSoftJets;
        
        // Generator information sources. They are not read for real data
        InputTag const generatorSrc, genParticlesSrc;
        InputTag const primaryVerticesSrc;  // collection of reconstructed PV
        InputTag const puSummarySrc;  // PU information. Not read for real data
        InputTag const rhoSrc;  // rho (mean energy density)
        
        
        vector<InputTag> jerSystJetsSrc;  // JER systematic shifted collections of jets
                
        edm::Service<TFileService> fs;  // object providing interface to the ROOT files
        JetCorrectionUncertainty *jecUncProvider;  // object to access the JEC uncertainty
        
        
        // The tree to store the event ID information
        TTree *eventIDTree;
        
        ULong64_t runNumber, lumiSection, eventNumber;
        
        
        // The tree to store the basic kinematics, quality requirements, etc.
        TTree *basicInfoTree;
        
        UChar_t eleSize;  // actual size of the electron collection
        Float_t elePt[MAX_LEN];    // electron 4-momenta
        Float_t eleEta[MAX_LEN];   //
        Float_t elePhi[MAX_LEN];   //
        //Float_t eleMass[MAX_LEN];  // it equals (0 +- 0.03) GeV, can be assumed the PDG value
        Bool_t eleCharge[MAX_LEN];  // electron's charge (true for electron, false for positron)
        Float_t eleDB[MAX_LEN];  // impact-parameter in the transverse plane
        Float_t eleRelIso[MAX_LEN];  // relative isolation
        
        // Trigger-emulating preselection required for triggering MVA ID [1-2]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Training_of_the_MVA
        //[2] https://hypernews.cern.ch/HyperNews/CMS/get/egamma-elecid/72.html
        Bool_t eleTriggerPreselection[MAX_LEN];
        
        // Electron MVA ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
        Float_t eleMVAID[MAX_LEN];
        
        // Old cut-based electron ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/SimpleCutBasedEleID
        UChar_t eleIDSimple70cIso[MAX_LEN];
        
        Bool_t elePassConversion[MAX_LEN];  // conversion veto (true for good electrons)
        Bool_t **eleSelectionBits;  // results of the additional selection
        
        UChar_t muSize;  // actual size of the muon collection
        Float_t muPt[MAX_LEN];    // muon 4-momenta
        Float_t muEta[MAX_LEN];   //
        Float_t muPhi[MAX_LEN];   //
        //Float_t muMass[MAX_LEN];  // it equals (0.106 +- 0.03) GeV, can be assumed the PDG value
        Bool_t muCharge[MAX_LEN];  // muon's charge (true for muon, false for anti-muon)
        Float_t muDB[MAX_LEN];  // impact-parameter in the transverse plane
        Float_t muRelIso[MAX_LEN];  // relative isolation
        Bool_t muQualityTight[MAX_LEN];  // quality cuts to define tight muons
        Bool_t **muSelectionBits;  // results of the additional selection
        
        UChar_t jetSize;  // actual size of the jet collection
        Float_t jetPt[MAX_LEN];    // jet corrected 4-momenta
        Float_t jetEta[MAX_LEN];   //
        Float_t jetPhi[MAX_LEN];   //
        Float_t jetMass[MAX_LEN];  //
        Float_t jecUncertainty[MAX_LEN];  // JEC uncertainty
        
        // JER systematics. The components of 4-momentum are scaled simultaneously (*). Therefore
        //phi and eta are not affected and are the same as for the nominal jets
        //(*) http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h?view=markup, function produce()
        Float_t jetPtJERUp[MAX_LEN];
        Float_t jetMassJERUp[MAX_LEN];
        Float_t jetPtJERDown[MAX_LEN];
        Float_t jetMassJERDown[MAX_LEN];
        
        Float_t jetTCHP[MAX_LEN];  // b-tagging discriminators
        Float_t jetCSV[MAX_LEN];   //
        Float_t jetSecVertexMass[MAX_LEN];  // mass of the secondary vertex (a'la SHyFT)
        
        // Electric charge of the jet
        //It simply copies the value returned by pat::Jet::jetCharge(), which is calculated as a sum
        //of electric charges of the jet's contituents weighted with their pt, as mentioned in [1].
        //Note, however, that other definitions are possible [2].
        //[1] https://hypernews.cern.ch/HyperNews/CMS/get/JetMET/1425.html
        //[2] http://arxiv.org/abs/1209.2421
        Float_t jetCharge[MAX_LEN];
        
        // Jet pull angle (radians)
        //The pull vector is defined in [1], Eq. (3.7). The pull angle is an angle between this
        //vector and the rapidity axis
        //[1] http://arxiv.org/abs/1010.3698
        Float_t jetPullAngle[MAX_LEN];
        
        Bool_t **jetSelectionBits;  // results of the additional selection
        
        UChar_t metSize;  // number of different METs stored in the event
        Float_t metPt[MAX_LEN];   // MET absolute value
        Float_t metPhi[MAX_LEN];  // MET phi
        
        
        // The tree to store the integral event characteristics
        TTree *integralPropTree;
        
        // The soft jets
        Float_t softJetPt;
        Float_t softJetEta;
        Float_t softJetPhi;
        Float_t softJetMass;
        Float_t softJetHt;
        
        // The soft jets JEC uncertainties. The weighted sum unc_i * p4_i, where i indexes the jets,
        //is sufficient
        Float_t softJetPtJECUnc;
        Float_t softJetEtaJECUnc;
        Float_t softJetPhiJECUnc;
        Float_t softJetMassJECUnc;
        Float_t softJetHtJECUnc;
        
        // The soft jets JER systematics
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
        
        
        // The tree to store generator information (except for one stored in basicInfoTree). It is
        //filled if only runOnData is false, otherwise the tree is not event stored in the file
        TTree *generatorTree;
        
        Short_t processID;  // the generator process ID
        Float_t genWeight;  // the generator weight for the event
        
        Char_t jetFlavour[MAX_LEN];  // algorithmic jet flavour definition
        Char_t jetGenPartonFlavour[MAX_LEN];  // flavour of the parton matched to jet (0 if no match)
        //^ See here (*) for the motivation of using the both flavour definitions
        //(*) https://hypernews.cern.ch/HyperNews/CMS/get/b2g-selections/103.html
                
        Float_t pdfX1, pdfX2;  // momenta fraction carried by initial-state partons
        Float_t pdfQ;  // scale used to evaluate PDF
        Char_t pdfId1, pdfId2;  // ID of the initial-state partons
        
        // Information about the hard interaction (status-3 particles). The initial section (i.e.
        //the first 6 entries in genParticles) is skipped
        UChar_t hardPartSize;  // number of the saved particles
        Char_t hardPartPdgId[MAX_LEN];  // their PDG ID
        Char_t hardPartFirstMother[MAX_LEN], hardPartLastMother[MAX_LEN];  // indices of mothers
        Float_t hardPartPt[MAX_LEN];    // 4-momenta of the particles
        Float_t hardPartEta[MAX_LEN];   //
        Float_t hardPartPhi[MAX_LEN];   //
        Float_t hardPartMass[MAX_LEN];  //
        
        
        // The tree to store pile-up information
        TTree *puTree;
        
        UChar_t pvSize;  // number of primary vertices
        Float_t puRho;  // mean energy density
        Float_t puTrueNumInteractions;  // true mean number of PU interactions in the event
        UChar_t puSize;  // number of stored pile-up bunch crossings
        Char_t puBunchCrossing[MAX_LEN];  // indices for the bunch crossings
        UChar_t puNumInteractions[MAX_LEN];  // number of PU interactions in each crossing
};
