#pragma once

#include <Rtypes.h>


namespace pec
{
    /**
     * \class Candidate
     * \brief A simple class that stores four-momentum
     */
    class Candidate
    {
    public:
        /**
         * \brief Default constructor
         * 
         * It must be defined to use the class with the ROOT i/o system.
         */
        Candidate();
        
        /// Copy constructor
        Candidate(Candidate const &src);
        
        /// Assignment operator
        Candidate &operator=(Candidate const &src);
        
    public:
        void SetPt(double pt);
        void SetEta(double eta);
        void SetPhi(double phi);
        void SetM(double mass);
        
        double Pt() const;
        double Eta() const;
        double Phi() const;
        double M() const;
        
    private:
        Float_t pt;
        Float_t eta;
        Float_t phi;
        Float_t mass;
    };
}
