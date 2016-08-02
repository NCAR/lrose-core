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


#include <toolsa/DateTime.hh>
#include <stdio.h>
#include <time.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include "Params.hh"
#include "FileLister.hh"
#include "IRProcess.hh"

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


  //
  // Grab TDRP parameters.
  //

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    usage();
    return(-1);
  } 

  //
  // Now that we know the instance, check in.
  //
  PMU_auto_init("IR2Height",P->Instance,
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
  }



  //
  // Set up the input from the IR field.
  //

  if ((endTime != DateTime::NEVER) && (startTime != DateTime::NEVER)){
    //
    // Archive mode assumed.
    //

    if (P->Debug)

      cerr << "Setting for ARCHIVE operation." << endl;


    if (IR.setArchive(P->IR_URL, startTime, endTime )){
      cerr << "Failed to set URL " << P->IR_URL << endl;
      exit(-1);
    }

  } else {

    if (P->Debug)
      cerr << "Setting for REALTIME operation." << endl;

    if (IR.setRealtime(P->IR_URL,
		       P->MaxRealtimeValidAge,
		       PMU_auto_register)){
      cerr << "Failed to set URL " << P->IR_URL << endl;
      exit(-1);
    }
  }


  return 0;

}

//////////////////////////////////////////////////////////
void  FileLister::usage()
{

  cerr << "USAGE : IR2Height -start \"YYYY MM DD hh mm ss\" ";
  cerr << "-end \"YYYY MM DD hh mm ss\" -params paramfile" << endl;
  cerr << " Specifying times on command line automatically invokes archive mode." << endl;
       
}

///////////////////////////////////////////////////////////
//
// Execution 
//
int  FileLister::run(Params *P)
{

  time_t IRTime;
  
  while(!(IR.getNext(IRTime))){

    if (P->Debug)
      cerr << "Processing for satellite time " << utimstr( IRTime ) << endl;

    if (IRTime == 0){
      //
      // Kludge to debug library problem.
      //
      cerr << "Null time !!" << endl;
    } else {
      IRProcess I;
      I.IRProcessData( IRTime, P );
    }

    if (P->Debug){
      time_t now;
      now=time(NULL);
      cerr << "The time is " << utimstr(now) << endl;
    }
  }

  return 0;

}















