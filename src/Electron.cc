#include <UserCode/SingleTop/interface/Electron.h>

#include <stdexcept>


using namespace pec;


Electron::Electron():
    Lepton(),
    cutBasedId(0), mvaId(0)
{}


Electron::Electron(Electron const &src):
    Lepton(src),
    cutBasedId(src.cutBasedId), mvaId(src.mvaId)
{}


Electron &Electron::operator=(Electron const &src)
{
    Lepton::operator=(src);
    
    cutBasedId = src.cutBasedId;
    mvaId = src.mvaId;
    
    return *this;
}


void Electron::Reset()
{
    Lepton::Reset();
    
    cutBasedId = 0;
    mvaId = 0;
}


void Electron::SetCutBasedIdBit(unsigned bitIndex, bool value /*= true*/)
{
    if (bitIndex >= 8)
        throw std::runtime_error("Electron::SetCutBasedIdBit: Given index exceeds the maximal "
         "allowed value.");
    
    if (value)
        cutBasedId |= (1 << bitIndex);
    else
        cutBasedId &= ~(1 << bitIndex);
}


void Electron::SetMvaId(double mva)
{
    mvaId = minifloat::encodeRange(-1., 1., mva);
}


bool Electron::CutBasedId(unsigned bitIndex) const
{
    if (bitIndex >= 8)
        throw std::runtime_error("Electron::CutBasedId: Given index exceeds the maximal allowed "
         "value.");
    
    return (cutBasedId & (1 << bitIndex));
}


double Electron::MvaId() const
{
    return minifloat::decodeRange(-1., 1., mvaId);
}
