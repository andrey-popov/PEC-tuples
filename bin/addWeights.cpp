/**
 * \file addWeights.cpp
 * \author Andrey Popov
 * 
 * A program that creates auxiliary files with event weights.
 */
 
#include "Manager.hpp"
#include "WeightPU.hpp"

#include <string>


using namespace std;


int main(int argc, char **argv)
{
    // Define the reweighting methods that will be evaluated
    WeightPU weightPU("PileUp_SingleMu_Run2012", "/afs/cern.ch/work/a/aapopov/public/tHq/2012Alpha/"
     "2013.04.14_PileUp/SingleMu2012ABCD_Alpha-v2_pixelLumi.pileupTruth_finebin.root", 0.05);
    
    
    // Location of the PEC files
    string const filePath("/afs/cern.ch/user/a/aapopov/workspace/data/2012Alpha/");
    
    
    // Define the manager
    Manager man("/afs/cern.ch/user/a/aapopov/workspace/data/2012Alpha/");
    man.AddWeightAlgo(&weightPU);
    
    // Add the files to the manager
    man.AddSample(filePath + "tqh-nc-mg_rev468_fPc.root");
    man.AddSample(filePath + "ttbar-mg_rev468_xYe.root");
    man.AddSample(filePath + "ttbar-semilep-mg_rev468_SQQ.root");
    man.AddSample(filePath + "ttbar-dilep-mg_rev468_Ple.root");
    man.AddSample(filePath + "t-tchan-pw_rev468_QJd.root");
    man.AddSample(filePath + "tbar-tchan-pw_rev468_koy.root");
    man.AddSample(filePath + "tth_rev468_swr.root");
    man.AddSample(filePath + "Wjets-mg_rev468_tkF.root");
    man.AddSample(filePath + "Wjets-2p-mg_rev468.root");
    man.AddSample(filePath + "Wjets-3p-mg_rev468.root");
    man.AddSample(filePath + "Wjets-4p-mg_rev468.root");
    
    // Run the manager
    man.Process();
    
    
    return 0;
}
