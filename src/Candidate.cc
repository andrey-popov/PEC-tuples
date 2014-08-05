#include <UserCode/SingleTop/interface/Candidate.h>
#include <UserCode/SingleTop/interface/minifloats.h>

using namespace pec;


Candidate::Candidate():
    pt(0.), eta(0.), phi(0), mass(0.)
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


void Candidate::Reset()
{
    pt = 0;
    eta = 0;
    phi = 0;
    mass = 0;
}


void Candidate::SetPt(double pt_)
{
    pt = pt_;
}


void Candidate::SetEta(double eta_)
{
    eta = eta_;
}


void Candidate::SetPhi(double phi_)
{
    phi = minifloat::encodeAngle(phi_);
}


void Candidate::SetM(double mass_)
{
    mass = mass_;
}


double Candidate::Pt() const
{
    return pt;
}


double Candidate::Eta() const
{
    return eta;
}


double Candidate::Phi() const
{
    return minifloat::decodeAngle(phi);
}


double Candidate::M() const
{
    return mass;
}
