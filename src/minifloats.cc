#include <Analysis/PECTuples/interface/minifloats.h>


minifloat::Repr_t minifloat::encodeRange(double min, double max, double value)
{
    #ifdef MINIFLOATS_DISABLED
    return value;
    #else
    
    // Should map value = min to 0 and value = max to 65535
    long const res = std::lrint((value - min) / (max - min) * 65535.);
    
    
    // Check over- and underflows
    if (res >= 65536)
        return 65535;
    
    if (res < 0)
        return 0;
    
    
    // If the workflow has reached this point, the representation is fine and can be converted to
    //the storage type
    return res;
    
    #endif
}


double minifloat::decodeRange(double min, double max, Repr_t representation)
{
    #ifdef MINIFLOATS_DISABLED
    return representation;
    #else
    
    return min + (max - min) * double(representation) / 65535.;
    
    #endif
}


minifloat::Repr_t minifloat::encodeCircular(double min, double max, double value)
{
    #ifdef MINIFLOATS_DISABLED
    return value;
    #else
    
    long const resNorm = std::lrint((value - min) * 65536. / (max - min)) % 65536;
    //^ The result might be negative
    
    return ((resNorm >= 0) ? resNorm : 65536 + resNorm);
    
    #endif
}


double minifloat::decodeCircular(double min, double max, Repr_t representation)
{
    #ifdef MINIFLOATS_DISABLED
    return representation;
    #else
    
    return min + (max - min) * double(representation) / 65536.;
    
    #endif
}


minifloat::Repr_t minifloat::encodeAngle(double value)
{
    #ifdef MINIFLOATS_DISABLED
    return value;
    #else
    
    return minifloat::encodeCircular(-M_PI, M_PI, value);
    
    #endif
}


double minifloat::decodeAngle(Repr_t representation)
{
    #ifdef MINIFLOATS_DISABLED
    return representation;
    #else
    
    return minifloat::decodeCircular(-M_PI, M_PI, representation);
    
    #endif
}
