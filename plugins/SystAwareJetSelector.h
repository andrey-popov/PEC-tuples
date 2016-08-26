#pragma once

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include <CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <DataFormats/PatCandidates/interface/Jet.h>

#include <memory>


/**
 * \class SystAwareJetSelector
 * \brief A plugin to select jets taking into account JEC and JER variations
 */
class SystAwareJetSelector: public edm::EDFilter
{
public:
    SystAwareJetSelector(edm::ParameterSet const &cfg);
    
public:
    virtual void beginRun(edm::Run const &, edm::EventSetup const &setup) override;
    
    /// Verifies plugin configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    virtual bool filter(edm::Event &event, edm::EventSetup const &) override;
    
private:
    /// Source collection of jets
    edm::EDGetTokenT<edm::View<pat::Jet>> jetToken;
    
    /// Preselection for jets
    StringCutObjectSelector<pat::Jet> const preselector;
    
    /// Selection on pt
    double minPt;
    
    /// Threshold on the number of selected jets
    unsigned minNumJets;
    
    /**
     * \brief Label identifying jet type for JES and JER corrections
     * 
     * Needed to access JEC uncertainties and jet energy resolutions and their scale factors.
     */
    std::string const jetTypeLabel;
    
    /// Flag showing whether JERC variations should be considered
    bool includeJERCVariations;
    
    /// An object to access JEC uncertainty
    std::unique_ptr<JetCorrectionUncertainty> jecUncProvider;
};
