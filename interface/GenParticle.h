#pragma once

#include <Analysis/PECTuples/interface/Candidate.h>


namespace pec
{
/**
 * \class GenParticle
 * \brief Minimalistic discription of a generator-level particle
 * 
 * The user is expected to use a small filtered collection of particles; the mother indices
 * correspond to positions in this filtered collection.
 */
class GenParticle: public Candidate
{
public:
    /// Constructor with no parameters
    GenParticle() noexcept;
    
public:
    /// Resets the object to a state right after the default initialisation
    virtual void Reset() override;
    
    /// Sets PDG ID
    void SetPdgId(int pdgId);
    
    /**
     * \brief Sets index of the first mother
     * 
     * The index must follow requirements specified in the documentation of the method
     * FirstMotherIndex.
     */
    void SetFirstMotherIndex(int index);
    
    /**
     * \brief Sets index of the last mother
     * 
     * The index must follow requirements specified in the documentation of the method
     * LastMotherIndex.
     */
    void SetLastMotherIndex(int index);
    
    /// Returns PDG ID
    int PdgId() const;
    
    /**
     * \brief Returns index of the first mother
     * 
     * Valid indices start from zero; (-1) indicates that there is no mother in the list. Note that
     * the index correponds to a position in the trimmed list of generator particles, not the
     * orginal genParticles collection in an EDM event.
     */
    int FirstMotherIndex() const;
    
    /**
     * \brief Returns index of the last mother
     * 
     * See documentation for the method FirstMotherIndex. The returned value is expected to differ
     * from (-1) if only the particle has more than one mother.
     */
    int LastMotherIndex() const;
    
private:
    /// PDG ID
    Int_t pdgId;
    
    /**
     * \brief Indices of the first and the last mother of the particle
     * 
     * Normally, one would use a filtered collection of generator-level particles. These indices
     * correspond to positions in this collection, not in the orignal collection genParticles in an
     * EDM event. Indices start from 1, and 0 is reserved to indicate that the stored collection
     * does not contain the mother (but the getter subtracts 1, so that normal indices start from
     * zero, and the special value is assigned to (-1)). The lastMotherIndex is non-zero if only the
     * particle has more that one mother.
     */
    UChar_t firstMotherIndex, lastMotherIndex;
};
}  // end of namespace pec
