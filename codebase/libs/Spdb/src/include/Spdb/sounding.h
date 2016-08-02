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
/**
 * @file sounding.h
 * @brief Primitive sounding definitions
 */
#include <stdio.h>
#include <dataport/port_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef SNDG_H
#define SNDG_H

#define SNDG_PNT_SPARE_FLOATS 2
#define SNDG_SPARE_INTS 2
#define SNDG_SPARE_FLOATS 2

#define SOURCE_NAME_LENGTH 80
#define SOURCE_FMT_LENGTH  40
#define SITE_NAME_LENGTH   80

#define SNDG_HDR_N_CHAR (SOURCE_NAME_LENGTH + \
                         SOURCE_FMT_LENGTH + \
                         SITE_NAME_LENGTH)

#define SNDG_VALUE_UNKNOWN -9999.0

/*
 * The following structures define the sounding product as it
 * appears in the spdb database
 */

typedef struct 
{
   fl32 pressure;
   fl32 altitude;
   fl32 u;
   fl32 v;
   fl32 w;
   fl32 rh;
   fl32 temp;
   fl32 div; /* divergence s-1 * 1.0e5 */
   fl32 spareFloats[SNDG_PNT_SPARE_FLOATS];

} SNDG_spdb_point_t;

typedef struct 
{
   si32 launchTime;
   si32 nPoints;
   si32 sourceId;
   si32 leadSecs;
   si32 spareInts[SNDG_SPARE_INTS];
   
   fl32 lat;
   fl32 lon;
   fl32 alt;
   fl32 missingVal;
   fl32 spareFloats[SNDG_SPARE_FLOATS];
   
   char sourceName[SOURCE_NAME_LENGTH];
   char sourceFmt[SOURCE_FMT_LENGTH];
   char siteName[SITE_NAME_LENGTH];

   SNDG_spdb_point_t points[1];

} SNDG_spdb_product_t;


/*************************************************
 * Function prototypes
 ************************************************/

/*********************************************************************
 * SNDG_print_spdb_product():  Print an SPDB sounding in ASCII format
 *                             to the given stream
 **********************************************************************/
void SNDG_print_spdb_product(FILE *stream, 
                             SNDG_spdb_product_t *sounding,
                             int printPoints);

/**********************************************************************
 * SNDG_spdb_product_to_BE():  Convert a given sounding product from
 *                             native format to big-endian format
 **********************************************************************/
void SNDG_spdb_product_to_BE(SNDG_spdb_product_t *sounding);

/***********************************************************************
 * SNDG_spdb_product_from_BE():  Convert a given sounding product from
 *                               big-endian format to native format
 ***********************************************************************/
void SNDG_spdb_product_from_BE(SNDG_spdb_product_t *sounding);

#endif

#ifdef __cplusplus
}
#endif
