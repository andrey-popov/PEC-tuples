#include "SystAwareJetSelector.h"

#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <CondFormats/JetMETObjects/interface/JetCorrectorParameters.h>
#include <JetMETCorrections/Modules/interface/JetResolution.h>
#include <JetMETCorrections/Objects/interface/JetCorrectionsRecord.h>

#include <cmath>


SystAwareJetSelector::SystAwareJetSelector(edm::ParameterSet const &cfg):
    edm::EDFilter(),
    preselector(cfg.getParameter<std::string>("preselection")),
    minPt(cfg.getParameter<double>("minPt")),
    minNumJets(cfg.getParameter<unsigned>("minNum")),
    jetTypeLabel(cfg.getParameter<std::string>("jetTypeLabel")),
    includeJERCVariations(cfg.getParameter<bool>("includeJERCVariations"))
{
    jetToken = consumes<edm::View<pat::Jet>>(cfg.getParameter<edm::InputTag>("src"));
    
    produces<std::vector<pat::Jet>>();
}


void SystAwareJetSelector::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src")->setComment("Source collection of jets.");
    desc.add<std::string>("jetTypeLabel")->
      setComment("Jet type label for JES and JER corrections.");
    desc.add<std::string>("preselection", "")->setComment("Preselection for jets.");
    desc.add<double>("minPt")->setComment("Cut on jet pt.");
    desc.add<bool>("includeJERCVariations", true)->
      setComment("Indicates whether variations in JEC and JER should be considered.");
    desc.add<unsigned>("minNum", 0)->
      setComment("Minimal number of selected jets to accept an event.");
    
    descriptions.add("jetSelector", desc);
}


void SystAwareJetSelector::beginRun(edm::Run const &, edm::EventSetup const &setup)
{
    // Construct an object to obtain JEC uncertainty [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=137#JetCorUncertainties
    edm::ESHandle<JetCorrectorParametersCollection> jecParametersCollection;
    setup.get<JetCorrectionsRecord>().get(jetTypeLabel, jecParametersCollection); 
    
    JetCorrectorParameters const &jecParameters = (*jecParametersCollection)["Uncertainty"];
    jecUncProvider.reset(new JetCorrectionUncertainty(jecParameters));
}


bool SystAwareJetSelector::filter(edm::Event &event, edm::EventSetup const &)
{
    edm::Handle<edm::View<pat::Jet>> srcJets;
    event.getByToken(jetToken, srcJets);
    
    
    std::unique_ptr<std::vector<pat::Jet>> selectedJets(new std::vector<pat::Jet>);
    
    for (auto const &j: *srcJets)
    {
        if (not preselector(j))
            continue;
        
        
        #ifdef DEBUG
        std::cout << "Consider jet with pt = " << j.pt() << '\n';
        #endif
        
        
        double jetPtUpVarFactor = 1.;
        
        if (includeJERCVariations)
        {
            // Find JEC uncertainty for the current jet [1]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=137#JetCorUncertainties
            jecUncProvider->setJetEta(j.eta());
            jecUncProvider->setJetPt(j.pt());
            double const jecUncertainty = std::abs(jecUncProvider->getUncertainty(true));
            
            
            #ifdef DEBUG
            std::cout << " JEC uncertainty: " << jecUncertainty << '\n';
            #endif
            
            
            jetPtUpVarFactor *= (1. + jecUncertainty);
        }
        
        
        if (j.pt() * jetPtUpVarFactor > minPt)
            selectedJets->emplace_back(j);
        
        
        #ifdef DEBUG
        if (j.pt() * jetPtUpVarFactor > minPt)
            std::cout << " Accepted\n";
        else
            std::cout << " Skipped\n";
        #endif
    }
    
    
    return (selectedJets->size() >= minNumJets);
}


DEFINE_FWK_MODULE(SystAwareJetSelector);
