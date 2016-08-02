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


#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <toolsa/DateTime.hh>


#include <stdio.h>
#include <time.h>
#include <toolsa/umisc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include "Params.hh"
#include "FileLister.hh"
#include "trecCast.hh"

////////////////////////////////////////////
//
// Constructor
//
FileLister::FileLister(){

  startTime = DateTime::NEVER;
  endTime = DateTime::NEVER;

}

///////////////////////////////////////////////////
//
// Initialization
//
int FileLister::init( int argc, char **argv, Params *P )
{


  PMU_auto_init(argv[0],P->Instance,
		PROCMAP_REGISTER_INTERVAL);

  //
  // Process the command line arguments - just
  // the start and end times.
  //
  for(int i=1; i<argc; i++){
    date_time_t T;

    if ( !strcmp(argv[i], "--" ) ||
	 !strcmp(argv[i], "-h" ) ||
	 !strcmp(argv[i], "-help" ) ||
	 !strcmp(argv[i], "-man" )) {
      usage();
      return -1;
    }
   
    if (!(strcmp(argv[i],"-start"))){
      i++;
      if (6!=sscanf(argv[i],"%d %d %d %d %d %d",
		   &T.year, &T.month, &T.day,
		   &T.hour, &T.min, &T.sec)){
	fprintf(stderr,"Start time not specified correctly.\n");
	usage();
	return -1;
      }
      uconvert_to_utime(&T);
      startTime = T.unix_time;
    }

    if (!(strcmp(argv[i],"-end"))){
      i++;
      if (6!=sscanf(argv[i],"%d %d %d %d %d %d",
		   &T.year, &T.month, &T.day,
		   &T.hour, &T.min, &T.sec)){
	fprintf(stderr,"End time not specified correctly.\n");
	usage();
	return -1;
      }
      uconvert_to_utime(&T);
      endTime = T.unix_time;
    }

    if (!(strcmp(argv[i],"-yesterday"))){

      // Get the time now.
      date_time_t Now;
      ugmtime(&Now);

      // Get the start and end of today
      date_time_t Begin,End;
      Begin=Now; End=Now;
      Begin.hour = 0; Begin.min = 0; Begin.sec=0;
      End.hour=23;    End.min=59; End.sec=59;
      uconvert_to_utime(&Begin);
      uconvert_to_utime(&End);

      // Back up a day.
      const long SecsPerDay = 86400;

      Begin.unix_time = Begin.unix_time - SecsPerDay;
      End.unix_time = End.unix_time - SecsPerDay;

      // Convert back - not really necessary
      // since I just want a utime, but I should
      // be careful with variable names like these.
      //
      uconvert_from_utime(&Begin);
      uconvert_from_utime(&End);

      startTime=Begin.unix_time; endTime=End.unix_time;

    }

  }

  if ((startTime == DateTime::NEVER) || (endTime == DateTime::NEVER)){
    if (P->Mode == Params::ARCHIVE){
      fprintf(stderr,"\nSpecify start and end times on the command line.\n\n");
      usage();
      return -1;
    }
  }
  


  //
  // Set up the input path.
  //

  switch (P->Mode) {

  case Params::ARCHIVE:
    inputPath = new DsInputPath( argv[0], P->Debug, P->InputDir,
				 startTime, endTime );
    break;


  case Params::REALTIME:
    inputPath = new DsInputPath( argv[0], P->Debug, P->InputDir,
				 P->MaxRealtimeValidAge,
				 PMU_auto_register );
    break;

  default :
    fprintf(stderr,"Unsupported mode.\n");
    exit(-1);
    break;
  }

  return 0;

}

//////////////////////////////////////////////////////////
void  FileLister::usage()
{

  fprintf(stderr,
	  "USAGE : trecCast -start \"YYYY MM DD hh mm ss\"\n"
	  "-end \"YYYY MM DD hh mm ss\" -params paramfile\n\n"
	  );
}

///////////////////////////////////////////////////////////
//
// Execution 
//
int  FileLister::run(Params *P)
{
  //
  // Process each input Input file
  //
  char *InputFile;
  int num=0, proc=0;

  while( (InputFile = inputPath->next()) != NULL ) {

    num++;
    fprintf(stderr,"Input file [%d] %s\n",num,InputFile);

    trecCast *T = new trecCast(InputFile,P);
    if (T->OK) proc++;

    delete T;

  }

  fprintf (stderr,"%d files processed.\n",proc);

  return proc;

}














