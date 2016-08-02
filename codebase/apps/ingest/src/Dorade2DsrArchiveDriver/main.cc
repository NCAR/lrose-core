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


#include<cstdio>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <unistd.h>

#include "Params.hh"

int main(int argc, char *argv[]){

  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"TDRP problem.\n");
    return -1;
  }       


  //
  // I used to get the file list with opendir/readdir but
  // it turns out that the order the files are red in is
  // highly variable. I now use a temporary file.
  //
  char dircom[1024];
  sprintf(dircom,"/bin/ls -C1 %s > %s", params.inputDir,
	  params.tempFile);
  if (system(dircom)){
    fprintf(stderr, "Command %s failed.\n", dircom);
    exit(-1);
  }


  int lastVolNum = -1;

  int numInBuffer = 0;
  char com[8192];
  sprintf(com, "Dorade2Dsr -params %s -f ", 
	  params.doradeParamFile);

  system( params.keepDsr2VolRunningScript );
  sleep( 1 );

  //
  // We need to check for a special case in which the volume
  // number never changes.
  //
  int first = 1;
  int numVolNumChanges = 0;
  int numValidFiles = 0;

  FILE *ifp = fopen( params.tempFile, "r" );
  
  char Line[1024];
  while (fgets(Line, 1024, ifp) != NULL)
  {

    char fullName[1024];
    sprintf(fullName,"%s/%s", params.inputDir, Line);

    fullName[strlen(fullName)-1]=char(0); // Chomp trailing return.


    //
    // Parse out the current volume number.
    // File naming convention is swp.1070630141224.DOW2cops.192.7.9_SUR_v6
    // 'PPI' may be used instead of 'SUR'
    //

    char *p = strstr(fullName, "_SUR_v");
    if (p == NULL){
      p = strstr(fullName, "_PPI_v");
      if (p == NULL) continue;
    }

    int currentVolNum = 0;
    if (1 != sscanf(p+strlen("_SUR_v"), "%d", &currentVolNum)) continue;

    numValidFiles++;

    if (first){
      lastVolNum = currentVolNum;
      first = 0;
    } else {
      if (lastVolNum != currentVolNum){
	numVolNumChanges++;
	lastVolNum = currentVolNum;
      }
    }

  }

  fclose(ifp);

  double percentChange = 100.0*double(numVolNumChanges)/double(numValidFiles);
  fprintf(stderr,"Percent of volume number changes : %g\n", percentChange);

  int volNumConst = 0;
  if (percentChange < 5) volNumConst = 1;

  lastVolNum = -1;

  if (volNumConst){
    fprintf(stderr,"The volume number is constant, treating as one file / volume\n");
  } else {
    fprintf(stderr,"Volume number varies.\n");
  }





  ifp = fopen( params.tempFile, "r" );
  
  while (fgets(Line, 1024, ifp) != NULL)
  {

    char fullName[1024];
    sprintf(fullName,"%s/%s", params.inputDir, Line);

    fullName[strlen(fullName)-1]=char(0); // Chomp trailing return.

    fprintf(stderr,"%s\n", fullName);

    //
    // Parse out the current volume number.
    // File naming convention is swp.1070630141224.DOW2cops.192.7.9_SUR_v6
    // 'PPI' may be used instead of 'SUR'
    //

    char *p = strstr(fullName, "_SUR_v");
    if (p == NULL){
      p = strstr(fullName, "_PPI_v");
      if (p == NULL) continue;
    }

    int currentVolNum = 0;
    if (1 != sscanf(p+strlen("_SUR_v"), "%d", &currentVolNum)) continue;

    if (lastVolNum == -1){
      lastVolNum = currentVolNum;
    }

    if ((numInBuffer > 0) && ((volNumConst) || (lastVolNum != currentVolNum))){
      fprintf(stderr,"Executing %s\n", com);
      system(com);

      sleep( params.sleepTime );
      system( params.keepDsr2VolRunningScript );

      sprintf(com, "Dorade2Dsr -params %s -f ", 
	      params.doradeParamFile);

      numInBuffer = 0;
      lastVolNum = currentVolNum;
    }

    sprintf(com, "%s %s", com, fullName);
    numInBuffer++;

  }

  fclose(ifp);

  if (numInBuffer){
    fprintf(stderr,"Executing %s\n", com);
    system(com);
  }


  unlink( params.tempFile );

  return 0;

}
