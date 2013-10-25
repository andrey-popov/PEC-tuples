/**
 * @author Andrey.Popov@cern.ch
 *
 * Description: see the header file.
 */

#include <UserCode/SingleTop/plugins/PlainEventContent.h>

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <DataFormats/PatCandidates/interface/Jet.h>
#include <DataFormats/PatCandidates/interface/Electron.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/PatCandidates/interface/MET.h>
#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>

#include <EgammaAnalysis/ElectronTools/interface/EcalIsolationCorrector.h>

#include <CondFormats/JetMETObjects/interface/JetCorrectorParameters.h>
#include <JetMETCorrections/Objects/interface/JetCorrectionsRecord.h>

#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>
#include <SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h>

#include <TLorentzVector.h>

#include <memory>
#include <cmath>
#include <sstream>


using namespace edm;
using std::abs;


PlainEventContent::PlainEventContent(edm::ParameterSet const &cfg):
    eleSrc(cfg.getParameter<InputTag>("electrons")),
    muSrc(cfg.getParameter<InputTag>("muons")),
    jetSrc(cfg.getParameter<InputTag>("jets")),
    METSrcs(cfg.getParameter<vector<InputTag>>("METs")),
    
    jetCut(cfg.exists("jetCut") ? cfg.getParameter<string>("jetCut") : ""),
    softJetCut(cfg.exists("softJetCut") ? cfg.getParameter<string>("softJetCut") : ""),
    
    eleSelection(cfg.exists("eleSelection") ?
     cfg.getParameter<vector<string>>("eleSelection") : vector<string>(0)),
    muSelection(cfg.exists("muSelection") ?
     cfg.getParameter<vector<string>>("muSelection") : vector<string>(0)),
    jetSelection(cfg.exists("jetSelection") ?
     cfg.getParameter<vector<string>>("jetSelection") : vector<string>(0)),
    
    runOnData(cfg.getParameter<bool>("runOnData")),
    saveHardInteraction(cfg.exists("saveHardInteraction") ?
     cfg.getParameter<bool>("saveHardInteraction") : false),
    
    generatorSrc(cfg.getParameter<InputTag>("generator")),
    genParticlesSrc(cfg.getParameter<InputTag>("genParticles")),
    primaryVerticesSrc(cfg.getParameter<InputTag>("primaryVertices")),
    PUSummarySrc(cfg.getParameter<InputTag>("PUInfo")),
    rhoSrc(cfg.getParameter<InputTag>("rho")),
    
    jecUncProvider(nullptr)
{
    if (!runOnData)
        JERSystJetsSrc = cfg.getParameter<vector<InputTag>>("JERSystJets");
    
    
    // Check the inputs
    if (!runOnData  &&  JERSystJetsSrc.size() != 2)
    {
        edm::Exception excp(edm::errors::LogicError);
        excp << "Vector of size 2 is expected for JERSystJets parameter but the given vector is " <<
         "of size " << JERSystJetsSrc.size() << '\n';
        excp.raise();
    }
    
    // Allocate the buffers to store the bits of additional selection
    eleSelectionBits = new Bool_t *[eleSelection.size()];
    muSelectionBits = new Bool_t *[muSelection.size()];
    jetSelectionBits = new Bool_t *[jetSelection.size()];
    
    for (unsigned i = 0; i < eleSelection.size(); ++i)
        eleSelectionBits[i] = new Bool_t[MAX_LEN];
    
    for (unsigned i = 0; i < muSelection.size(); ++i)
        muSelectionBits[i] = new Bool_t[MAX_LEN];
    
    for (unsigned i = 0; i < jetSelection.size(); ++i)
        jetSelectionBits[i] = new Bool_t[MAX_LEN];
}


PlainEventContent::~PlainEventContent()
{
    // Free the memory allocated for the additional selection
    for (unsigned i = 0; i < eleSelection.size(); ++i)
        delete [] eleSelectionBits[i];
    
    for (unsigned i = 0; i < muSelection.size(); ++i)
        delete [] muSelectionBits[i];
    
    for (unsigned i = 0; i < jetSelection.size(); ++i)
        delete [] jetSelectionBits[i];
    
    delete [] eleSelectionBits;
    delete [] muSelectionBits;
    delete [] jetSelectionBits;
}


void PlainEventContent::beginJob()
{
    eventIDTree = fs->make<TTree>("EventID", "Tree contrains event ID information");
    
    eventIDTree->Branch("run", &runNumber);
    eventIDTree->Branch("lumi", &lumiSection);
    eventIDTree->Branch("event", &eventNumber);
    
    
    basicInfoTree = fs->make<TTree>("BasicInfo",
     "Tree contains kinematics and other basic properties");
    
    basicInfoTree->Branch("eleSize", &eleSize);
    basicInfoTree->Branch("elePt", elePt, "elePt[eleSize]/F");
    basicInfoTree->Branch("eleEta", eleEta, "eleEta[eleSize]/F");
    basicInfoTree->Branch("elePhi", elePhi, "elePhi[eleSize]/F");
    basicInfoTree->Branch("eleCharge", eleCharge, "eleCharge[eleSize]/O");
    basicInfoTree->Branch("eleDB", eleDB, "eleDB[eleSize]/F");
    basicInfoTree->Branch("eleRelIso", eleRelIso, "eleRelIso[eleSize]/F");
    basicInfoTree->Branch("eleTriggerPreselection", eleTriggerPreselection,
     "eleTriggerPreselection[eleSize]/O");
    basicInfoTree->Branch("eleMVAID", eleMVAID, "eleMVAID[eleSize]/F");
    basicInfoTree->Branch("eleIDSimple70cIso", eleIDSimple70cIso, "eleIDSimple70cIso[eleSize]/I");
    basicInfoTree->Branch("elePassConversion", elePassConversion, "elePassConversion[eleSize]/O");
    
    for (unsigned i = 0; i < eleSelection.size(); ++i)
    {
        string branchName("eleSelection");
        branchName += char(65 + i);  // char(65) == 'A'
        basicInfoTree->Branch(branchName.c_str(), eleSelectionBits[i],
         (branchName + "[eleSize]/O").c_str());
    }
    
    basicInfoTree->Branch("muSize", &muSize);
    basicInfoTree->Branch("muPt", muPt, "muPt[muSize]/F");
    basicInfoTree->Branch("muEta", muEta, "muEta[muSize]/F");
    basicInfoTree->Branch("muPhi", muPhi, "muPhi[muSize]/F");
    basicInfoTree->Branch("muCharge", muCharge, "muCharge[muSize]/O");
    basicInfoTree->Branch("muDB", muDB, "muDB[muSize]/F");
    basicInfoTree->Branch("muRelIso", muRelIso, "muRelIso[muSize]/F");
    basicInfoTree->Branch("muQualityTight", muQualityTight, "muQualityTight[muSize]/O");
    
    for (unsigned i = 0; i < muSelection.size(); ++i)
    {
        string branchName("muSelection");
        branchName += char(65 + i);  // char(65) == 'A'
        basicInfoTree->Branch(branchName.c_str(), muSelectionBits[i],
         (branchName + "[muSize]/O").c_str());
    }
    
    basicInfoTree->Branch("jetSize", &jetSize);
    basicInfoTree->Branch("jetPt", jetPt, "jetPt[jetSize]/F");
    basicInfoTree->Branch("jetEta", jetEta, "jetEta[jetSize]/F");
    basicInfoTree->Branch("jetPhi", jetPhi, "jetPhi[jetSize]/F");
    basicInfoTree->Branch("jetMass", jetMass, "jetMass[jetSize]/F");
    basicInfoTree->Branch("jecUncertainty", jecUncertainty, "jecUncertainty[jetSize]/F");
    
    if (!runOnData)
    {
        basicInfoTree->Branch("jetPtJERUp", jetPtJERUp, "jetPtJERUp[jetSize]/F");
        basicInfoTree->Branch("jetMassJERUp", jetMassJERUp, "jetMassJERUp[jetSize]/F");
        basicInfoTree->Branch("jetPtJERDown", jetPtJERDown, "jetPtJERDown[jetSize]/F");
        basicInfoTree->Branch("jetMassJERDown", jetMassJERDown, "jetMassJERDown[jetSize]/F");
    }
    
    basicInfoTree->Branch("jetNDaughters", jetNDaughters, "jetNDaughters[jetSize]/I");
    basicInfoTree->Branch("jetEleMultiplicity", jetEleMultiplicity,
     "jetEleMultiplicity[jetSize]/I");
    basicInfoTree->Branch("jetMuMultiplicity", jetMuMultiplicity, "jetMuMultiplicity[jetSize]/I");
    basicInfoTree->Branch("jetTCHP", jetTCHP, "jetTCHP[jetSize]/F");
    basicInfoTree->Branch("jetCSV", jetCSV, "jetCSV[jetSize]/F");
    basicInfoTree->Branch("jetJP", jetJP, "jetJP[jetSize]/F");
    
    basicInfoTree->Branch("jetSecVertexMass", jetSecVertexMass, "jetSecVertexMass[jetSize]/F");
    
    basicInfoTree->Branch("jetPUCutBasedDiscr", jetPUCutBasedDiscr, "jetPUCutBasedDiscr[jetSize]/F");
    basicInfoTree->Branch("jetPUCutBasedID", jetPUCutBasedID, "jetPUCutBasedID[jetSize]/I");
    basicInfoTree->Branch("jetPUSimpleDiscr", jetPUSimpleDiscr, "jetPUSimpleDiscr[jetSize]/F");
    basicInfoTree->Branch("jetPUSimpleID", jetPUSimpleID, "jetPUSimpleID[jetSize]/I");
    basicInfoTree->Branch("jetPUFullDiscr", jetPUFullDiscr, "jetPUFullDiscr[jetSize]/F");
    basicInfoTree->Branch("jetPUFullID", jetPUFullID, "jetPUFullID[jetSize]/I");
    
    for (unsigned i = 0; i < jetSelection.size(); ++i)
    {
        string branchName("jetSelection");
        branchName += char(65 + i);  // char(65) == 'A'
        basicInfoTree->Branch(branchName.c_str(), jetSelectionBits[i],
         (branchName + "[jetSize]/O").c_str());
    }
    
    basicInfoTree->Branch("metSize", &metSize);
    basicInfoTree->Branch("metPt", metPt, "metPt[metSize]/F");
    basicInfoTree->Branch("metPhi", metPhi, "metPhi[metSize]/F");
    
    
    integralPropTree = fs->make<TTree>("IntegralProperties",
     "The tree keeps integral properties of the event");
    
    /*integralPropTree->Branch("sphericity", &sphericity);
    integralPropTree->Branch("aplanarity", &aplanarity);
    integralPropTree->Branch("planarity", &planarity);*/
    
    integralPropTree->Branch("softJetPt", &softJetPt);
    integralPropTree->Branch("softJetEta", &softJetEta);
    integralPropTree->Branch("softJetPhi", &softJetPhi);
    integralPropTree->Branch("softJetMass", &softJetMass);
    integralPropTree->Branch("softJetHt", &softJetHt);
    
    if (!runOnData)
    {
        integralPropTree->Branch("softJetPtJECUnc", &softJetPtJECUnc);
        integralPropTree->Branch("softJetEtaJECUnc", &softJetEtaJECUnc);
        integralPropTree->Branch("softJetPhiJECUnc", &softJetPhiJECUnc);
        integralPropTree->Branch("softJetMassJECUnc", &softJetMassJECUnc);
        integralPropTree->Branch("softJetHtJECUnc", &softJetHtJECUnc);
        
        integralPropTree->Branch("softJetPtJERUp", &softJetPtJERUp);
        integralPropTree->Branch("softJetEtaJERUp", &softJetEtaJERUp);
        integralPropTree->Branch("softJetPhiJERUp", &softJetPhiJERUp);
        integralPropTree->Branch("softJetMassJERUp", &softJetMassJERUp);
        integralPropTree->Branch("softJetHtJERUp", &softJetHtJERUp);
        
        integralPropTree->Branch("softJetPtJERDown", &softJetPtJERDown);
        integralPropTree->Branch("softJetEtaJERDown", &softJetEtaJERDown);
        integralPropTree->Branch("softJetPhiJERDown", &softJetPhiJERDown);
        integralPropTree->Branch("softJetMassJERDown", &softJetMassJERDown);
        integralPropTree->Branch("softJetHtJERDown", &softJetHtJERDown);
    }
    
   
    if (!runOnData)
    {
        generatorTree = fs->make<TTree>("GeneratorInfo",
         "The tree keeps some generator information");
        
        generatorTree->Branch("processID", &processID);
        generatorTree->Branch("genWeight", &genWeight);
        
        generatorTree->Branch("jetSize", &jetSize);  // it's a duplication from basicInfoTree
        generatorTree->Branch("jetFlavour", jetFlavour, "jetFlavour[jetSize]/I");
        generatorTree->Branch("jetGenPartonFlavour", jetGenPartonFlavour,
         "jetGenPartonFlavour[jetSize]/I");
        
        generatorTree->Branch("pdfX1", &pdfX1);
        generatorTree->Branch("pdfX2", &pdfX2);
        generatorTree->Branch("pdfQ", &pdfQ);
        generatorTree->Branch("pdfId1", &pdfId1);
        generatorTree->Branch("pdfId2", &pdfId2);
        
        if (saveHardInteraction)
        {
            generatorTree->Branch("hardPartSize", &hardPartSize);
            generatorTree->Branch("hardPartPdgId", hardPartPdgId, "hardPartPdgId[hardPartSize]/I");
            generatorTree->Branch("hardPartFirstMother", hardPartFirstMother,
             "hardPartFirstMother[hardPartSize]/I");
            generatorTree->Branch("hardPartLastMother", hardPartLastMother,
             "hardPartLastMother[hardPartSize]/I");
            generatorTree->Branch("hardPartPt", hardPartPt, "hardPartPt[hardPartSize]/F");
            generatorTree->Branch("hardPartEta", hardPartEta, "hardPartEta[hardPartSize]/F");
            generatorTree->Branch("hardPartPhi", hardPartPhi, "hardPartPhi[hardPartSize]/F");
            generatorTree->Branch("hardPartMass", hardPartMass, "hardPartMass[hardPartSize]/F");
        }
    }
    
    
    PUTree = fs->make<TTree>("PUInfo", "Pile-up information");
    PUTree->Branch("PVSize", &PVSize);
    PUTree->Branch("rho", &PURho);
    
    if (!runOnData)
    {
        PUTree->Branch("PUTrueNumInteractions", &PUTrueNumInteractions);
        PUTree->Branch("PUSize", &PUSize);
        PUTree->Branch("PUBunchCrossing", PUBunchCrossing, "PUBunchCrossing[PUSize]/I");
        PUTree->Branch("PUNumInteractions", PUNumInteractions, "PUNumInteractions[PUSize]/I");
    }
}


void PlainEventContent::endJob()
{}


void PlainEventContent::beginRun(edm::Run const &run, edm::EventSetup const &setup)
{
    // Construct the JEC uncertainty provider (*)
    //(*) https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetCorUncertainties
    edm::ESHandle<JetCorrectorParametersCollection> jecParametersCollection;
    setup.get<JetCorrectionsRecord>().get("AK5PFchs", jecParametersCollection);
    JetCorrectorParameters const &jecParameters = (*jecParametersCollection)["Uncertainty"];
    jecUncProvider = new JetCorrectionUncertainty(jecParameters);
}


void PlainEventContent::endRun(edm::Run const &run, edm::EventSetup const &setup)
{
    delete jecUncProvider;
}


void PlainEventContent::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Fill the event ID tree
    runNumber = event.id().run();
	eventNumber = event.id().event();
	lumiSection = event.luminosityBlock();
    
    
    // Read the primary vertices
    Handle<reco::VertexCollection> vertices;
    event.getByLabel(primaryVerticesSrc, vertices);
    
    PVSize = vertices->size();
    
    if (PVSize == 0)
    {
        edm::Exception excp(edm::errors::LogicError);
        excp << "Event contains zero good primary vertices.\n";
        excp.raise();
    }
    
    
    // Fill the tree with basic information
    // Read the electron collection
    Handle<View<pat::Electron>> electrons;
    event.getByLabel(eleSrc, electrons);
    
    
    // Construct the electron selectors (s. SWGuidePhysicsCutParser)
    vector<StringCutObjectSelector<pat::Electron>> eleSelectors;
    
    for (vector<string>::const_iterator sel = eleSelection.begin(); sel != eleSelection.end();
     ++sel)
        eleSelectors.push_back(*sel);
    
    
    // A corrector for ECAL-based electron isolation [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMS/EcalIsolationCorrection2012Data
    EcalIsolationCorrector ecalIsoCorr(true);
    
    
    // Loop through the electron collection and fill the relevant variables
    for (eleSize = 0; eleSize < int(electrons->size())  &&  eleSize < MAX_LEN; ++eleSize)
    {
        pat::Electron const &el = electrons->at(eleSize);
        
        
        elePt[eleSize] = el.ecalDrivenMomentum().pt();
        eleEta[eleSize] = el.ecalDrivenMomentum().eta();
        elePhi[eleSize] = el.ecalDrivenMomentum().phi();
        //^ Gsf momentum is used instead of the one calculated by the particle-flow algorithm
        //https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
        
        eleCharge[eleSize] = (el.charge() == -1) ? true : false;
        eleDB[eleSize] = el.dB();
        
        
        // Effective-area (rho) correction to isolation (*)
        //(*) https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections
        eleRelIso[eleSize] = (el.chargedHadronIso() + std::max(el.neutralHadronIso() +
         el.photonIso() - 1. * el.userIsolation("User1Iso"), 0.)) / el.pt();
        
        
        // Trigger-emulating preselection for MVA ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Training_of_the_MVA
        eleTriggerPreselection[eleSize] = false;
        
        if (el.dr03TkSumPt() / el.pt() < 0.2 and /* ECAL-based isolation is addressed later */
         el.dr03HcalTowerSumEt() / el.pt() < 0.2 and
         el.gsfTrack()->trackerExpectedHitsInner().numberOfLostHits() == 0)
        {
            // Calculate a corrected ECAL-based isolation as described in [1]. The corrected
            //isolation might be negative (confirmed by Matteo Sani privately)
            //[1] https://twiki.cern.ch/twiki/bin/view/CMS/EcalIsolationCorrection2012Data
            float correctedECALIso;
            
            if (runOnData)
                correctedECALIso = ecalIsoCorr.correctForHLTDefinition(el, runNumber, true);
            else
                correctedECALIso = ecalIsoCorr.correctForHLTDefinition(el, false);
            
            // Check the rest of requirements for trigger-emulating preselection
            if (correctedECALIso / el.pt() < 0.2)
                eleTriggerPreselection[eleSize] =  (fabs(el.superCluster()->eta()) < 1.479) ?
                 (el.sigmaIetaIeta() < 0.014 and el.hadronicOverEm() < 0.15) :
                 (el.sigmaIetaIeta() < 0.035 and el.hadronicOverEm() < 0.10);
        }
        
        
        // Triggering MVA ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification
        eleMVAID[eleSize] = el.electronID("mvaTrigV0");
        
        // Cut-based electron ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/SimpleCutBasedEleID
        eleIDSimple70cIso[eleSize] = el.electronID("simpleEleId70cIso");
        
        
        elePassConversion[eleSize] = el.passConversionVeto()
         and (el.gsfTrack()->trackerExpectedHitsInner().numberOfHits() <= 0);
        //^ See (*). The decision is stored by PATElectronProducer based on the collection
        //"allConversions" (the name is hard-coded). The additional requirement to reject electrons
        //from the photon conversion is set according to (**).
        //(*) https://twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
        //(**) https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel#Electrons
        
        
        for (unsigned i = 0; i < eleSelectors.size(); ++i)
            eleSelectionBits[i][eleSize] = eleSelectors[i](el);
    }
    

    // Read the muon collection
    Handle<View<pat::Muon>> muons;
    event.getByLabel(muSrc, muons);
    
    // Constuct the muon selectors
    vector<StringCutObjectSelector<pat::Muon>> muSelectors;
    
    for (vector<string>::const_iterator sel = muSelection.begin(); sel != muSelection.end(); ++sel)
        muSelectors.push_back(*sel);
    
    // Loop through the muon collection and fill the relevant variables
    for (muSize = 0; muSize < int(muons->size())  &&  muSize < MAX_LEN; ++muSize)
    {
        pat::Muon const &mu = muons->at(muSize);
        
        muPt[muSize] = mu.pt();
        muEta[muSize] = mu.eta();
        muPhi[muSize] = mu.phi();
        
        muCharge[muSize] = (mu.charge() == -1) ? true : false;
        muDB[muSize] = mu.dB();
        
        // Relative isolation with delta-beta correction. Logic of the calculation follows [1]. Note
        //that it is calculated differently from [2], but the adopted recipe is more natural for
        //PFBRECO
        //[1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/CommonTools/ParticleFlow/interface/IsolatedPFCandidateSelectorDefinition.h?revision=1.4&view=markup
        //[2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#Accessing_PF_Isolation_from_reco
        muRelIso[muSize] = (mu.chargedHadronIso() +
         std::max(mu.neutralHadronIso() + mu.photonIso() - 0.5 * mu.puChargedHadronIso(), 0.)) /
         mu.pt();
        
        // Tight muons are defined according to [1]. Note it does not imply selection on isolation
        //or kinematics
        //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId?rev=48#Tight_Muon
        muQualityTight[muSize] = mu.isTightMuon(vertices->front());
        
        for (unsigned i = 0; i < muSelectors.size(); ++i)
            muSelectionBits[i][muSize] = muSelectors[i](mu);
    }
    
    
    // Read the jets collections
    Handle<View<pat::Jet>> jets;
    event.getByLabel(jetSrc, jets);
    
    Handle<View<pat::Jet>> jetsJERUp, jetsJERDown;
    
    
    // Jet PU ID maps. See [1]; though, the actual code differs from what is written in the TWiki
    //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID
    Handle<ValueMap<float>> jetPUCutBasedDiscrHandle;
    event.getByLabel("puJetMvaChs", "cutbasedDiscriminant", jetPUCutBasedDiscrHandle);
    
    Handle<ValueMap<int>> jetPUCutBasedIDHandle;
    event.getByLabel("puJetMvaChs", "cutbasedId", jetPUCutBasedIDHandle);
    
    Handle<ValueMap<float>> jetPUSimpleDiscrHandle;
    event.getByLabel("puJetMvaChs", "simpleDiscriminant", jetPUSimpleDiscrHandle);
    
    Handle<ValueMap<int>> jetPUSimpleIDHandle;
    event.getByLabel("puJetMvaChs", "simpleId", jetPUSimpleIDHandle);
    
    Handle<ValueMap<float>> jetPUFullDiscrHandle;
    event.getByLabel("puJetMvaChs", "fullDiscriminant", jetPUFullDiscrHandle);
    
    Handle<ValueMap<int>> jetPUFullIDHandle;
    event.getByLabel("puJetMvaChs", "fullId", jetPUFullIDHandle);
    
    
    if (!runOnData)
    {
        event.getByLabel(JERSystJetsSrc[0], jetsJERUp);
        event.getByLabel(JERSystJetsSrc[1], jetsJERDown);
    }
    
    // Construct the jet selectors
    StringCutObjectSelector<pat::Jet> jetSaveSelector(jetCut);
    StringCutObjectSelector<pat::Jet> softJetSelector(softJetCut);
    vector<StringCutObjectSelector<pat::Jet>> jetSelectors;
    
    for (vector<string>::const_iterator sel = jetSelection.begin(); sel != jetSelection.end();
     ++sel)
        jetSelectors.push_back(*sel);
    
    jetSize = 0;
    TLorentzVector softJetSumP4, softJetSumP4JERUp, softJetSumP4JERDown;
    softJetHt = softJetHtJERUp = softJetHtJERDown = 0.;
    TLorentzVector softJetSumP4JECUnc = 0.;
    
    
    // Loop through the jet collection and fill the relevant variables
    for (unsigned int i = 0; i < jets->size(); ++i)
    {
        pat::Jet const &j = jets->at(i);
        pat::Jet const *jJERUp = NULL, *jJERDown = NULL;
        
        if (!runOnData)
        {
            jJERUp = &jetsJERUp->at(i);
            jJERDown = &jetsJERDown->at(i);
        }
        
        if (jetSaveSelector(j)  &&  jetSize < MAX_LEN)
        {
            jetPt[jetSize] = j.pt();
            jetEta[jetSize] = j.eta();
            jetPhi[jetSize] = j.phi();
            jetMass[jetSize] = j.mass();
            
            if (!runOnData)
            {
                // JEC uncertainties (*)
                //(*) https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetCorUncertainties
                jecUncProvider->setJetEta(j.eta());
                jecUncProvider->setJetPt(j.pt());
                jecUncertainty[jetSize] = jecUncProvider->getUncertainty(true);
                
                
                // JER systematics
                jetPtJERUp[jetSize] = jJERUp->pt();
                jetMassJERUp[jetSize] = jJERUp->mass();
                jetPtJERDown[jetSize] = jJERDown->pt();
                jetMassJERDown[jetSize] = jJERDown->mass();
            }
            
            jetNDaughters[jetSize] = j.numberOfDaughters();
            jetEleMultiplicity[jetSize] = j.electronMultiplicity();
            jetMuMultiplicity[jetSize] = j.muonMultiplicity();
            
            jetTCHP[jetSize] = j.bDiscriminator("trackCountingHighPurBJetTags");
            jetCSV[jetSize] = j.bDiscriminator("combinedSecondaryVertexBJetTags");
            jetJP[jetSize] = j.bDiscriminator("jetProbabilityBJetTags");
            
            
            // Calculate the secondary vertex mass [1-3]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookPATExampleTrackBJet#ExerCise5
            //[2] https://hypernews.cern.ch/HyperNews/CMS/get/btag/718/1.html
            //[3] https://hypernews.cern.ch/HyperNews/CMS/get/physTools/2714.html
            reco::SecondaryVertexTagInfo const *svTagInfo = j.tagInfoSecondaryVertex();
            
            if (svTagInfo  &&  svTagInfo->nVertices() > 0)
                jetSecVertexMass[jetSize] = svTagInfo->secondaryVertex(0).p4().mass();
            else
                jetSecVertexMass[jetSize] = -100.;
            
            
            // PU jet ID [1]
            //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID
            jetPUCutBasedDiscr[jetSize] = (*jetPUCutBasedDiscrHandle)[jets->refAt(jetSize)];
            jetPUCutBasedID[jetSize] = (*jetPUCutBasedIDHandle)[jets->refAt(jetSize)];
            
            jetPUSimpleDiscr[jetSize] = (*jetPUSimpleDiscrHandle)[jets->refAt(jetSize)];
            jetPUSimpleID[jetSize] = (*jetPUSimpleIDHandle)[jets->refAt(jetSize)];
            
            jetPUFullDiscr[jetSize] = (*jetPUFullDiscrHandle)[jets->refAt(jetSize)];
            jetPUFullID[jetSize] = (*jetPUFullIDHandle)[jets->refAt(jetSize)];
            
            
            for (unsigned i = 0; i < jetSelectors.size(); ++i)
             jetSelectionBits[i][jetSize] = jetSelectors[i](j);
            
            
            if (!runOnData)
            // These are variables is from the generator tree, but it's more convenient to
            //calculate it here
            {
                jetFlavour[jetSize] = j.partonFlavour();
                jetGenPartonFlavour[jetSize] = (j.genParton() == nullptr) ? 0 :
                 j.genParton()->pdgId();
            }
            
            
            ++jetSize;
        }
        else if (softJetSelector(j))
        {
            TLorentzVector p4(j.px(), j.py(), j.pz(), j.energy());
            softJetSumP4 += p4;
            softJetHt += j.pt();
            
            if (!runOnData)
            {
                jecUncProvider->setJetPt(j.pt());
                jecUncProvider->setJetEta(j.eta());
                double const unc = jecUncProvider->getUncertainty(true);
                
                softJetSumP4JECUnc += unc * p4;
                softJetHtJECUnc += unc * j.pt();
                
                softJetSumP4JERUp += TLorentzVector(jJERUp->px(), jJERUp->py(), jJERUp->pz(),
                 jJERUp->energy());
                softJetHtJERUp += jJERUp->pt();
                
                softJetSumP4JERDown += TLorentzVector(jJERDown->px(), jJERDown->py(),
                 jJERDown->pz(), jJERDown->energy());
                softJetHtJERDown += jJERDown->pt();
            }
        }
    }
    
    softJetPt = softJetSumP4.Pt();
    softJetEta = (softJetPt > 0.) ? softJetSumP4.Eta() : 10.e10;
    softJetPhi = softJetSumP4.Phi();
    softJetMass = softJetSumP4.M();
    
    if (!runOnData)
    {
        softJetPtJECUnc = softJetSumP4JECUnc.Pt();
        softJetEtaJECUnc = (softJetSumP4JECUnc.Pt() > 0.) ? softJetSumP4JECUnc.Eta() : 10.e10;
        softJetPhiJECUnc = softJetSumP4JECUnc.Phi();
        softJetMassJECUnc = softJetSumP4JECUnc.M();
        
        softJetPtJERUp = softJetSumP4JERUp.Pt();
        softJetEtaJERUp = (softJetPtJERUp > 0.) ? softJetSumP4JERUp.Eta() : 10.e10;
        softJetPhiJERUp = softJetSumP4JERUp.Phi();
        softJetMassJERUp = softJetSumP4JERUp.M();
        
        softJetPtJERDown = softJetSumP4JERDown.Pt();
        softJetEtaJERDown = (softJetPtJERDown > 0.) ? softJetSumP4JERDown.Eta() : 10.e10;
        softJetPhiJERDown = softJetSumP4JERDown.Phi();
        softJetMassJERDown = softJetSumP4JERDown.M();
    }
    
    
    // Read the MET
    metSize = 0;
    
    for (vector<InputTag>::const_iterator tag = METSrcs.begin(); tag != METSrcs.end(); ++tag)
    {
        Handle<View<pat::MET>> METs;
        event.getByLabel(*tag, METs);
        
        metPt[metSize] = METs->front().pt();
        metPhi[metSize] = METs->front().phi();
        
        ++metSize;
    }
    
    
    // Save the generator information (however the jet generator info is already saved)
    // Particles in the final state of ME
    if (!runOnData and saveHardInteraction)
    {
        // Read the generator-level particles
        Handle<View<reco::GenParticle>> genParticles;
        event.getByLabel(genParticlesSrc, genParticles);
        
        hardPartSize = 0;
        vector<reco::GenParticle const *> visitedParticles;
        
        for (unsigned i = 2; i < genParticles->size(); ++i)  // skip the protons
        {
            reco::GenParticle const &p = genParticles->at(i);
            
            if (p.status() != 3)  // all the status 3 particles are at the beginning of listing
                break;
            
            int const nMothers = p.numberOfMothers();
            
            if (nMothers > 0)
            {
                auto it = find(visitedParticles.begin(), visitedParticles.end(), p.mother(0));
                hardPartFirstMother[hardPartSize] = (it != visitedParticles.end()) ?
                 it - visitedParticles.begin() : -1;
                //^ The indices correspond to the position in the stored arrays, not genParticles
                
                if (nMothers > 1)
                {
                    it = find(visitedParticles.begin(), visitedParticles.end(),
                     p.mother(nMothers - 1));
                    hardPartLastMother[hardPartSize] = (it != visitedParticles.end()) ?
                     it - visitedParticles.begin() : -1;
                }
                else
                    hardPartLastMother[hardPartSize] = hardPartFirstMother[hardPartSize];
                
            }
            else
                hardPartFirstMother[hardPartSize] = hardPartLastMother[hardPartSize] = -1;
            
            hardPartPdgId[hardPartSize] = p.pdgId();
            hardPartPt[hardPartSize] = p.pt();
            hardPartEta[hardPartSize] = p.eta();
            hardPartPhi[hardPartSize] = p.phi();
            hardPartMass[hardPartSize] = p.mass();
            
            
            ++hardPartSize;
            visitedParticles.push_back(&genParticles->at(i));
        }
    }
    
    // Save the PDF and other generator information
    if (!runOnData)
    {
        Handle<GenEventInfoProduct> generator;
        event.getByLabel(generatorSrc, generator);
        
        processID = generator->signalProcessID();
        genWeight = generator->weight();
        
        GenEventInfoProduct::PDF const *pdf = generator->pdf();
        pdfX1 = pdf->x.first;
        pdfX2 = pdf->x.second;
        pdfQ = pdf->scalePDF;
        pdfId1 = pdf->id.first;
        pdfId2 = pdf->id.second;
    }
    
    
    // Save the pile-up information
    //https://twiki.cern.ch/twiki/bin/view/CMS/Pileup_MC_Information
    if (!runOnData)
    {
        Handle<View<PileupSummaryInfo>> PUSummary;
        event.getByLabel(PUSummarySrc, PUSummary);
        
        PUSize = PUSummary->size();
        // The true number of interactions is the same for the whole event
        PUTrueNumInteractions = PUSummary->front().getTrueNumInteractions();
        
        for (int i = 0; i < PUSize  &&  i < MAX_LEN; ++i)
        {
            PUBunchCrossing[i] = PUSummary->at(i).getBunchCrossing();
            PUNumInteractions[i] = PUSummary->at(i).getPU_NumInteractions();
        }
    }
    
    // Read rho
    Handle<double> rho;
    event.getByLabel(rhoSrc, rho);
    PURho = *rho;
    
    // Primary vertices have already been read before
   
    
    // Fill all the trees
    eventIDTree->Fill();
    basicInfoTree->Fill();
    integralPropTree->Fill();
    PUTree->Fill();
    
    if (!runOnData)
        generatorTree->Fill();
}


DEFINE_FWK_MODULE(PlainEventContent);
