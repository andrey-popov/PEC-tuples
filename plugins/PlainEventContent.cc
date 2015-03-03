#include "PlainEventContent.h"

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <TLorentzVector.h>
#include <Math/GenVector/VectorUtil.h>

#include <memory>
#include <cmath>
#include <sstream>


using namespace edm;
using namespace std;


PlainEventContent::PlainEventContent(edm::ParameterSet const &cfg):
    jetMinPt(cfg.getParameter<double>("jetMinPt")),
    jetMinRawPt(cfg.getParameter<double>("jetMinRawPt")),    
    runOnData(cfg.getParameter<bool>("runOnData"))
{
    // Register required input data
    electronToken = consumes<edm::View<pat::Electron>>(cfg.getParameter<InputTag>("electrons"));
    muonToken = consumes<edm::View<pat::Muon>>(cfg.getParameter<InputTag>("muons"));
    jetToken = consumes<edm::View<pat::Jet>>(cfg.getParameter<InputTag>("jets"));
    
    #if 0
    for (edm::InputTag const &tag: cfg.getParameter<vector<InputTag>>("METs"))
        metTokens.emplace_back(consumes<edm::View<pat::MET>>(tag));
    #endif
    
    for (edm::InputTag const &tag: cfg.getParameter<vector<InputTag>>("eleIDMaps"))
        eleIDMapTokens.emplace_back(consumes<edm::ValueMap<bool>>(tag));
    
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<InputTag>("generator"));
    primaryVerticesToken =
     consumes<reco::VertexCollection>(cfg.getParameter<InputTag>("primaryVertices"));
    puSummaryToken = consumes<edm::View<PileupSummaryInfo>>(cfg.getParameter<InputTag>("puInfo"));
    rhoToken = consumes<double>(cfg.getParameter<InputTag>("rho"));
    
    
    // Construct string-based selectors for all objects
    for (string const &selection: cfg.getParameter<vector<string>>("eleSelection"))
        eleSelectors.emplace_back(selection);
    
    for (string const &selection: cfg.getParameter<vector<string>>("muSelection"))
        muSelectors.emplace_back(selection);
    
    for (string const &selection: cfg.getParameter<vector<string>>("jetSelection"))
        jetSelectors.emplace_back(selection);
}


void PlainEventContent::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    // Documentation for descriptions of the configuration is available in [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideConfigurationValidationAndHelp
    
    edm::ParameterSetDescription desc;
    desc.add<bool>("runOnData")->
     setComment("Indicates whether data or simulation is being processed.");
    desc.add<InputTag>("primaryVertices")->
     setComment("Collection of reconstructed primary vertices.");
    desc.add<InputTag>("electrons")->setComment("Collection of electrons.");
    desc.add<vector<InputTag>>("eleIDMaps", vector<InputTag>(0))->
     setComment("Maps with electron ID decisions.");
    desc.add<vector<string>>("eleSelection", vector<string>(0))->
     setComment("User-defined selections for electrons whose results will be stored in the output "
     "tree.");
    desc.add<InputTag>("muons")->setComment("Collection of muons.");
    desc.add<vector<string>>("muSelection", vector<string>(0))->
     setComment("User-defined selections for muons whose results will be stored in the ouput "
     "tree.");
    desc.add<InputTag>("jets")->setComment("Collection of jets.");
    desc.add<vector<string>>("jetSelection", vector<string>(0))->
     setComment("User-defined selections for jets whose results will be stored in the output "
     "tree.");
    desc.add<double>("jetMinPt", 20.)->
     setComment("Jets with corrected pt above this threshold will be stored in the output tree.");
    desc.add<double>("jetMinRawPt", 10.)->
     setComment("Jets with raw pt above this threshold will be stored in the output tree.");
    desc.add<vector<InputTag>>("METs")->setComment("MET. Several versions of it can be stored.");
    desc.add<InputTag>("generator", InputTag("generator"))->
     setComment("Tag to access information about generator. If runOnData is true, this parameter "
     "is ignored.");
    desc.add<InputTag>("rho", InputTag("kt6PFJets", "rho"))->
     setComment("Rho (mean angular pt density).");
    desc.add<InputTag>("puInfo", InputTag("addPileupInfo"))->
     setComment("True pile-up information. If runOnData is true, this parameter is ignored.");
    
    descriptions.add("eventContent", desc);
}


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


void PlainEventContent::analyze(edm::Event const &event, edm::EventSetup const &setup)
{
    // Fill the event ID tree
    eventId.Reset();
    
    eventId.SetRunNumber(event.id().run());
    eventId.SetEventNumber(event.id().event());
    eventId.SetLumiSectionNumber(event.luminosityBlock());
    
    
    // Read the primary vertices
    Handle<reco::VertexCollection> vertices;
    event.getByToken(primaryVerticesToken, vertices);
    
    if (vertices->size() == 0)
    {
        edm::Exception excp(edm::errors::LogicError);
        excp << "Event contains zero good primary vertices.\n";
        excp.raise();
    }
    
    
    
    // Fill the tree with basic information
    // Read the electron collection
    Handle<View<pat::Electron>> srcElectrons;
    event.getByToken(electronToken, srcElectrons);
    
    
    // Read electron ID maps
    vector<Handle<ValueMap<bool>>> eleIDMaps(eleIDMapTokens.size());
    
    for (unsigned i = 0; i < eleIDMapTokens.size(); ++i)
        event.getByToken(eleIDMapTokens.at(i), eleIDMaps.at(i));
    
    
    // Loop through the electron collection and fill the relevant variables
    storeElectrons.clear();
    pec::Electron storeElectron;  // will reuse this object to fill the vector
    
    for (unsigned i = 0; i < srcElectrons->size(); ++i)
    {
        pat::Electron const &el = srcElectrons->at(i);
        storeElectron.Reset();
        
        
        // Set four-momentum. Mass is ignored
        storeElectron.SetPt(el.pt());
        storeElectron.SetEta(el.eta());
        storeElectron.SetPhi(el.phi());
        
        storeElectron.SetCharge(el.charge());
        storeElectron.SetDB(el.dB());
        
        
        // Isolation with delta-beta correction. The delta-beta factor of 0.5 is taken from
        //configuration of electron ID modules, which also apply a cut on the isolation
        storeElectron.SetRelIso((el.chargedHadronIso() + max(el.neutralHadronIso() +
         el.photonIso() - 0.5 * el.puChargedHadronIso(), 0.)) / el.pt());
        
        
        // Copy electron IDs from the maps
        Ptr<pat::Electron> const elPtr(srcElectrons, i);
        
        for (unsigned i = 0; i < eleIDMaps.size(); ++i)
            storeElectron.SetBit(i, (*eleIDMaps.at(i))[elPtr]);
        
        
        // Conversion rejection. True for a "good" electron
        storeElectron.SetBit(eleIDMaps.size(), el.passConversionVeto());
        //^ See [1]. The decision is stored by PATElectronProducer based on the collection
        //"allConversions" (the name is hard-coded). In the past, there used to be an additional
        //requirement to reject electrons from the photon conversion is set according to [2]; but
        //it caused a compile error in 72X and has been dropped
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
        //[2] https://twiki.cern.ch/twiki/bin/view/CMS/TWikiTopRefEventSel#Electrons
        
        
        // Evaluate user-defined selectors if any
        for (unsigned i = 0; i < eleSelectors.size(); ++i)
            storeElectron.SetBit(eleIDMaps.size() + 1 + i, eleSelectors[i](el));
        
        
        // The electron is set up. Add it to the vector
        storeElectrons.push_back(storeElectron);
    }
    
    
    // Read the muon collection
    Handle<View<pat::Muon>> srcMuons;
    event.getByToken(muonToken, srcMuons);
    
    
    // Loop through the muon collection and fill the relevant variables
    storeMuons.clear();
    pec::Muon storeMuon;  // will reuse this object to fill the vector
    
    for (unsigned i = 0; i < srcMuons->size(); ++i)
    {
        pat::Muon const &mu = srcMuons->at(i);
        storeMuon.Reset();
        
        
        // Set four-momentum. Mass is ignored
        storeMuon.SetPt(mu.pt());
        storeMuon.SetEta(mu.eta());
        storeMuon.SetPhi(mu.phi());
        
        storeMuon.SetCharge(mu.charge());
        storeMuon.SetDB(mu.dB());
        
        // Relative isolation with delta-beta correction. Definition from 2012 is used, and it is
        //likely to change for 2015
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
    event.getByToken(jetToken, srcJets);
    
    
    // Loop through the jet collection and fill the relevant variables
    storeJets.clear();
    pec::Jet storeJet;  // will reuse this object to fill the vector
    
    for (unsigned int i = 0; i < srcJets->size(); ++i)
    {
        pat::Jet const &j = srcJets->at(i);
        reco::Candidate::LorentzVector const &rawP4 = j.correctedP4("Uncorrected");
        storeJet.Reset();
        
        if (j.pt() > jetMinPt or rawP4.pt() > jetMinRawPt)
        {
            // Set four-momentum
            storeJet.SetPt(rawP4.pt());
            storeJet.SetEta(rawP4.eta());
            storeJet.SetPhi(rawP4.phi());
            storeJet.SetM(rawP4.mass());
            
            
            storeJet.SetArea(j.jetArea());
            storeJet.SetCharge(j.jetCharge());
            storeJet.SetBTagCSV(j.bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"));
            
            // Mass of the secondary vertex is available as userFloat [1]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD?rev=32#Jets
            storeJet.SetSecVertexMass(j.userFloat("vtxMass"));
            
            
            // Calculate the jet pull angle
            double const y = rawP4.Rapidity();
            double const phi = rawP4.phi();
            double pullY = 0., pullPhi = 0.;  // projections of the pull vector (unnormalised)
            
            // Loop over constituents of the jet
            for (unsigned iDaughter = 0; iDaughter < j.numberOfDaughters(); ++iDaughter)
            {
                reco::Candidate const *p = j.daughter(iDaughter);
                //^ Actually jet constituents are of type pat::PackedCandidate, but here only their
                //four-momenta are needed, so I do not upcast them
                
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
    
    
    #if 0
    // Read METs
    storeMETs.clear();
    pec::Candidate storeMET;  // will reuse this object to fill the vector
    
    for (auto const &metToken: metTokens)
    {
        Handle<View<pat::MET>> met;
        event.getByToken(metToken, met);
        
        storeMET.Reset();
        
        storeMET.SetPt(met->front().pt());
        storeMET.SetPhi(met->front().phi());
        
        storeMETs.push_back(storeMET);
    }
    #endif
    
    
    // Save the generator information (however the jet generator info is already saved)
    // Save the PDF and other generator information
    if (!runOnData)
    {
        Handle<GenEventInfoProduct> generator;
        event.getByToken(generatorToken, generator);
        
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
    puInfo.Reset();
    //^ Same object is used for all events, hence need to reset it
    
    puInfo.SetNumPV(vertices->size());

    Handle<double> rho;
    event.getByToken(rhoToken, rho);
    
    puInfo.SetRho(*rho);
    
    
    if (!runOnData)
    {
        Handle<View<PileupSummaryInfo>> puSummary;
        event.getByToken(puSummaryToken, puSummary);
        
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


DEFINE_FWK_MODULE(PlainEventContent);
