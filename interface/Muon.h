#pragma once

#include <Analysis/PECTuples/interface/Lepton.h>


namespace pec
{
    /**
     * \class Muon
     * \author Andrey Popov
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
        
        /// Copy constructor
        Muon(Muon const &src);
        
        /// Assignment operator
        Muon &operator=(Muon const &src);
    };
}
