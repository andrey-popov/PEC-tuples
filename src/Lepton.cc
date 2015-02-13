#include <UserCode/SingleTop/interface/Lepton.h>
#include <UserCode/SingleTop/interface/minifloats.h>

#include <stdexcept>


using namespace pec;


Lepton::Lepton():
    CandidateWithID(),
    charge(false), relIso(0), dB(0)
{}


Lepton::Lepton(Lepton const &src):
    CandidateWithID(src),
    charge(src.charge), relIso(src.relIso), dB(src.dB)
{}


Lepton &Lepton::operator=(Lepton const &src)
{
    CandidateWithID::operator=(src);
    
    charge = src.charge;
    relIso = src.relIso;
    dB = src.dB;
    
    return *this;
}


Lepton::~Lepton()
{}


void Lepton::Reset()
{
    CandidateWithID::Reset();
    
    charge = false;
    relIso = 0;
    dB = 0;
}


void Lepton::SetCharge(int charge_)
{
    if (charge_ == 0)
        throw std::logic_error("Lepton::SetCharge: The class is meant for charged leptons only.");
    
    charge = (charge_ < 0);
}


void Lepton::SetRelIso(double relIso_)
{
    relIso = minifloat::encodeGeneric<false, 13, 1>(relIso_);
}


void Lepton::SetDB(double dB_)
{
    // The impact parameter is encoded as an unsigned number. Make sure it is not negate
    dB = minifloat::encodeGeneric<false, 13, 1>(fabs(dB_));
}


int Lepton::Charge() const
{
    return ((charge) ? -1 : 1);
}


double Lepton::RelIso() const
{
    return minifloat::decodeGeneric<false, 13, 1>(relIso);
}


double Lepton::DB() const
{
    return minifloat::decodeGeneric<false, 13, 1>(dB);
}
