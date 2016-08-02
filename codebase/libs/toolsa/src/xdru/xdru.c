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
/**************************************************************************
 * xdru.c
 *
 * Routines to convert to and from xdr format
 *
 * Mike Dixon, Gerry Wiener, RAP, NCAR, Boulder, CO, USA, 80307
 *
 * April 1992
 *
 * 5/3/92 - add to tool2 shared library JCaron
 * 6/29/92 - ansi c, put in toolsa JCaron
 *
 **************************************************************************/

#include <sys/types.h>
#include <toolsa/globals.h>
#include <toolsa/xdru.h>

/**********************************************************************
 * XDRU_tohl() - xdr to host format - longs
 *
 */

#ifndef XDRU_SWAP
/*ARGSUSED*/
#endif

void XDRU_tohl(unsigned long *array, unsigned long nbytes)
{

#ifndef XDRU_SWAP

  return;
  
#else

  unsigned long i, l, nlongs;
  unsigned long *this_long;

  nlongs = nbytes / sizeof(unsigned long);
  this_long = array;

  for (i = 0; i < nlongs; i++) {

    l = *this_long;

    *this_long = (((l & 0xff000000) >> 24) |
		  ((l & 0x00ff0000) >> 8) |
		  ((l & 0x0000ff00) << 8) |
		  ((l & 0x000000ff) << 24));

    this_long++;

  }

#endif

}

/**********************************************************************
 * XDRU_fromhl() - host to xdr format - longs
 *
 */

#ifndef XDRU_SWAP
/*ARGSUSED*/
#endif

void XDRU_fromhl(unsigned long *array, unsigned long nbytes)
{

#ifndef XDRU_SWAP

  return;
  
#else

  unsigned long i, l, nlongs;
  unsigned long *this_long;

  nlongs = nbytes / sizeof(unsigned long);
  this_long = array;

  for (i = 0; i < nlongs; i++) {

    l = *this_long;

    *this_long = (((l & 0xff000000) >> 24) |
		  ((l & 0x00ff0000) >> 8) |
		  ((l & 0x0000ff00) << 8) |
		  ((l & 0x000000ff) << 24));

    this_long++;

  }

#endif

}

/**********************************************************************
 * XDRU_tohs() - xdr to host format - shorts
 *
 */

#ifndef XDRU_SWAP
/*ARGSUSED*/
#endif

void XDRU_tohs(unsigned short *array, unsigned short nbytes)
{

#ifndef XDRU_SWAP

  return;
  
#else

  unsigned long i, nlongs, nshorts;
  unsigned long l;
  unsigned short s;
  unsigned long *this_long;

  nlongs = nbytes / sizeof(unsigned long);
  this_long = (unsigned long *)array;

  for (i = 0; i < nlongs; i++) {
    
    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 8) |
		   ((l & 0x00ff0000) << 8) |
		   ((l & 0x0000ff00) >> 8) |
		   ((l & 0x000000ff) << 8));

    this_long++;
  }
  
  if (nlongs * sizeof(unsigned long) != nbytes) {
    nshorts = nbytes / sizeof(unsigned short);
    s = array[nshorts-1];
    array[nshorts-1]= (((s & 0xff00) >> 8) | ((s & 0x00ff) << 8));
  }
	
#endif
  
}

/**********************************************************************
 * XDRU_fromhs() - host to xdr format - shorts
 *
 */

#ifndef XDRU_SWAP
/*ARGSUSED*/
#endif

void XDRU_fromhs(unsigned short *array, unsigned short nbytes)
{

#ifndef XDRU_SWAP

  return;
  
#else

  unsigned long i, nlongs, nshorts;
  unsigned long l;
  unsigned short s;
  unsigned long *this_long;

  nlongs = nbytes / sizeof(unsigned long);
  this_long = (unsigned long *)array;

  for (i = 0; i < nlongs; i++) {
    
    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 8) |
		   ((l & 0x00ff0000) << 8) |
		   ((l & 0x0000ff00) >> 8) |
		   ((l & 0x000000ff) << 8));

    this_long++;
  }
  
  if (nlongs * sizeof(unsigned long) != nbytes) {
    nshorts = nbytes / sizeof(unsigned short);
    s = array[nshorts-1];
    array[nshorts-1]= (((s & 0xff00) >> 8) | ((s & 0x00ff) << 8));
  }
	
#endif
  
}

