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
// Niles Oien under time pressure November 2005
//
//


#include <cstdio>
#include <cstdlib>

#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

#include "Params.hh"

using namespace std;

void _updateDir(char *inDir, char *outDir, time_t mdvTime);

int main(int argc, char *argv[])
{

  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  PMU_auto_init("milkDebugfs", P.instance,
                PROCMAP_REGISTER_INTERVAL);

  for (int i=0; i < P.mirrored_directories_n; i++){
    fprintf(stderr,"Mapping %s to %s\n",
	    P._mirrored_directories[i].inputDir,
	    P._mirrored_directories[i].outputDir);
  }

  time_t *lastTime = (time_t *)calloc( P.mirrored_directories_n, 
				       sizeof(time_t));


  if (lastTime == NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }

  while (1){


    //
    // Loop through the directory listings.
    //
    for (int id=0; id < P.mirrored_directories_n; id++){

      //
      // Look at _latest_data_info to get latest time.
      //

      char com[1024];

      sprintf(com, "/sbin/debugfs -c -R \"cat %s/_latest_data_info\" /dev/sdb1 > /tmp/fil",
	      P._mirrored_directories[id].inputDir);

      fprintf(stderr,"Executing : %s\n", com);

      system( com );

      FILE *ifp = fopen("/tmp/fil","r");
      if (ifp == NULL){
	fprintf(stderr,"ERROR - failed to open temp file.\n");
	exit(-1);
      }

      time_t latestTime = 0L;
      fscanf(ifp,"%ld", &latestTime);

      fclose(ifp);

      remove("/tmp/fil");

      if (latestTime > lastTime[id]){
	fprintf(stderr,"Directory %s needs update, data time %s\n",
		P._mirrored_directories[id].inputDir,
		utimstr(latestTime));
	lastTime[id] = latestTime;
	_updateDir(P._mirrored_directories[id].inputDir,
		   P._mirrored_directories[id].outputDir,
		   latestTime);
      }

    }

    //
    // Delay 
    //
    for (int i=0; i < P.delay; i++){
      PMU_auto_register("delay");
      sleep(1);
    }

  }

  free(lastTime);

  return (0);

}


void _updateDir(char *inDir, char *outDir, time_t mdvTime){

  fprintf(stderr,"Updating %s to %s at %s\n",
	  inDir, outDir, utimstr(mdvTime));

  date_time_t T;
  T.unix_time = mdvTime;
  uconvert_from_utime( &T );

  char mdvFilename[1024];
  sprintf(mdvFilename,"%d%02d%02d/%02d%02d%02d.mdv",
	  T.year, T.month, T.day, T.hour, T.min, T.sec);
  //
  // Create the output dated directory.
  //
  char outDirDated[1024];
  sprintf(outDirDated,"%s/%d%02d%02d", outDir, T.year, T.month, T.day);
  
  fprintf(stderr,"Making directory %s\n", outDirDated);

  if (ta_makedir_recurse( outDirDated )){
    fprintf(stderr,"Failed to create directory %s\n", outDirDated);
    return;
  }

  //
  // Assemble the command to put the mdv file in place, and execute it.
  //
  char com[1024];
  sprintf(com, "/sbin/debugfs -c -R \"cat %s/%s\" /dev/sdb1 > %s/%s",
	  inDir, mdvFilename, outDir, mdvFilename );

  fprintf(stderr,"Executing : %s\n", com);

  system(com);


  //
  // Then the latest data info xml file.
  //
  sprintf(com, "/sbin/debugfs -c -R \"cat %s/%s\" /dev/sdb1 > %s/%s",
	  inDir, "_latest_data_info.xml", outDir, "_latest_data_info.xml" );

  fprintf(stderr,"Executing : %s\n", com);

  system(com);


  //
  // And finally the ordinary latest data info file.
  //

  sprintf(com, "/sbin/debugfs -c -R \"cat %s/%s\" /dev/sdb1 > %s/%s",
	  inDir, "_latest_data_info", outDir, "_latest_data_info" );

  fprintf(stderr,"Executing : %s\n", com);

  system(com);

  return;

}








