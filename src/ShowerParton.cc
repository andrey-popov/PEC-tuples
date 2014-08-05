#include <UserCode/SingleTop/interface/ShowerParton.h>

#include <stdexcept>


using namespace pec;


ShowerParton::ShowerParton():
    Candidate(),
    pdgId(0),
    origin(0)
{}


ShowerParton::ShowerParton(ShowerParton const &src):
    Candidate(src),
    pdgId(src.pdgId),
    origin(src.origin)
{}


ShowerParton &ShowerParton::operator=(ShowerParton const &src)
{
    Candidate::operator=(src);
    
    pdgId = src.pdgId;
    origin = src.origin;
    
    return *this;
}


void ShowerParton::Reset()
{
    Candidate::Reset();
    
    pdgId = 0;
    origin = 0;
}


void ShowerParton::SetPdgId(int pdgId_)
{
    if (pdgId_ > 127 or pdgId_ < -128)
        throw std::runtime_error("ShowerParton::SetPdgId: Current implementation allows only one "
         "byte for the PDG ID.");
    
    pdgId = pdgId_;
}


void ShowerParton::SetOrigin(Origin origin_)
{
    origin = UChar_t(origin_);
}


int ShowerParton::PdgId() const
{
    return pdgId;
}


ShowerParton::Origin ShowerParton::GetOrigin() const
{
    return Origin(origin);
}
