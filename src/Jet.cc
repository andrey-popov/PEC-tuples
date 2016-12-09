#include <Analysis/PECTuples/interface/Jet.h>

#include <stdexcept>


pec::Jet::Jet() noexcept:
    CandidateWithID(),
    corrFactor(0), jecUncertainty(0), jerUncertainty(0),
    bTags{0, 0}, cTags{0, 0},
    pileUpMVA(0),
    area(0),
    charge(0),
    pullAngle(0),
    flavour(0)
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
    flavour = 0;
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


void pec::Jet::SetFlavour(int flavour_)
{
    flavour = flavour_;
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


int pec::Jet::Flavour() const
{
    return flavour;
}
