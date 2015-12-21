#include <Analysis/PECTuples/interface/Jet.h>

#include <stdexcept>


using namespace pec;


Jet::Jet():
    CandidateWithID(),
    jecFactor(0), jecUncertainty(0),
    bTagCSV(0), secVertexMass(0),
    area(0),
    charge(0),
    pullAngle(0),
    flavour(0)
{}


Jet::Jet(Jet const &src):
    CandidateWithID(src),
    jecFactor(src.jecFactor), jecUncertainty(src.jecUncertainty),
    bTagCSV(src.bTagCSV), secVertexMass(src.secVertexMass),
    area(src.area),
    charge(src.charge),
    pullAngle(src.pullAngle),
    flavour(src.flavour)
{}


Jet &Jet::operator=(Jet const &src)
{
    CandidateWithID::operator=(src);
    
    jecFactor = src.jecFactor;
    jecUncertainty = src.jecUncertainty;
    bTagCSV = src.bTagCSV;
    secVertexMass = src.secVertexMass;
    area = src.area;
    charge = src.charge;
    pullAngle = src.pullAngle;
    flavour = src.flavour;
    
    return *this;
}


void Jet::Reset()
{
    CandidateWithID::Reset();
    
    jecFactor = 0;
    jecUncertainty = 0;
    bTagCSV = 0;
    secVertexMass = 0;
    area = 0;
    charge = 0;
    pullAngle = 0;
    flavour = 0;
}


void Jet::SetJECFactor(double jecFactor_)
{
    jecFactor = jecFactor_;
}


void Jet::SetJECUncertainty(double jecUncertainty_)
{
    jecUncertainty = jecUncertainty_;
}


void Jet::SetBTagCSV(double bTag)
{
    bTagCSV = bTag;
}


void Jet::SetSecVertexMass(double mass)
{
    // Usually, the mass is set to a negative value when there is no secondary vertex associated
    //with a jet. Do not to waste one bit for the sign, so reset the mass to zero in such cases
    if (mass < 0.)
        mass = 0.;
    
    secVertexMass = mass;
}


void Jet::SetArea(double area_)
{
    area = area_;
}


void Jet::SetCharge(double charge_)
{
    charge = charge_;
}


void Jet::SetPullAngle(double angle)
{
    pullAngle = angle;
}


void Jet::SetFlavour(int flavour_)
{
    flavour = flavour_;
}


double Jet::JECFactor() const
{
    return jecFactor;
}


double Jet::JECUncertainty() const
{
    return jecUncertainty;
}


double Jet::BTagCSV() const
{
    return bTagCSV;
}


double Jet::SecVertexMass() const
{
    return secVertexMass;
}


double Jet::Area() const
{
    return area;
}


double Jet::Charge() const
{
    return charge;
}


double Jet::PullAngle() const
{
    return pullAngle;
}


int Jet::Flavour() const
{
    return flavour;
}
