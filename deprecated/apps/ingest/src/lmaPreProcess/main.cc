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


#include <cstdio>
#include <dirent.h>
#include <cstring>
#include <toolsa/umisc.h>
#include <stdlib.h>

#include "Params.hh"

int main(int argc, char *argv[]){

  // Read in TDRP params.

  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr, "Specify params file with -params option.\n");
    return -1;
  }   

  // Parse out start, end dates.

  date_time_t startTime, endTime;

  startTime.hour = startTime.min = startTime.sec = 0;

  endTime.hour = 23;
  endTime.min = 59; 
  endTime.sec = 59;

  if (6 != sscanf(params.timeRange, "%d/%d/%d %d/%d/%d",
		  &startTime.year, &startTime.month, &startTime.day,
		  &endTime.year,   &endTime.month,   &endTime.day)){
    fprintf(stderr, "Cannot decode time range %s\n", params.timeRange);
    return -1;
  }

  uconvert_to_utime( &startTime );
  uconvert_to_utime( &endTime );

  // Scan the directory. Use the scandir function to make sure the
  // entries are in order.

  struct dirent **namelist;
  int numFiles = scandir(params.inDir, &namelist, 0, alphasort);

  if (numFiles < 0)
  {
    fprintf(stderr, "Failed to open input directory.\n");
    perror(params.inDir);
    return -1;
  }

  // Open output file.

  FILE *ofp =fopen(params.outFile,"w");
  if (ofp == NULL){
    fprintf(stderr,"Failed to create %s\n", params.outFile);
    return -1;
  }

  FILE *ofp2 =fopen(params.listFile,"w");
  if (ofp2 == NULL){
    fprintf(stderr,"Failed to create %s\n", params.listFile);
    return -1;
  }

  
  unsigned long numPoints = 0L;
  unsigned long numFilesUsed = 0L;

  // Loop through input files. Only actually open them if
  // their name indicates that they are in the range we want.
  
  for (int iFile=0; iFile <numFiles; iFile++){

    date_time_t fileTime;

    if (3==sscanf(namelist[iFile]->d_name, "LYLOUT_%2d%2d%2d",
		  &fileTime.year, &fileTime.month, &fileTime.day)){

      if (params.verbose) fprintf(stderr, "Considering %s\n", namelist[iFile]->d_name);

      fileTime.year += 2000;
      fileTime.hour = 12;
      fileTime.min = fileTime.sec = 0;

      uconvert_to_utime( &fileTime );

      if (
	  (fileTime.unix_time > endTime.unix_time) ||
	  (fileTime.unix_time < startTime.unix_time)
	  ){

	if (params.verbose) fprintf(stderr, "%s outside time range\n", namelist[iFile]->d_name);

      } else {

	char fullName[1024];
	sprintf(fullName,"%s/%s", params.inDir, namelist[iFile]->d_name);

	if (params.verbose) fprintf(stderr,"%s in time range, processing\n", fullName);

	// Process this file into the output file.

	fprintf(ofp2,"%s\n", fullName);
	numFilesUsed++;

	FILE *fp = fopen(fullName,"r");
	
	char line[1024];
	
	while (NULL != fgets(line, 1024, fp)){
	  
	  double timSec, lat, lon, alt;
	  
	  if (4==sscanf(line, "%lf %lf %lf %lf",
			&timSec, &lat, &lon, &alt)){
	    
	    date_time_t dataTime;
	    dataTime.year  = fileTime.year;
	    dataTime.month = fileTime.month;
	    dataTime.day   = fileTime.day;
	    dataTime.hour = dataTime.min = dataTime.sec = 0;

	    uconvert_to_utime( &dataTime );
	    dataTime.unix_time += (int)rint(timSec);
	    uconvert_from_utime( &dataTime );
	    
	    numPoints++;
	    fprintf(ofp, "%02d/%02d/%02d %02d:%02d:%02d %g %g %g\n",
		    dataTime.month, dataTime.day, dataTime.year%100,
		    dataTime.hour, dataTime.min, dataTime.sec,
		    lat, lon, alt);
	    
	  }
	}
	fclose(fp);
      }
    }
  }

  fclose(ofp); fclose(ofp2);

  // Free up the scandir memory.

  for (int iFile=0; iFile <numFiles; iFile++){
    free(namelist[iFile]);
  }
  free(namelist);

  fprintf(stdout,"%ld points in %s from %ld input files.\n", 
	  numPoints, params.outFile, numFilesUsed);

  return 0;

}
