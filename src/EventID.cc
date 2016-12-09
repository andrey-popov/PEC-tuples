#include <Analysis/PECTuples/interface/EventID.h>


pec::EventID::EventID() noexcept:
    run(0),
    lumi(0),
    event(0),
    bunchCrossing(0)
{}


void pec::EventID::Reset()
{
    run = 0;
    lumi = 0;
    event = 0;
    bunchCrossing = 0;
}


void pec::EventID::SetRunNumber(unsigned long run_)
{
    run = run_;
}


void pec::EventID::SetLumiSectionNumber(unsigned long lumi_)
{
    lumi = lumi_;
}


void pec::EventID::SetEventNumber(unsigned long long event_)
{
    event = event_;
}


void pec::EventID::SetBunchCrossing(unsigned bunchCrossing_)
{
    bunchCrossing = bunchCrossing_;
}


unsigned long pec::EventID::RunNumber() const
{
    return run;
}


unsigned long pec::EventID::LumiSectionNumber() const
{
    return lumi;
}


unsigned long long pec::EventID::EventNumber() const
{
    return event;
}


unsigned pec::EventID::BunchCrossing() const
{
    return bunchCrossing;
}
