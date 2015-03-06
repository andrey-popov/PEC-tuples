#pragma once

#include <UserCode/SingleTop/interface/GenParticle.h>

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
 * Particles from the hard interaction are identified with their status, which is hard-coded to the
 * value used in Pythia 6.
 */
class HardInteractionInfo: public edm::EDAnalyzer
{
private:
    /// Supported generators
    enum class Generator
    {
        Pythia6,
        Pythia8
    };
    
    /**
     * \class ParticleMother
     * \brief Keeps a pointer to reco::Candidate and overrides its mother
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
        
        /// Constructor with complete initialisation
        ParticleWithMother(reco::Candidate const *particle,
         reco::Candidate const *mother = nullptr);
        
    public:
        /// Forwards the call to the wrapped particle
        reco::Candidate const *operator->() const;
        
        /// Compares wrapped particle to the given pointer
        bool operator==(reco::Candidate const *rhs) const;
        
        /// Returns the referenced particle
        reco::Candidate const *Get() const;
        
        /// Resets the mother
        void ResetMother(reco::Candidate const *mother);
        
        /// Returns number of mothers of the referenced particle
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
        /// Particle referenced by the object
        reco::Candidate const *particle;
        
        /**
         * \brief Overriding mother
         * 
         * The overriding mother is considered defined if this pointer is non-zero.
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
    virtual void beginJob();
    
    /// Reads the event and stores the relevant information in the output tree
    virtual void analyze(edm::Event const &event, edm::EventSetup const &setup);
    
private:
    /**
     * \brief Adds given particle to the list of particles that will be stored
     * 
     * A particle is added if only it has not been added before, i.e. duplicates are not avoided.
     * The return value indicates if it has been added.
     */
    bool BookParticle(reco::Candidate const *p, reco::Candidate const *overwriteMother = nullptr);
    
private:
    /// Collection of generator-level particles
    edm::EDGetTokenT<edm::View<reco::GenParticle>> genParticlesToken;
    
    /// Generator used
    Generator generator;
    
    /// (Absolute) PDG IDs of additional particles to be saved
    std::set<int> addPartToSave;
    
    /// An object to handle the output ROOT file
    edm::Service<TFileService> fileService;
    
    /**
     * \brief Pointers to particles that are going to be stored
     * 
     * The vector is utilised to keep track of particles that have been accepted to be stored and
     * helps to avoid duplicates. The pointers refer to elements of the collection read by
     * genParticlesToken. This vector and storeParticles are synchronised.
     */
    std::vector<ParticleWithMother> bookedParticles;
    
    /// Tree to be written in the output ROOT file
    TTree *outTree;
    
    /**
     * \brief Trimmed generator-level particles to be stored in the output file
     * 
     * This vector and bookedParticles are synchronised.
     */
    std::vector<pec::GenParticle> storeParticles;
    
    /**
     * \brief An auxiliary pointer
     * 
     * ROOT needs a variable with a pointer to an object to store the object in a tree.
     */
    std::vector<pec::GenParticle> *storeParticlesPointer;
};
