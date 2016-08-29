#include "JERCJetSelector.h"

#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <CondFormats/JetMETObjects/interface/JetCorrectorParameters.h>
#include <JetMETCorrections/Objects/interface/JetCorrectionsRecord.h>

#include <Math/GenVector/VectorUtil.h>

#include <algorithm>
#include <cmath>
#include <limits>


JERCJetSelector::JERCJetSelector(edm::ParameterSet const &cfg):
    edm::EDFilter(),
    preselector(cfg.getParameter<std::string>("preselection")),
    minPt(cfg.getParameter<double>("minPt")),
    minRawPt(cfg.getParameter<double>("minRawPt")),
    minNumJets(cfg.getParameter<unsigned>("minNum")),
    includeJERCVariations(cfg.getParameter<bool>("includeJERCVariations")),
    jetTypeLabel(cfg.getParameter<std::string>("jetTypeLabel")),
    jetConeSize(cfg.getParameter<double>("jetConeSize")),
    rGen(cfg.getParameter<unsigned>("seed")),
    nSigmaJERUnmatched(std::abs(cfg.getParameter<double>("nSigmaJERUnmatched")))
{
    jetToken = consumes<edm::View<pat::Jet>>(cfg.getParameter<edm::InputTag>("src"));
    genJetToken = consumes<edm::View<reco::GenJet>>(cfg.getParameter<edm::InputTag>("genJets"));
    rhoToken = consumes<double>(cfg.getParameter<edm::InputTag>("rho"));
    
    produces<std::vector<pat::Jet>>();
}


void JERCJetSelector::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("src")->setComment("Source collection of jets.");
    desc.add<std::string>("jetTypeLabel")->
      setComment("Jet type label for JES and JER corrections.");
    desc.add<double>("jetConeSize", 0.4)->setComment("Jet cone size.");
    desc.add<std::string>("preselection", "")->setComment("Preselection for jets.");
    desc.add<double>("minPt")->setComment("Cut on jet pt.");
    desc.add<double>("minRawPt", 9999.)->setComment("Cut on jet raw pt.");
    desc.add<bool>("includeJERCVariations", true)->
      setComment("Indicates whether variations in JEC and JER should be considered.");
    desc.add<edm::InputTag>("genJets")->setComment("GEN-level jets.");
    desc.add<edm::InputTag>("rho")->setComment("Rho (mean angular pt density).");
    desc.add<double>("nSigmaJERUnmatched", 3.)->
      setComment("JER variation to be used to choose jets without GEN-level matches.");
    desc.add<unsigned>("minNum", 0)->
      setComment("Minimal number of selected jets to accept an event.");
    desc.add<unsigned>("seed", 0)->setComment("Seed for random number generator.");
    
    descriptions.add("jetSelector", desc);
}


void JERCJetSelector::beginRun(edm::Run const &, edm::EventSetup const &setup)
{
    // Construct an object to obtain JEC uncertainty [1]
    //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=137#JetCorUncertainties
    edm::ESHandle<JetCorrectorParametersCollection> jecParametersCollection;
    setup.get<JetCorrectionsRecord>().get(jetTypeLabel, jecParametersCollection); 
    
    JetCorrectorParameters const &jecParameters = (*jecParametersCollection)["Uncertainty"];
    jecUncProvider.reset(new JetCorrectionUncertainty(jecParameters));
    
    
    // Objects that provide jet energy resolution and its scale factors
    jerProvider.reset(
      new JME::JetResolution(std::move(JME::JetResolution::get(setup, jetTypeLabel + "_pt"))));
    jerSFProvider.reset(new JME::JetResolutionScaleFactor(
      std::move(JME::JetResolutionScaleFactor::get(setup, jetTypeLabel))));
}


bool JERCJetSelector::filter(edm::Event &event, edm::EventSetup const &)
{
    // Read the source collection of jets and rho. The latter is only used in JER smearing and thus
    //is only read when the corresponding flag is set.
    edm::Handle<edm::View<pat::Jet>> srcJets;
    event.getByToken(jetToken, srcJets);
    
    edm::Handle<double> rho;
    if (includeJERCVariations)
        event.getByToken(rhoToken, rho);
    
    edm::Handle<edm::View<reco::GenJet>> genJets;
    if (includeJERCVariations and not event.isRealData())
        event.getByToken(genJetToken, genJets);
    
    
    // Build a collection of jets passing the selection
    std::unique_ptr<std::vector<pat::Jet>> selectedJets(new std::vector<pat::Jet>);
    
    for (auto const &j: *srcJets)
    {
        if (not preselector(j))
            continue;
        
        
        #ifdef DEBUG
        std::cout << "Consider jet with pt = " << j.pt() << '\n';
        #endif
        
        
        double jecUncertainty = 0.;
        double jerFactorNominal = 1., jerFactorUp = 1., jerFactorDown = 1.;
        double jerSafetyFactor = 1.;  // Used to choose if the jet is to be selected
        bool hasGenMatch = false;
        
        if (includeJERCVariations)
        {
            // Find JEC uncertainty for the current jet [1]
            //[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=137#JetCorUncertainties
            jecUncProvider->setJetEta(j.eta());
            jecUncProvider->setJetPt(j.pt());
            jecUncertainty = std::abs(jecUncProvider->getUncertainty(true));
            
            
            // Evaluate JER smearing factors. This is only done for simulation.
            if (not event.isRealData())
            {
                // JER pt resolution (relative) and scale factors
                double const ptResolution =
                  jerProvider->getResolution({{JME::Binning::JetPt, j.pt()},
                  {JME::Binning::JetEta, j.eta()}, {JME::Binning::Rho, *rho}});
                
                double const jerSFNominal = jerSFProvider->getScaleFactor(
                  {{JME::Binning::JetEta, j.eta()}}, Variation::NOMINAL);
                double const jerSFUp = jerSFProvider->getScaleFactor(
                  {{JME::Binning::JetEta, j.eta()}}, Variation::UP);
                double const jerSFDown = jerSFProvider->getScaleFactor(
                  {{JME::Binning::JetEta, j.eta()}}, Variation::DOWN);
                
                
                // Try to match the current jet to a generator-level one.  The maximal pt
                //difference is chosen as recommended in [1].
                //[1] https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution?rev=54#Smearing_procedures
                reco::GenJet const *genJet = MatchGenJet(j, *genJets, 3 * ptResolution * j.pt());
                
                if (genJet)
                {
                    // Apply scaling as in [1].
                    //[1] https://github.com/cms-sw/cmssw/blob/CMSSW_8_0_18/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L236-L237
                    hasGenMatch = true;
                    double const energyFactor = (j.pt() - genJet->pt()) / j.pt();
                    
                    jerFactorNominal = 1. + (jerSFNominal - 1.) * energyFactor;
                    jerFactorUp = 1. + (jerSFUp - 1.) * energyFactor;
                    jerFactorDown = 1. + (jerSFDown - 1.) * energyFactor;
                    
                    jerSafetyFactor = std::max({jerFactorNominal, jerFactorUp, jerFactorDown});
                }
                else
                {
                    // A shift in jet pt is randomly sampled according to the resolution in
                    //simulation and then scaled based on the data-to-simulation scale factors, as
                    //in [1]. It is important that the sampling is only done once and then reused
                    //for the systematical variations. Otherwise the variations would also include
                    //the effect of resampling and not just the shift in the scale factor.
                    //[1] https://github.com/cms-sw/cmssw/blob/CMSSW_8_0_18/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L244-L250
                    double const mcShift = rGen.Gaus(0., ptResolution);
                    
                    jerFactorNominal = 1. + mcShift *
                      std::sqrt(std::max(std::pow(jerSFNominal, 2) - 1., 0.));
                    jerFactorUp = 1. + mcShift *
                      std::sqrt(std::max(std::pow(jerSFUp, 2) - 1., 0.));
                    jerFactorDown = 1. + mcShift *
                      std::sqrt(std::max(std::pow(jerSFDown, 2) - 1., 0.));
                    
                    
                    // Decide whether the jet should be selected based on the requested n-sigma
                    //variation instead of the sampled factors. This allows to reapply stochastic
                    //JER smearing on the selected jets.
                    double const maxJERSF = std::max({jerSFNominal, jerSFUp, jerSFDown});
                    jerSafetyFactor = 1. + nSigmaJERUnmatched *
                      std::sqrt(std::max(std::pow(maxJERSF, 2) - 1., 0.));
                }
            }
        }
        
        
        // Copy the jet if it has a chance to pass one of the pt thresholds
        double const jetPtUpVarFactor = std::max({1. + jecUncertainty, jerSafetyFactor});
        
        if (j.pt() * jetPtUpVarFactor > minPt or j.correctedP4("Uncorrected").pt() > minRawPt)
        {
            pat::Jet copyJet(j);
            
            copyJet.addUserFloat("jecUncertainty", jecUncertainty);
            copyJet.addUserFloat("jerFactorNominal", jerFactorNominal);
            copyJet.addUserFloat("jerFactorUp", jerFactorUp);
            copyJet.addUserFloat("jerFactorDown", jerFactorDown);
            copyJet.addUserInt("hasGenMatch", int(hasGenMatch));
            
            selectedJets->emplace_back(std::move(copyJet));
        }
        
        
        #ifdef DEBUG
        std::cout << " JEC uncertainty: " << jecUncertainty << '\n';

        if (hasGenMatch)
            std::cout << " GEN-level match found\n";
        else
            std::cout << " No GEN-level match found\n";

        std::cout << " JER factors: " << jerFactorNominal << ", " << jerFactorUp << ", " <<
          jerFactorDown << '\n';
        
        if (j.pt() * jetPtUpVarFactor > minPt)
            std::cout << " Accepted\n";
        else
            std::cout << " Skipped\n";
        #endif
    }
    
    #ifdef DEBUG
    std::cout << std::endl;
    #endif
    
    
    // Evaluate the filter dicision and write selected jets into the event
    bool const filterDecision = (selectedJets->size() >= minNumJets);
    event.put(std::move(selectedJets));
    return filterDecision;
}


reco::GenJet const *JERCJetSelector::MatchGenJet(reco::Jet const &jet,
  edm::View<reco::GenJet> const &genJets, double maxDPt) const
{
    reco::GenJet const *matchedJet = nullptr;
    double minDR2 = std::numeric_limits<double>::infinity();
    double const maxDR2 = jetConeSize * jetConeSize / 4.;
    
    for (auto const &genJet: genJets)
    {
        double const dR2 = ROOT::Math::VectorUtil::DeltaR2(jet.p4(), genJet.p4());
        
        if (dR2 > maxDR2 or dR2 > minDR2)
            continue;
        
        if (std::abs(jet.pt() - genJet.pt()) > maxDPt)
            continue;
        
        minDR2 = dR2;
        matchedJet = &genJet;
    }
    
    
    return matchedJet;
}


DEFINE_FWK_MODULE(JERCJetSelector);
