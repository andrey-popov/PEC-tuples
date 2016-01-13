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
 * \class PECGenParticles
 * \brief Stores particles from the hard(est) interaction and other selected ones in a ROOT file
 * 
 * Stores particles from the final and initial states of the hard(est) interaction.
 * 
 * In addition, can store extra particles according to a list of PDG ID codes provided by the user.
 * Same particle can be written many times in the event record, as it proceeds through various steps
 * of simulation and undergoes radiation. The plugin stores only the oldest ancestor (of the same
 * PDG ID). It also saves daughers of the youngest descendant. Note that in MiniAOD these daughters
 * are not always available (for instance, this happens often for J/psi).
 * 
 * Mother-daughter relationships among selected particles are reconstructed. If the direct mother
 * of a particle is not selected to be stored, its older ancestors are checked. For instance, if
 * user configured the plugin to store top quarks and J/psi, and in an event there is a J/psi
 * originating from t -> b -> ... -> B0 -> J/psi, the b quark (which is stored because its a
 * daughter of the top quark) will be listed as the mother of the J/psi in the stored collection.
 * 
 * The plugin is designed for samples produced with Pythia 6 or 8 (possibly, with an external LHE
 * generator). It might not work properly with other showering and hadronization programs.
 */
class PECGenParticles: public edm::EDAnalyzer
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
    PECGenParticles(edm::ParameterSet const &cfg);
    
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
