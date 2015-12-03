#include <Analysis/PECTuples/interface/Candidate.h>

using namespace pec;


Candidate::Candidate():
    pt(0), eta(0), phi(0), mass(0)
{}


Candidate::Candidate(Candidate const &src):
    pt(src.pt), eta(src.eta), phi(src.phi), mass(src.mass)
{}


Candidate &Candidate::operator=(Candidate const &src)
{
    pt = src.pt;
    eta = src.eta;
    phi = src.phi;
    mass = src.mass;
    
    return *this;
}


Candidate::~Candidate()
{}


void Candidate::Reset()
{
    pt = 0;
    eta = 0;
    phi = 0;
    mass = 0;
}


void Candidate::SetPt(double pt_)
{
    pt = minifloat::encodeGeneric<false, 12, 1>(pt_);
}


void Candidate::SetEta(double eta_)
{
    eta = minifloat::encodeGeneric<true, 13, -1>(eta_);
}


void Candidate::SetPhi(double phi_)
{
    phi = minifloat::encodeAngle(phi_);
}


void Candidate::SetM(double mass_)
{
    mass = minifloat::encodeGeneric<false, 12, 2>(mass_);
}


double Candidate::Pt() const
{
    return minifloat::decodeGeneric<false, 12, 1>(pt);
}


double Candidate::Eta() const
{
    return minifloat::decodeGeneric<true, 13, -1>(eta);
}


double Candidate::Phi() const
{
    return minifloat::decodeAngle(phi);
}


double Candidate::M() const
{
    return minifloat::decodeGeneric<false, 12, 2>(mass);
}
