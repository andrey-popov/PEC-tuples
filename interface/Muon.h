#pragma once

#include <UserCode/SingleTop/interface/Lepton.h>


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
        /// Constructor with no parameters
        Muon();
        
        /// Copy constructor
        Muon(Muon const &src);
        
        /// Assignment operator
        Muon &operator=(Muon const &src);
    };
}
