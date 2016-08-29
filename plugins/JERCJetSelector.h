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
 * \class JERCJetSelector
 * \brief A plugin to evaluate JEC uncertainty and JER factors and select jets taking into account
 * JEC and JER variations
 * 
 * This plugin selects PAT jets that could possibly pass the given pt threshold thanks to
 * variations in JEC and JER smearing. JEC are varied within their uncertainties. JER smearing is
 * applied only to simulation, following the recommendations in [1]. When there is no matching
 * generator-level jet, the reconstructed jet is accepted if it could pass the pt threshold using
 * a JER variation some factor larger than the pt resolution in simulation (as given by parameter
 * "nSigmaJERUnmatched"). Selected jets are written in the event without reordering.
 * 
 * Jet variations can be switched off by setting flag "includeJERCVariations" to false. In this
 * case the plugin simply selects jets with pt larger than then given threshold. A string-based
 * preselection can be applied to jets (parameter "preselection"). Jets failing it are rejected
 * unconditionally. If the number of selected jets is less than the given value (parameter
 * "minNum", set to zero by default), the event is rejected.
 * 
 * Additional information is added to the produced collection of jets. JEC uncertainty and JER
 * factors are written as userFloats "jecUncertainty", "jerFactor[Nominal|Up|Down]", and a flag
 * indicating the presence of a matching generator-level jet is written as userInt "hasGenMatch".
 * The matching is performed as recommended in [1].
 * 
 * [1] https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution?rev=54#Smearing_procedures
 */
class JERCJetSelector: public edm::EDFilter
{
public:
    /// Constructor
    JERCJetSelector(edm::ParameterSet const &cfg);
    
public:
    /// Creates objects that provide JEC uncertainty and JER resolution and scale factors
    virtual void beginRun(edm::Run const &, edm::EventSetup const &setup) override;
    
    /// Verifies plugin configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Produces collection of selected jets and performs event filtering
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
    
    /**
     * \brief Selection on raw pt
     * 
     * Added in disjunction with the selection on the corrected pt.
     */
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
