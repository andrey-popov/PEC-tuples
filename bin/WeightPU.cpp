#include "WeightPU.hpp"
#include "Sample.hpp"

#include <TH1.h>

#include <vector>
 

using namespace std;


WeightPU::WeightPU(string const &title, string const &dataPUFileName, float systError):
    WeightAlgo(title, 1)
{
    // Declare vectors that will define distrubutions over the number of PU interactions
    vector<float> dataDistr, dataDistrUp, dataDistrDown, MCDistr;
    
    
    // Get the data PU histogram (with fine binning)
    TFile dataPUFile(dataPUFileName.c_str());
    TH1 *hist = dynamic_cast<TH1 *>(dataPUFile.Get("pileup"));
    
    // The maximal number of PU events
    unsigned const maxDataPUEvents =
     ceil((1. + systError) * hist->GetBinLowEdge(hist->GetNbinsX() + 1));
    dataDistr.resize(maxDataPUEvents, 0.);
    dataDistrUp.resize(maxDataPUEvents, 0.);
    dataDistrDown.resize(maxDataPUEvents, 0.);
    
    // Fill the data vectors. The data PU histogram is treated as a continuous function
    for (int bin = 1; bin <= hist->GetNbinsX(); ++bin)
    {
        double const binCenter = hist->GetBinCenter(bin);
        double const binContent = hist->GetBinContent(bin);
        
        dataDistr[floor(binCenter)] += binContent;
        dataDistrUp[floor(binCenter * (1. + systError))] += binContent;
        dataDistrDown[floor(binCenter * (1. - systError))] += binContent;
    }
    
    delete hist;
    dataPUFile.Close();
    
    
    // Make the vector for MC PU distribution
    unsigned const nMCBins = 60;
    MCDistr.resize(nMCBins, 0.);
    
    // MC distribution for Summer2012, S10
    //[1] https://twiki.cern.ch/twiki/bin/view/CMS/Pileup_MC_Gen_Scenarios
    double Summer2012Truth[nMCBins] = {2.560E-06, 5.239E-06, 1.420E-05, 5.005E-05, 1.001E-04,
     2.705E-04, 1.999E-03, 6.097E-03, 1.046E-02, 1.383E-02, 1.685E-02, 2.055E-02, 2.572E-02,
     3.262E-02, 4.121E-02, 4.977E-02, 5.539E-02, 5.725E-02, 5.607E-02, 5.312E-02, 5.008E-02,
     4.763E-02, 4.558E-02, 4.363E-02, 4.159E-02, 3.933E-02, 3.681E-02, 3.406E-02, 3.116E-02,
     2.818E-02, 2.519E-02, 2.226E-02, 1.946E-02, 1.682E-02, 1.437E-02, 1.215E-02, 1.016E-02,
     8.400E-03, 6.873E-03, 5.564E-03, 4.457E-03, 3.533E-03, 2.772E-03, 2.154E-03, 1.656E-03,
     1.261E-03, 9.513E-04, 7.107E-04, 5.259E-04, 3.856E-04, 2.801E-04, 2.017E-04, 1.439E-04,
     1.017E-04, 7.126E-05, 4.948E-05, 3.405E-05, 2.322E-05, 1.570E-05, 5.005E-06};
    
    for (unsigned i = 0; i < nMCBins; ++i)
        MCDistr[i] = Summer2012Truth[i];
    
    
    // Make sure the vectors agree in size
    unsigned const maxSize = max(maxDataPUEvents, nMCBins);
    dataDistr.resize(maxSize, 0.);
    dataDistrUp.resize(maxSize, 0.);
    dataDistrDown.resize(maxSize, 0.);
    MCDistr.resize(maxSize, 0.);
    
    
    // Create the reweighting objects [1]
    //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupMCReweightingUtilities
    lumiReweight = new edm::LumiReWeighting(MCDistr, dataDistr);
    lumiReweightUp = new edm::LumiReWeighting(MCDistr, dataDistrUp);
    lumiReweightDown = new edm::LumiReWeighting(MCDistr, dataDistrDown);
}


WeightPU::~WeightPU()
{
    delete lumiReweight;
    delete lumiReweightUp;
    delete lumiReweightDown;
}


void WeightPU::FillWeight(Sample const &sample) const
{
    // The true number of pile-up interactions (i.e. the Poisson parameter used in simulation)
    float const n = sample.GetTrueNumPUInteractions();
    
    central = lumiReweight->weight(n);
    up[0] = lumiReweightUp->weight(n);
    down[0] = lumiReweightDown->weight(n);
    
    tree->Fill();
}
