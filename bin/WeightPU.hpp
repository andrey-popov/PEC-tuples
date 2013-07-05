/**
 * \file WeightPU.hpp
 * \author Andrey Popov
 * 
 * The module defines a class to perform reweigthing on pile-up.
 */
 
#pragma once

#include "WeightAlgo.hpp"

#include <PhysicsTools/Utilities/interface/LumiReWeighting.h>

#include <string>


/**
 * \class WeightPU
 * \brief Performs reweighting over the number of pile-up interactions
 *
 * The so-called reweighting based on the "true" number of PU interactions is performed. The
 * CMS-wide recommendations are described in [1-2].
 * [1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupMCReweightingUtilities
 * [2] https://twiki.cern.ch/twiki/bin/view/CMS/PileupSystematicErrors
 */
class WeightPU: public WeightAlgo
{
    public:
        /**
         * \brief Constructor
         * 
         * \param title The identification string described in the base class.
         * \param dataPUFileName Name of a file containing the target PU distribution.
         * \param systError The value of the systematical error.
         */
        WeightPU(std::string const &title, std::string const &dataPUFileName, float systError);
        
        /// Destructor
        ~WeightPU();
    
    public:
        /// Calculates the weights for the current event
        void FillWeight(Sample const &sample) const;
    
    private:
        /// Objects to perform the reweighting
        edm::LumiReWeighting *lumiReweight, *lumiReweightUp, *lumiReweightDown;
};
