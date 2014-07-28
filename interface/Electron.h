#pragma once

#include <UserCode/SingleTop/interface/Lepton.h>


namespace pec
{
    /**
     * \class Electron
     * \author Andrey Popov
     * \brief Represents a reconstructed electron
     * 
     * 
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
        /**
         * \brief Sets the bit mask for a cut-based ID
         * 
         * Expected to be used with legacy "simple cut-based" ID [1], 70% w. p. with combined
         * isolation. The value is a mask that encodes results of several components of the ID.
         * [1] https://twiki.cern.ch/twiki/bin/view/CMS/SimpleCutBasedEleID
         */
        void SetCutBasedID(unsigned mask);
        
        /**
         * \brief Sets the value of MVA-based ID
         * 
         * Expected to be used with the triggering MVA ID documented in [1].
         * [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/MultivariateElectronIdentification
         */
        void SetMvaID(double mva);
        
        /**
         * \brief Returns the bit mask with results of the cut-based ID
         * 
         * See documentation for the SetCutBasedID method.
         */
        UChar_t CutBasedID() const;
        
        /**
         * \brief Returns the value of MVA-based ID
         * 
         * See documentation for the SetMvaID method.
         */
        double MvaID() const;
        
    private:
        /**
         * \brief Legacy cut-based ID
         * 
         * Set documentation for the SetCutBasedID method.
         */
        UChar_t cutBasedID;
        
        /**
         * \brief MVA-based ID
         * 
         * See documentation for the SetMvaID method.
         */
        Float_t mvaID;
    };
}
