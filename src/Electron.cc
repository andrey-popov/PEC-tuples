#include <Analysis/PECTuples/interface/Electron.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>


using namespace pec;


unsigned const Electron::contIdSize;


Electron::Electron():
    Lepton(),
    cutBasedId(0)
{
    std::fill(std::begin(mvaId), std::end(mvaId), 0);
}


Electron::Electron(Electron const &src):
    Lepton(src),
    cutBasedId(src.cutBasedId)
{
    std::copy(std::begin(src.mvaId), std::end(src.mvaId), std::begin(mvaId));
}


Electron &Electron::operator=(Electron const &src)
{
    Lepton::operator=(src);
    
    cutBasedId = src.cutBasedId;
    std::copy(std::begin(src.mvaId), std::end(src.mvaId), std::begin(mvaId));
    
    return *this;
}


void Electron::Reset()
{
    Lepton::Reset();
    
    cutBasedId = 0;
    std::fill(std::begin(mvaId), std::end(mvaId), 0);
}


void Electron::SetBooleanID(unsigned bitIndex, bool value /*= true*/)
{
    if (bitIndex >= 8)
        throw std::runtime_error("pec::Electron::SetBooleanID: Given index exceeds the maximal "
         "allowed value.");
    
    if (value)
        cutBasedId |= (1 << bitIndex);
    else
        cutBasedId &= ~(1 << bitIndex);
}


void Electron::SetContinuousID(unsigned index, float mva)
{
    if (index >= contIdSize)
        throw std::runtime_error("pec::Electron::SetContinuousID: Given index exceeds the maximal "
         "allowed value. Consider increasing constant pec::Electron::contIdSize.");
    
    mvaId[index] = mva;
}


bool Electron::BooleanID(unsigned bitIndex) const
{
    if (bitIndex >= 8)
        throw std::runtime_error("pec::Electron::CutBasedId: Given index exceeds the maximal "
         "allowed value.");
    
    return (cutBasedId & (1 << bitIndex));
}


float Electron::ContinuousID(unsigned index) const
{
    if (index >= contIdSize)
        throw std::runtime_error("pec::Electron::SetMvaId: Given index exceeds the maximal allowed "
         "value.");
    
    return mvaId[index];
}
