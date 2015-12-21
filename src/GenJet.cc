#include <Analysis/PECTuples/interface/GenJet.h>


pec::GenJet::GenJet():
    Candidate(),
    bcMult(0)
{}


pec::GenJet::GenJet(GenJet const &src):
    Candidate(src),
    bcMult(src.bcMult)
{}


pec::GenJet &pec::GenJet::operator=(GenJet const &src)
{
    Candidate::operator=(src);
    
    bcMult = src.bcMult;
    
    return *this;
}


void pec::GenJet::Reset()
{
    Candidate::Reset();
    
    bcMult = 0;
}


void pec::GenJet::SetBottomMult(unsigned mult)
{
    if (mult > 15)
        mult = 15;
    
    bcMult = mult * 16 + bcMult % 16;
}


void pec::GenJet::SetCharmMult(unsigned mult)
{
    if (mult > 15)
        mult = 15;
    
    bcMult = (bcMult - bcMult % 16) + mult;
}


unsigned pec::GenJet::BottomMult() const
{
    return bcMult / 16;
}


unsigned pec::GenJet::CharmMult() const
{
    return bcMult % 16;
}
