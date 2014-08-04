#include <UserCode/SingleTop/interface/Electron.h>


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
    mvaID = mva;
}


UChar_t Electron::CutBasedID() const
{
    return cutBasedID;
}


double Electron::MvaID() const
{
    return mvaID;
}
