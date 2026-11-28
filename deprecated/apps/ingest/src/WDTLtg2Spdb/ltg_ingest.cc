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
// ltg_ingest:  Read the file, and put to Spdb
//
//
#include <stdio.h>
#include <time.h>
#include <rapformats/ltg.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <Spdb/DsSpdb.hh>

#include "Params.hh"

#define LINE_LENGTH 256

int ltg_ingest(char *in_file, Params *P)
{
	char	buffer[LINE_LENGTH];
	char	buf2[LINE_LENGTH];

	int		count;
	float	latitude, longitude;
	int	amplitude;

	UTIMstruct t;

	FILE	*source_fd = NULL;

  if ((source_fd = fopen(in_file, "r")) == NULL) {
    fprintf (stderr,"Could not open input file %s\n",in_file);
    return (1);
  }

  /* Get rid of the ccb Header */
  fgets(buffer, LINE_LENGTH, source_fd);

  // Init the SPDB data
  DsSpdb OutSpdb;
  OutSpdb.clearPutChunks();
  OutSpdb.clearUrls();
  OutSpdb.addUrl(P->OutputUrl);

  /* Read lightning data. */
  while (fgets(buffer, LINE_LENGTH, source_fd) != NULL) {
      if (P->Debug) {
	      fputs("\nInput: ",stderr);
	      fputs(buffer,stderr);
	  }

	  strncpy(buf2,buffer,4); // Pick apart time field
	  buf2[4] = '\0';
	  t.year = atoi(buf2);

	  strncpy(buf2,buffer+5,2);
	  buf2[2] = '\0';
	  t.month = atoi(buf2);

	  strncpy(buf2,buffer+8,2);
	  buf2[2] = '\0';
	  t.day = atoi(buf2);

	  strncpy(buf2,buffer+11,2);
	  buf2[2] = '\0';
	  t.hour = atoi(buf2);

	  strncpy(buf2,buffer+14,2);
	  buf2[2] = '\0';
	  t.min = atoi(buf2);

	  strncpy(buf2,buffer+17,2);
	  buf2[2] = '\0';
	  t.sec = atoi(buf2);

	  if(sscanf((buffer+20),"%f , %f , %d, %d",&latitude,&longitude,&amplitude,&count) != 4) {
		  fprintf(stderr,"Problems parsing line: %s\n",buffer);
	  }

	  UTIMdate_to_unix(&t);

	  //fprintf(stderr,"%d-%d-%d, %d:%d:%d\t",t.year,t.month, t.day,t.hour,t.min,t.sec);
	  //fprintf(stderr,"Lat, Lon:%f, %f  Amplitude: %d\n",latitude,longitude,amplitude);

      // If Strike is within our bounding box - Add it to the database
      if( (longitude > P->MinLon) && (longitude < P->MaxLon) &&
	  (latitude > P->MinLat) && (latitude < P->MaxLat) ) { 

	  time_t now; 

	  now = time(0);

	// Reject data more than one hour old when in real-time mode.
	if((P->Mode == Params::REALTIME) && (t.unix_time < (now - 3600))) continue;

         LTG_strike_t strk;
         strk.type =  LTG_TYPE_UNKNOWN;
         strk.time = t.unix_time;
         strk.longitude = longitude;
         strk.latitude = latitude;
	     strk.amplitude = amplitude;
		 if (P->Debug)
		   fprintf(stderr, "Strike: Latlon: %g, %g , %d - %s",
		   strk.latitude, strk.longitude, strk.amplitude,ctime((time_t *) &t.unix_time));


	 // Assign ID's based on Location 
	 int dataType, dataType2;
	 dataType = (int) rint( (strk.latitude + 90.0) * 100.0);
	 dataType2 = (int) rint( (strk.longitude + 180.0) * 100.0);

	 LTG_to_BE( &strk );

	 OutSpdb.addPutChunk( dataType,t.unix_time, t.unix_time + P->ExpireSecs,
			     sizeof(strk),  (void *) &strk, dataType2);


      } // Is within the bounding box 

  }/* While there is still data */

  // Commit the data to the database
  OutSpdb.put( SPDB_LTG_ID, SPDB_LTG_LABEL);

  if (source_fd) fclose(source_fd);
   
  return 0;
}
