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


UShort_t minifloat::encodeGenericSigned(unsigned nBitFrac, int expBias, double value)
{
    // A short-cut for zero
    if (value == 0.)  // true for both positive and negative zeros
        return 0;
    
    
    // A variable to build the representation
    UShort_t repr = 0;
    
    
    // Parse the value into the significand and the exponent
    int e;
    double frac = std::frexp(value, &e);
    
    // The returned significand is in the range [0.5, 1), but [1, 2) is more comfortable. Rearrange
    //the significand and the exponent accordingly
    frac *= 2;
    --e;
    
    
    // Check if the number is not too small. If it is the case, simply encode as zero (do not mess
    //with subnormal numbers)
    if (e + expBias < 0)
        return 0;
    
    // Check if the number is too large
    if (e + expBias >= (1 << (16 - nBitFrac - 1)))
    {
        if (value > 0.)
            return (1 << 15) - 1;  // i.e. all bits but the highest one are set to 1
        else
            return (1 << 16) - 1;  // i.e. all bits are set to 1
    }
    
    
    // Extract the sign and make the significand positive
    if (frac < 0.)
    {
        repr |= (1 << 15);
        frac = -frac;
    }
    
    
    // Encode the significand in the lowest bits
    unsigned fracRepr = std::floor((frac - 1.) * (1 << nBitFrac));
    
    if (fracRepr >= (1 << nBitFrac))
    //^ Could happen if in reality frac >= 2 because of rounding errors
        fracRepr = (1 << nBitFrac) - 1;
    
    repr |= fracRepr;
    
    
    // Encode the exponent. It has already been checked that the number (e + expBias) can be encoded
    //with appropriate bits. Just put this number into its place in the representation.
    repr |= (unsigned(e + expBias) << nBitFrac);
    
    
    // Everything is done
    return repr;
}


double minifloat::decodeGenericSigned(unsigned nBitFrac, int expBias, UShort_t representation)
{
    // A short-cut for zero
    if (representation == 0)
        return 0.;
    
    
    // A variable to accumulate the result
    double res;
    
    
    // Extract the significand (fraction) encoded in the nBitFrac lowest bits
    unsigned const fracRepr = representation & ((1 << nBitFrac) - 1);
    
    // Decode the significand and put it into the result. It occupies the range [1, 2)
    res = 1. + fracRepr / double(1 << nBitFrac);
    
    
    // Extract the sign, which is encoded in the highest bit
    if (representation & (1 << 15))  // this is a negative number
        res = -res;
    
    
    // Extract the exponent encoded in the remaining bits
    unsigned const e = (representation & ((1 << 15) - 1)) >> nBitFrac;
    
    // Include the exponent in the result
    res = std::ldexp(res, e - expBias);
    
    
    // Everything is done
    return res;
}


UShort_t minifloat::encodeAngle(double value)
{
    return minifloat::encodeUniformRange(-M_PI, M_PI, value);
}


double minifloat::decodeAngle(UShort_t representation)
{
    return minifloat::decodeUniformRange(-M_PI, M_PI, representation);
}
