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
    /// Resets the object to a state right after the default initialisation
    virtual void Reset();
    
    /// Constructor with no parameters
    Muon();
    
    /// Default copy constructor
    Muon(Muon const &) = default;
    
    /// Default assignment operator
    Muon &operator=(Muon const &) = default;
};
}  // end of namespace pec
