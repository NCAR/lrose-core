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

/*********************************************************
 * sndgUtils.cc:  Utility routines for printing and 
 *                    swapping sounding chunk data.
 *
 * RAP, NCAR, Boulder CO
 * 
 * December 1998
 * 
 * Jaimi Yee
 *
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/umisc.h>
#include <dataport/port_types.h>
#include <dataport/bigend.h>
#include <rapformats/sounding_chunk.h>

void print_sounding_chunk( FILE *fptr, sounding_chunk_t *soundingChunk ) 
{
   sounding_chunk_from_BE( soundingChunk );
   
   fprintf( fptr, "Sounding Chunk Data:\n" );
   fprintf( fptr, "  data time = %s\n", utimstr( soundingChunk->dataTime ) );
   fprintf( fptr, "  average u = %f\n", soundingChunk->avgU );
   fprintf( fptr, "  average v = %f\n", soundingChunk->avgV );
   fprintf( fptr, "  name      = %s\n", soundingChunk->name );
}

void sounding_chunk_from_BE( sounding_chunk_t *soundingChunk ) 
{
   
   /*
    * Swap ints
    */
   soundingChunk->dataTime = BE_from_si32( soundingChunk->dataTime );

   BE_from_array_32( soundingChunk->spareInts, 
		     SNDG_CHUNK_SPARE_INTS * sizeof(si32) );
   
   /*
    * Swap floats
    */
   BE_from_array_32( &soundingChunk->avgU, sizeof(fl32) );
   BE_from_array_32( &soundingChunk->avgV, sizeof(fl32) );
   
   BE_from_array_32( soundingChunk->spareFloats, 
		     SNDG_CHUNK_SPARE_FLOATS * sizeof(fl32) );
}

void sounding_chunk_to_BE( sounding_chunk_t *soundingChunk ) 
{
   /*
    * Swap ints
    */
   soundingChunk->dataTime = BE_to_si32( soundingChunk->dataTime );

   BE_to_array_32( soundingChunk->spareInts, 
		   SNDG_CHUNK_SPARE_INTS * sizeof(si32) );
   
   /*
    * Swap floats
    */
   BE_to_array_32( &soundingChunk->avgU, sizeof(fl32) );
   BE_to_array_32( &soundingChunk->avgV, sizeof(fl32) );
   
   BE_to_array_32( soundingChunk->spareFloats, 
		   SNDG_CHUNK_SPARE_FLOATS * sizeof(fl32) );
}
