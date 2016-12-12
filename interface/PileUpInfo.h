#pragma once

#include <Rtypes.h>


namespace pec
{
/**
 * \class PileUpInfo
 * \brief Combines information related to pile-up
 */
class PileUpInfo
{
public:
    /// Constructor without parameters
    PileUpInfo() noexcept;
    
    /// Default copy constructor
    PileUpInfo(PileUpInfo const &) = default;
    
    /// Default assignment operator
    PileUpInfo &operator=(PileUpInfo const &) = default;
    
public:
    /// Resets the object to a state right after the default initialisation
    void Reset();
    
    /// Sets the number of good reconstructed primary vertices
    void SetNumPV(unsigned numPV);
    
    /// Sets average angular pt density
    void SetRho(float rho);
    
    /// Sets average angular pt density computed in the central region
    void SetRhoCentral(float rho);
    
    /**
     * \brief Sets the true number of pile-up interactions
     * 
     * The method must be used for simulation only.
     */
    void SetTrueNumPU(float lambda);
    
    /**
     * \brief Sets the number of pile-up interactions in the nominal bunch crossing
     * 
     * The method must be used for simulation only.
     */
    void SetInTimePU(unsigned n);
    
    /// Returns the number of good reconstructed primary vertices
    unsigned NumPV() const;
    
    /// Returns average angular pt density, GeV
    float Rho() const;
    
    /// Returns average angular pt density computed in the central region only
    float RhoCentral() const;
    
    /**
     * \brief Returns the true number of pile-up interactions
     * 
     * Actually, this is instantaneous luminosity measured in terms of the number of pile-up
     * interactions in a bunch crossing. When used with real data, the method is expected to return
     * a zero.
     */
    float TrueNumPU() const;
    
    /**
     * \brief Returns the number of pile-up interactions in the nominal bunch crossing
     * 
     * Returns a zero in case of real data.
     */
    unsigned InTimePU() const;
    
private:
    /// Number of good reconstructed primary vertices
    UChar_t numPV;
    
    /// Average angular pt densities, GeV
    Float_t rho, rhoCentral;
    
    /**
     * \brief "True" number of pile-up interactions
     * 
     * Zero in case of real data.
     */
    Float_t trueNumPU;
    
    /**
     * \brief Number of pile-up interactions in the in-time bunch crossing
     * 
     * Zero in case of real data.
     */
    UChar_t inTimeNumPU;
};
}  // end of namespace pec
