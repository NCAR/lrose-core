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

//
// Small program to rename REAL lidar files to have the date in them.
//
#include <cstdio>
#include <toolsa/umisc.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  //
  // argv[1] needs to be set to lidar filename
  //
  if (argc < 3){
    fprintf(stderr, "USAGE : %s <real lidar filename> <output directory> [filename tag, defaults to REAL]\n", argv[0]);
    return -1;
  }

  //
  // Open the input file, read the unix time, close file
  //
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) return -1;

  //
  // If there is a "tag name" for the files, use it, otherwise
  // default.
  //
  char tagName[1024];
  sprintf(tagName,"%s", "REAL");
  if (argc > 3) sprintf(tagName, "%s", argv[3]);

  date_time_t firstBeamTime;

  if (1 != fread(&firstBeamTime.unix_time, sizeof(firstBeamTime.unix_time), 
		 1, fp)) return -1;

  fclose(fp);

  //
  // Convert the time fields
  //
  uconvert_from_utime( &firstBeamTime );

  //
  // Output directory name
  //
  char *dirName = argv[2];

  //
  // Generate the output file name
  //
  char outFileName[1024];
  sprintf(outFileName, "%s/%s_%d%02d%02d_%02d%02d%02d.lidar",
	  dirName, tagName,
	  firstBeamTime.year, firstBeamTime.month, firstBeamTime.day,
	  firstBeamTime.hour, firstBeamTime.min, firstBeamTime.sec);

  //
  // Move the file
  //
  char com[1024];
  sprintf(com, "/bin/mkdir -p %s", dirName);
  system(com);

  sprintf(com, "/bin/cp -f %s %s", argv[1], outFileName);
  fprintf(stderr, "Renaming lidar file, executing '%s'\n", com);
  return system(com);

}
