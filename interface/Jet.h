#pragma once

#include <Analysis/PECTuples/interface/CandidateWithID.h>


namespace pec
{
/**
 * \class Jet
 * \brief Represents a reconstructed jet
 * 
 * Stored four-momentum (via the Candidate base class) is uncorrected. Some properties of a jet
 * (especially of a soft one) might be left uninitialized if they are not expected to be used in an
 * analysis. Properties that make sence for simulations only (like flavours) are not expected to be
 * set in case of real data.
 */
class Jet: public CandidateWithID
{
public:
    /// Supported b-tagging algorithms
    enum class BTagAlgo: unsigned
    {
        CSV = 0,
        CMVA = 1
    };
    
    /// Supported c-tagging algorithms
    enum class CTagAlgo: unsigned
    {
        CvsB = 0,
        CvsL = 1
    };
    
public:
    /// Constructor with no parameters
    Jet() noexcept;
    
public:
    /// Resets the object to a state right after the default initialisation
    virtual void Reset();
    
    /// Sets full jet energy correction factor
    void SetCorrFactor(float jecFactor);
    
    /// Sets relative uncertainty of the JEC factor
    void SetJECUncertainty(float jecUncertainty);
    
    /// Sets relative uncertainty of the JER smearing factor
    void SetJERUncertainty(float jecUncertainty);
    
    /// Sets value of the given b-tagging discriminator
    void SetBTag(BTagAlgo algo, float value);
    
    /// Sets value of the given c-tagging discriminator
    void SetCTag(CTagAlgo algo, float value);
    
    /// Sets mass of the secondary vertex associated with the jet
    void SetSecVertexMass(float mass);
    
    /// Sets value of the pile-up discriminator
    void SetPileUpID(float pileUpMVA);
    
    /// Sets jet area
    void SetArea(float area);
    
    /**
     * \brief Sets electric charge of the jet
     * 
     * See documentation for the method Charge for a description of this quantity.
     */
    void SetCharge(float charge);
    
    /**
     * \brief Sets the pull angle
     * 
     * See documentation of the method PullAngle for a description of this quantity.
     */
    void SetPullAngle(float angle);
    
    /**
     * \brief Sets jet flavour
     * 
     * This method must be used for simulation only. It is assumed that the flavour is determined
     * with the clustering of ghost hadrons as described in [1].
     * [1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools
     */
    void SetFlavour(int flavour);
    
    /**
     * \brief Returns full jet energy correction factor
     * 
     * The raw momentum should be rescaled using this factor in order to apply full JEC and JER. The
     * returned value might be zero if only raw momentum is saved and the correction must be applied
     * by the user.
     */
    float CorrFactor() const;
    
    /**
     * \brief Returns relative uncertainty of the JEC factor
     * 
     * Jet four-momentum corresponding to the up/down variation in the JEC systematics can be
     * obtained as P4() * CorrFactor() * (1 +/- JECUncertainty()). If only raw momentum has been
     * stored and the correction is to be applied by the user, the method returns zero.
     */
    float JECUncertainty() const;
    
    /**
     * \brief Returns relative uncertainty of the JER smearing factor
     * 
     * Jet four-momentum corresponding to the up/down variation in the JER systematics can be
     * obtained as P4() * CorrFactor() * (1 +/- JERUncertainty()). If only raw momentum has been
     * stored and the correction is to be applied by the user, the method returns zero.
     */
    float JERUncertainty() const;
    
    /// Returns value of the requested b-tagging discriminator
    float BTag(BTagAlgo algo) const;
    
    /// Returns value of the requested c-tagging discriminator
    float CTag(CTagAlgo algo) const;
    
    /// Returns value of the pile-up discriminator
    float PileUpID() const;
    
    /**
     * \brief Returns mass of the secondary vertex associated with the jet
     * 
     * When there is no secondary vertex associated with the jet, the method returns 0.
     */
    float SecVertexMass() const;
    
    /// Returns jet area
    float Area() const;
    
    /**
     * \brief Returns electric charge of the jet
     * 
     * Several definitions of jet charge are used [1]. This method is expected to return the value
     * given by pat::Jet::jetCharge, which is calculated as a sum of electric charges of jet
     * constituents weighted with their pt, as documented in [2].
     * [1] http://arxiv.org/abs/1209.2421
     * [2] https://hypernews.cern.ch/HyperNews/CMS/get/JetMET/1425.html
     */
    float Charge() const;
    
    /**
     * \brief Returns jet pull angle
     * 
     * The pull vector is defined in [1], Eq. (3.7). The pull angle is an angle between this vector
     * and the rapidity axis. The range is [-pi, pi).
     * [1] http://arxiv.org/abs/1010.3698
     */
    float PullAngle() const;
    
    /**
     * \brief Returns jet flavour
     * 
     * Expected to return zero for real data. If the flavour of a jet in a simulated event is
     * undefined, a zero is returned.
     */
    int Flavour() const;
    
private:
    /**
     * \brief Full jet energy correction factor
     * 
     * Set to zero if only raw momentum is stored.
     */
    Float_t corrFactor;
    
    /**
     * \brief Relative uncertainty of JEC factor
     * 
     * Set to zero if only raw momentum is stored.
     */
    Float_t jecUncertainty;
    
    /**
     * \brief Relative uncertainty of JER smearing factor
     * 
     * Set to zero if only raw momentum is stored.
     */
    Float_t jerUncertainty;
    
    /// Values of b-tagging  and c-tagging discriminators
    Float_t bTags[2], cTags[2];
    
    /// Mass of the secondary vertex associated to the jet (if any), GeV/c^2
    Float_t secVertexMass;
    
    /// Value of an MVA discriminator against pile-up
    Float_t pileUpMVA;
    
    /// Jet area
    Float_t area;
    
    /**
     * \brief Electric charge of the jet
     * 
     * See documentation for the method Charge.
     */
    Float_t charge;
    
    /**
     * \brief Jet pull angle, [-pi, pi)
     * 
     * See documentation for the method PullAngle.
     */
    Float_t pullAngle;
    
    /// Jet flavour
    Char_t flavour;
};
}  // end of namespace pec
