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
//
// This is a horrible, quick-and-dirty program written
// to satisfy a pressing need in the Pentagon Shield project.
//
//
// This version for a flat directory structure rather than an MDV deal
//
// Niles Oien under time pressure November 2005
//
//

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

#include "Params.hh"

using namespace std;

void _updateFile(char *inDir, char *outDir, char *fileName);

int main(int argc, char *argv[]){

  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  PMU_auto_init("milkDebugfsFlat", P.instance,
                PROCMAP_REGISTER_INTERVAL);

  for (int i=0; i < P.mirrored_directories_n; i++){
    fprintf(stderr,"Mapping flat directory %s to %s\n",
	    P._mirrored_directories[i].inputDir,
	    P._mirrored_directories[i].outputDir);
  }

  while (1){

    //
    // Loop through the directory listings.
    //
    for (int id=0; id < P.mirrored_directories_n; id++){

      //
      // Do a directory listing to see what's there.
      //

      char com[1024];

      sprintf(com, "/sbin/debugfs -c -R \"ls %s\" /dev/sdb1 > /tmp/fil",
	      P._mirrored_directories[id].inputDir);

      fprintf(stderr,"Executing : %s\n", com);

      system( com );

      //      FILE *ifp = fopen("/tmp/fil","r");
      FILE *ifp = fopen("./out.dat","r");

      if (ifp == NULL){
	fprintf(stderr,"ERROR - failed to open temp file.\n");
	exit(-1);
      }
      
      char Line[1024];
      memset(Line, 0, 1024);
      while (NULL != fgets(Line, 1024, ifp)){
	_updateFile(P._mirrored_directories[id].inputDir,
		    P._mirrored_directories[id].outputDir,
		    Line);
      }
      
      fclose(ifp);
      
      remove("/tmp/fil");
      
    }
    
    //
    // Delay 
    //
    for (int i=0; i < P.delay; i++){
      PMU_auto_register("delay");
      sleep(1);
    }
    
  }


  return (0);

}


void _updateFile(char *inDir, char *outDir, char *fileName){


  //
  // Because we got the filename with fgets() get rid of trailing \n, spaces
  //
  int ilen = strlen( fileName );

  for (int i=ilen; i > -1; i--){
    if (int(fileName[i]) < 33){
      fileName[i] = char(0);
    } else {
      fileName[i+1]=char(0);
      break;
    }
  }

  //
  // And truncate off leading spaces.
  //
  ilen = strlen( fileName );
  int istart = 0;
  for (int ik=0; ik < ilen; ik++){
    if (((int) fileName[ik]) < 33){
      istart = ik + 1;
      // fprintf(stderr,"%d %d \"%s\"\n", ik, istart, fileName + istart);
    }
  }


  fprintf(stderr,"Updating %s to %s with file %s\n",
	  inDir, outDir, fileName + istart);


  //
  // See if the output filename exists, if so, return - no updates.
  //
  char outFilename[1024];
  sprintf(outFilename,"%s/%s", outDir, fileName + istart );
  
  struct stat buf;
  if (stat(outFilename, &buf) == 0){
    fprintf(stderr," File %s exists, not updating.\n", outFilename);
    return;
  }

  char inFilename[1024];
  sprintf(inFilename,"%s/%s", inDir, fileName + istart);


  if (ta_makedir_recurse( outDir )){
    fprintf(stderr,"Failed to create directory %s\n", outDir);
    return;
  }

  //
  // Assemble the command to put the file in place, and execute it.
  //
  char com[1024];
  sprintf(com, "/sbin/debugfs -c -R \"cat %s\" /dev/sdb1 > %s",
	  inFilename, outFilename );

  fprintf(stderr,"Executing : %s\n", com);

  system(com);


  return;

}








