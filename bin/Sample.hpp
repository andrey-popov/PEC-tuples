/**
 * \file WeightSample.h
 * \author Andrey Popov
 * 
 * The class provides the interface for the reweighting algorithms to access the event content.
 */
 
#pragma once

#include "WeightAlgo.hpp"

#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>


/**
 * \class Sample
 * \brief A wrapper class to provide an access to PEC files
 */
class Sample
{
    public:
        /// Constuctor
        Sample(std::string const &fileName_);
        
        /// Destructor
        ~Sample();
    
    public:
        /// Open the source file for reading
        void Open();
        
        /**
         * \brief Reads the next event in the source file
         * 
         * Returns true if the event has been read successfully and false if there are no more
         * events.
         */
        bool NextEvent() const;
        
        /// Returns the basename of the file without the file extension
        std::string GetShortName() const;
        
        /// Returns the fully-qualified file name
        std::string const &GetFileName() const;
        
        
        /// Returns the "true" number of pile-up interactions
        Float_t GetTrueNumPUInteractions() const;
    
    private:
        std::string fileName;  ///< Fully-qualified source file's name
        TFile *srcFile;  ///< Pointer to the source file
        unsigned long nEvents;  ///< Total number of events in the tree
        mutable unsigned long curEvent;  ///< Index of the current event in the tree
        mutable TTree *srcTree;  ///< Ponter to (an aggregate of) the source trees
        
        // Buffers to read the branches
        mutable Float_t PUTrueNumInteractions;
};
