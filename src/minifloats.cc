#include <UserCode/SingleTop/interface/minifloats.h>


UShort_t minifloat::encodeRange(double min, double max, double value)
{
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
}


double minifloat::decodeRange(double min, double max, UShort_t representation)
{
    return min + (max - min) * double(representation) / 65535.;
}


UShort_t minifloat::encodeCircular(double min, double max, double value)
{
    long const resNorm = std::lrint((value - min) * 65536. / (max - min)) % 65536;
    //^ The result might be negative
    
    return ((resNorm >= 0) ? resNorm : 65536 + resNorm);
}


double minifloat::decodeCircular(double min, double max, UShort_t representation)
{
    return min + (max - min) * double(representation) / 65536.;
}


UShort_t minifloat::encodeAngle(double value)
{
    return minifloat::encodeCircular(-M_PI, M_PI, value);
}


double minifloat::decodeAngle(UShort_t representation)
{
    return minifloat::decodeCircular(-M_PI, M_PI, representation);
}
