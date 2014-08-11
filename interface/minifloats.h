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
     * it should be positive in order to allow representing numbers smaller than 1). To have a
     * representation resembling binary16 from the IEE 754 standard, one should set isSigned = true,
     * nBitFrac = 10 and expBias = 14.
     * 
     * The value must not be a NaN or infinity. If isSigned is false, the value must not be
     * negative. For performance reasons these conditions are not checked, and the behaviour is
     * undefined if they are violated. Positive and negative zeros are not distinguished.
     * 
     * The implementation does not comply with the IEE 754 standard. It does not support subnormal
     * numbers, NaN, infinities, or negative zero. Numbers that fall outside the representable
     * range are rounded either to zero or to the largest (in absolute value) representable number.
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
    // A short-cut for zero
    if (value == 0.)  // true for both positive and negative zeros
        return 0;
    
    
    // A variable to build the representation
    UShort_t repr = 0;
    
    
    // Number of bits in the exponent
    unsigned nBitExp = 16 - nBitFrac;
    
    if (isSigned)
        --nBitExp;
    
    
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
    if (e + expBias >= (1 << nBitExp))
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


template<bool isSigned, unsigned nBitFrac, int expBias>
double minifloat::decodeGeneric(UShort_t representation)
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
    if (isSigned and representation & (1 << 15))  // this is a negative number
        res = -res;
    
    
    // Extract the exponent encoded in the remaining bits
    unsigned e;
    
    if (isSigned)
        e = (representation & ((1 << 15) - 1)) >> nBitFrac;
    else
        e = (representation & ((1 << 16) - 1)) >> nBitFrac;
    
    
    // Include the exponent in the result
    res = std::ldexp(res, e - expBias);
    
    
    // Everything is done
    return res;
}
