#include <Analysis/PECTuples/interface/CandidateWithID.h>

#include <stdexcept>


using namespace pec;


CandidateWithID::CandidateWithID():
    Candidate(),
    id(0)
{}


CandidateWithID::CandidateWithID(CandidateWithID const &src):
    Candidate(src),
    id(src.id)
{}


CandidateWithID &CandidateWithID::operator=(CandidateWithID const &src)
{
    Candidate::operator=(src);
    id = src.id;
    
    return *this;
}


CandidateWithID::~CandidateWithID()
{}


void CandidateWithID::Reset()
{
    Candidate::Reset();
    
    id = 0;
}


void CandidateWithID::SetBit(unsigned index, bool value /*= true*/)
{
    if (index >= 8)
        throw std::runtime_error("CandidateWithID::SetBit: Given index exceeds the maximal allowed "
         "value.");
    
    if (value)
        id |= (1 << index);
    else
        id &= ~(1 << index);
}


bool CandidateWithID::TestBit(unsigned index) const
{
    if (index >= 8)
        throw std::runtime_error("CandidateWithID::TestBit: Given index exceeds the maximal "
         "allowed value.");
    
    return (id & (1 << index));
}
