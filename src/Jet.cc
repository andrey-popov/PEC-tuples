#include <Analysis/PECTuples/interface/Jet.h>

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <stdexcept>


pec::Jet::Jet() noexcept:
    CandidateWithID(),
    corrFactor(0), jecUncertainty(0), jerUncertainty(0),
    bTags{0, 0}, cTags{0, 0}, bTagsDNN{0, 0, 0, 0},
    pileUpMVA(0),
    qgTag(0),
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
    bTagsDNN[0] = bTagsDNN[1] = bTagsDNN[2] = bTagsDNN[3] = 0;
    pileUpMVA = 0;
    qgTag = 0;
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


void pec::Jet::SetBTagDNN(float valueBB, float valueB, float valueCC, float valueC,
  float valueUDSG)
{
    // A sanity check. The discriminators must add up either to 1 or to (-5). The latter happens
    //when b-tags cannot be evaluated.
    float const sum = valueBB + valueB + valueCC + valueC + valueUDSG;
    
    if (std::abs(sum - 1) > 1e-4 and std::abs(sum + 5) > 1e-4)
    {
        std::ostringstream message;
        message << "pec::Jet::SetBTagDBB: Values of given DNN b-tagging discriminators sum up " <<
          "to " << sum << ", while the sum must be either 1 or (-5).";
        throw std::logic_error(message.str());
    }
    
    
    // Store just the necessary four discriminators
    bTagsDNN[0] = valueBB;
    bTagsDNN[1] = valueB;
    bTagsDNN[2] = valueCC;
    bTagsDNN[3] = valueC;
}


void pec::Jet::SetPileUpID(float pileUpMVA_)
{
    pileUpMVA = pileUpMVA_;
}


void pec::Jet::SetQGTag(float value)
{
    qgTag = value;
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
        throw std::runtime_error("pec::Jet::SetFlavour: Illegal value for jet flavour is given.");
    
    
    unsigned hadronFlavourEncoded, partonFlavourEncoded, meFlavourEncoded;
    
    if (hadronFlavour == 21)
        hadronFlavourEncoded = 0xF;
    else if (hadronFlavour != 0)
        hadronFlavourEncoded = hadronFlavour + 6;
    else
        hadronFlavourEncoded = 0;
    
    if (partonFlavour == 21)
        partonFlavourEncoded = 0xF;
    else if (partonFlavour != 0)
        partonFlavourEncoded = partonFlavour + 6;
    else
        partonFlavourEncoded = 0;
    
    if (meFlavour == 21)
        meFlavourEncoded = 0xF;
    else if (meFlavour != 0)
        meFlavourEncoded = meFlavour + 6;
    else
        meFlavourEncoded = 0;
    
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


float pec::Jet::BTagDNN(BTagDNNType type) const
{
    switch (type)
    {
        case BTagDNNType::BB:
            return bTagsDNN[0];
        
        case BTagDNNType::B:
            return bTagsDNN[1];
        
        case BTagDNNType::CC:
            return bTagsDNN[2];
        
        case BTagDNNType::C:
            return bTagsDNN[3];
        
        case BTagDNNType::UDSG:
        {
            float value = 1.f - (bTagsDNN[0] + bTagsDNN[1] + bTagsDNN[2] + bTagsDNN[3]);
            
            if (std::abs(value - 5) < 1e-4)
            {
                // There are no valid b-tags for this jet
                value = -1.f;
            }
            
            return value;
        }
        
        default:
        {
            std::ostringstream message;
            message << "pec::Jet::BTagDNN: Unhandled value of the enumeration given (" <<
              unsigned(type) << ").";
            throw std::logic_error(message.str());
        }
    }
}


float pec::Jet::CTag(CTagAlgo algo) const
{
    return cTags[unsigned(algo)];
}


float pec::Jet::PileUpID() const
{
    return pileUpMVA;
}


float pec::Jet::QGTag() const
{
    return qgTag;
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
    
    if (encodedFlavour == 0xF)
        return 21;
    else if (encodedFlavour == 0)
        return 0;
    else
        return encodedFlavour - 6;
}
