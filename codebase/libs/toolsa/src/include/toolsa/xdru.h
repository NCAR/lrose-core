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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef XDRU_WAS_INCLUDED
#define XDRU_WAS_INCLUDED


/**********************************************************************
 Library to convert to and from xdr format, as in networking
 applications.

 In the case of shorts and longs, this is equivalent but more efficient
 than using the htonl() etc. routines, because a single call may be made
 to reorder an array of data

 */

/*
 * If the host computer has shorts and longs in network byte order
 * (e.g. SUN) do not define XDRU_SWAP.
 *
 * If the host machine must swap bytes from network byte order,
 * (e.g. DEC3100, Intel PC's ) define XDRU_SWAP.
 *
 */

#if defined(DEC) || defined(__linux) || defined (SUNOS5_INTEL)
#define XDRU_SWAP 
#endif

/* 
 * XDRU_tohl - xdr to host format - longs
 */

extern void XDRU_tohl(unsigned long *array, unsigned long nbytes);

/* 
 * XDRU_fromhl - host to xdr format - longs
 */

extern void XDRU_fromhl(unsigned long *array, unsigned long nbytes);

/* 
 * XDRU_tohs - xdr to host format - shorts
 */

extern void XDRU_tohs(unsigned short *array, unsigned short nbytes);

/* 
 * XDRU_fromhs - host to xdr format - shorts
 */

extern void XDRU_fromhs(unsigned short *array, unsigned short nbytes);

#endif
#ifdef __cplusplus
}
#endif
