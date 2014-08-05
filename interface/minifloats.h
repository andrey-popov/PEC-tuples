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
     * Performs an inverse operation w.r.t. function encodeUniformRange.
     */
    double decodeUniformRange(double min, double max, UShort_t representation);
    
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
