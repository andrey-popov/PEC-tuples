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
    
    generatorTag(cfg.getParameter<InputTag>("generator")),
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
    // Create the output tree
    outTree = fileService->make<TTree>("EventContent", "Minimalistic description of events");
    
    
    // A branch with event ID
    eventIdPointer = &eventId;
    outTree->Branch("eventId", &eventIdPointer);
    
    
    // Branches with reconstucted objects
    storeElectronsPointer = &storeElectrons;
    outTree->Branch("electrons", &storeElectronsPointer);
    
    storeMuonsPointer = &storeMuons;
    outTree->Branch("muons", &storeMuonsPointer);
    
    storeJetsPointer = &storeJets;
    outTree->Branch("jets", &storeJetsPointer);
    
    storeMETsPointer = &storeMETs;
    outTree->Branch("METs", &storeMETsPointer);
    
    
    // A branch with most basic generator-level information
    if (!runOnData)
    {
        generatorInfoPointer = &generatorInfo;
        outTree->Branch("genInfo", &generatorInfoPointer);
    }
    
    
    // A branch with per-event information on pile-up
    puInfoPointer = &puInfo;
    outTree->Branch("puInfo", &puInfoPointer);
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
    eventId.SetRunNumber(event.id().run());
    eventId.SetEventNumber(event.id().event());
    eventId.SetLumiSectionNumber(event.luminosityBlock());
    
    
    // Read the primary vertices
    Handle<reco::VertexCollection> vertices;
    event.getByLabel(primaryVerticesTag, vertices);
    
    if (vertices->size() == 0)
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
                correctedECALIso = ecalIsoCorr.correctForHLTDefinition(el, event.id().run(), true);
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
    
    
    // Loop through the jet collection and fill the relevant variables
    storeJets.clear();
    pec::Jet storeJet;  // will reuse this object to fill the vector
    
    for (unsigned int i = 0; i < srcJets->size(); ++i)
    {
        pat::Jet const &j = srcJets->at(i);
        reco::Candidate::LorentzVector const &rawP4 = j.correctedP4("Uncorrected");
        
        if (j.pt() > jetMinPt or rawP4.pt() > jetMinRawPt)
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
                int const pileUpID = (*jetPileUpIDHandles.at(0))[srcJets->refAt(i)];
                
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
                int const pileUpID = (*jetPileUpIDHandles.at(1))[srcJets->refAt(i)];
                
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
            
            
            if (!runOnData)
            // These are variables is from the generator tree, but it's more convenient to
            //calculate it here
            {
                storeJet.SetFlavour(pec::Jet::FlavourType::Algorithmic, j.partonFlavour());
                storeJet.SetFlavour(pec::Jet::FlavourType::Physics,
                 (j.genParton() == nullptr) ? 0 : j.genParton()->pdgId());
                
                storeJet.SetBit(0, (j.genJet() and j.genJet()->pt() > 8. and
                 ROOT::Math::VectorUtil::DeltaR(j.p4(), j.genJet()->p4()) < 0.25));
                //^ The matching is performed according to the definition from JME-13-005. By
                //default, PAT uses a looser definition
            }
            
            
            // User-difined selectors if any. The first bit has already been used for the match with
            //generator-level jet
            for (unsigned i = 0; i < jetSelectors.size(); ++i)
                storeJet.SetBit(i + 1, jetSelectors[i](j));
            
            
            // The jet is set up. Add it to the vector
            storeJets.push_back(storeJet);
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
    // Save the PDF and other generator information
    if (!runOnData)
    {
        Handle<GenEventInfoProduct> generator;
        event.getByLabel(generatorTag, generator);
        
        generatorInfo.Reset();
        //^ Same object is used for all events, hence need to reset it
        
        generatorInfo.SetProcessId(generator->signalProcessID());
        generatorInfo.SetWeight(generator->weight());
        
        
        GenEventInfoProduct::PDF const *pdf = generator->pdf();
        
        if (pdf)
        {
            generatorInfo.SetPdfXs(pdf->x.first, pdf->x.second);
            generatorInfo.SetPdfIds(pdf->id.first, pdf->id.second);
            generatorInfo.SetPdfQScale(pdf->scalePDF);
        }
    }
    
    
    // Save the pile-up information
    puInfo.SetNumPV(vertices->size());

    Handle<double> rho;
    event.getByLabel(rhoTag, rho);
    
    puInfo.SetRho(*rho);
    
    
    if (!runOnData)
    {
        Handle<View<PileupSummaryInfo>> puSummary;
        event.getByLabel(puSummaryTag, puSummary);
        
        puInfo.SetTrueNumPU(puSummary->front().getTrueNumInteractions());
        //^ The true number of interactions is same for all bunch crossings
        
        for (unsigned i = 0; i < puSummary->size(); ++i)
            if (puSummary->at(i).getBunchCrossing() == 0)
            {
                puInfo.SetInTimePU(puSummary->at(i).getPU_NumInteractions());
                break;
            }
    }
    
    
    // Fill the output tree
    outTree->Fill();
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
