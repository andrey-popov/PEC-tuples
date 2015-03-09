#include <UserCode/SingleTop/interface/PileUpInfo.h>


using namespace pec;


PileUpInfo::PileUpInfo():
    numPV(0),
    rho(0),
    trueNumPU(0),
    inTimeNumPU(0)
{}


PileUpInfo::PileUpInfo(PileUpInfo const &src):
    numPV(src.numPV),
    rho(src.rho),
    trueNumPU(src.trueNumPU),
    inTimeNumPU(src.inTimeNumPU)
{}


PileUpInfo &PileUpInfo::operator=(PileUpInfo const &src)
{
    numPV = src.numPV;
    rho = src.rho;
    trueNumPU = src.trueNumPU;
    inTimeNumPU = src.inTimeNumPU;
    
    return *this;
}


void PileUpInfo::Reset()
{
    numPV = 0;
    rho = 0;
    trueNumPU = 0;
    inTimeNumPU = 0;
}


void PileUpInfo::SetNumPV(unsigned numPV_)
{
    numPV = numPV_;
}


void PileUpInfo::SetRho(double rho_)
{
    rho = minifloat::encodeGeneric<false, 13, 1>(rho_);
}


void PileUpInfo::SetTrueNumPU(double lambda)
{
    trueNumPU = minifloat::encodeGeneric<false, 13, -1>(lambda);
}


void PileUpInfo::SetInTimePU(unsigned n)
{
    inTimeNumPU = n;
}


unsigned PileUpInfo::NumPV() const
{
    return numPV;
}


double PileUpInfo::Rho() const
{
    return minifloat::decodeGeneric<false, 13, 1>(rho);
}


double PileUpInfo::TrueNumPU() const
{
    return minifloat::decodeGeneric<false, 13, -1>(trueNumPU);
}


unsigned PileUpInfo::InTimePU() const
{
    return inTimeNumPU;
}
