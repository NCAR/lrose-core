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
#include <stdio.h>
#include <dataport/port_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef SNDG_CHUNK_H
#define SNDG_CHUNK_H

#define SNDG_CHUNK_SPARE_INTS 3
#define SNDG_CHUNK_SPARE_FLOATS 2
#define SNDG_CHUNK_NAME_LEN 80

typedef struct 
{
  si32 dataTime;
  si32 spareInts[SNDG_CHUNK_SPARE_INTS];
      
  fl32 avgU;
  fl32 avgV;
  fl32 spareFloats[SNDG_CHUNK_SPARE_FLOATS];
      
  char name[SNDG_CHUNK_NAME_LEN];
} sounding_chunk_t;

/*********************************************************************
 * print_sounding_chunk():  Print a sounding chunk in ASCII format
 *                             to the given stream
 **********************************************************************/
void print_sounding_chunk( FILE *fptr, sounding_chunk_t *soundingChunk );

/**********************************************************************
 * sounding_chunk_to_BE():  Convert a given sounding chunk from
 *                             native format to big-endian format
 **********************************************************************/
void sounding_chunk_to_BE( sounding_chunk_t *soundingChunk );

/***********************************************************************
 * sounding_chunk_from_BE():  Convert a given sounding chunk from
 *                               big-endian format to native format
 ***********************************************************************/
void sounding_chunk_from_BE( sounding_chunk_t *soundingChunk );

#endif

#ifdef __cplusplus
}
#endif
