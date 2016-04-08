#pragma once

#include <Analysis/PECTuples/interface/Candidate.h>


namespace pec
{
/**
 * \class CandidateWithID
 * \brief Adds a set of user-defined booleand IDs to the class Candidate
 * 
 * The ID flags are accessed by index, up to 8 flags are supported. If a flag is set to true, the
 * candidate is supposed to be "good" in what concerns the corresponding ID.
 */
class CandidateWithID: public Candidate
{
public:
    /// Constructor with no parameters
    CandidateWithID() noexcept;
    
public:
    /// Resets the object to a state right after the default initialisation
    virtual void Reset() override;
    
    /**
     * \brief Sets or unsets an ID bit
     * 
     * Throws an exception if the index exceeds the maximal allowed number of flags.
     */
    void SetBit(unsigned index, bool value = true);
    
    /**
     * \brief Tests an ID bit
     * 
     * Throws an exception if the index exceeds the maximal allowed number of flags.
     */
    bool TestBit(unsigned index) const;
    
private:
    /// Variable to hold ID flags
    UChar_t id;
};
}  // end of namespace pec
