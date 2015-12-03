#include <Analysis/PECTuples/interface/GenParticle.h>

#include <stdexcept>


using namespace pec;


GenParticle::GenParticle():
    pdgId(0),
    firstMotherIndex(0), lastMotherIndex(0)
{}


GenParticle::GenParticle(GenParticle const &src):
    Candidate(src),
    pdgId(src.pdgId),
    firstMotherIndex(src.firstMotherIndex), lastMotherIndex(src.lastMotherIndex)
{}


GenParticle &GenParticle::operator=(GenParticle const &src)
{
    Candidate::operator=(src);
    
    pdgId = src.pdgId;
    firstMotherIndex = src.firstMotherIndex;
    lastMotherIndex = src.lastMotherIndex;
    
    return *this;
}


void GenParticle::Reset()
{
    Candidate::Reset();
    
    pdgId = 0;
    firstMotherIndex = lastMotherIndex = 0;
}


void GenParticle::SetPdgId(long pdgId_)
{
    if (pdgId_ > 127 or pdgId_ < -128)
        throw std::runtime_error("GenParticle::SetPdgId: Current implementation allows only one "
         "byte for the PDG ID.");
    
    pdgId = pdgId_;
}


void GenParticle::SetFirstMotherIndex(int index)
{
    if (index < -1)
        throw std::runtime_error("GenParticle::SetFirstMotherIndex: Illegal index.");
    
    firstMotherIndex = index + 1;
}


void GenParticle::SetLastMotherIndex(int index)
{
    if (index < -1)
        throw std::runtime_error("GenParticle::SetFirstMotherIndex: Illegal index.");
    
    lastMotherIndex = index + 1;
}


long GenParticle::PdgId() const
{
    return pdgId;
}


int GenParticle::FirstMotherIndex() const
{
    return firstMotherIndex - 1;
}


int GenParticle::LastMotherIndex() const
{
    return lastMotherIndex - 1;
}
