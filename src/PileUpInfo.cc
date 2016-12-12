#include <Analysis/PECTuples/interface/PileUpInfo.h>


pec::PileUpInfo::PileUpInfo() noexcept:
    numPV(0),
    rho(0), rhoCentral(0),
    trueNumPU(0),
    inTimeNumPU(0)
{}


void pec::PileUpInfo::Reset()
{
    numPV = 0;
    rho = 0;
    rhoCentral = 0;
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


void pec::PileUpInfo::SetRhoCentral(float rho_)
{
    rhoCentral = rho_;
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


float pec::PileUpInfo::RhoCentral() const
{
    return rhoCentral;
}


float pec::PileUpInfo::TrueNumPU() const
{
    return trueNumPU;
}


unsigned pec::PileUpInfo::InTimePU() const
{
    return inTimeNumPU;
}
