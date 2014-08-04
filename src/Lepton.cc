#include <UserCode/SingleTop/interface/Lepton.h>

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
    relIso = relIso_;
}


void Lepton::SetDB(double dB_)
{
    dB = dB_;
}


int Lepton::Charge() const
{
    return ((charge) ? -1 : 1);
}


double Lepton::RelIso() const
{
    return relIso;
}


double Lepton::DB() const
{
    return dB;
}
