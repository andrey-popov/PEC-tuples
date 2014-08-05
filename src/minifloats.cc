#include <UserCode/SingleTop/interface/minifloats.h>

#include <cmath>


UShort_t minifloat::encodeUniformRange(double min, double max, double value)
{
    // The allowed range is split into bins of equal size. Find the lower edge of the bin to which
    //the given value belongs
    double const res = std::floor((value - min) / (max - min) * 65536.);
    
    
    // Make sure the result is in the supported range for the representation
    if (res >= 65536.)
        return 65535;
    
    if (res < 0.)
        return 0;
    
    
    // If the workflow has reached this point, the representation is fine and can be converted to
    //the storage type
    return res;
}


double minifloat::decodeUniformRange(double min, double max, UShort_t representation)
{
    return min + (max - min) * (double(representation) + 0.5) / 65536.;
    //^ The decoded value should correspond to the centre of the bin given by the representation.
    //This is why 0.5 is added
}


UShort_t minifloat::encodeAngle(double value)
{
    return minifloat::encodeUniformRange(-M_PI, M_PI, value);
}


double minifloat::decodeAngle(UShort_t representation)
{
    return minifloat::decodeUniformRange(-M_PI, M_PI, representation);
}
