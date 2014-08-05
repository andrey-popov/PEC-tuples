/**
 * \file PartonShowerOutcome.h
 * \author Andrey Popov
 * 
 * 
 * Example configuration of the plugin:
 *   heavyFlavours = cms.EDAnalyzer('PartonShowerOutcome',
 *       absPdgId = cms.vint32(4, 5),
 *       genParticles = cms.InputTag('genParticles'))
 */

#pragma once

#include <UserCode/SingleTop/interface/ShowerParton.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>
#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <DataFormats/HepMCCandidate/interface/GenParticle.h>


#include <TTree.h>

#include <vector>


/**
 * \class PartonShowerOutcome
 * \brief Saves information about a final state after the parton shower
 * 
 * The plugin saves properties of selected particles in the final state after the parton shower but
 * before hadronisation. It is intended to be used for heavy-flavour quarks to allow classification
 * of W+jets or ttbar+jets events, but it can be deployed for other particles as well.
 * 
 * User can choose particles to be saved by giving a list of absolute values of allowed PDG ID. In
 * addition to the filtering on PDG ID, a particle must have status 2 and have a daughter that is
 * either stable (has status 1) or is a special object (PDG ID from 81 to 100, which most notably
 * includes strings and clusters). It means that the plugin is suitable for events showered with
 * Pythia 6 only.
 * 
 * Selected particles are stored as instances of the class pec::ShowerParton. Consult documentation
 * for this class for further details.
 * 
 * In order to fully classify a W+jets event, results of this plugin should be complemented with
 * information about the hard scattering.
 */
class PartonShowerOutcome: public edm::EDAnalyzer
{
public:
    /// Constructor from a configuration
    PartonShowerOutcome(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates the output tree
    virtual void beginJob();
    
    /// Processes the current event and fills the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &eventSetup);
    
private:
    /**
     * \brief Identifies origin of the given particle
     * 
     * The origin is deduced based on the (genetic) distance from the first mother with status 3
     * to the beam particle. If this distance is zero, i.e. the first mother with status 3 is
     * nothing but a beam particle, the method returns ParticleOrigin::Proton. If the distance is
     * one, the particle is classified as ISR. Otherwise it is attributed to FSR. Note that
     * descendants of particles in the final state of the hard process are also considered as FSR.
     */
    static pec::ShowerParton::Origin DeduceOrigin(reco::Candidate const &particle);
    
private:
    /// Absolute values of PDG ID of particles to be saved
    std::vector<int> absPdgIdToSave;
    
    /// An input tag to read generator particles
    edm::InputTag genParticlesSrc;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief The output tree
     * 
     * The tree is managed by the fileService object and will be deleted by its descructor.
     */
    TTree *outTree;
    
    /**
     * \brief Trimmer shower partons to be stored in the output file
     * 
     * Masses are set to zero for a more efficient compression as they can be recovered from PDG ID
     * easily.
     */
    std::vector<pec::ShowerParton> storePartons;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::ShowerParton> *storePartonsPointer;
};
