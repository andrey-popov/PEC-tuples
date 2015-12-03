#pragma once

#include <Analysis/PECTuples/interface/minifloats.h>


namespace pec
{
    /**
     * \class Candidate
     * \author Andrey Popov
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
        /**
         * \brief Transverse momentum, GeV/c
         * 
         * Encoded as a generic minifloat with parameters (false, 12, 1), the range representable
         * with normal numbers is [0.5, 16k).
         */
        minifloat::Repr_t pt;
        
        /**
         * \brief Pseudorapidity
         * 
         * Encoded as a generic minifloat with parameters (true, 13, -1), the range representable
         * with normal numbers is [2, 16).
         */
        minifloat::Repr_t eta;
        
        /**
         * \brief Azimuthal angle, [-pi, pi)
         * 
         * Encoded with a uniform minifloat at a range (-pi, pi).
         */
        minifloat::Repr_t phi;
        
        /**
         * \brief Mass, GeV/c^2
         * 
         * Encoded as a generic minifloat with parameters (false, 12, 2), the range representable
         * with normal numbers is [0.25, 8k).
         */
        minifloat::Repr_t mass;
    };
}
