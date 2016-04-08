#pragma once

#include <Analysis/PECTuples/interface/Lepton.h>


namespace pec
{
/**
 * \class Muon
 * \brief Represents a reconstructed muon
 * 
 * At the moment it adds nothing to the base class.
 */
class Muon: public Lepton
{
public:    
    /// Constructor with no parameters
    Muon() noexcept;
    
public:
    /// Resets the object to a state right after the default initialisation
    virtual void Reset() override;
};
}  // end of namespace pec
