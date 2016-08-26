#pragma once

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include <CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h>
#include <CommonTools/Utils/interface/StringCutObjectSelector.h>
#include <DataFormats/JetReco/interface/GenJet.h>
#include <DataFormats/PatCandidates/interface/Jet.h>
#include <JetMETCorrections/Modules/interface/JetResolution.h>

#include <TRandom3.h>

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
    /**
     * \brief Performs matching between reconstructed and generator-level jets
     * 
     * Considers only GEN-level jets with dR less than half of the jet cone size and with the
     * absolute pt difference less than the given value. Among them, returns the jet closest in dR.
     * If no match is found, a nullptr is returned.
     */
    reco::GenJet const *MatchGenJet(reco::Jet const &jet, edm::View<reco::GenJet> const &genJets,
      double maxDPt) const;
    
private:
    /// Source collection of jets
    edm::EDGetTokenT<edm::View<pat::Jet>> jetToken;
    
    /// Preselection for jets
    StringCutObjectSelector<pat::Jet> const preselector;
    
    /// Selection on corrected pt
    double minPt;
    
    /// Selection on raw pt
    double minRawPt;
    
    /// Threshold on the number of selected jets
    unsigned minNumJets;
    
    /// Flag showing whether JERC variations should be considered
    bool includeJERCVariations;
    
    /**
     * \brief Label identifying jet type for JES and JER corrections
     * 
     * Needed to access JEC uncertainties and jet energy resolutions and their scale factors.
     */
    std::string const jetTypeLabel;
    
    /// An object to access JEC uncertainty
    std::unique_ptr<JetCorrectionUncertainty> jecUncProvider;
    
    /**
     * \brief Collection of GEN-level jets
     * 
     * Used in JER smearing.
     */
    edm::EDGetTokenT<edm::View<reco::GenJet>> genJetToken;
    
    /**
     * \brief Jet cone size
     * 
     * Used in JER smearing to perform matching to GEN-level jets.
     */
    double jetConeSize;
    
    /**
     * \brief Rho (mean angular pt density)
     * 
     * Used in JER smearing.
     */
    edm::EDGetTokenT<double> rhoToken;
    
    /// An object that provides jet energy resolution in simulation
    std::unique_ptr<JME::JetResolution> jerProvider;
    
    /// An object that provides JER scale factors
    std::unique_ptr<JME::JetResolutionScaleFactor> jerSFProvider;
    
    /**
     * \brief Random-number generator
     * 
     * Used to apply JER smearing for jets that do not have a generator-level match.
     */
    TRandom3 rGen;
    
    /// Variation of this size is used to determine if a jet w/o GEN-level match to be saved
    double nSigmaJERUnmatched;
};
