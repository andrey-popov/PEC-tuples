#pragma once

#include <UserCode/SingleTop/interface/CandidateWithID.h>


namespace pec
{
    /**
     * \class Lepton
     * \author Andrey Popov
     * \brief Base class for charged leptons
     */
    class Lepton: public CandidateWithID
    {
    public:
        /// Constructor with no parameters
        Lepton();
        
        /// Copy constructor
        Lepton(Lepton const &src);
        
        /// Assignment operator
        Lepton &operator=(Lepton const &src);
        
    public:
        /**
         * \brief Sets lepton charge
         * 
         * Checks only the sign of the argument. If it is zero, throws an exception.
         */
        void SetCharge(int charge);
        
        /// Sets relative isolation
        void SetRelIso(double relIso);
        
        /// Sets transverse impact parameter (cm)
        void SetDB(double dB);
        
        /**
         * \brief Returns electric charge of the lepton
         * 
         * The returned value is +1 or -1.
         */
        int Charge() const;
        
        /// Returns relative isolation
        double RelIso() const;
        
        /// Returns transverse impact parameter (cm)
        double DB() const;
        
    private:
        /**
         * \brief Electric charge
         * 
         * True for negative charge (particle), false for positive charge (antiparticle).
         */
        Bool_t charge;
        
        /// Relative isolation
        Float_t relIso;
        
        /// Transverse impact parameter
        Float_t dB;
    };
}
