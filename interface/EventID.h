#pragma once

#include <Rtypes.h>


namespace pec
{
/**
 * \class EventID
 * \brief A class to aggregate event ID
 */
class EventID
{
public:
    /// Constructor without parameters
    EventID() noexcept;
    
    /// Default copy constructor
    EventID(EventID const &) = default;
    
    /// Default assignment operator
    EventID &operator=(EventID const &) = default;
    
public:
    /// Resets the object to a state right after the default initialisation
    void Reset();
    
    /// Sets run number
    void SetRunNumber(unsigned long long run);
    
    /// Sets number of the luminosity secion
    void SetLumiSectionNumber(unsigned long long lumi);
    
    /// Sets event number
    void SetEventNumber(unsigned long long event);
    
    /// Returns run number
    unsigned long long RunNumber() const;
    
    /// Returns number of the luminosity secion
    unsigned long long LumiSectionNumber() const;
    
    /// Returns event number
    unsigned long long EventNumber() const;
    
private:
    /// Run number
    UInt_t run;
    
    /// Number of the luminosity section within the run
    UInt_t lumi;
    
    /// Event number within the run
    ULong64_t event;
};
}  // end of namespace pec
