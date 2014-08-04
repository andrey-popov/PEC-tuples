#include <UserCode/SingleTop/interface/EventID.h>


using namespace pec;


EventID::EventID():
    run(0),
    lumi(0),
    event(0)
{}


EventID::EventID(EventID const &src):
    run(src.run),
    lumi(src.lumi),
    event(src.event)
{}


EventID &EventID::operator=(EventID const &src)
{
    run = src.run;
    lumi = src.lumi;
    event = src.event;
    
    return *this;
}


void EventID::SetRunNumber(unsigned long long run_)
{
    run = run_;
}


void EventID::SetLumiSectionNumber(unsigned long long lumi_)
{
    lumi = lumi_;
}


void EventID::SetEventNumber(unsigned long long event_)
{
    event = event_;
}


unsigned long long EventID::RunNumber() const
{
    return run;
}


unsigned long long EventID::LumiSectionNumber() const
{
    return lumi;
}


unsigned long long EventID::EventNumber() const
{
    return event;
}
