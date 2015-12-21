#include <Analysis/PECTuples/interface/PileUpInfo.h>


pec::PileUpInfo::PileUpInfo():
    numPV(0),
    rho(0),
    trueNumPU(0),
    inTimeNumPU(0)
{}


pec::PileUpInfo::PileUpInfo(PileUpInfo const &src):
    numPV(src.numPV),
    rho(src.rho),
    trueNumPU(src.trueNumPU),
    inTimeNumPU(src.inTimeNumPU)
{}


pec::PileUpInfo &pec::PileUpInfo::operator=(PileUpInfo const &src)
{
    numPV = src.numPV;
    rho = src.rho;
    trueNumPU = src.trueNumPU;
    inTimeNumPU = src.inTimeNumPU;
    
    return *this;
}


void pec::PileUpInfo::Reset()
{
    numPV = 0;
    rho = 0;
    trueNumPU = 0;
    inTimeNumPU = 0;
}


void pec::PileUpInfo::SetNumPV(unsigned numPV_)
{
    numPV = numPV_;
}


void pec::PileUpInfo::SetRho(float rho_)
{
    rho = rho_;
}


void pec::PileUpInfo::SetTrueNumPU(float lambda)
{
    trueNumPU = lambda;
}


void pec::PileUpInfo::SetInTimePU(unsigned n)
{
    inTimeNumPU = n;
}


unsigned pec::PileUpInfo::NumPV() const
{
    return numPV;
}


float pec::PileUpInfo::Rho() const
{
    return rho;
}


float pec::PileUpInfo::TrueNumPU() const
{
    return trueNumPU;
}


unsigned pec::PileUpInfo::InTimePU() const
{
    return inTimeNumPU;
}
