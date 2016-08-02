/* **********************************************************************
 * *                                                                    *
 * *                 Routines to Help With Data Types                   *
 * *                                                                    *
 * **********************************************************************
 * File: libs/user/DataConvert.c
 *
 *      COPYRIGHT (c) 1995, 1997, 1998, 1999, 2001, 2003, 2005 BY
 *          SIGMET INCORPORATED, WESTFORD MASSACHUSETTS, U.S.A.
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef LINUX
#include <ieee754.h>
#endif /* #ifdef LINUX */

#include "sigtypes.h"
#include "user_lib.h"

/* ************************************
 * *                                  *
 * *  Packed Floating (I,Q) Routines  *
 * *                                  *
 * ************************************
 *
 * Convert a normalized floating "I" or "Q" value to/from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 */

/* ------------------------------
 * Convert an array of FLT4's to packed floating.
 */
void vecPackIQFromFloatIQ_
( volatile UINT2 iCodes_a[], volatile const FLT4 fIQVals_a[],
  SINT4 iCount_a, UINT4 iFlags_a )
{
  SINT4 iCount ; volatile const FLT4 *fIQVals = fIQVals_a ;
  UINT1 lSwap = (iFlags_a & PACKIQ_BYTESWAP) != 0 ;
  volatile UINT2 *iCodes = iCodes_a ;

  if( iFlags_a & PACKIQ_HIGHSNR ) {
    /* High SNR packed format with 12-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode ; FLT4 fIQVal = *fIQVals++ ;

      if     (  fIQVal >=  4.0 ) iCode = 0xF7FF ;
      else if(  fIQVal <= -4.0 ) iCode = 0xF800 ;
      else if( (fIQVal >  -1.221299E-4) &&
	       (fIQVal <   1.220703E-4) ) {
	SINT4 iUnderFlow = NINT( 1.677721E7 * fIQVal ) ;
	iCode = 0xFFF & MAX( -2048, MIN( 2047, iUnderFlow ) ) ;
      } else {
	int iSign, iExp, iMan ;
#ifdef LINUX
	/* Much faster (x4.5) version on LINUX uses hardware definition
	 * of FLT4.  Native mantissa has 23 bits, we want 12, rounded.
	 * Courtesy of Nathan Parker, MIT/LL, Aug 2002.
	 */
	union ieee754_float u ; int uMan ;
	u.f = fIQVal ; uMan = u.ieee.mantissa ; iSign = u.ieee.negative ;
	iMan = (uMan >> 12) | (1<<11) ; if( iSign ) iMan = ~iMan ;
	iMan += ((uMan >> 11) & 1 ) ^ iSign ;

	iExp = (int)u.ieee.exponent - (IEEE754_FLOAT_BIAS - 1) + 13 ;
#else
	iMan = NINT( 4096.0 * frexp( fIQVal, &iExp ) ) ;
	iExp += 13 ; iSign = (fIQVal < 0.0) ;
#endif

	if( iMan ==  4096 ) iExp++ ; /* Fix up mantissa overflows */
	if( iMan == -2048 ) iExp-- ; /*   by adjusting the exponent. */

	iCode = (iExp << 12) | (iSign << 11) | (0x7FF & iMan) ;
      }
      if( lSwap ) iCode = (iCode << 8) | (iCode >> 8) ;
      *iCodes++ = iCode ;
    }
  }
  else {
    /* Legacy packed format with 11-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode ; FLT4 fIQVal = *fIQVals++ ;

      if     ( fIQVal ==  0.0 ) iCode = 0x0000 ;
      else if( fIQVal >=  4.0 ) iCode = 0xFBFF ;
      else if( fIQVal <= -4.0 ) iCode = 0xFC00 ;
      else {
	int iSign, iExp, iMan ;
#ifdef LINUX
	/* Much faster (x4.5) version on LINUX uses hardware definition
	 * of FLT4.  Native mantissa has 23 bits, we want 11, rounded.
	 * Courtesy of Nathan Parker, MIT/LL, Aug 2002.
	 */
	union ieee754_float u ; int uMan ;
	u.f = fIQVal ; uMan = u.ieee.mantissa ; iSign = u.ieee.negative ;
	iMan = (uMan >> 13) | (1<<10) ; if( iSign ) iMan = ~iMan ;
	iMan += ((uMan >> 12) & 1 ) ^ iSign ;

	iExp = (int)u.ieee.exponent - (IEEE754_FLOAT_BIAS - 1) + 29 ;
#else
	iMan = NINT( 2048.0 * frexp( fIQVal, &iExp ) ) ;
	iExp += 29 ; iSign = (fIQVal < 0.0) ;
#endif

	if( iMan ==  2048 ) iExp++ ; /* Fix up mantissa overflows */
	if( iMan == -1024 ) iExp-- ; /*   by adjusting the exponent. */

	if( iExp < 0 ) iCode = 0x0000 ;
	else           iCode = (iExp << 11) | (iSign << 10) | (0x3FF & iMan) ;
      }
      if( lSwap ) iCode = (iCode << 8) | (iCode >> 8) ;
      *iCodes++ = iCode ;
    }
  }
}

/* ------------------------------
 * Convert an array of packed floating to FLT4.  This uses table
 * lookup to run at the maximum possible speed.
 */
void vecFloatIQFromPackIQ_
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
  SINT4 iCount_a, UINT4 iFlags_a )
{
  /* Maintain separate tables for each of the possible combinations of
   * requested flag bits.
   */
  static FLT4 *pLookupTabs_c[ 4 ] = { NULL } ;

  FLT4 **pTableSlot = &pLookupTabs_c[ iFlags_a & 3 ], *pTable ;

  /* Get a pointer to our table of floats, but build the table now if
   * we haven't done it already.  If the malloc fails, then just call
   * the internal routine and do the work directly.
   */
  if( ! ( pTable = *pTableSlot ) ) {
    if( ! ( *pTableSlot = malloc( 0x10000 * sizeof(FLT4) ) ) ) {
      vecFloatIQFromPackIQ_comp( fIQVals_a, iCodes_a, iCount_a, iFlags_a ) ;
      return ;
    } else {
      UINT2 iCode = 0 ; pTable = *pTableSlot ;
      for(;;) {
	vecFloatIQFromPackIQ_comp( &pTable[iCode], &iCode, 1, iFlags_a ) ;
	if( 0 == ++iCode ) break ;
      }
    }
  }
  /* Now buzz out the actual data conversion.  Remove the volatile
   * casts just in case it allows the compiler to make better code.
   */
  {
    FLT4 *pfIQVals = (void *)fIQVals_a ; SINT4 iCount ;
    const UINT2 *piCodes = (void *)iCodes_a ;

    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      *pfIQVals++ = pTable[ *piCodes++ ] ;
    }
  }
}

/* ------------------------------
 * Kernel packing routine that actually knows how to compute the
 * floating values from the packed data.
 */
void vecFloatIQFromPackIQ_comp
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
  SINT4 iCount_a, UINT4 iFlags_a )
{
  SINT4 iCount ; volatile const UINT2 *iCodes = iCodes_a ;
  UINT1 lSwap = (iFlags_a & PACKIQ_BYTESWAP) != 0 ;
  volatile FLT4 *fIQVals = fIQVals_a ;

  if( iFlags_a & PACKIQ_HIGHSNR ) {
    /* High SNR packed format with 12-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;
      if( lSwap ) iCode = (iCode << 8) | (iCode >> 8) ;

      if( iCode & 0xF000 ) {
	SINT4 iMan =  iCode        & 0x7FF ;
	SINT4 iExp = (iCode >> 12) & 0x00F ;

	if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
	else                 iMan |= 0x00000800 ;

	fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 3.355443E7 ;
      }
      else {
	fVal = ( (FLT4)(((SINT4)iCode) << 20) ) / 1.759218E13 ;
      }
      *fIQVals++ = fVal ;
    }
  } else {
    /* Legacy packed format with 11-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;
      if( lSwap ) iCode = (iCode << 8) | (iCode >> 8) ;

      if( iCode ) {
	SINT4 iMan =  iCode        & 0x3FF ;
	SINT4 iExp = (iCode >> 11) & 0x01F ;

	if( iCode & 0x0400 ) iMan |= 0xFFFFF800 ;
	else                 iMan |= 0x00000400 ;

	fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 1.099512E12 ;
      }
      *fIQVals++ = fVal ;
    }
  }
}

/* ------------------------------
 * Cover routines for converting one item at a time.
 */
UINT2 iPackIQFromFloatIQ_( FLT4 fIQVal_a, UINT4 iFlags_a )
{
  UINT2 iCode ;
  vecPackIQFromFloatIQ_( &iCode, &fIQVal_a, 1, iFlags_a ) ;
  return( iCode ) ;
}
FLT4  fFloatIQFromPackIQ_( UINT2 iCode_a, UINT4 iFlags_a )
{
  FLT4 fFloatIQ ;
  vecFloatIQFromPackIQ_comp( &fFloatIQ, &iCode_a, 1, iFlags_a ) ;
  return( fFloatIQ ) ;
}

/* ------------------------------
 * Test program to verify the conversion routines
 */
#if 0
#define FLAGS PACKIQ_HIGHSNR
int main( int argc_a, char *argv_a[] )
{
  SINT4 iIter ;
  for( iIter=0 ; iIter < 0x10000 ; iIter++ ) {
#if FLAGS
    SINT4 iEnCode = (iIter & 0x07FF) | ((iIter>>4) & 0x0800) | ((iIter<<1) & 0xF000) ;
#else
    SINT4 iEnCode = (iIter & 0x03FF) | ((iIter>>5) & 0x0400) | ((iIter<<1) & 0xF800) ;
#endif
    FLT4     fVal = fFloatIQFromPackIQ_( iEnCode, FLAGS ) ;
    SINT4 iDeCode = iPackIQFromFloatIQ_( fVal   , FLAGS ) ;

    if( iDeCode != iEnCode ) {
      printf( "MISMATCH!!  In:%4.4X  Out:%4.4X  fVal:%16.13f\n",
	      iEnCode, iDeCode, fVal ) ;

    } else if( (iEnCode & 0x00FF) == 0 ) {
      printf( "Code:%4.4X  fVal:%16.13f\n", iEnCode, fVal ) ;
    }
  }
  return(0) ;
}
#endif
