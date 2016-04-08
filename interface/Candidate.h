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
    Candidate() noexcept;
    
    /// Default move constructor
    Candidate(Candidate &&) = default;
    
    /// Default copy constructor
    Candidate(Candidate const &) = default;
    
    /// Default assignment operator
    Candidate &operator=(Candidate const &) = default;
    
    /// Default virtual destructor
    virtual ~Candidate() = default;
    
public:
    /// Resets the object to a state right after the default initialisation
    virtual void Reset();
    
    /// Sets transverse momentum (GeV/c)
    void SetPt(float pt);
    
    /// Sets pseudorapidity
    void SetEta(float eta);
    
    /**
     * \brief Sets azimuthal angle
     * 
     * The range is [-pi, pi).
     */
    void SetPhi(float phi);
    
    /// Sets mass (GeV/c^2)
    void SetM(float mass);
    
    /// Returns transverse momentum (GeV/c)
    float Pt() const;
    
    /// Returns pseudorapidity
    float Eta() const;
    
    /**
     * \brief Returns azimuthal angle
     * 
     * The range is [-pi, pi).
     */
    float Phi() const;
    
    /// Returns mass (GeV/c^2)
    float M() const;
    
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
}  // end of namespace pec
