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
    /// Resets the object to a state right after the default initialization
    void Reset();
    
    /// Sets run number
    void SetRunNumber(unsigned long run);
    
    /// Sets number of the luminosity section
    void SetLumiSectionNumber(unsigned long lumi);
    
    /// Sets event number
    void SetEventNumber(unsigned long long event);
    
    /// Sets bunch crossing number
    void SetBunchCrossing(unsigned bunchCrossing);
    
    /// Returns run number
    unsigned long RunNumber() const;
    
    /// Returns number of the luminosity section
    unsigned long LumiSectionNumber() const;
    
    /// Returns event number
    unsigned long long EventNumber() const;
    
    /// Returns bunch crossing number
    unsigned BunchCrossing() const;
    
private:
    /// Run number
    UInt_t run;
    
    /// Number of the luminosity section within the run
    UInt_t lumi;
    
    /// Event number within the run
    ULong64_t event;
    
    /**
     * \brief Bunch crossing number
     * 
     * In simulation this field is expected to be set to zero.
     */
    UShort_t bunchCrossing;
};
}  // end of namespace pec
