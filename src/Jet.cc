#include <Analysis/PECTuples/interface/Jet.h>

#include <cstdlib>
#include <stdexcept>


pec::Jet::Jet() noexcept:
    CandidateWithID(),
    corrFactor(0), jecUncertainty(0), jerUncertainty(0),
    bTags{0, 0}, cTags{0, 0},
    pileUpMVA(0),
    area(0),
    charge(0),
    pullAngle(0),
    flavours(0)
{}


void pec::Jet::Reset()
{
    CandidateWithID::Reset();
    
    corrFactor = 0;
    jecUncertainty = jerUncertainty = 0;
    bTags[0] = bTags[1] = cTags[0] = cTags[1] = 0;
    pileUpMVA = 0;
    area = 0;
    charge = 0;
    pullAngle = 0;
    flavours = 0;
}


void pec::Jet::SetCorrFactor(float corrFactor_)
{
    corrFactor = corrFactor_;
}


void pec::Jet::SetJECUncertainty(float jecUncertainty_)
{
    jecUncertainty = jecUncertainty_;
}


void pec::Jet::SetJERUncertainty(float jerUncertainty_)
{
    jerUncertainty = jerUncertainty_;
}


void pec::Jet::SetBTag(BTagAlgo algo, float value)
{
    bTags[unsigned(algo)] = value;
}


void pec::Jet::SetCTag(CTagAlgo algo, float value)
{
    cTags[unsigned(algo)] = value;
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


void pec::Jet::SetFlavour(int hadronFlavour, int partonFlavour /*= 0*/, int meFlavour /*= 0*/)
{
    if ((std::abs(hadronFlavour) > 5 and hadronFlavour != 21) or
      (std::abs(partonFlavour) > 5 and partonFlavour != 21) or
      (std::abs(meFlavour) > 5 and meFlavour != 21))
        throw std::runtime_error("Jet::SetFlavour: Illegal value for jet flavour is given.");
    
    
    unsigned const hadronFlavourEncoded = (hadronFlavour == 21) ? 0 : hadronFlavour + 6;
    unsigned const partonFlavourEncoded = (partonFlavour == 21) ? 0 : partonFlavour + 6;
    unsigned const meFlavourEncoded = (meFlavour == 21) ? 0 : meFlavour + 6;
    
    flavours = hadronFlavourEncoded + (partonFlavourEncoded<<4) + (meFlavourEncoded<<8);
}


float pec::Jet::CorrFactor() const
{
    return corrFactor;
}


float pec::Jet::JECUncertainty() const
{
    return jecUncertainty;
}


float pec::Jet::JERUncertainty() const
{
    return jerUncertainty;
}


float pec::Jet::BTag(BTagAlgo algo) const
{
    return bTags[unsigned(algo)];
}


float pec::Jet::CTag(CTagAlgo algo) const
{
    return cTags[unsigned(algo)];
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


int pec::Jet::Flavour(FlavourType type /*= FlavourType::Hadron*/) const
{
    unsigned const encodedFlavour = flavours>>(4 * unsigned(type)) & 0xF;
    
    if (encodedFlavour == 0)
        return 21;
    else
        return encodedFlavour - 6;
}
