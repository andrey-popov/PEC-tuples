/**
 * \file PartonShowerOutcome.h
 * \author Andrey Popov
 * 
 * The module defines a cmsRun plugin to save properties of selected particles in the final state
 * after parton shower but before hadronisation. It is intended to be used for heavy-flavour quarks
 * to allow classification of W+jets or ttbar+jets events, but it can be used for other particles
 * as well.
 * 
 * User can choose particles to be saved by giving a list of absolute values of allowed PDG ID. In
 * addition to the filtering on PDG ID, a particle must have status 2 and have a daughter that is
 * either stable (has status 1) or is a special object (PDG ID from 81 to 100, which most notably
 * includes strings and clusters).
 * 
 * For each particle that satisfies the above requirements several properties are stored in a plain
 * ROOT tree. The plugins saves their PDG ID and three-momentum in parametrisation (pt, eta, phi).
 * The mass (or energy) component is dropped to save space as it can be recovered easily from type
 * of the particle and in most cases is not needed at all (for instance, to perform angular matching
 * with reconstructed jets). In addition to it each particle is classified depending on its origin.
 * If it is an immediate daughter of a beam particle, it is assigned code ParticleOrigin::Proton
 * (see the corresponding enumeration defined below). If its first mother with status 3 is a
 * granddaughter of a beam particle, the particle is from ISR. In all the rest cases it is
 * attributed to FSR.
 * 
 * In order to fully classify a W+jets event, results of this plugin should be complemented with
 * information about hard process.
 * 
 * Example configuration of the plugin:
 *   heavyFlavours = cms.EDAnalyzer('PartonShowerOutcome',
 *       absPdgId = cms.vint32(4, 5),
 *       genParticles = cms.InputTag('genParticles'))
 */

#pragma once

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
 * Consult the file's documentation section for details.
 */
class PartonShowerOutcome: public edm::EDAnalyzer
{
private:
    /// Specifies the origin of a particle
    enum class ParticleOrigin
    {
        Undefined = 0,
        ISR = 1,    // initial-state radiation
        FSR = 2,    // final-state radiation
        Proton = 3  // an immediate daughter of one of the initial beam particles
    };
    
public:
    /// Constructor from a configuration
    PartonShowerOutcome(edm::ParameterSet const &cfg);
    
public:
    /// Creates the output tree
    virtual void beginJob();
    
    /// Processes the current event and fills the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &eventSetup);
    
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
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
    static ParticleOrigin DeduceOrigin(reco::Candidate const &particle);
    
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
    
    /// Maximal number of particles that can be stored in an event
    static int const maxSize = 32;
    
    
    // Output buffers
    /// Number of particles stored by the plugin in the current event
    UChar_t bfSize;
    
    /// PDG ID of a particle
    Short_t bfPdgId[maxSize];
    
    /**
     * \brief Encodes the origin of a particle
     * 
     * Possible values are given by the ParticleOrigin enumeration.
     */
    UChar_t bfOrigin[maxSize];
    
    /// Three-momentum of a particle
    Float_t bfPt[maxSize], bfEta[maxSize], bfPhi[maxSize];
};
