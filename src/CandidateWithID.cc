#include <Analysis/PECTuples/interface/CandidateWithID.h>

#include <stdexcept>


pec::CandidateWithID::CandidateWithID():
    Candidate(),
    id(0)
{}


pec::CandidateWithID::~CandidateWithID()
{}


void pec::CandidateWithID::Reset()
{
    Candidate::Reset();
    
    id = 0;
}


void pec::CandidateWithID::SetBit(unsigned index, bool value /*= true*/)
{
    if (index >= 8)
        throw std::runtime_error("CandidateWithID::SetBit: Given index exceeds the maximal allowed "
         "value.");
    
    if (value)
        id |= (1 << index);
    else
        id &= ~(1 << index);
}


bool pec::CandidateWithID::TestBit(unsigned index) const
{
    if (index >= 8)
        throw std::runtime_error("CandidateWithID::TestBit: Given index exceeds the maximal "
         "allowed value.");
    
    return (id & (1 << index));
}
