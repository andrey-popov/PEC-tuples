#pragma once

#include <UserCode/SingleTop/interface/Candidate.h>


namespace pec
{
    /**
     * \class ShowerParton
     * \author Andrey Popov
     * \brief Represents a parton from parton shower
     * 
     * 
     */
    class ShowerParton: public Candidate
    {
    public:
        /// Specifies the origin of the parton
        enum Origin
        {
            Undefined = 0,
            ISR = 1,    /// initial-state radiation
            FSR = 2,    /// final-state radiation
            Proton = 3  /// an immediate daughter of one of the initial beam particles
        };
        
    public:
        /// Constructor with no parameters
        ShowerParton();
        
        /// Copy constructor
        ShowerParton(ShowerParton const &src);
        
        /// Assignment operator
        ShowerParton &operator=(ShowerParton const &src);
        
    public:
        /// Resets the object to a state right after the default initialisation
        virtual void Reset();
        
        /**
         * \brief Sets PDG ID
         * 
         * If the given ID cannot be stored in Char_t, an exception is thrown.
         */
        void SetPdgId(int pdgId);
        
        /**
         * \brief Sets origin of the parton
         * 
         * See documentation for the Origin enumeration.
         */
        void SetOrigin(Origin origin);
        
        /// Returns PDG ID
        int PdgId() const;
        
        /**
         * \brief Returns origin of the parton
         * 
         * See documentation for the Origin enumeration.
         */
        Origin GetOrigin() const;
        
    private:
        /// PDG ID
        Char_t pdgId;
        
        /**
         * \brief Encodes origin of the parton
         * 
         * The variable takes values specified in the Origin enumeration.
         */
        UChar_t origin;
    };
}
