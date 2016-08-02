// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

/*********************************************************
 * sndgUtils.cc:  Utility routines for printing and 
 *                    swapping sounding spdb data.
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
#include <Spdb/sounding.h>

void SNDG_print_spdb_product(FILE *fptr, 
			     SNDG_spdb_product_t *sounding,
                             int printPoints) 
{
   int i, j, ipoint;
   int dataOffset = sizeof( SNDG_spdb_product_t ) - 
	  sizeof( SNDG_spdb_point_t );

   SNDG_spdb_point_t *dataPtr;
   
   fprintf( fptr, "Sounding Product\n" );
   fprintf( fptr, "  launch time      = %s\n", 
	    utimstr(sounding->launchTime) );
   fprintf( fptr, "  number of points = %d\n", sounding->nPoints );
   fprintf( fptr, "  sourceId         = %d\n", sounding->sourceId ); 
   fprintf( fptr, "  leadSecs         = %d\n", sounding->leadSecs ); 
   
   for( i = 0; i < SNDG_SPARE_INTS; i++ ) {
      fprintf( fptr, "  spare int[%d] = %d\n", 
	       i, sounding->spareInts[i] );
   }
   
   fprintf( fptr, "  lat = %f\n", sounding->lat );
   fprintf( fptr, "  lon = %f\n", sounding->lon );
   fprintf( fptr, "  alt = %f\n", sounding->alt );
   fprintf( fptr, "  missing = %f\n", sounding->missingVal );
   
   for( i = 0; i < SNDG_SPARE_FLOATS; i++ ) {
      fprintf( fptr, "  spare float[%d] = %f\n", 
	       i, sounding->spareFloats[i] );
   }

   fprintf( fptr, "  sourceName = %s\n", sounding->sourceName );
   fprintf( fptr, "  sourceFmt  = %s\n", sounding->sourceFmt );
   fprintf( fptr, "  siteName   = %s\n", sounding->siteName );

   if( printPoints ) {

       dataPtr = (SNDG_spdb_point_t *) ((char *) sounding + dataOffset);
       for( ipoint = 0; ipoint < sounding->nPoints; ipoint++ ) {

	  fprintf( fptr, "  Sounding Point %d\n", ipoint );
	  fprintf( fptr, "    pressure = %f\n", dataPtr->pressure );
	  fprintf( fptr, "    altitude = %f\n", dataPtr->altitude );
	  fprintf( fptr, "    u  wind  = %f\n", dataPtr->u );
	  fprintf( fptr, "    v  wind  = %f\n", dataPtr->v );
	  fprintf( fptr, "    w  wind  = %f\n", dataPtr->w );
	  fprintf( fptr, "    relHum   = %f\n", dataPtr->rh );
	  fprintf( fptr, "    temp     = %f\n", dataPtr->temp );
	  fprintf( fptr, "    div      = %f\n", dataPtr->div );

	  for( j = 0; j < SNDG_PNT_SPARE_FLOATS; j++ ) {
	     fprintf( fptr, "    spare[%d] = %f\n", 
		      j, dataPtr->spareFloats[j] );
	  }

	  dataPtr++;
       }
   }
   
}

void SNDG_spdb_product_to_BE(SNDG_spdb_product_t *sounding) 
{
   int ipoint;
   int nPts        = sounding->nPoints;
   int pointOffset = sizeof( SNDG_spdb_product_t ) - 
	 sizeof( SNDG_spdb_point_t );

   SNDG_spdb_point_t *dataPtr;
   
   /*
    * Swap ints
    */
   sounding->launchTime = BE_from_si32( sounding->launchTime );
   sounding->nPoints    = BE_from_si32( sounding->nPoints );
   sounding->sourceId   = BE_from_si32( sounding->sourceId );
   sounding->leadSecs   = BE_from_si32( sounding->leadSecs );
   
   BE_from_array_32( sounding->spareInts, 
		     SNDG_SPARE_INTS * sizeof(si32) );
   
   /*
    * Swap floats
    */
   BE_from_array_32( &sounding->lat, sizeof(fl32) );
   BE_from_array_32( &sounding->lon, sizeof(fl32) );
   BE_from_array_32( &sounding->alt, sizeof(fl32) );
   BE_from_array_32( &sounding->missingVal, sizeof(fl32) );
   
   BE_from_array_32( sounding->spareFloats, 
		     SNDG_SPARE_FLOATS * sizeof(fl32) );
   
   /*
    * Swap points
    */
   for( ipoint = 0; ipoint < nPts; ipoint++ ) {
      dataPtr = (SNDG_spdb_point_t *)((char *) sounding + pointOffset );
      
      BE_from_array_32( &dataPtr->pressure, sizeof(fl32) );
      BE_from_array_32( &dataPtr->altitude, sizeof(fl32) );
      BE_from_array_32( &dataPtr->u, sizeof(fl32) );
      BE_from_array_32( &dataPtr->v, sizeof(fl32) );
      BE_from_array_32( &dataPtr->w, sizeof(fl32) );
      BE_from_array_32( &dataPtr->rh, sizeof(fl32) );
      BE_from_array_32( &dataPtr->temp, sizeof(fl32) );
      BE_from_array_32( &dataPtr->div, sizeof(fl32) );
      
      BE_from_array_32( dataPtr->spareFloats, 
			SNDG_PNT_SPARE_FLOATS * sizeof(fl32) );

      pointOffset += sizeof( SNDG_spdb_point_t );
   }
}

void SNDG_spdb_product_from_BE(SNDG_spdb_product_t *sounding) 
{
   int ipoint;
   int nPts;
   int pointOffset = sizeof( SNDG_spdb_product_t ) - 
	 sizeof( SNDG_spdb_point_t );

   SNDG_spdb_point_t *dataPtr;
   
   /*
    * Swap ints
    */
   sounding->launchTime = BE_to_si32( sounding->launchTime );
   sounding->nPoints    = BE_to_si32( sounding->nPoints );
   sounding->sourceId   = BE_to_si32( sounding->sourceId );
   sounding->leadSecs   = BE_to_si32( sounding->leadSecs );
   
   BE_to_array_32( sounding->spareInts, 
		   SNDG_SPARE_INTS * sizeof(si32) );
   
   /*
    * Swap floats
    */
   BE_to_array_32( &sounding->lat, sizeof(fl32) );
   BE_to_array_32( &sounding->lon, sizeof(fl32) );
   BE_to_array_32( &sounding->alt, sizeof(fl32) );
   BE_to_array_32( &sounding->missingVal, sizeof(fl32) );
   
   BE_to_array_32( sounding->spareFloats, 
		   SNDG_SPARE_FLOATS * sizeof(fl32) );
   
   /*
    * Swap points
    */
   nPts = sounding->nPoints;
   for( ipoint = 0; ipoint < nPts; ipoint++ ) {
      dataPtr = (SNDG_spdb_point_t *)((char *) sounding + pointOffset );
      
      BE_to_array_32( &dataPtr->pressure, sizeof(fl32) );
      BE_to_array_32( &dataPtr->altitude, sizeof(fl32) );
      BE_to_array_32( &dataPtr->u, sizeof(fl32) );
      BE_to_array_32( &dataPtr->v, sizeof(fl32) );
      BE_to_array_32( &dataPtr->w, sizeof(fl32) );
      BE_to_array_32( &dataPtr->rh, sizeof(fl32) );
      BE_to_array_32( &dataPtr->temp, sizeof(fl32) );
      BE_to_array_32( &dataPtr->div, sizeof(fl32) );
      
      BE_to_array_32( dataPtr->spareFloats, 
		      SNDG_PNT_SPARE_FLOATS * sizeof(fl32) );

      pointOffset += sizeof( SNDG_spdb_point_t );
   }
}
