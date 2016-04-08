#include <Analysis/PECTuples/interface/Muon.h>


pec::Muon::Muon() noexcept:
    Lepton()
{}


void pec::Muon::Reset()
{
    Lepton::Reset();
}
