#pragma once

#include <Analysis/PECTuples/interface/minifloats.h>
#include <Analysis/PECTuples/interface/Lepton.h>


namespace pec
{
    /**
     * \class Electron
     * \brief Represents a reconstructed electron
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
         * The decision is encoded in a bit field. Decision for several working points can be
         * written using several bits.
         */
        void SetCutBasedIdBit(unsigned bitIndex, bool value = true);
        
        /**
         * \brief Sets the value of MVA-based ID
         * 
         * Expected to be used with the triggering MVA ID documented in [1].
         * [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentification
         */
        void SetMvaId(double mva);
        
        /**
         * \brief Returns decision of selected version of the cut-based ID
         * 
         * See documentation for the SetCutBasedId method.
         */
        bool CutBasedId(unsigned bitIndex) const;
        
        /**
         * \brief Returns the value of MVA-based ID
         * 
         * See documentation for the SetMvaID method.
         */
        double MvaId() const;
        
    private:
        /**
         * \brief Encodes flags for cut-based ID
         * 
         * Decisions for various working points are incorporated into this variable.
         */
        UChar_t cutBasedId;
        
        /**
         * \brief MVA-based ID
         * 
         * See documentation for the SetMvaId method. Encoded with a minifloat on a range [-1, 1].
         */
        minifloat::Repr_t mvaId;
    };
}
