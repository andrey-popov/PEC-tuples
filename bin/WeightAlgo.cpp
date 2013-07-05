#include "WeightAlgo.hpp"

#include <stdexcept>


using namespace std;


WeightAlgo::WeightAlgo(string const &title_, unsigned nVars_ /*= 0*/):
    title(title_), nVars(nVars_),
    tree(nullptr),
    up(nullptr), down(nullptr)
{
    up = new Float_t[nVars];
    down = new Float_t[nVars];
}


WeightAlgo::~WeightAlgo()
{
    delete [] up;
    delete [] down;
}


void WeightAlgo::NewTree() const
{
    if (tree not_eq nullptr)
        throw runtime_error("WeightAlgo::NewTree: Creating a new tree when the previous one has "
         "not been written properly.");
    
    // Create a tree to store the event weights (along with their variations)
    tree = new TTree(title.c_str(), "");
    
    tree->Branch("central", &central);
    tree->Branch("nVars", &nVars);
    tree->Branch("up", up, "up[nVars]/F");
    tree->Branch("down", down, "down[nVars]/F");
    
    
    // Perform the customization of the tree
    CustomNewTree();
}


void WeightAlgo::CustomNewTree() const
{}


void WeightAlgo::WriteTree() const
{
    tree->Write(0, TObject::kOverwrite);
    delete tree;
    tree = nullptr;
}


string const &WeightAlgo::GetTitle() const
{
    return title;
}
