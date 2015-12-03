#include <Analysis/PECTuples/interface/GenJet.h>


using namespace pec;


GenJet::GenJet():
    Candidate(),
    bcMult(0)
{}


GenJet::GenJet(GenJet const &src):
    Candidate(src),
    bcMult(src.bcMult)
{}


GenJet &GenJet::operator=(GenJet const &src)
{
    Candidate::operator=(src);
    
    bcMult = src.bcMult;
    
    return *this;
}


void GenJet::Reset()
{
    Candidate::Reset();
    
    bcMult = 0;
}


void GenJet::SetBottomMult(unsigned mult)
{
    if (mult > 15)
        mult = 15;
    
    bcMult = mult * 16 + bcMult % 16;
}


void GenJet::SetCharmMult(unsigned mult)
{
    if (mult > 15)
        mult = 15;
    
    bcMult = (bcMult - bcMult % 16) + mult;
}


unsigned GenJet::BottomMult() const
{
    return bcMult / 16;
}


unsigned GenJet::CharmMult() const
{
    return bcMult % 16;
}
