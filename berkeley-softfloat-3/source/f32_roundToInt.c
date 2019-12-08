
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2017 The Regents of the University of
California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifdef _KERNEL
#include <sys/types.h>
#else
#include <stdbool.h>
#include <stdint.h>
#endif
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

float32_t f32_roundToInt( float32_t a, uint_fast8_t roundingMode, bool exact )
{
    union ui32_f32 uA;
    uint_fast32_t uiA;
    int_fast16_t exp;
    uint_fast32_t uiZ, lastBitMask, roundBitsMask;
    union ui32_f32 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    exp = expF32UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp <= 0x7E ) {
        if ( !(uint32_t) (uiA<<1) ) return a;
        if ( exact ) softfloat_exceptionFlags |= softfloat_flag_inexact;
        uiZ = uiA & packToF32UI( 1, 0, 0 );
        switch ( roundingMode ) {
         case softfloat_round_near_even:
            if ( !fracF32UI( uiA ) ) break;
         case softfloat_round_near_maxMag:
            if ( exp == 0x7E ) uiZ |= packToF32UI( 0, 0x7F, 0 );
            break;
         case softfloat_round_min:
            if ( uiZ ) uiZ = packToF32UI( 1, 0x7F, 0 );
            break;
         case softfloat_round_max:
            if ( !uiZ ) uiZ = packToF32UI( 0, 0x7F, 0 );
            break;
#ifdef SOFTFLOAT_ROUND_ODD
         case softfloat_round_odd:
            uiZ |= packToF32UI( 0, 0x7F, 0 );
            break;
#endif
        }
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( 0x96 <= exp ) {
        if ( (exp == 0xFF) && fracF32UI( uiA ) ) {
            uiZ = softfloat_propagateNaNF32UI( uiA, 0 );
            goto uiZ;
        }
        return a;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uiZ = uiA;
    lastBitMask = (uint_fast32_t) 1<<(0x96 - exp);
    roundBitsMask = lastBitMask - 1;
    if ( roundingMode == softfloat_round_near_maxMag ) {
        uiZ += lastBitMask>>1;
    } else if ( roundingMode == softfloat_round_near_even ) {
        uiZ += lastBitMask>>1;
        if ( !(uiZ & roundBitsMask) ) uiZ &= ~lastBitMask;
    } else if (
        roundingMode
            == (signF32UI( uiZ ) ? softfloat_round_min : softfloat_round_max)
    ) {
        uiZ += roundBitsMask;
    }
    uiZ &= ~roundBitsMask;
    if ( uiZ != uiA ) {
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == softfloat_round_odd ) uiZ |= lastBitMask;
#endif
        if ( exact ) softfloat_exceptionFlags |= softfloat_flag_inexact;
    }
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}
