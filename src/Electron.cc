#include <UserCode/SingleTop/interface/Electron.h>
#include <UserCode/SingleTop/interface/minifloats.h>


using namespace pec;


Electron::Electron():
    Lepton(),
    cutBasedID(0), mvaID(0)
{}


Electron::Electron(Electron const &src):
    Lepton(src),
    cutBasedID(src.cutBasedID), mvaID(src.mvaID)
{}


Electron &Electron::operator=(Electron const &src)
{
    Lepton::operator=(src);
    
    cutBasedID = src.cutBasedID;
    mvaID = src.mvaID;
    
    return *this;
}


void Electron::Reset()
{
    Lepton::Reset();
    
    cutBasedID = 0;
    mvaID = 0;
}


void Electron::SetCutBasedID(unsigned mask)
{
    cutBasedID = mask;
}


void Electron::SetMvaID(double mva)
{
    mvaID = minifloat::encodeRange(-1., 1., mva);
}


UChar_t Electron::CutBasedID() const
{
    return cutBasedID;
}


double Electron::MvaID() const
{
    return minifloat::decodeRange(-1., 1., mvaID);
}
