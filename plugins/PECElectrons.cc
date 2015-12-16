#include "PECElectrons.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/ParameterSet/interface/FileInPath.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <algorithm>


using namespace edm;
using namespace std;


PECElectrons::PECElectrons(ParameterSet const &cfg):
    embeddedBoolIDLabels(cfg.getParameter<vector<string>>("embeddedBoolIDs")),
    embeddedContIDLabels(cfg.getParameter<vector<string>>("embeddedContIDs")),
    eaReader((cfg.getParameter<FileInPath>("effAreas")).fullPath())
{
    // Register required input data
    electronToken = consumes<View<pat::Electron>>(cfg.getParameter<InputTag>("src"));
    rhoToken = consumes<double>(cfg.getParameter<InputTag>("rho"));
    
    for (InputTag const &tag: cfg.getParameter<vector<InputTag>>("boolIDMaps"))
        boolIDMapTokens.emplace_back(consumes<ValueMap<bool>>(tag));
    
    for (InputTag const &tag: cfg.getParameter<vector<InputTag>>("contIDMaps"))
        contIDMapTokens.emplace_back(consumes<ValueMap<float>>(tag));
    
    
    // Construct string-based selectors
    for (string const &selection: cfg.getParameter<vector<string>>("selection"))
        eleSelectors.emplace_back(selection);
}


void PECElectrons::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("src")->setComment("Source collection of electrons.");
    desc.add<InputTag>("rho", InputTag("fixedGridRhoFastjetAll"))->
     setComment("Rho (mean angular pt density).");
    desc.add<FileInPath>("effAreas")->
     setComment("Data file with effective areas for electron isolation.");
    desc.add<vector<string>>("embeddedBoolIDs", vector<string>(0))->
     setComment("Labels of embedded boolean electron ID decisions to be stored.");
    desc.add<vector<InputTag>>("boolIDMaps", vector<InputTag>(0))->
     setComment("Maps with additional boolean electron ID decisions to be stored.");
    desc.add<vector<string>>("embeddedContIDs", vector<string>(0))->
     setComment("Labels of embedded real-valued electron ID decisions to be stored.");
    desc.add<vector<InputTag>>("contIDMaps", vector<InputTag>(0))->
     setComment("Maps with additional real-valued electron ID decisions to be stored.");
    desc.add<vector<string>>("selection", vector<string>(0))->
     setComment("User-defined selections for electrons whose results will be stored in the output "
     "tree.");
    
    descriptions.add("electrons", desc);
}


void PECElectrons::beginJob()
{
    outTree = fileService->make<TTree>("Electrons", "Properties of selected electrons");
    
    storeElectronsPointer = &storeElectrons;
    outTree->Branch("electrons", &storeElectronsPointer);
}


void PECElectrons::analyze(Event const &event, EventSetup const &)
{
    // Read the electron collection and rho
    Handle<View<pat::Electron>> srcElectrons;
    Handle<double> rho;
    
    event.getByToken(electronToken, srcElectrons);
    event.getByToken(rhoToken, rho);
    
    
    // Read ID maps
    vector<Handle<ValueMap<bool>>> boolIDMaps(boolIDMapTokens.size());
    vector<Handle<ValueMap<float>>> contIDMaps(contIDMapTokens.size());
    
    for (unsigned i = 0; i < boolIDMapTokens.size(); ++i)
        event.getByToken(boolIDMapTokens.at(i), boolIDMaps.at(i));
    
    for (unsigned i = 0; i < contIDMapTokens.size(); ++i)
        event.getByToken(contIDMapTokens.at(i), contIDMaps.at(i));
    
    
    // Loop through the collection and store relevant properties of electrons
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
        
        
        // Isolation is calculated by a dedicated method
        storeElectron.SetRelIso(CalculateRhoIsolation(el, *rho));
        
        
        // Copy embedded ID decisions
        unsigned const nEmbeddedBoolIDs = embeddedBoolIDLabels.size();
        unsigned const nEmbeddedContIDs = embeddedContIDLabels.size();
        
        for (unsigned i = 0; i < nEmbeddedBoolIDs; ++i)
            storeElectron.SetBooleanID(i, (el.electronID(embeddedBoolIDLabels.at(i)) > 0.5f));
            //^ Since pat::Electron::electronID returns a float, need to be accurate with the
            //conversion to a boolean value
        
        for (unsigned i = 0; i < nEmbeddedContIDs; ++i)
            storeElectron.SetContinuousID(i, el.electronID(embeddedContIDLabels.at(i)));
        
        
        // Copy additional ID decisions from the maps
        Ptr<pat::Electron> const elPtr(srcElectrons, i);
        
        for (unsigned i = 0; i < boolIDMaps.size(); ++i)
            storeElectron.SetBooleanID(nEmbeddedBoolIDs + i, (*boolIDMaps.at(i))[elPtr]);
        
        for (unsigned i = 0; i < contIDMaps.size(); ++i)
            storeElectron.SetContinuousID(nEmbeddedContIDs + i, (*contIDMaps.at(i))[elPtr]);
        
        
        // Conversion rejection [1]. True for a "good" electron
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
        storeElectron.SetBit(0, el.passConversionVeto());
        
        
        // Evaluate user-defined selectors if any
        unsigned const nUsedBits = 1;
        
        for (unsigned i = 0; i < eleSelectors.size(); ++i)
            storeElectron.SetBit(nUsedBits + i, eleSelectors[i](el));
        
        
        // The electron is set up. Add it to the vector
        storeElectrons.emplace_back(storeElectron);
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


double PECElectrons::CalculateRhoIsolation(reco::GsfElectron const &el, double const rho) const
{
    // Isolation is calculated following an example in [1], as recommended in [2]
    //[1] https://github.com/cms-sw/cmssw/blob/CMSSW_7_6_2/RecoEgamma/ElectronIdentification/plugins/cuts/GsfEleEffAreaPFIsoCut.cc#L75-L90
    //[2] https://hypernews.cern.ch/HyperNews/CMS/get/egamma/1664/1.html
    reco::GsfElectron::PflowIsolationVariables const &pfIso = el.pfIsolationVariables();
    double const ea = eaReader.getEffectiveArea(fabs(el.superCluster()->eta()));
    double const iso = pfIso.sumChargedHadronPt +
     max(pfIso.sumNeutralHadronEt + pfIso.sumPhotonEt - rho * ea, 0.);
    
    return iso / el.pt();
}


DEFINE_FWK_MODULE(PECElectrons);
