#pragma once

#include <Rtypes.h>


namespace pec
{
    /**
     * \class Candidate
     * \brief Stores four-momentum
     * 
     * Components of the four-momentum are compressed using minifloat functions.
     */
    class Candidate
    {
    public:
        /**
         * \brief Constructor with no parameters
         * 
         * It must be defined in order to use the class with the ROOT i/o system.
         */
        Candidate();
        
        /// Copy constructor
        Candidate(Candidate const &src);
        
        /// Assignment operator
        Candidate &operator=(Candidate const &src);
        
        /// Trivial virtual destructor
        virtual ~Candidate();
        
    public:
        /// Resets the object to a state right after the default initialisation
        virtual void Reset();
        
        /// Sets transverse momentum (GeV/c)
        void SetPt(double pt);
        
        /// Sets pseudorapidity
        void SetEta(double eta);
        
        /**
         * \brief Sets azimuthal angle
         * 
         * The range is [-pi, pi).
         */
        void SetPhi(double phi);
        
        /// Sets mass (GeV/c^2)
        void SetM(double mass);
        
        /// Returns transverse momentum (GeV/c)
        double Pt() const;
        
        /// Returns pseudorapidity
        double Eta() const;
        
        /**
         * \brief Returns azimuthal angle
         * 
         * The range is [-pi, pi).
         */
        double Phi() const;
        
        /// Returns mass (GeV/c^2)
        double M() const;
        
    private:
        /// Transverse momentum, GeV/c
        Float_t pt;
        
        /// Pseudorapidity
        Float_t eta;
        
        /// Azimuthal angle, [-pi, pi)
        Float_t phi;
        
        /// Mass, GeV/c^2
        Float_t mass;
    };
}
