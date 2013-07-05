/**
 * \file Manager.hpp
 * \author Andrey Popov
 * 
 * Module defines a class to manage calculation of event weights for a given list of PEC files.
 */

#pragma once

#include "WeightAlgo.hpp"
#include "Sample.hpp"

#include <string>
#include <list>


/**
 * \class Manager
 * \brief Calculates requested weights for a set of PEC files and stores them in additional ROOT
 * files
 */
class Manager
{
    public:
        /**
         * \brief Constructor
         * 
         * See documentation for SetOutputDirectory for a description of the argument.
         */
        Manager(std::string const &outDirectory_);
    
    public:
        /// Adds an instance of a reweighting class
        void AddWeightAlgo(WeightAlgo const *algo);
        
        /// Adds a new source file
        void AddSample(std::string const &fileName);
        
        /// Specifies the directory to store produced files
        void SetOutputDirectory(std::string const &outDirectory_);
        
        /**
         * \brief Calculates the requested weights for all the source files
         * 
         * For each source file creates a complementary one in outDirectory directory. The name is
         * the same but with a postfix "_weights.root". The file contains a tree with weights for
         * each of the provided reweigthing objects.
         */
        void Process();
    
    private:
        /// List of instances of reweighting classes
        std::list<WeightAlgo const *> weightAlgos;
        /// List of source sample. They are managed by the class
        std::list<Sample> samples;
        std::string outDirectory;  ///< Directory to store produced files
};