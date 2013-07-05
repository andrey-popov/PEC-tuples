/**
 * \file WeightAlgo.hpp
 * \author Andrey Popov
 * 
 * The module defines the base class for all the reweighting classes.
 */

#pragma once

#include <string>

#include <TTree.h>


/// Forward declaration of a class to provide an interface to PEC files
class Sample;


/**
 * \class WeightAlgo
 * \brief The abstract base class for all the reweighting classes
 * 
 * The class creates, sets up, and releases the tree that stores the event weights. By default the
 * tree contains a central weight and an arbitrary number of up/down variations, but a derived class
 * can also add additional branches with the help of overrided method PostAssignSample.
 */
class WeightAlgo
{
    public:
        /**
         * \brief Contructor
         * 
         * The first argument is a string label, which will identify the created set of weights in a
         * unique way. Technically, it will be the name of the created tree. The second argument
         * specifies the number of the systematical variations of the event weight.
         */
        WeightAlgo(std::string const &title_, unsigned nVars_ = 0);
        
        /// Destructor
        virtual ~WeightAlgo();
    
    public:
        /**
         * \brief Creates a new tree to store the weights
         * 
         * Creates a new tree to store the weights and adds branches to it. Afterwards method
         * CustomNewTree is called.
         */
        void NewTree() const;
        
        /// Writes the tree with the weights to current file
        void WriteTree() const;
        
        /// Calculates the weights for the current event
        virtual void FillWeight(Sample const &) const = 0;
        
        /// Returns the title of the instance of the reweighting class
        std::string const &GetTitle() const;
    
    protected:
        /**
         * \brief Additional actions to be performed when a new tree is created
         * 
         * The method is called from NewTree and is intended to add new branches to the tree if
         * needed. By default, the method does nothing.
         */
        virtual void CustomNewTree() const;

    
    private:
        /// String label to identify the instance of the reweighting class
        std::string const title;
        
        /// The number of the systematical up/down variations
        mutable Int_t nVars;
        //^ Was forces to make it mutable because TTree::Branch accepts non-const pointer only =-!
    
    protected:
        /// The tree to store the event weights
        mutable TTree *tree;
        
        /// The nominal (central) weight of the current event
        mutable Float_t central;
        
        /// The event weights for the up and down variations
        Float_t *up, *down;
};
