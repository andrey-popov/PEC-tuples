#pragma once

#include <Rtypes.h>


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
     * \brief Encodes a generic signed floating-point number
     * 
     * The user specifies desired number of bits to be used for significand and the offset for the
     * exponent (the offset is added to the actual exponent, i.e. it should be positive in order to
     * allow representing of numbers smaller than 1). To have a representation resembling binary16
     * in the IEE 754 standard, one should set nBitFrac = 10 and expBias = 14.
     * 
     * The value must not be NaN or infinity. Positive and negative zeros are not distinguished.
     * 
     * The implementation does not comply with the IEE 754 standard. It does not support subnormal
     * numbers nor NaN, infinities, or negative zero. Numbers that fall outside the representable
     * range are rounded either to zero or to the largest (in absolute value) representable number.
     */
    UShort_t encodeGenericSigned(unsigned nBitFrac, int expBias, double value);
    
    /**
     * \brief Decodes a generic signed floating-point number
     * 
     * Performs a back transformation of a result of the function encodeGenericSigned. Consult its
     * documentation for details.
     */
    double decodeGenericSigned(unsigned nBitFrac, int expBias, UShort_t representation);
    
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
