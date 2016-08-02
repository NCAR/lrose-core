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
// PROCESS_FILE:  Read the file, deconvolute, and put to Spdb
//
//
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>

#include "Params.hh"
#include "WWA2Spdb.hh"

int WWA2Spdb::process_warn(char *start_ptr, int warn_len,DsSpdb &OutSpdb)
{
	NWS_WWA  W;
	int num_points = 0;
	int found = 0;
	char full_ms[1024];

	// Sanity check for warning - Must be at least 20 chars long.
	if(warn_len <= 20) return -1;

	// Find type 
	for(int i=0; i < NUM_NWS_WWA_TYPES && found == 0; i++) {
		// Match string should be on a line all its own
		sprintf(full_ms,"\n%s\015",wwa[i].match_str);

		if(strstr(start_ptr,full_ms)) {
				found = 1;
				W._hdr.hazard_type = wwa[i].code;
				if(P.Debug) fprintf(stderr,"Found Warn Type %d, %s\n",i,wwa[i].match_str);
		}
	}
	if(found == 0) return -1;

	// Now search for the /O.XXX.XXXX 
	char *oc_line;

	if((oc_line = strstr(start_ptr,"/O.")) == NULL) {
		if(P.Debug) fprintf(stderr,"Warning Found No /O. line\n");
		return -1;
	}

	UTIMstruct it;
	UTIMstruct et;
	char buf[32]; // Temp parsing buffer
	memset(buf,0,32); // Clean out buffer

	// Grab Issuing FO ID
	memcpy(buf,oc_line+7,4);
	int Fo_Id = Spdb::hash4CharsToInt32(buf);
	if(P.Debug) fprintf(stderr,"    Station: %s\n",buf);

	memset(buf,0,32); // Clean out buffer

        /* Reference : http://weather.gov/os/vtec/pdfs/VTEC_explanation6.pdf
	   Actions string meaning:
	   NEW New event
	   CON Event continued
	   EXT Event extended (time)
	   EXA Event extended (area)
	   EXB Event extended (both time and area)
	   UPG Event upgraded
	   CAN Event cancelled
	   EXP Event expired
	   COR Correction
	   ROU Routine
	*/

	// Grab Actions type
	memcpy(buf,oc_line+3,3);
	string A_type;
	A_type = buf;
	if(A_type == "NEW" || A_type == "CAN" || A_type == "CON"){
	  cerr << "Action type: " << A_type << endl;
	  cerr << "Future work to use this\n";
	  cerr << "information properly.\n";
	}
	
	memset(buf,0,32); // Clean out buffer

	// Grab Expire Time
	memcpy(buf,oc_line+35,2);
	et.year = atoi(buf) + 2000; 

	memcpy(buf,oc_line+37,2);
	et.month = atoi(buf);

	memcpy(buf,oc_line+39,2);
	et.day = atoi(buf);

	memcpy(buf,oc_line+42,2);
	et.hour = atoi(buf);

	memcpy(buf,oc_line+44,2);
	et.min = atoi(buf);

	et.sec = 0;
	W._hdr.expire_time = UTIMdate_to_unix(&et);

	
	// Grab Issue Time
	memcpy(buf,oc_line+22,2);
	it.year = atoi(buf) + 2000; 

	memcpy(buf,oc_line+24,2);
	it.month = atoi(buf);

	memcpy(buf,oc_line+26,2);
	it.day = atoi(buf);

	memcpy(buf,oc_line+29,2);
	it.hour = atoi(buf);

	memcpy(buf,oc_line+31,2);
	it.min = atoi(buf);

	it.sec = 0;
	if( it.month == 0 || it.day == 0 ) {
		  // Grab it from the second line Instead.
		  char *line;
		  line = strstr(start_ptr,"\r\r\n"); // first line
		  line = strstr(line+3,"\r\r\n"); // second line
		  it.year = et.year;
		  it.month = et.month;
		  line += 15;  // to start of date on 
		  memcpy(buf,line,2);
		  it.day = atoi(buf);
	      memcpy(buf,line+2,2);
	      it.hour = atoi(buf);
	      memcpy(buf,line+4,2);
	      it.min = atoi(buf);
	}
	W._hdr.issue_time = UTIMdate_to_unix(&it);
	
	if(P.Debug) {
	  DateTime issue_time;
	  DateTime expire_time;
	  cerr << "Issue Time = " << issue_time.str(W._hdr.issue_time) << endl;
	  cerr << "Expire Time = " << expire_time.str(W._hdr.expire_time) << endl;
	}	

	char *ll_line;  // Lat lon line.
	char *ll_line_end;  // Lat lon line.
	if((ll_line = strstr(start_ptr,"LAT...LON ")) == NULL ) {
	  if(P.Debug){ 
	    cerr << "ERROR: Found no LAT...LON line\n";
	  }
	  return -1;
	}
	if((ll_line_end = strstr(ll_line,"TIME...MOT...LOC")) == NULL) {
	  if((ll_line_end = strstr(ll_line,"$$")) == NULL) {
	    if(P.Debug) fprintf(stderr,"ERROR: Found No Ending line in LAT...LON line\n");
	    return -1;
	  }
	}
	
	ll_line += 10; // Skip to beginning of first Latitude

	num_points = 0;
	while (ll_line < ll_line_end && num_points < NWS_WWA_MAX_POINTS) { // Parse LAT LON Text
	     memset(buf,0,32); // Clean out buffer
	     memcpy(buf,ll_line,4);
		 W._hdr.wpt[num_points].lat = atoi(buf) / 100.0;
		 // Skip over latitude
		 while(isdigit(*ll_line) && ll_line < ll_line_end) ll_line++;
		 // Skip over white space
		 while(isspace(*ll_line) && ll_line < ll_line_end) ll_line++;
	     memset(buf,0,32); // Clean out buffer
	     memcpy(buf,ll_line,5);
		 W._hdr.wpt[num_points].lon = atoi(buf) / -100.0;
		 // Skip over longitude
		 while(isdigit(*ll_line) && ll_line < ll_line_end) ll_line++;
		 num_points++;
		 while(!isdigit(*ll_line) && ll_line < ll_line_end) ll_line++;
	}

	W._hdr.num_points = num_points;
	W._hdr.lat_min = 90.0;
	W._hdr.lat_max= -90.0;
	W._hdr.lon_min = 900.0;
	W._hdr.lon_max= -900.0;

	// Find bounding Box
	for(int i = 0; i < num_points; i++ ) {
		if(W._hdr.wpt[i].lat < W._hdr.lat_min) W._hdr.lat_min = W._hdr.wpt[i].lat;
		if(W._hdr.wpt[i].lat > W._hdr.lat_max) W._hdr.lat_max = W._hdr.wpt[i].lat;
		if(W._hdr.wpt[i].lon < W._hdr.lon_min) W._hdr.lon_min = W._hdr.wpt[i].lon;
		if(W._hdr.wpt[i].lon > W._hdr.lon_max) W._hdr.lon_max = W._hdr.wpt[i].lon;

	    if(P.Debug) fprintf(stderr,"Point %d: %g, %g\n",i,W._hdr.wpt[i].lat,W._hdr.wpt[i].lon);
	}
	W._hdr.text_length = warn_len;

	char *mo_line;  // TIME...MOT...LOC  Motion line.
	char *mo_field_end;
	char *mo_line_end;  
	if((mo_line = strstr(start_ptr,"TIME...MOT...LOC ")) == NULL ) {
	  if(P.Debug) fprintf(stderr,"Warning: Found No TIME...MOT...LOC line\n");
	  W._hdr.motion_time =  NULL;
	  W._hdr.motion_dir = NULL;
	  W._hdr.motion_kts = NULL;
	  W._hdr.motion_lat = NULL;
	  W._hdr.motion_lon = NULL;
	}
	else if((mo_line_end = strstr(mo_line,"$$")) == NULL) {
	  if(P.Debug) fprintf(stderr,"Warning: Found No Ending line in TIME...MOT...LOC line\n");
	  W._hdr.motion_time =  NULL;
	  W._hdr.motion_dir = NULL;
	  W._hdr.motion_kts = NULL;
	  W._hdr.motion_lat = NULL;
	  W._hdr.motion_lon = NULL;
	} else {
	  
	  mo_line += 17; // Skip to beginning of Time reference.
	  memset(buf,0,32); // Clean out buffer
	  memcpy(buf,mo_line,2);
	  it.hour = atoi(buf);
	  memcpy(buf,mo_line+2,2);
	  it.min = atoi(buf);
	  W._hdr.motion_time =  UTIMdate_to_unix(&it);
	
	  mo_line += 6; // Skip to beginning of Direction reference.
	  memset(buf,0,32); // Clean out buffer
	  memcpy(buf,mo_line,3);
	  W._hdr.motion_dir = atoi(buf);

	  mo_line += 6; // Skip to beginning of Speed reference.
	  if((mo_field_end = strstr(mo_line,"KT")) == NULL) {
	    if(P.Debug) fprintf(stderr,"Warning: Found No SPEED (KT) in TIME...MOT...LOC line\n");
	    W._hdr.motion_kts = NULL;
	    W._hdr.motion_lat = NULL;
	    W._hdr.motion_lon = NULL;
//	    return -1;
	  } else {
	    
	    int spd_size = mo_field_end - mo_line;
	    memset(buf,0,32); // Clean out buffer
	    memcpy(buf,mo_line,spd_size);
	    W._hdr.motion_kts = atoi(buf);
	    mo_line += spd_size + 3; // Skip to beginning of Lat Lons.

	    memset(buf,0,32); // Clean out buffer
	    memcpy(buf,mo_line,4);
	    W._hdr.motion_lat = atoi(buf) / 100.0;
	    // Skip over latitude
	    while(isdigit(*mo_line) && mo_line < mo_line_end) mo_line++;
	    // Skip over white space
	    while(isspace(*mo_line) && mo_line < mo_line_end) mo_line++;
	
	    memset(buf,0,32); // Clean out buffer
	    memcpy(buf,mo_line,5);
	    W._hdr.motion_lon = atoi(buf) / -100.0;
	  }
	}
	
	char *t_ptr = start_ptr;
	for(int i= 0; i < warn_len; i++) {
		if(*t_ptr < ' ' && *t_ptr != '\n') *t_ptr = ' '; //replace control chars with spaces
		if(*t_ptr > '~' ) *t_ptr = ' '; //replace Non-ascii chars with spaces
		t_ptr++;
	}
	start_ptr[warn_len -1] = '\0'; // Last character forced to null

	W._text = start_ptr;

	W.assemble(); // Build output data chunk 

    OutSpdb.addPutChunk(Fo_Id,W._hdr.issue_time,W._hdr.expire_time,
			            W.getBufLen(), W.getBufPtr(),0);
	return 0;
}
