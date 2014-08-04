#pragma once

#include <UserCode/SingleTop/interface/Candidate.h>


namespace pec
{
    /**
     * \class GenJet
     * \author Andrey Popov
     * \brief Generator-level jet
     * 
     * A bare four-momentum with an optional counter of hadrons with b and c quarks as constituents
     * of the jet.
     */
    class GenJet: public Candidate
    {
    public:
        /// Constructor with no parameters
        GenJet();
        
        /// Copy constructor
        GenJet(GenJet const &src);
        
        /// Assignment operator
        GenJet &operator=(GenJet const &src);
        
    public:
        /// Resets the object to a state right after the default initialisation
        virtual void Reset();
        
        /**
         * \brief Sets multiplicity of B hadrons
         * 
         * If the argument exceeds 15, the multiplicity is set to 15.
         */
        void SetBottomMult(unsigned mult);
        
        /**
         * \brief Sets multiplicity of C hadrons
         * 
         * If the argument exceeds 15, the multiplicity is set to 15.
         */
        void SetCharmMult(unsigned mult);
        
        /// Returns multiplicity of B hadrons inside the jet
        unsigned BottomMult() const;
        
        /// Returns multipliciyt of C hadrons inside the jet
        unsigned CharmMult() const;
        
    private:
        /**
         * \brief Number of hadrons with b or c quarks among jet constituents
         * 
         * Constructed as the number of B hadrons multiplied by 16 plus number of C hadrons.
         */
        UChar_t bcMult;
    };
}
