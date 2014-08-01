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
    electronTag(cfg.getParameter<InputTag>("electrons")),
    muonTag(cfg.getParameter<InputTag>("muons")),
    jetTag(cfg.getParameter<InputTag>("jets")),
    metTags(cfg.getParameter<vector<InputTag>>("METs")),
    
    jetMinPt(cfg.getParameter<double>("jetMinPt")),
    jetMinRawPt(cfg.getParameter<double>("jetMinRawPt")),
    
    eleSelection(cfg.getParameter<vector<string>>("eleSelection")),
    muSelection(cfg.getParameter<vector<string>>("muSelection")),
    jetSelection(cfg.getParameter<vector<string>>("jetSelection")),
    
    runOnData(cfg.getParameter<bool>("runOnData")),
    saveHardInteraction(cfg.getParameter<bool>("saveHardInteraction")),
    
    generatorTag(cfg.getParameter<InputTag>("generator")),
    genParticlesTag(cfg.getParameter<InputTag>("genParticles")),
    primaryVerticesTag(cfg.getParameter<InputTag>("primaryVertices")),
    puSummaryTag(cfg.getParameter<InputTag>("puInfo")),
    rhoTag(cfg.getParameter<InputTag>("rho")),
    jetPileUpIDTags(cfg.getParameter<vector<InputTag>>("jetPileUpID"))
{
    if (jetPileUpIDTags.size() > 2)
    {
        edm::Exception excp(edm::errors::LogicError);
        excp << "Two sources of jet pile-up ID are expected at maximum while " <<
         jetPileUpIDTags.size() << "have been provided.\n";
        excp.raise();
    }
}


PlainEventContent::~PlainEventContent()
{}


void PlainEventContent::beginJob()
{
    eventIDTree = fileService->make<TTree>("EventID", "Tree contrains event ID information");
    
    eventIDTree->Branch("run", &runNumber);
    eventIDTree->Branch("lumi", &lumiSection);
    eventIDTree->Branch("event", &eventNumber);
    
    
    basicInfoTree = fileService->make<TTree>("BasicInfo",
     "Tree contains kinematics and other basic properties");
    
    storeElectronsPointer = &storeElectrons;
    basicInfoTree->Branch("electrons", &storeElectronsPointer);
    
    storeMuonsPointer = &storeMuons;
    basicInfoTree->Branch("muons", &storeMuonsPointer);
    
    storeJetsPointer = &storeJets;
    basicInfoTree->Branch("jets", &storeJetsPointer);
    
    storeMETsPointer = &storeMETs;
    basicInfoTree->Branch("METs", &storeMETsPointer);
    
    
    if (!runOnData)
    {
        generatorTree = fileService->make<TTree>("GeneratorInfo",
         "The tree keeps some generator information");
        
        generatorTree->Branch("processID", &processID);
        generatorTree->Branch("genWeight", &genWeight);
        
        generatorTree->Branch("jetSize", &jetSize);  // it's a duplication from basicInfoTree
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
    event.getByLabel(primaryVerticesTag, vertices);
    
    pvSize = vertices->size();
    
    if (pvSize == 0)
    {
        edm::Exception excp(edm::errors::LogicError);
        excp << "Event contains zero good primary vertices.\n";
        excp.raise();
    }
    
    
    // Fill the tree with basic information
    // Read the electron collection
    Handle<View<pat::Electron>> srcElectrons;
    event.getByLabel(electronTag, srcElectrons);
    
    
    // Construct the electron selectors (s. SWGuidePhysicsCutParser)
    vector<StringCutObjectSelector<pat::Electron>> eleSelectors;
    
    for (vector<string>::const_iterator sel = eleSelection.begin(); sel != eleSelection.end();
     ++sel)
        eleSelectors.push_back(*sel);
    
    
    // A corrector for ECAL-based electron isolation [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMS/EcalIsolationCorrection2012Data
    EcalIsolationCorrector ecalIsoCorr(true);
    
    
    // Loop through the electron collection and fill the relevant variables
    storeElectrons.clear();
    pec::Electron storeElectron;  // will reuse this object to fill the vector
    
    for (unsigned i = 0; i < srcElectrons->size(); ++i)
    {
        pat::Electron const &el = srcElectrons->at(i);
        
        
        // Set four-momentum. Mass is ignored
        storeElectron.SetPt(el.ecalDrivenMomentum().pt());
        storeElectron.SetEta(el.ecalDrivenMomentum().eta());
        storeElectron.SetPhi(el.ecalDrivenMomentum().phi());
        //^ Gsf momentum is used instead of the one calculated by the particle-flow algorithm
        //https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel?rev=178#Electrons
        
        double const pt = el.ecalDrivenMomentum().pt();  // a short-cut to be used for isolation
        
        storeElectron.SetCharge(el.charge());
        storeElectron.SetDB(el.dB());
        
        // Effective-area (rho) correction to isolation [1]
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/TwikiTopRefHermeticTopProjections
        storeElectron.SetRelIso((el.chargedHadronIso() + max(el.neutralHadronIso() +
         el.photonIso() - 1. * el.userIsolation("User1Iso"), 0.)) / pt);
        
        // Triggering MVA ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification
        storeElectron.SetMvaID(el.electronID("mvaTrigV0"));
        
        // Cut-based electron ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/SimpleCutBasedEleID
        storeElectron.SetCutBasedID(el.electronID("simpleEleId70cIso"));
        
        // Conversion rejection. True for a "good" electron
        storeElectron.SetBit(0, el.passConversionVeto() and
         (el.gsfTrack()->trackerExpectedHitsInner().numberOfHits() <= 0));
        //^ See [1]. The decision is stored by PATElectronProducer based on the collection
        //"allConversions" (the name is hard-coded). The additional requirement to reject electrons
        //from the photon conversion is set according to [2].
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
        //[2] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel#Electrons
        
        
        // Trigger-emulating preselection for MVA ID [1]
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentification#Training_of_the_MVA
        bool passTriggerPreselection = false;
        
        if (el.dr03TkSumPt() / pt < 0.2 and /* ECAL-based isolation is addressed later */
         el.dr03HcalTowerSumEt() / pt < 0.2 and
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
            if (correctedECALIso / pt < 0.2)
                passTriggerPreselection =  (fabs(el.superCluster()->eta()) < 1.479) ?
                 (el.sigmaIetaIeta() < 0.014 and el.hadronicOverEm() < 0.15) :
                 (el.sigmaIetaIeta() < 0.035 and el.hadronicOverEm() < 0.10);
        }
        
        storeElectron.SetBit(1, passTriggerPreselection);
        
        
        // Evaluate user-defined selectors if any
        for (unsigned i = 0; i < eleSelectors.size(); ++i)
            storeElectron.SetBit(2 + i, eleSelectors[i](el));
        
        
        // The electron is set up. Add it to the vector
        storeElectrons.push_back(storeElectron);
    }
    
    
    // Read the muon collection
    Handle<View<pat::Muon>> srcMuons;
    event.getByLabel(muonTag, srcMuons);
    
    // Constuct the muon selectors
    vector<StringCutObjectSelector<pat::Muon>> muSelectors;
    
    for (vector<string>::const_iterator sel = muSelection.begin(); sel != muSelection.end(); ++sel)
        muSelectors.push_back(*sel);
    
    
    // Loop through the muon collection and fill the relevant variables
    storeMuons.clear();
    pec::Muon storeMuon;  // will reuse this object to fill the vector
    
    for (unsigned i = 0; i < srcMuons->size(); ++i)
    {
        pat::Muon const &mu = srcMuons->at(i);
        
        
        // Set four-momentum. Mass is ignored
        storeMuon.SetPt(mu.pt());
        storeMuon.SetEta(mu.eta());
        storeMuon.SetPhi(mu.phi());
        
        storeMuon.SetCharge(mu.charge());
        storeMuon.SetDB(mu.dB());
        
        // Relative isolation with delta-beta correction. Logic of the calculation follows [1]. Note
        //that it is calculated differently from [2], but the adopted recipe is more natural for
        //PFBRECO
        //[1] http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/CMSSW/CommonTools/ParticleFlow/interface/IsolatedPFCandidateSelectorDefinition.h?revision=1.4&view=markup
        //[2] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId#Accessing_PF_Isolation_from_reco
        storeMuon.SetRelIso((mu.chargedHadronIso() + max(mu.neutralHadronIso() + mu.photonIso() -
         0.5 * mu.puChargedHadronIso(), 0.)) / mu.pt());
        
        // Tight muons are defined according to [1]. Note it does not imply selection on isolation
        //or kinematics
        //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId?rev=48#Tight_Muon
        storeMuon.SetBit(0, mu.isTightMuon(vertices->front()));
        
        // Evaluate user-defined selectors if any
        for (unsigned i = 0; i < muSelectors.size(); ++i)
            storeMuon.SetBit(2 + i, muSelectors[i](mu));
        
        
        // The muon is set up. Add it to the vector
        storeMuons.push_back(storeMuon);
    }
    
    
    // Read the jets collections
    Handle<View<pat::Jet>> srcJets;
    event.getByLabel(jetTag, srcJets);
    
    
    // Jet pile-up ID maps
    vector<Handle<ValueMap<int>>> jetPileUpIDHandles(jetPileUpIDTags.size());
    
    for (unsigned i = 0; i < jetPileUpIDTags.size(); ++i)
        event.getByLabel(jetPileUpIDTags.at(i), jetPileUpIDHandles.at(i));
    
    
    // Construct the jet selectors
    vector<StringCutObjectSelector<pat::Jet>> jetSelectors;
    
    for (vector<string>::const_iterator sel = jetSelection.begin(); sel != jetSelection.end();
     ++sel)
        jetSelectors.push_back(*sel);
    
    
    jetSize = 0;
    
    // Loop through the jet collection and fill the relevant variables
    storeJets.clear();
    pec::Jet storeJet;  // will reuse this object to fill the vector
    
    for (unsigned int i = 0; i < srcJets->size(); ++i)
    {
        pat::Jet const &j = srcJets->at(i);
        reco::Candidate::LorentzVector const &rawP4 = j.correctedP4("Uncorrected");
        
        if ((j.pt() > jetMinPt or rawP4.pt() > jetMinRawPt) and jetSize < maxSize)
        {
            // Set four-momentum
            storeJet.SetPt(rawP4.pt());
            storeJet.SetEta(rawP4.eta());
            storeJet.SetPhi(rawP4.phi());
            storeJet.SetM(rawP4.mass());
            
            
            storeJet.SetArea(j.jetArea());
            storeJet.SetCharge(j.jetCharge());
            
            storeJet.SetBTagCSV(j.bDiscriminator("combinedSecondaryVertexBJetTags"));
            storeJet.SetBTagTCHP(j.bDiscriminator("trackCountingHighPurBJetTags"));
            
            
            // Calculate the secondary vertex mass [1-3]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookPATExampleTrackBJet#ExerCise5
            //[2] https://hypernews.cern.ch/HyperNews/CMS/get/btag/718/1.html
            //[3] https://hypernews.cern.ch/HyperNews/CMS/get/physTools/2714.html
            double secVertexMass = -100.;
            reco::SecondaryVertexTagInfo const *svTagInfo = j.tagInfoSecondaryVertex();
            
            if (svTagInfo and svTagInfo->nVertices() > 0)
                secVertexMass = svTagInfo->secondaryVertex(0).p4().mass();
            
            storeJet.SetSecVertexMass(secVertexMass);
            
            
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
            
            storeJet.SetPullAngle(atan2(pullPhi, pullY));
            
            
            // Pack jet pile-up ID into a single short. If single pile-up ID is provided in the
            //plugin's configuration, it is assumed to be cut-based. If two IDs are given, the first
            //one is cut-based, and the second is MVA.
            if (jetPileUpIDHandles.size() > 0)
            {
                int const pileUpID = (*jetPileUpIDHandles.at(0))[srcJets->refAt(jetSize)];
                
                storeJet.SetPileUpID(pec::Jet::PileUpIDAlgo::CutBased,
                 pec::Jet::PileUpIDWorkingPoint::Loose,
                 PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kLoose));
                storeJet.SetPileUpID(pec::Jet::PileUpIDAlgo::CutBased,
                 pec::Jet::PileUpIDWorkingPoint::Medium,
                 PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kMedium));
                storeJet.SetPileUpID(pec::Jet::PileUpIDAlgo::CutBased,
                 pec::Jet::PileUpIDWorkingPoint::Tight,
                 PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kTight));
            }
            
            if (jetPileUpIDHandles.size() > 1)
            {
                int const pileUpID = (*jetPileUpIDHandles.at(1))[srcJets->refAt(jetSize)];
                
                storeJet.SetPileUpID(pec::Jet::PileUpIDAlgo::MVA,
                 pec::Jet::PileUpIDWorkingPoint::Loose,
                 PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kLoose));
                storeJet.SetPileUpID(pec::Jet::PileUpIDAlgo::MVA,
                 pec::Jet::PileUpIDWorkingPoint::Medium,
                 PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kMedium));
                storeJet.SetPileUpID(pec::Jet::PileUpIDAlgo::MVA,
                 pec::Jet::PileUpIDWorkingPoint::Tight,
                 PileupJetIdentifier::passJetId(pileUpID, PileupJetIdentifier::kTight));
            }
            
            
            // User-difined selectors if any
            for (unsigned i = 0; i < jetSelectors.size(); ++i)
                storeJet.SetBit(i, jetSelectors[i](j));
            
            
            if (!runOnData)
            // These are variables is from the generator tree, but it's more convenient to
            //calculate it here
            {
                storeJet.SetFlavour(pec::Jet::FlavourType::Algorithmic, j.partonFlavour());
                storeJet.SetFlavour(pec::Jet::FlavourType::Physics,
                 (j.genParton() == nullptr) ? 0 : j.genParton()->pdgId());
                
                jetGenJetMatch[jetSize] = (j.genJet() and j.genJet()->pt() > 8. and
                 ROOT::Math::VectorUtil::DeltaR(j.p4(), j.genJet()->p4()) < 0.25);
                //^ The matching is performed according to the definition from JME-13-005. By
                //default, PAT uses a looser definition
            }
            
            
            // The jet is set up. Add it to the vector
            storeJets.push_back(storeJet);
            
            ++jetSize;
        }
    }
    
    
    // Read METs
    storeMETs.clear();
    pec::Candidate storeMET;  // will reuse this object to fill the vector
    
    for (vector<InputTag>::const_iterator tag = metTags.begin(); tag != metTags.end(); ++tag)
    {
        Handle<View<pat::MET>> met;
        event.getByLabel(*tag, met);
        
        storeMET.SetPt(met->front().pt());
        storeMET.SetPhi(met->front().phi());
        
        storeMETs.push_back(storeMET);
    }
    
    
    // Save the generator information (however the jet generator info is already saved)
    // Particles in the final state of ME
    if (!runOnData and saveHardInteraction)
    {
        // Read the generator-level particles
        Handle<View<reco::GenParticle>> genParticles;
        event.getByLabel(genParticlesTag, genParticles);
        
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
        event.getByLabel(generatorTag, generator);
        
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
        event.getByLabel(puSummaryTag, puSummary);
        
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
    event.getByLabel(rhoTag, rho);
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
