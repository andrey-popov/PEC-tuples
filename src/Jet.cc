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
    bTagCSV = minifloat::encodeGeneric<true, 12, 1>(bTag);
}


void Jet::SetSecVertexMass(double mass)
{
    // Usually, the mass is set to a negative value when there is no secondary vertex associated
    //with a jet. Do not to waste one bit for the sign, so reset the mass to zero in such cases
    if (mass < 0.)
        mass = 0.;
    
    secVertexMass = minifloat::encodeGeneric<false, 12, 2>(mass);
}


void Jet::SetArea(double area_)
{
    area = minifloat::encodeGeneric<false, 14, 0>(area_);
}


void Jet::SetCharge(double charge_)
{
    charge = minifloat::encodeRange(-1., 1., charge_);
}


void Jet::SetPullAngle(double angle)
{
    pullAngle = minifloat::encodeAngle(angle);
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
    return minifloat::decodeGeneric<true, 12, 1>(bTagCSV);
}


double Jet::SecVertexMass() const
{
    return minifloat::decodeGeneric<false, 12, 2>(secVertexMass);
}


double Jet::Area() const
{
    return minifloat::decodeGeneric<false, 14, 0>(area);
}


double Jet::Charge() const
{
    return minifloat::decodeRange(-1., 1., charge);
}


double Jet::PullAngle() const
{
    return minifloat::decodeAngle(pullAngle);
}


int Jet::Flavour() const
{
    return flavour;
}
