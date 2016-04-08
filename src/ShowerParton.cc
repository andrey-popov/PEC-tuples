#include <Analysis/PECTuples/interface/ShowerParton.h>

#include <stdexcept>


pec::ShowerParton::ShowerParton() noexcept:
    Candidate(),
    pdgId(0),
    origin(0)
{}


void pec::ShowerParton::Reset()
{
    Candidate::Reset();
    
    pdgId = 0;
    origin = 0;
}


void pec::ShowerParton::SetPdgId(int pdgId_)
{
    if (pdgId_ > 127 or pdgId_ < -128)
        throw std::runtime_error("ShowerParton::SetPdgId: Current implementation allows only one "
         "byte for the PDG ID.");
    
    pdgId = pdgId_;
}


void pec::ShowerParton::SetOrigin(Origin origin_)
{
    origin = UChar_t(origin_);
}


int pec::ShowerParton::PdgId() const
{
    return pdgId;
}


pec::ShowerParton::Origin pec::ShowerParton::GetOrigin() const
{
    return Origin(origin);
}
