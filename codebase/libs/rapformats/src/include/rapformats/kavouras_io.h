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
/*************************************************************************
 * KAVOURAS_IO.H : Defines necessary for using KAVOURAS format data and routines.
 *
 * F. Hage Dec 1993. NCAR, RAP.
 *
 */

#define KAVOURAS_IO_H

#include "kavouras_data.h"


#ifndef KAVOURAS_IO_C

#ifdef __cplusplus
extern "C" {
#endif

#define dcmp6h_get_image(f) Dcmp6H(f)
#define dcmp6l_get_image(f) Dcmp6L(f)
 
/* 6 Bit high resolution image file format support */
extern dcmp6h_header_t *dcmp6h_get_header( FILE *infile);
extern unsigned char *dcmp6h_get_image( FILE *infile);
extern unsigned char *Dcmp6H( FILE *infile);

/* 6 Bit Low resolution image file format support */
extern unsigned char *Dcmp6L( FILE *infile);
extern unsigned char *dcmp6l_get_image( FILE *infile);
#endif

#ifdef __cplusplus
}
#endif
