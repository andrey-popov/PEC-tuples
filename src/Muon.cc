#include <UserCode/SingleTop/interface/Muon.h>


using namespace pec;


Muon::Muon():
    Lepton()
{}


Muon::Muon(Muon const &src):
    Lepton(src)
{}


Muon &Muon::operator=(Muon const &src)
{
    Lepton::operator=(src);
    
    return *this;
}
