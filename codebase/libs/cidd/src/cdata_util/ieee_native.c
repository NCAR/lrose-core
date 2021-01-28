/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/****************************************************************************
 * NATIVE.C: Subroutines that depend on binary native mode operations/formats
 *
 * IEEE VERSION! 
 */

#define BINCVT

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cidd/cidd_lib.h>
#if defined(__linux)
extern void swab(const void *from, void *to, ssize_t n);
#endif

/*****************************************************************************
 * Note: I consider some of these routines to be kludges. Some are based on ones
 *        Obtained from UCSD, others are based on Advice given by C.S. types
 *        here at UNC. Especially kludgy are the float conversions. 
 *        Much better error/range checking could be preformed. Although
 *        they seem to work, I don't want any credit (or blame)     -F.H. 1989 
 */

/**************************************************************************
 * IEEEI_VAXI  integer conversion.  
 * Based on C. Neilson's code.                  FH oct 1985
 */
void ieeei_vaxi(int *dp, int n)
{
  int    i,j,k,l,m;

  for(l = 0,m = n;l < m;l++) {
    if((i = *dp) != 0)  {   /* leave zeroes alone! */
      j = (i >>16) & 0x00ffff;/* mov hi byte to lo       */
      k = (i <<16) & 0xffff0000;/* mov lo to hi    */
      *dp = j | k;
      swab(dp,dp,4);
      dp++;
    } else {
      dp++;
    } 
  }
}

/***************************************************************************
 *  IEEEF_VAXF  Single precision float point conversion. 
 *  float *dp: pointer to data that needs fixed (4 bytes )
 *  int     n: number of data values of same type to fix
 */

void ieeef_vaxf(float *dp, int n)
{
  int    k,l;
  int    *ptr;

  ptr = (int *) dp;

  for(k = 0,l = n;k < l;k++) {
    if(*ptr) {/* leave zeroes alone! */
      if(*ptr == 0xfe967699 ) {    /* inconvertable numbers ?*/
        *ptr++ = 0x96ff9976;
        dp++;
      } else {
        *dp *= 4.0; /* do strange mult by 4 */
        swab(ptr,ptr,4);  /* flip bytes  */
        dp++;
        ptr++;
      }
    } else {
      dp++;
      ptr++;
    }
  }
}

/***************************************************************************
 * VAXF_IEEEF : VAX to IEEE single precision floation point conversion
 * DEC to IEEE FLOAT conversion.
 * float     *dp: pointer to data that needs fixed (4 bytes )
 *   int     n:   number of data values of same type to fix
 */

void vaxf_ieeef(float *dp, int n)
{
  int    k,l;
  int    *ptr;

  ptr = (int *) dp;
    
  for(k = 0,l = n;k < l;k++) {
    if(*ptr) {    /* leave zeroes alone! */
      if(*ptr == 0x96ff9976) {    /* inconvertable numbers ?*/
        *ptr++ = 0xfe967699;
        dp++;
      } else {
        swab(ptr,ptr,4);  /* flip bytes                   */
        *dp /= 4.0;       /*  do strange division by four */
        dp++;
        ptr++;
      }
    } else {
      dp++;
      ptr++;
    }
  }
}

/*************************************************************************
 * IEEED_VAXD : IEEE to VAX double precision floating point conversion
 *                IEEED has 11 exp bits (biased by 1023) ,52 fraction bits
 *                VAXD has 8 exp bits (biased by 129), 55 fraction bits
 *    Note: No error checking performed - nonsense conversions will occur
 *            if ieee number > ~ 10^+/-38 or NAN or UNDERFLOW, OVERFLOW or 
 *            INIFINITY is encoded.
 * IEEE version.
 *    FH. Oct 1989
 */

void ieeed_vaxd(double *dp, int n)
{
  int    k,l;
  unsigned int    *ptr;
  unsigned int    frac;
  unsigned int    exp;    /* exponet */

  for(k = 0,l = n;k < l;k++) {     /* loop thru n values */
    ptr = (unsigned int *) dp;    /* set int pointer to same as dp */
    if(*ptr | *(ptr+1)) {            /* leave zeroes alone! */
         
      exp =((*ptr & 0x7ff00000) >>20) -1023;/* extract unbiased exponet */
      exp += 129;                            /* rebias exponet */
      exp &= 0x000000ff;    /* make sure its 8 bits */

      /* gather MSBits of fractional portion */
      frac =((*ptr & 0x000fffff) << 3) | ((*(ptr +1) & 0xe0000000) >> 29); 

      /* keep sign, add in exponet and MSBits of fraction */
      *ptr = (*ptr & 0x80000000) | (exp << 23) | frac;      

      *(ptr+1) = *(ptr+1) << 3; /* shift into position */
      swab(ptr,ptr,8);  /* flip bytes for vax  */
    }
    dp++;
  }
}
/*************************************************************************
 * VAXD_IEEED : VAX to IEEE double precision floating point conversion
 *                VAXD has 8 exp bits (biased by 129), 55 fraction bits
 *                IEEED has 11 exp bits (biased by 1023) ,52 fraction bits
 *    Note: 3 bit of precision in the fractional portion may be lost
 * IEEE version.
 *    FH Oct 1989
 */

void vaxd_ieeed(double *dp, int n)
{
  int    k,l;
  unsigned int    *ptr;
  unsigned int    frac;
  unsigned int    exp;    /* exponet */

  for(k = 0,l = n;k < l;k++) {
    ptr = (unsigned int *) dp;    /* set int pointers to same as dp */

    if(*ptr | *(ptr +1)) {     /* if its not all zeroes */
      swab(ptr,ptr,8);  /* flip bytes back into std order */
         
      if((*ptr & 0xff800000) == 0x80000000) {    /* is NAN */
        *ptr = 0x7ff00000;    /* set to IEEE NAN */
        *(ptr+1) = 0x00000000;
                 
      } else {    /* do bitwise conversion */

        /*extract unbiased exponet*/ 
        exp = ((*ptr & 0x7f800000) >> 23) -129;
        exp += 1023;                            /* rebias exponet */
        exp &= 0x000007ff;    /* make sure its 11 bits */

        /* gather MSBits of fractional portion */
        frac = ((*ptr & 0x007fffff) >> 3);

        /* take 3 lsb of MSB word & place into LSB word after shiftin */
        *(ptr+1)  = ((*ptr & 0x00000007) << 29) | (*(ptr+1) >> 3);

        /* insert exponet and MSBits of frac */
        *ptr = (*ptr & 0x80000000) | (exp << 20) | frac;
      } 
    }
    dp++;
  }
}
