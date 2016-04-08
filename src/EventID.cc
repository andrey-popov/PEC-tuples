#include <Analysis/PECTuples/interface/EventID.h>


pec::EventID::EventID() noexcept:
    run(0),
    lumi(0),
    event(0)
{}


void pec::EventID::Reset()
{
    run = 0;
    lumi = 0;
    event = 0;
}


void pec::EventID::SetRunNumber(unsigned long long run_)
{
    run = run_;
}


void pec::EventID::SetLumiSectionNumber(unsigned long long lumi_)
{
    lumi = lumi_;
}


void pec::EventID::SetEventNumber(unsigned long long event_)
{
    event = event_;
}


unsigned long long pec::EventID::RunNumber() const
{
    return run;
}


unsigned long long pec::EventID::LumiSectionNumber() const
{
    return lumi;
}


unsigned long long pec::EventID::EventNumber() const
{
    return event;
}
