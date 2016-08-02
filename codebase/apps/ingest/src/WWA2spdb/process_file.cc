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
// WWA2Spdb::process_file:  Read the file, deconvolute, and put to Spdb
//
//
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>

#include <rapformats/NWS_WWA.hh>

#include "Params.hh"

#include "WWA2Spdb.hh"

int WWA2Spdb::process_file(char *file_name)
{
	FILE *input_file = NULL;
	char *file_buf = NULL;
	struct stat sbuf;

	// Open File
	if ((input_file = fopen(file_name, "r")) == NULL) {
		fprintf (stderr,"Could not open input file %s\n",file_name);
		perror("process_file");
		return (1);
	}
	
	// Find its size
	if (lstat(file_name, &sbuf ) != 0) {
		fprintf (stderr,"Could not stat input file %s\n",file_name);
		perror("process_file");
	    if (input_file) fclose(input_file);
		return (1);
    }

	// Allocate space for file in memory --  1 char larger than file
	if((file_buf = (char *) calloc(sbuf.st_size+1,1)) == NULL) {
		fprintf (stderr,"Could not Allocate buffer for input file %s\n",file_name);
		perror("process_file");
	    if (input_file) fclose(input_file);
		return (1);
	}

	// Read in the file
	if(fread(file_buf,1,sbuf.st_size,input_file) != sbuf.st_size ) {
		fprintf (stderr,"Could not Read input file %s\n",file_name);
		perror("process_file");
	    if (input_file) fclose(input_file);
		if(file_buf) free(file_buf);
		return (1);
	}
	file_buf[sbuf.st_size] = '\0'; // Make sure file has a null termination.

	if(P.Debug) {
		time_t now = time(0);
		fprintf(stderr,"Processing %s at %s\n",file_name,ctime(&now));
	}


	// Init the SPDB data
	DsSpdb OutSpdb;
	OutSpdb.clearPutChunks();
	OutSpdb.clearUrls();
	OutSpdb.addUrl(P.OutputUrl);

	int file_pos = 0;
	int warn_len = 0;
	char *start_ptr = NULL;
	char *end_ptr = NULL;
	if((start_ptr = strchr(file_buf,'')) == NULL) {
	    start_ptr = file_buf;
	} else {
		start_ptr += 1; // skip over the ^A
	}
	while (file_pos < sbuf.st_size) {
		// Look for the end of record
		if((end_ptr = strchr(start_ptr,'')) == NULL) { 
			end_ptr = file_buf + sbuf.st_size; // Must be the end, otherwise
		} 
		*end_ptr = '\000'; // Null terminate
		file_pos = end_ptr - file_buf + 1; 
		warn_len  = end_ptr - start_ptr;
		process_warn(start_ptr,warn_len,OutSpdb);
		if((start_ptr = strchr(end_ptr,'')) == NULL) {
			start_ptr = end_ptr +1;
		} else {
		}  start_ptr += 1; // skip over the ^A
	}

	// Commit the data to the database, close the file, and exit
	//
	OutSpdb.put( SPDB_NWS_WWA_ID, SPDB_NWS_WWA_LABEL);

	if (input_file) fclose(input_file);
	if(file_buf) free(file_buf);
   
	return 0;
}
