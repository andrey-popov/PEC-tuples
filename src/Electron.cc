#include <Analysis/PECTuples/interface/Electron.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>


unsigned const pec::Electron::contIdSize;


pec::Electron::Electron():
    Lepton(),
    etaSC(0.f),
    cutBasedId(0)
{
    std::fill(std::begin(mvaId), std::end(mvaId), 0);
}


pec::Electron::Electron(Electron const &src):
    Lepton(src),
    etaSC(src.etaSC),
    cutBasedId(src.cutBasedId)
{
    std::copy(std::begin(src.mvaId), std::end(src.mvaId), std::begin(mvaId));
}


pec::Electron &pec::Electron::operator=(Electron const &src)
{
    Lepton::operator=(src);
    
    etaSC = src.etaSC;
    cutBasedId = src.cutBasedId;
    std::copy(std::begin(src.mvaId), std::end(src.mvaId), std::begin(mvaId));
    
    return *this;
}


void pec::Electron::Reset()
{
    Lepton::Reset();
    
    etaSC = 0.f;
    cutBasedId = 0;
    std::fill(std::begin(mvaId), std::end(mvaId), 0);
}


void pec::Electron::SetBooleanID(unsigned bitIndex, bool value /*= true*/)
{
    if (bitIndex >= 8)
        throw std::runtime_error("pec::Electron::SetBooleanID: Given index exceeds the maximal "
         "allowed value.");
    
    if (value)
        cutBasedId |= (1 << bitIndex);
    else
        cutBasedId &= ~(1 << bitIndex);
}


void pec::Electron::SetContinuousID(unsigned index, float mva)
{
    if (index >= contIdSize)
        throw std::runtime_error("pec::Electron::SetContinuousID: Given index exceeds the maximal "
         "allowed value. Consider increasing constant pec::Electron::contIdSize.");
    
    mvaId[index] = mva;
}


void pec::Electron::SetEtaSC(float etaSC_)
{
    etaSC = etaSC_;
}


bool pec::Electron::BooleanID(unsigned bitIndex) const
{
    if (bitIndex >= 8)
        throw std::runtime_error("pec::Electron::CutBasedId: Given index exceeds the maximal "
         "allowed value.");
    
    return (cutBasedId & (1 << bitIndex));
}


float pec::Electron::ContinuousID(unsigned index) const
{
    if (index >= contIdSize)
        throw std::runtime_error("pec::Electron::SetMvaId: Given index exceeds the maximal allowed "
         "value.");
    
    return mvaId[index];
}


float pec::Electron::EtaSC() const
{
    return etaSC;
}
