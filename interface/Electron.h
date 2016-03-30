#pragma once

#include <Analysis/PECTuples/interface/Lepton.h>


namespace pec
{
/**
 * \class Electron
 * \brief Represents a reconstructed electron
 * 
 * Extends class Lepton by adding sets of boolean and real-valued identification decisions. They are
 * intended to be used to store results of cut-based and MVA algorithms, respectively. Up to eight
 * boolean flags can be stored. The maximal number of MVA-based decisions is given by contIdSize,
 * which can be changed at compile time only.
 */
class Electron: public Lepton
{
public:
    /// Constructor with no parameters
    Electron();
    
    /// Copy constructor
    Electron(Electron const &src);
    
    /// Assignment operator
    Electron &operator=(Electron const &src);
    
public:
    /// Resets the object to a state right after the default initialisation
    virtual void Reset();
    
    /**
     * \brief Sets a decision of a cut-based ID
     * 
     * The decision is encoded in a bit field. Decision for several working points can be written
     * using several bits. Throws an exception if the index of out of the supported range.
     */
    void SetBooleanID(unsigned bitIndex, bool value = true);
    
    /**
     * \brief Saves real-valued response of an MVA discriminator
     * 
     * Throws an exception if the index is out of the supported range.
     */
    void SetContinuousID(unsigned index, float mva);
    
    /// Sets pseudorapidity of the associated supercluster
    void SetEtaSC(float etaSC);
    
    /**
     * \brief Returns decision of selected version of the cut-based ID
     * 
     * See documentation for the SetCutBasedId method.
     */
    bool BooleanID(unsigned bitIndex) const;
    
    /**
     * \brief Returns the value of MVA-based ID
     * 
     * See documentation for the SetMvaID method.
     */
    float ContinuousID(unsigned index) const;
    
    /// Returns pseudorapidity of the associated supercluster
    float EtaSC() const;
    
private:
    
    /// Pseudorapidity of the associated supercluster
    Float_t etaSC;
    
    /**
     * \brief Encodes flags for boolean ID decisions
     * 
     * Usually, decisions for various working points of a cut-based identification algorithm are
     * incorporated into this variable.
     */
    UChar_t cutBasedId;
    
    /// Maximal number of continuous ID discriminators that can be stored
    static unsigned const contIdSize = 1;

    /**
     * \brief Continuous ID decisions
     * 
     * See documentation for the SetContinuousId method.
     */
    Float_t mvaId[contIdSize];
};
}  // end of namespace pec
