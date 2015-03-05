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
        /// Considered algorithms for jet pile-up ID
        enum PileUpIDAlgo
        {
            CutBased,
            MVA
        };
        
        /// Working points of jet pile-up ID algorithms
        enum PileUpIDWorkingPoint
        {
            Loose,
            Medium,
            Tight
        };
        
        /**
         * \brief Supported definitions of jet flavour
         * 
         * Described in [1].
         * [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools?rev=30#Legacy_jet_flavour_definition
         */
        enum FlavourType
        {
            Algorithmic,
            Physics
        };
        
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
        
        /// Sets the value of the TCHP b-tagging discriminator
        void SetBTagTCHP(double bTag);
        
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
        
        /// Saves decision of a pile-up ID algorithm
        void SetPileUpID(PileUpIDAlgo algo, PileUpIDWorkingPoint wp, bool pass = true);
        
        /**
         * \brief Sets jet flavour
         * 
         * This method must be used for simulation only. See further details in the documentation
         * for the Flavour method.
         */
        void SetFlavour(FlavourType type, int flavour);
        
        /// Returns the value of the CSV b-tagging discriminator
        double BTagCSV() const;
        
        /// Returns the value of the THCP b-tagging discriminator
        double BTagTCHP() const;
        
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
        
        /// Returns decision of a pile-up ID algorithm
        bool PileUpID(PileUpIDAlgo algo, PileUpIDWorkingPoint wp) const;
        
        /**
         * \brief Returns jet flavour for the given definition
         * 
         * Expected to return zero for real data. If the flavour of a jet in a simulated event is
         * undefined, a zero is returned. See the list of supported flavour definitions in the
         * documentation for the FlavourType enumeration.
         */
        int Flavour(FlavourType type) const;
        
    private:
        /**
         * \brief Values of b-tagging discriminators
         * 
         * Encoded as a generic minifloat with parameters (true, 12, 1), the range representable
         * with normal numbers is [0.5, 64).
         */
        minifloat::Repr_t bTagCSV, bTagTCHP;
        
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
        
        /**
         * \brief Encodes jet pile-up ID
         * 
         * A bit set to store decisions of two jet pile-up ID algorithms. First three bits (0x1,
         * 0x2, 0x4) are set to true if the jet passes loose, medium, or tight working point of
         * the cut-based algorithm, respectively. The next three bits (0x8, 0x10, 0x20) are set in a
         * similar fashion if the jet passes working points of an MVA algorithm.
         * [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID
         */
        UChar_t pileUpID;
        
        /**
         * \brief "Algorithmic" definition of jet flavour
         * 
         * See documentation for the Flavour method.
         */
        Char_t flavourAlgorithmic;
        
        /**
         * \brief "Physics" definition of jet flavour
         * 
         * See documentation of the Flavour method.
         */
        Char_t flavourPhysics;
    };
}
