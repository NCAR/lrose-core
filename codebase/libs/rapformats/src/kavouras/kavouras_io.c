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
/******************************************************************************
 *  KAVOURAS_IO.C  Subroutines useful for accessing KAVOURAS data.
 *
 *  F. Hage.  Feb 1995. RAP
 *
 */


#define KAVOURAS_IO_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <toolsa/os_config.h>
#include <rapformats/kavouras_io.h>
#include <dataport/bigend.h>

/* Elevation angles as of 1/96 - F.H */
double  Elev_angles_vcp11[] = {0.5, 0.5, 1.45, 1.45, 2.4, 3.35, 4.3, 5.25, 6.2, 7.5, 8.7, 10.0, 12.0, 14.0, 16.7, 19.5};
double  Elev_angles_vcp21[] = {0.5, 0.5, 1.45, 1.45, 2.4, 3.35, 4.3, 6.0, 9.9, 14.6, 19.5};
double  Elev_angles_vcp31[] = {0.5, 0.5, 1.5, 1.5, 2.5, 3.5, 4.5};
double  Elev_angles_vcp32[] = {0.5, 0.5, 1.5, 1.5, 2.5, 3.5, 4.5};

/******************************************************************************
 * DCMP6H_GET_HEADER: Load data file header from the file and return a pointer
 *  to the header
 */

#define KAV_HEADER_OFFSET -128   /* Distance from the end of the file */
#if defined(__linux)
extern void swab(const void *from, void *to, ssize_t n);
#endif

dcmp6h_header_t *dcmp6h_get_header( FILE *infile)
{
    static dcmp6h_header_t head;

    if (fseek(infile,KAV_HEADER_OFFSET,2)) {
        return NULL;
    }

    if((fread(&head,sizeof(dcmp6h_header_t),1,infile)) != 1) {
        return NULL;
    }

    if(BE_is_big_endian()) swab(&head,&head,sizeof(head));

    return &head;
}

