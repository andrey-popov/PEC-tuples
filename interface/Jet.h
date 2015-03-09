#pragma once

#include <UserCode/SingleTop/interface/minifloats.h>
#include <UserCode/SingleTop/interface/CandidateWithID.h>


namespace pec
{
    /**
     * \class Jet
     * \author Andrey Popov
     * \brief Represents a reconstructed jet
     * 
     * Stored four-momentum (via the Candidate base class) is uncorrected. Some properties of a jet
     * (especially of a soft one) might be left uninitialised if they are not expected to be used in
     * an analysis. Properties that make sence for simulations only (like flavours) are not expected
     * to be set in case of real data.
     */
    class Jet: public CandidateWithID
    {
    public:
        /// Constructor with no parameters
        Jet();
        
        /// Copy constructor
        Jet(Jet const &src);
        
        /// Assignment operator
        Jet &operator=(Jet const &src);
        
    public:
        /// Resets the object to a state right after the default initialisation
        virtual void Reset();
        
        /// Sets the value of the CSV b-tagging discriminator
        void SetBTagCSV(double bTag);
        
        /// Sets mass of the secondary vertex associated with the jet
        void SetSecVertexMass(double mass);
        
        /// Sets jet area
        void SetArea(double area);
        
        /**
         * \brief Sets electric charge of the jet
         * 
         * See documentation for the method Charge for a description of this quantity.
         */
        void SetCharge(double charge);
        
        /**
         * \brief Sets the pull angle
         * 
         * See documentation of the method PullAngle for a description of this quantity.
         */
        void SetPullAngle(double angle);
        
        /**
         * \brief Sets jet flavour
         * 
         * This method must be used for simulation only. It is assumed that the flavour is
         * determined with the clustering of ghost hadrons as described in [1].
         * [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools
         */
        void SetFlavour(int flavour);
        
        /// Returns the value of the CSV b-tagging discriminator
        double BTagCSV() const;
        
        /**
         * \brief Returns mass of the secondary vertex associated with the jet
         * 
         * When there is no secondary vertex associated with the jet, the method returns 0.
         */
        double SecVertexMass() const;
        
        /// Returns jet area
        double Area() const;
        
        /**
         * \brief Returns electric charge of the jet
         * 
         * Several definitions of jet charge are used [1]. This method is expected to return the
         * value given by pat::Jet::jetCharge, which is calculated as a sum of electric charges of
         * jet's constituents weighted with their pt, as documented in [2].
         * [1] http://arxiv.org/abs/1209.2421
         * [2] https://hypernews.cern.ch/HyperNews/CMS/get/JetMET/1425.html
         */
        double Charge() const;
        
        /**
         * \brief Returns jet pull angle
         * 
         * The pull vector is defined in [1], Eq. (3.7). The pull angle is an angle between this
         * vector and the rapidity axis. The range is [-pi, pi).
         * [1] http://arxiv.org/abs/1010.3698
         */
        double PullAngle() const;
        
        /**
         * \brief Returns jet flavour
         * 
         * Expected to return zero for real data. If the flavour of a jet in a simulated event is
         * undefined, a zero is returned.
         */
        int Flavour() const;
        
    private:
        /**
         * \brief Value of b-tagging discriminator
         * 
         * Encoded as a generic minifloat with parameters (true, 12, 1), the range representable
         * with normal numbers is [0.5, 64).
         */
        minifloat::Repr_t bTagCSV;
        
        /**
         * \brief Mass of the secondary vertex associated to the jet (if any)
         * 
         * Encoded as a generic minifloat with parameters (false, 12, 2), the range representable
         * with normal numbers is [0.25, 8k).
         */
        minifloat::Repr_t secVertexMass;
        
        /**
         * \brief Jet area
         * 
         * Encoded as a generic minifloat with parameters (false, 14, 0), the range representable
         * with normal numbers is [1, 8).
         */
        minifloat::Repr_t area;
        
        /**
         * \brief Electric charge of the jet
         * 
         * See documentation for the method Charge. Encoded with a uniform minifloat at a
         * range [-1, 1].
         */
        minifloat::Repr_t charge;
        
        /**
         * \brief Jet pull angle
         * 
         * See documentation for the method PullAngle. Encoded with a uniform minifloat at a
         * range (-pi, pi).
         */
        minifloat::Repr_t pullAngle;
        
        /// Jet flavour
        Char_t flavour;
    };
}
