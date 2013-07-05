#include "Manager.hpp"

#include <iostream>


using namespace std;


Manager::Manager(string const &outDirectory_)
{
    SetOutputDirectory(outDirectory_);
}


void Manager::AddWeightAlgo(WeightAlgo const *algo)
{
    weightAlgos.push_back(algo);
}


void Manager::AddSample(string const &fileName)
{
    samples.emplace_back(fileName);
}


void Manager::SetOutputDirectory(string const &outDirectory_)
{
    outDirectory = outDirectory_;
    unsigned const length = outDirectory.length();
    
    if (length > 0 and outDirectory[length - 1] != '/')
        outDirectory += '/';
}


void Manager::Process()
{
    // Loop over the samples
    for (auto &sample: samples)
    {
        cout << "Processing file \"" << sample.GetFileName() << '\"' << endl;
        
        // Open the current source PEC file for reading
        sample.Open();
        
        // Create a new file to write the weights
        string const outFileName(outDirectory + sample.GetShortName() + "_weights.root");
        TFile outFile(outFileName.c_str(), "recreate");
        
        // Create new trees for each reweigting class
        for (auto const &algo: weightAlgos)
            algo->NewTree();
        
        
        // Loop over the events in the sample
        while (sample.NextEvent())
            for (auto const &algo: weightAlgos)
                algo->FillWeight(sample);
        
        
        // Write the trees with weights
        outFile.cd();
        
        for (auto const &algo: weightAlgos)
            algo->WriteTree();
        
        outFile.Close();
    }
}