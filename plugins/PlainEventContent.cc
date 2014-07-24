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

#include <CMGTools/External/interface/PileupJetIdentifier.h>

#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>
#include <SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h>

#include <TLorentzVector.h>
#include <Math/GenVector/VectorUtil.h>

#include <memory>
#include <cmath>
#include <sstream>


using namespace edm;
using namespace std;


// A static data member
unsigned const PlainEventContent::maxSize;


PlainEventContent::PlainEventContent(edm::ParameterSet const &cfg):
    eleSrc(cfg.getParameter<InputTag>("electrons")),
    muSrc(cfg.getParameter<InputTag>("muons")),
    jetSrc(cfg.getParameter<InputTag>("jets")),
    metSrc(cfg.getParameter<vector<InputTag>>("METs")),
    
    jetMinPt(cfg.getParameter<double>("jetMinPt")),
    jetMinRawPt(cfg.getParameter<double>("jetMinRawPt")),
    
    eleSelection(cfg.getParameter<vector<string>>("eleSelection")),
    muSelection(cfg.getParameter<vector<string>>("muSelection")),
    jetSelection(cfg.getParameter<vector<string>>("jetSelection")),
    
    runOnData(cfg.getParameter<bool>("runOnData")),
    saveHardInteraction(cfg.getParameter<bool>("saveHardInteraction")),
    
    generatorSrc(cfg.getParameter<InputTag>("generator")),
    genParticlesSrc(cfg.getParameter<InputTag>("genParticles")),
    primaryVerticesSrc(cfg.getParameter<InputTag>("primaryVertices")),
    puSummarySrc(cfg.getParameter<InputTag>("puInfo")),
    rhoSrc(cfg.getParameter<InputTag>("rho")),
    jetPileUpIDSrc(cfg.getParameter<vector<InputTag>>("jetPileUpID"))
{
    if (jetPileUpIDSrc.size() > 2)
    {
        edm::Exception excp(edm::errors::LogicError);
        excp << "Two sources of jet pile-up ID are expected at maximum while " <<
         jetPileUpIDSrc.size() << "have been provided.\n";
        excp.raise();
    }
    
    
    // Allocate the buffers to store the bits of additional selection
    eleSelectionBits = new Bool_t *[eleSelection.size()];
    muSelectionBits = new Bool_t *[muSelection.size()];
    jetSelectionBits = new Bool_t *[jetSelection.size()];
    
    for (unsigned i = 0; i < eleSelection.size(); ++i)
        eleSelectionBits[i] = new Bool_t[maxSize];
    
    for (unsigned i = 0; i < muSelection.size(); ++i)
        muSelectionBits[i] = new Bool_t[maxSize];
    
    for (unsigned i = 0; i < jetSelection.size(); ++i)
        jetSelectionBits[i] = new Bool_t[maxSize];
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
    eventIDTree = fileService->make<TTree>("EventID", "Tree contrains event ID information");
    
    eventIDTree->Branch("run", &runNumber);
    eventIDTree->Branch("lumi", &lumiSection);
    eventIDTree->Branch("event", &eventNumber);
    
    
    basicInfoTree = fileService->make<TTree>("BasicInfo",
     "Tree contains kinematics and other basic properties");
    
    electronsPointer = &electrons;
    basicInfoTree->Branch("electrons", &electronsPointer);
    
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
    basicInfoTree->Branch("eleIDSimple70cIso", eleIDSimple70cIso, "eleIDSimple70cIso[eleSize]/b");
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
    basicInfoTree->Branch("jetRawPt", jetRawPt, "jetRawPt[jetSize]/F");
    basicInfoTree->Branch("jetRawEta", jetRawEta, "jetRawEta[jetSize]/F");
    basicInfoTree->Branch("jetRawPhi", jetRawPhi, "jetRawPhi[jetSize]/F");
    basicInfoTree->Branch("jetRawMass", jetRawMass, "jetRawMass[jetSize]/F");
    
    basicInfoTree->Branch("jetTCHP", jetTCHP, "jetTCHP[jetSize]/F");
    basicInfoTree->Branch("jetCSV", jetCSV, "jetCSV[jetSize]/F");
    
    basicInfoTree->Branch("jetSecVertexMass", jetSecVertexMass, "jetSecVertexMass[jetSize]/F");
    
    basicInfoTree->Branch("jetArea", jetArea, "jetArea[jetSize]/F");
    basicInfoTree->Branch("jetCharge", jetCharge, "jetCharge[jetSize]/F");
    basicInfoTree->Branch("jetPullAngle", jetPullAngle, "jetPullAngle[jetSize]/F");
    
    if (jetPileUpIDSrc.size() > 0)
        basicInfoTree->Branch("jetPileUpID", jetPileUpID, "jetPileUpID[jetSize]/b");
    
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
    
    
    if (!runOnData)
    {
        generatorTree = fileService->make<TTree>("GeneratorInfo",
         "The tree keeps some generator information");
        
        generatorTree->Branch("processID", &processID);
        generatorTree->Branch("genWeight", &genWeight);
        
        generatorTree->Branch("jetSize", &jetSize);  // it's a duplication from basicInfoTree
        generatorTree->Branch("jetFlavour", jetFlavour, "jetFlavour[jetSize]/B");
        generatorTree->Branch("jetGenPartonFlavour", jetGenPartonFlavour,
         "jetGenPartonFlavour[jetSize]/B");
        generatorTree->Branch("jetGenJetMatch", jetGenJetMatch, "jetGenJetMatch[jetSize]/O");
        
        generatorTree->Branch("pdfX1", &pdfX1);
        generatorTree->Branch("pdfX2", &pdfX2);
        generatorTree->Branch("pdfQ", &pdfQ);
        generatorTree->Branch("pdfId1", &pdfId1);
        generatorTree->Branch("pdfId2", &pdfId2);
        
        if (saveHardInteraction)
        {
            generatorTree->Branch("hardPartSize", &hardPartSize);
            generatorTree->Branch("hardPartPdgId", hardPartPdgId, "hardPartPdgId[hardPartSize]/B");
            generatorTree->Branch("hardPartFirstMother", hardPartFirstMother,
             "hardPartFirstMother[hardPartSize]/B");
            generatorTree->Branch("hardPartLastMother", hardPartLastMother,
             "hardPartLastMother[hardPartSize]/B");
            generatorTree->Branch("hardPartPt", hardPartPt, "hardPartPt[hardPartSize]/F");
            generatorTree->Branch("hardPartEta", hardPartEta, "hardPartEta[hardPartSize]/F");
            generatorTree->Branch("hardPartPhi", hardPartPhi, "hardPartPhi[hardPartSize]/F");
            generatorTree->Branch("hardPartMass", hardPartMass, "hardPartMass[hardPartSize]/F");
        }
    }
    
    
    puTree = fileService->make<TTree>("PUInfo", "Pile-up information");
    puTree->Branch("pvSize", &pvSize);
    puTree->Branch("rho", &puRho);
    
    if (!runOnData)
    {
        puTree->Branch("puTrueNumInteractions", &puTrueNumInteractions);
        puTree->Branch("puSize", &puSize);
        puTree->Branch("puBunchCrossing", puBunchCrossing, "puBunchCrossing[puSize]/B");
        puTree->Branch("puNumInteractions", puNumInteractions, "puNumInteractions[puSize]/b");
    }
}


void PlainEventContent::endJob()
{}


void PlainEventContent::beginRun(edm::Run const &run, edm::EventSetup const &setup)
{}


void PlainEventContent::endRun(edm::Run const &run, edm::EventSetup const &setup)
{}


void PlainEventContent::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Fill the event ID tree
    runNumber = event.id().run();
	eventNumber = event.id().event();
	lumiSection = event.luminosityBlock();
    
    
    // Read the primary vertices
    Handle<reco::VertexCollection> vertices;
    event.getByLabel(primaryVerticesSrc, vertices);
    
    pvSize = vertices->size();
    
    if (pvSize == 0)
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
    PlainEventContent::electrons.clear();
    
    for (eleSize = 0; eleSize < int(electrons->size()) and eleSize < maxSize; ++eleSize)
    {
        pat::Electron const &el = electrons->at(eleSize);
        
        pec::Candidate storeElectron;
        storeElectron.SetPt(el.ecalDrivenMomentum().pt());
        storeElectron.SetEta(el.ecalDrivenMomentum().eta());
        storeElectron.SetPhi(el.ecalDrivenMomentum().phi());
        PlainEventContent::electrons.push_back(storeElectron);
        
        
        elePt[eleSize] = el.ecalDrivenMomentum().pt();
        eleEta[eleSize] = el.ecalDrivenMomentum().eta();
        elePhi[eleSize] = el.ecalDrivenMomentum().phi();
        //^ Gsf momentum is used instead of the one calculated by the particle-flow algorithm
        //https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
        
        eleCharge[eleSize] = (el.charge() == -1) ? true : false;
        eleDB[eleSize] = el.dB();
        
        
        // Effective-area (rho) correction to isolation (*)
        //(*) https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections
        eleRelIso[eleSize] = (el.chargedHadronIso() + max(el.neutralHadronIso() +
         el.photonIso() - 1. * el.userIsolation("User1Iso"), 0.)) / elePt[eleSize];
        
        
        // Trigger-emulating preselection for MVA ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Training_of_the_MVA
        eleTriggerPreselection[eleSize] = false;
        
        if (el.dr03TkSumPt() / elePt[eleSize] < 0.2 and /* ECAL-based isolation is addressed later */
         el.dr03HcalTowerSumEt() / elePt[eleSize] < 0.2 and
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
            //^ Code snippet in the reference above operates with a Gsf electron instead of a PAT
            //one given here, but it only reads an isolation from it and check if it is in the
            //barrel [1]; thus, a PAT electron is also suitable
            //[1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/EgammaAnalysis/ElectronTools/src/EcalIsolationCorrector.cc?view=markup
            
            // Check the rest of requirements for trigger-emulating preselection
            if (correctedECALIso / elePt[eleSize] < 0.2)
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
    for (muSize = 0; muSize < int(muons->size()) and muSize < maxSize; ++muSize)
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
         max(mu.neutralHadronIso() + mu.photonIso() - 0.5 * mu.puChargedHadronIso(), 0.)) / mu.pt();
        
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
    
    
    // Jet pile-up ID maps
    vector<Handle<ValueMap<int>>> jetPileUpIDHandles(jetPileUpIDSrc.size());
    
    for (unsigned i = 0; i < jetPileUpIDSrc.size(); ++i)
        event.getByLabel(jetPileUpIDSrc.at(i), jetPileUpIDHandles.at(i));
    
    
    // Construct the jet selectors
    vector<StringCutObjectSelector<pat::Jet>> jetSelectors;
    
    for (vector<string>::const_iterator sel = jetSelection.begin(); sel != jetSelection.end();
     ++sel)
        jetSelectors.push_back(*sel);
    
    
    jetSize = 0;
    
    // Loop through the jet collection and fill the relevant variables
    for (unsigned int i = 0; i < jets->size(); ++i)
    {
        pat::Jet const &j = jets->at(i);
        reco::Candidate::LorentzVector const &rawP4 = j.correctedP4("Uncorrected");
        
        if ((j.pt() > jetMinPt or rawP4.pt() > jetMinRawPt) and jetSize < maxSize)
        {
            jetRawPt[jetSize] = rawP4.pt();
            jetRawEta[jetSize] = rawP4.eta();
            jetRawPhi[jetSize] = rawP4.phi();
            jetRawMass[jetSize] = rawP4.mass();
            
            jetTCHP[jetSize] = j.bDiscriminator("trackCountingHighPurBJetTags");
            jetCSV[jetSize] = j.bDiscriminator("combinedSecondaryVertexBJetTags");
            
            
            // Calculate the secondary vertex mass [1-3]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookPATExampleTrackBJet#ExerCise5
            //[2] https://hypernews.cern.ch/HyperNews/CMS/get/btag/718/1.html
            //[3] https://hypernews.cern.ch/HyperNews/CMS/get/physTools/2714.html
            reco::SecondaryVertexTagInfo const *svTagInfo = j.tagInfoSecondaryVertex();
            
            if (svTagInfo and svTagInfo->nVertices() > 0)
                jetSecVertexMass[jetSize] = svTagInfo->secondaryVertex(0).p4().mass();
            else
                jetSecVertexMass[jetSize] = -100.;
            
            
            // Jet area
            jetArea[jetSize] = j.jetArea();
            
            
            // Jet electric charge
            jetCharge[jetSize] = j.jetCharge();
            
            
            // Calculate the jet pull angle
            double const y = rawP4.Rapidity();
            double const phi = rawP4.phi();
            double pullY = 0., pullPhi = 0.;  // projections of the pull vector (unnormalised)
            
            // Loop over constituents of the jet
            for (reco::PFCandidatePtr const &p: j.getPFConstituents())
            {
                double dPhi = p->phi() - phi;
                
                if (dPhi < -TMath::Pi())
                    dPhi = 2 * TMath::Pi() + dPhi;
                else if (dPhi > TMath::Pi())
                    dPhi = -2 * TMath::Pi() + dPhi;
                
                double const r = hypot(p->rapidity() - y, dPhi);
                pullY += p->pt() * r * (p->rapidity() - y);
                pullPhi += p->pt() * r * dPhi;
            }
            //^ The pull vector should be normalised by the jet's pt, but since I'm interested in
            //the polar angle only, it is not necessary
            
            jetPullAngle[jetSize] = atan2(pullPhi, pullY);
            
            
            // Pack jet pile-up ID into a single short
            jetPileUpID[jetSize] = 0;
            
            if (jetPileUpIDHandles.size() > 0)
            {
                int const pileUpID = (*jetPileUpIDHandles.at(0))[jets->refAt(jetSize)];
                
                if (PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kLoose))
                    jetPileUpID[jetSize] |= (1<<0);
                
                if (PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kMedium))
                    jetPileUpID[jetSize] |= (1<<1);
                
                if (PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kTight))
                    jetPileUpID[jetSize] |= (1<<2);
            }
            
            if (jetPileUpIDHandles.size() > 1)
            {
                int const pileUpID = (*jetPileUpIDHandles.at(1))[jets->refAt(jetSize)];
                
                if (PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kLoose))
                    jetPileUpID[jetSize] |= (1<<3);
                
                if (PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kMedium))
                    jetPileUpID[jetSize] |= (1<<4);
                
                if (PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kTight))
                    jetPileUpID[jetSize] |= (1<<5);
            }
            
            
            for (unsigned i = 0; i < jetSelectors.size(); ++i)
             jetSelectionBits[i][jetSize] = jetSelectors[i](j);
            
            
            if (!runOnData)
            // These are variables is from the generator tree, but it's more convenient to
            //calculate it here
            {
                jetFlavour[jetSize] = j.partonFlavour();
                jetGenPartonFlavour[jetSize] = (j.genParton() == nullptr) ? 0 :
                 j.genParton()->pdgId();
                 
                jetGenJetMatch[jetSize] = (j.genJet() and j.genJet()->pt() > 8. and
                 ROOT::Math::VectorUtil::DeltaR(j.p4(), j.genJet()->p4()) < 0.25);
                //^ The matching is performed according to the definition from JME-13-005. By
                //default, PAT uses a looser definition
            }
            
            
            ++jetSize;
        }
    }
    
    
    // Read the MET
    metSize = 0;
    
    for (vector<InputTag>::const_iterator tag = metSrc.begin(); tag != metSrc.end(); ++tag)
    {
        Handle<View<pat::MET>> met;
        event.getByLabel(*tag, met);
        
        metPt[metSize] = met->front().pt();
        metPhi[metSize] = met->front().phi();
        
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
        
        if (pdf)
        {
            pdfX1 = pdf->x.first;
            pdfX2 = pdf->x.second;
            pdfQ = pdf->scalePDF;
            pdfId1 = pdf->id.first;
            pdfId2 = pdf->id.second;
        }
        else  // information on PDF is not available for datasets with particle gun
        {
            pdfX1 = pdfX2 = -1.f;
            pdfQ = -1.f;
            pdfId1 = pdfId2 = -100;
        }
    }
    
    
    // Save the pile-up information
    //https://twiki.cern.ch/twiki/bin/view/CMS/Pileup_MC_Information
    if (!runOnData)
    {
        Handle<View<PileupSummaryInfo>> puSummary;
        event.getByLabel(puSummarySrc, puSummary);
        
        puSize = puSummary->size();
        // The true number of interactions is the same for the whole event
        puTrueNumInteractions = puSummary->front().getTrueNumInteractions();
        
        for (unsigned i = 0; i < puSize and i < maxSize; ++i)
        {
            puBunchCrossing[i] = puSummary->at(i).getBunchCrossing();
            puNumInteractions[i] = puSummary->at(i).getPU_NumInteractions();
        }
    }
    
    // Read rho
    Handle<double> rho;
    event.getByLabel(rhoSrc, rho);
    puRho = *rho;
    
    // Primary vertices have already been read before
    
    
    // Fill all the trees
    eventIDTree->Fill();
    basicInfoTree->Fill();
    puTree->Fill();
    
    if (!runOnData)
        generatorTree->Fill();
}


void PlainEventContent::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    // Documentation for descriptions of the configuration is available in [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideConfigurationValidationAndHelp
    
    edm::ParameterSetDescription desc;
    desc.add<bool>("runOnData")->
     setComment("Indicates whether data or simulation is being processed.");
    desc.add<bool>("saveHardInteraction", false)->
     setComment("Determines if particles from the hard interaction should be stored. It is ignored "
     "if runOnData is true.");
    desc.add<InputTag>("primaryVertices")->
     setComment("Collection of reconstructed primary vertices.");
    desc.add<InputTag>("electrons")->setComment("Collection of electrons.");
    desc.add<vector<string>>("eleSelection", vector<string>(0))->
     setComment("User-defined selections for electrons whose results will be stored in the output "
     "trees.");
    desc.add<InputTag>("muons")->setComment("Collection of muons.");
    desc.add<vector<string>>("muSelection", vector<string>(0))->
     setComment("User-defined selections for muons whose results will be stored in the ouput "
     "trees.");
    desc.add<InputTag>("jets")->setComment("Collection of jets.");
    desc.add<vector<string>>("jetSelection", vector<string>(0))->
     setComment("User-defined selections for jets whose results will be stored in the output "
     "trees.");
    desc.add<double>("jetMinPt", 20.)->
     setComment("Jets with pt above this threshold will be stored in the output trees.");
    desc.add<double>("jetMinRawPt", 10.)->
     setComment("Jets with raw pt above this threshold will be stored in the output trees.");
    desc.add<vector<InputTag>>("METs")->setComment("MET. Several versions of it can be stored.");
    desc.add<InputTag>("generator", InputTag("generator"))->
     setComment("Tag to access information about generator. If runOnData is true, this parameter "
     "is ignored.");
    desc.add<InputTag>("genParticles", InputTag("genParticles"))->
     setComment("Tag to access generator particles. If runOnData is true, this parameter is "
     "ignored.");
    desc.add<InputTag>("rho", InputTag("kt6PFJets", "rho"))->
     setComment("Rho (mean angular energy density).");
    desc.add<InputTag>("puInfo", InputTag("addPileupInfo"))->
     setComment("True pile-up information. If runOnData is true, this parameter is ignored.");
    desc.add<vector<InputTag>>("jetPileUpID", vector<InputTag>(0))->
     setComment("Value maps with jet pile-up ID. Can contain 0, 1, or 2 entries. In the latter "
     "case the first InputTag is supposed to denote the map for cut-based ID and the second one "
     "should provide MVA ID.");
    
    descriptions.add("eventContent", desc);
}


DEFINE_FWK_MODULE(PlainEventContent);
