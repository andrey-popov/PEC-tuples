#include <UserCode/SingleTop/interface/Jet.h>

#include <stdexcept>


using namespace pec;


Jet::Jet():
    CandidateWithID(),
    bTagCSV(0), secVertexMass(0),
    area(0),
    charge(0),
    pullAngle(0),
    flavourAlgorithmic(0), flavourPhysics(0)
{}


Jet::Jet(Jet const &src):
    CandidateWithID(src),
    bTagCSV(src.bTagCSV), secVertexMass(src.secVertexMass),
    area(src.area),
    charge(src.charge),
    pullAngle(src.pullAngle),
    flavourAlgorithmic(src.flavourAlgorithmic), flavourPhysics(src.flavourPhysics)
{}


Jet &Jet::operator=(Jet const &src)
{
    CandidateWithID::operator=(src);
    
    bTagCSV = src.bTagCSV;
    secVertexMass = src.secVertexMass;
    area = src.area;
    charge = src.charge;
    pullAngle = src.pullAngle;
    flavourAlgorithmic = src.flavourAlgorithmic;
    flavourPhysics = src.flavourPhysics;
    
    return *this;
}


void Jet::Reset()
{
    CandidateWithID::Reset();
    
    bTagCSV = 0;
    secVertexMass = 0;
    area = 0;
    charge = 0;
    pullAngle = 0;
    flavourAlgorithmic = 0;
    flavourPhysics = 0;
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


void Jet::SetFlavour(FlavourType type, int flavour)
{
    switch (type)
    {
        case FlavourType::Algorithmic:
            flavourAlgorithmic = flavour;
            break;
        
        case FlavourType::Physics:
            flavourPhysics = flavour;
            break;
        
        default:
            throw std::runtime_error("Jet::SetFlavour: Requested flavour type is not supported.");
    }
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


int Jet::Flavour(FlavourType type) const
{
    switch (type)
    {
        case FlavourType::Algorithmic:
            return flavourAlgorithmic;
        
        case FlavourType::Physics:
            return flavourPhysics;
        
        default:
            throw std::runtime_error("Jet::SetFlavour: Requested flavour type is not supported.");
    }
}
