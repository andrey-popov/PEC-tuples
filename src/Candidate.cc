#include <Analysis/PECTuples/interface/Candidate.h>


pec::Candidate::Candidate():
    pt(0), eta(0), phi(0), mass(0)
{}


pec::Candidate::~Candidate()
{}


void pec::Candidate::Reset()
{
    pt = 0;
    eta = 0;
    phi = 0;
    mass = 0;
}


void pec::Candidate::SetPt(float pt_)
{
    pt = pt_;
}


void pec::Candidate::SetEta(float eta_)
{
    eta = eta_;
}


void pec::Candidate::SetPhi(float phi_)
{
    phi = phi_;
}


void pec::Candidate::SetM(float mass_)
{
    mass = mass_;
}


float pec::Candidate::Pt() const
{
    return pt;
}


float pec::Candidate::Eta() const
{
    return eta;
}


float pec::Candidate::Phi() const
{
    return phi;
}


float pec::Candidate::M() const
{
    return mass;
}
