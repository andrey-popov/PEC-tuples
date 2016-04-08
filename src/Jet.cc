#include <Analysis/PECTuples/interface/Jet.h>

#include <stdexcept>


pec::Jet::Jet():
    CandidateWithID(),
    jecFactor(0), jecUncertainty(0),
    bTagCMVA(0), bTagCSV(0), secVertexMass(0),
    pileUpMVA(0),
    area(0),
    charge(0),
    pullAngle(0),
    flavour(0)
{}


void pec::Jet::Reset()
{
    CandidateWithID::Reset();
    
    jecFactor = 0;
    jecUncertainty = 0;
    bTagCMVA = 0;
    bTagCSV = 0;
    secVertexMass = 0;
    pileUpMVA = 0;
    area = 0;
    charge = 0;
    pullAngle = 0;
    flavour = 0;
}


void pec::Jet::SetJECFactor(float jecFactor_)
{
    jecFactor = jecFactor_;
}


void pec::Jet::SetJECUncertainty(float jecUncertainty_)
{
    jecUncertainty = jecUncertainty_;
}


void pec::Jet::SetBTagCMVA(float bTag)
{
    bTagCMVA = bTag;
}


void pec::Jet::SetBTagCSV(float bTag)
{
    bTagCSV = bTag;
}


void pec::Jet::SetSecVertexMass(float mass)
{
    // Usually, the mass is set to a negative value when there is no secondary vertex associated
    //with a jet. Do not to waste one bit for the sign, so reset the mass to zero in such cases
    if (mass < 0.)
        mass = 0.;
    
    secVertexMass = mass;
}


void pec::Jet::SetPileUpID(float pileUpMVA_)
{
    pileUpMVA = pileUpMVA_;
}


void pec::Jet::SetArea(float area_)
{
    area = area_;
}


void pec::Jet::SetCharge(float charge_)
{
    charge = charge_;
}


void pec::Jet::SetPullAngle(float angle)
{
    pullAngle = angle;
}


void pec::Jet::SetFlavour(int flavour_)
{
    flavour = flavour_;
}


float pec::Jet::JECFactor() const
{
    return jecFactor;
}


float pec::Jet::JECUncertainty() const
{
    return jecUncertainty;
}


float pec::Jet::BTagCMVA() const
{
    return bTagCMVA;
}


float pec::Jet::BTagCSV() const
{
    return bTagCSV;
}


float pec::Jet::SecVertexMass() const
{
    return secVertexMass;
}


float pec::Jet::PileUpID() const
{
    return pileUpMVA;
}


float pec::Jet::Area() const
{
    return area;
}


float pec::Jet::Charge() const
{
    return charge;
}


float pec::Jet::PullAngle() const
{
    return pullAngle;
}


int pec::Jet::Flavour() const
{
    return flavour;
}
