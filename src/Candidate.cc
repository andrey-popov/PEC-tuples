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


void Candidate::SetPt(float pt_)
{
    pt = pt_;
}


void Candidate::SetEta(float eta_)
{
    eta = eta_;
}


void Candidate::SetPhi(float phi_)
{
    phi = phi_;
}


void Candidate::SetM(float mass_)
{
    mass = mass_;
}


float Candidate::Pt() const
{
    return pt;
}


float Candidate::Eta() const
{
    return eta;
}


float Candidate::Phi() const
{
    return phi;
}


float Candidate::M() const
{
    return mass;
}
