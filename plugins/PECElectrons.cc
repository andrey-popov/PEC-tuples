#include "PECElectrons.h"

#include <FWCore/Utilities/interface/InputTag.h>
#include <FWCore/Framework/interface/MakerMacros.h>


using namespace edm;
using namespace std;


PECElectrons::PECElectrons(ParameterSet const &cfg)
{
    // Register required input data
    electronToken = consumes<View<pat::Electron>>(cfg.getParameter<InputTag>("src"));
    
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
    desc.add<vector<InputTag>>("boolIDMaps", vector<InputTag>(0))->
     setComment("Maps with boolean electron ID decisions.");
    desc.add<vector<InputTag>>("contIDMaps", vector<InputTag>(0))->
     setComment("Maps with real-valued electron ID decisions.");
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
    // Read the electron collection and ID maps
    Handle<View<pat::Electron>> srcElectrons;
    vector<Handle<ValueMap<bool>>> boolIDMaps(boolIDMapTokens.size());
    vector<Handle<ValueMap<float>>> contIDMaps(contIDMapTokens.size());
    
    event.getByToken(electronToken, srcElectrons);
    
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
        
        
        // Isolation with delta-beta correction. The delta-beta factor of 0.5 is taken from
        //configuration of electron ID modules, which also apply a cut on the isolation
        storeElectron.SetRelIso((el.chargedHadronIso() + max(el.neutralHadronIso() +
         el.photonIso() - 0.5 * el.puChargedHadronIso(), 0.)) / el.pt());
        
        
        // Copy electron IDs from the maps
        Ptr<pat::Electron> const elPtr(srcElectrons, i);
        
        for (unsigned i = 0; i < boolIDMaps.size(); ++i)
            storeElectron.SetBooleanID(i, (*boolIDMaps.at(i))[elPtr]);
        
        for (unsigned i = 0; i < contIDMaps.size(); ++i)
            storeElectron.SetContinuousID(i, (*contIDMaps.at(i))[elPtr]);
        
        
        // Conversion rejection [1]. True for a "good" electron
        //[1] https://twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
        storeElectron.SetBit(0, el.passConversionVeto());
        
        
        // Evaluate user-defined selectors if any
        for (unsigned i = 0; i < eleSelectors.size(); ++i)
            storeElectron.SetBit(1 + i, eleSelectors[i](el));
        
        
        // The electron is set up. Add it to the vector
        storeElectrons.emplace_back(storeElectron);
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECElectrons);
