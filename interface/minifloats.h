#pragma once

#include <Rtypes.h>

#include <cmath>


/**
 * \brief Conversion functions to provide a 16-bit representation for a storage of floating-point
 * numbers
 * \author Andrey Popov
 * 
 * 
 */
namespace minifloat
{
    /**
     * \brief Encodes a floating-point value defined over a finite range
     * 
     * Represents the given value with an index of a bin of a uniform histogram defined over the
     * range. The range is specified by arguments min and max; the upper edge is not included. The
     * value must not be a NaN or infinity, otherwise the behaviour is undefined. If the value
     * falls outside the range, it is silently changed to the nearest representable number.
     */
    UShort_t encodeUniformRange(double min, double max, double value);
    
    /**
     * \brief Decodes a floating-point value defined over a finite range
     * 
     * Performs an inverse operation w.r.t. function encodeUniformRange. Consult its documentation
     * for details.
     */
    double decodeUniformRange(double min, double max, UShort_t representation);
    
    /**
     * \brief Encodes a generic floating-point number
     * 
     * The user specifies if the number is signed, the desired number of bits to be used for the
     * significand and the offset for the exponent (the offset is added to the actual exponent, i.e.
     * it should be positive in order to allow representing numbers smaller than 1 with normalised
     * values). Subnormal numbers are supported in the representation. To have a representation
     * resembling binary16 from the IEE 754-2008 standard, one should set isSigned = true,
     * nBitFrac = 10, and expBias = 15.
     * 
     * The value must not be a NaN or infinity. Positive and negative zeros are not distinguished.
     * If isSigned is false, negative values are mapped to zero. Values that are too large to be
     * represented are rounded to the closest representable numbers.
     * 
     * The implementation does not comply fully with the IEE 754 standard. It does not support NaNs,
     * infinities, or negative zero.
     */
    template<bool isSigned, unsigned nBitFrac, int expBias>
    UShort_t encodeGeneric(double value);
    
    /**
     * \brief Decodes a generic floating-point number
     * 
     * Performs a back transformation of a result of the function encodeGeneric. Consult its
     * documentation for details.
     */
    template<bool isSigned, unsigned nBitFrac, int expBias>
    double decodeGeneric(UShort_t representation);
    
    /**
     * \brief Encodes a floating-point angle defined over a range [-pi, pi)
     * 
     * This is a specialisation of the funtion encodeUniformRange.
     */
    UShort_t encodeAngle(double value);
    
    /**
     * \brief Decodes a floating-point angle defined over a range [-pi, pi)
     * 
     * This is a specialisation of the funtion decodeUniformRange.
     */
    double decodeAngle(UShort_t representation);
}


// Implementations of templated functions
template<bool isSigned, unsigned nBitFrac, int expBias>
UShort_t minifloat::encodeGeneric(double value)
{
    // A short-cut for the zero as it is a popular value
    if (value == 0.)  // true for both positive and negative zeros
        return 0;
    
    // Negative values for an unsigned target are also encoded as zeros
    if (not isSigned and value < 0.)
        return 0;
    
    
    // A variable to build the representation
    UShort_t repr = 0;
    
    
    // Number of bits in the exponent
    unsigned nBitExp = 16 - nBitFrac;
    
    if (isSigned)
        --nBitExp;
    
    
    // Parse the value into the significand and the exponent. It works even for subnormal numbers
    int e;
    double frac = std::frexp(value, &e);
    
    // The returned significand is in the range [0.5, 1) (if not zero), but [1, 2) is more
    //comfortable. Rearrange the significand and the exponent accordingly
    frac *= 2;
    --e;
    
    
    // Check if the number is too large to be representable. Note that the exponent is increased by
    //one when stored
    if (e + expBias + 1 >= (1 << nBitExp))
    {
        if (isSigned and value > 0.)
            return (1 << 15) - 1;  // i.e. all bits but the highest one are set to 1
        else
            return (1 << 16) - 1;  // i.e. all bits are set to 1
    }
    
    
    // Extract the sign and make the significand positive
    if (isSigned and frac < 0.)
    {
        repr |= (1 << 15);
        frac = -frac;
    }
    
    
    // Deal with subnormal numbers
    if (e + expBias < 0)
    {
        // The subnormal significand is encoded as unsigned(value / 2^(-expBias) * (1 << nBitFrac)),
        //which is simplified to the expression below
        unsigned const fracRepr = unsigned(std::ldexp(frac, e + expBias + nBitFrac));
        
        // Build the result
        if (fracRepr >= (1 << nBitFrac))  // might happen because of rounding
            return repr | (1 << nBitFrac);
            //^ This is the smallest (in absolute value) normal number with appropriate sign: the
            //exponent is 0x1, the significand is 0x0
        else
            return repr | fracRepr;
            //^ The exponent bits are zero, so there is no need to set them
    }
    
    // If the workflow reaches this point, the number in the target representation is not subnormal
    
    
    // Encode the significand
    unsigned fracRepr = unsigned((frac - 1.) * (1 << nBitFrac));
    
    if (fracRepr >= (1 << nBitFrac))  // might happen because of rounding
    {
        // Round the number towards the smallest significand in the next order of magnitude
        fracRepr = 0;
        ++e;
    }
    
    repr |= fracRepr;
    
    
    // Encode the exponent, which is non-negative. It is increased by 1 as a zero exponent is
    //reserved to indicate a subnormal number
    repr |= (unsigned(e + expBias + 1) << nBitFrac);
    
    
    // Everything is done
    return repr;
}


template<bool isSigned, unsigned nBitFrac, int expBias>
double minifloat::decodeGeneric(UShort_t representation)
{
    // A short-cut for the zero as it is a popular value
    if (representation == 0)
        return 0.;
    
    
    // Extract the significand (fraction) encoded in the nBitFrac lowest bits
    unsigned const fracRepr = representation & ((1 << nBitFrac) - 1);
    double frac = fracRepr / double(1 << nBitFrac);
    

    // Extract the sign from the highest bit
    int const sign = (isSigned and representation & (1 << 15)) ? -1 : +1;
    
    
    // Extract the exponent encoded in the highest bits
    unsigned e;
    
    if (isSigned)
        e = (representation & ((1 << 15) - 1)) >> nBitFrac;  // mask out the sign bit
    else
        e = representation >> nBitFrac;
    
    
    // Build the result
    if (e == 0)
        // This is a subnormal number
        return sign * std::ldexp(frac, -expBias);
    else
        // This is a normal floating-point number
        return sign * std::ldexp(1. + frac, e - expBias - 1);
}
