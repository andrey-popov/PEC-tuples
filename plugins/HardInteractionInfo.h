#pragma once

#include <Analysis/PECTuples/interface/GenParticle.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/ParameterSet/interface/ConfigurationDescriptions.h>
#include <FWCore/ParameterSet/interface/ParameterSetDescription.h>

#include <DataFormats/HepMCCandidate/interface/GenParticle.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>

#include <set>
#include <vector>


/**
 * \class HardInteractionInfo
 * \author Andrey Popov
 * \brief Stores particles from the hard interaction in a ROOT file
 * 
 * Stores particles from the final and initial states of the hard(est) interaction. However, in
 * Pythia 8 gluons in the initial state are not kept in miniAOD and therefore are not available.
 * 
 * In addition, can store extra particles according to a list of PDG ID codes provided by the user
 * (only those with abs(PDG ID) < 80 and status > 2 are considered). In Pythia 8 same particle can
 * be written many times in the event record, as it proceeds through various steps of simulation and
 * undergoes radiation. The plugin stores only the oldest ancestor (of the same PDG ID). It also
 * saves daughers of the youngest descendant.
 * 
 * The plugin can handle samples produced with Pythia 6 or 8.
 */
class HardInteractionInfo: public edm::EDAnalyzer
{
private:
    /**
     * \class ParticleMother
     * \brief Wraps a pointer to reco::Candidate and overrides its mother
     * 
     * If the user constructs an object of this class providing a mother, the given one overrides
     * real mothers of the particle. If the mother is not specified, the class serves as a proxy
     * providing access to the real mothers.
     */
    class ParticleWithMother
    {
    public:
        /// Constructor with no parameters
        ParticleWithMother();
        
        /**
         * \brief Constructor with complete initialisation
         * 
         * If a null pointer is given as the second argument, the real mothers are not overridden.
         */
        ParticleWithMother(reco::Candidate const *particle,
         reco::Candidate const *mother = nullptr);
        
        /// Default move constructor
        ParticleWithMother(ParticleWithMother &&) = default;
        
    public:
        /// Forwards the call to the wrapped particle
        reco::Candidate const *operator->() const;
        
        /// Compares the wrapped pointer to a particle to the argument
        bool operator==(reco::Candidate const *rhs) const;
        
        /// Returns the wrapped pointer to a particle
        reco::Candidate const *Get() const;
        
        /// Resets the mother
        void ResetMother(reco::Candidate const *mother);
        
        /// Returns the number of mothers of the referenced particle
        unsigned NumberOfMothers() const;
        
        /**
         * \brief Returns mother with the given index
         * 
         * Negative indices are allowed and interpreted as starting from the last mother (for
         * instance, (-1) means the last mother). If the index is out of the allowed range, returns
         * a null pointer.
         */
        reco::Candidate const *Mother(int index) const;
        
    private:
        /// Wrapped pointer to a particle
        reco::Candidate const *particle;
        
        /**
         * \brief Overriding mother
         * 
         * The overriding mother is considered defined if this pointer is not null.
         */
        reco::Candidate const *mother;
    };
    
public:
    /// Constructor
    HardInteractionInfo(edm::ParameterSet const &cfg);
    
public:
    /// A method to verify plugin's configuration
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    
    /// Creates the output tree
    virtual void beginJob() override;
    
    /// Reads the event and stores the relevant information in the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &setup) override;
    
private:
    /**
     * \brief Adds the given particle to the collection of particles that are going to be stored
     * 
     * The particle is added if only is has not been added before, i.e. duplicates are avoided. The
     * return value indicates if the particle has been added (if not, it was a duplicate).
     * Regardless of whether the given particle is new or already present in the collection, its
     * mother is updated if the second argument is not null.
     */
    bool BookParticle(reco::Candidate const *p, reco::Candidate const *mother = nullptr);
    
private:
    /// Collection of generator-level particles
    edm::EDGetTokenT<edm::View<reco::GenParticle>> genParticlesToken;
    
    /// (Absolute) PDG IDs of additional particles to be saved
    std::set<int> desiredExtraPartIds;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief Particles that are going to be stored
     * 
     * The container is utilised to keep track of paricles that have been accepted to be stored in
     * the output file and helps to avoid duplicates. Pointers in elements of the container refer to
     * particles in the collection read via genPartilcesToken. Original mothers of some of the
     * particles are overridden.
     */
    std::vector<ParticleWithMother> bookedParticles;
    
    /// Tree to be written in the output ROOT file
    TTree *outTree;
    
    /// Trimmed generator-level particles to be stored in the output file
    std::vector<pec::GenParticle> storeParticles;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::GenParticle> *storeParticlesPointer;
};
