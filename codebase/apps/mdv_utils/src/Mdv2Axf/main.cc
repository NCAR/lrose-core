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
#include <iostream>

#include "Params.hh"
#include "Reader.hh"
#include "Trigger.hh"

#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/MdvxTimeStamp.hh>
#include <Mdv/DsMdvx.hh>   
#include <Mdv/DsMdvxTimes.hh> 

#include <signal.h>
using namespace std;

static void tidy_and_exit (int sig);  
void Usage(int ExitCode);
void DoArgs(int argc, char *argv[], 
	    date_time_t& Start, date_time_t& End);

int main(int argc, char *argv[]){

  date_time_t Start, End;

  DoArgs(argc,argv, Start, End);

  // Trap signals.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);   

  // Grab params.
  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    Usage(-1);
  }    

  // Check in
  PMU_auto_init("Mdv2Axf",P->Instance,PROCMAP_REGISTER_INTERVAL); 
  //
  // Set up according to trigger mode.
  //
  if (P->TriggerMode == Params::TRIGGER_ASYNC){
    // Set up an DsMdvxTimes object.

    DsMdvxTimes D; 

    if ((Start.unix_time == 0) && (End.unix_time == 0)){ // Realtime mode.
      if (D.setRealtime(P->InMdvURL,
			P->RealtimeMaxAge,
			PMU_auto_register)){
	cerr << "Failed to set realtime mode." << endl;
	exit(-1);
      }      
    } else { // Archive mode.
      if (D.setArchive(P->InMdvURL,Start.unix_time, End.unix_time)){
	cerr << "Failed to set archive mode." << endl;
	exit(-1);       
      }
    }

    time_t t;
    while (D.getNext(t)==0){
      cout << "Doing time " << utimstr(t) << endl;

      Reader R;
      R.Process(t,P);
    
    }       

  } else {
    //
    // Time-triggered mode. Makes web display look neat.
    //
    RealtimeTimeTrigger Trig((char *) "Mdv2Axf", P);
    time_t triggerTime;
    
    while ((triggerTime = Trig.next()) >= 0) {
      cerr << "Processing for " <<  utimstr(triggerTime) << endl;
      PMU_auto_register(utimstr(triggerTime));
      
      Reader R;
      R.Process(triggerTime, P);
    }




  }  
  return 0;

}

//////////////////////////////////////////////

void DoArgs(int argc, char *argv[], 
	    date_time_t& Start, date_time_t& End){


  if (argc < 2) Usage(0);

  Start.unix_time = 0; End.unix_time = 0; // Default, no archive specified.
  uconvert_from_utime( &Start ); uconvert_from_utime( &End );


  for (int i=1; i < argc; i++){

    if (!(strcmp(argv[i],"-h"))){
      Usage(0);
    }
 


    if (!(strcmp(argv[i],"-archive"))){
      if (argc <= i + 2) Usage(-1); // Make sure we have enough arguments.
       

      if (6 != sscanf(argv[i+1],"%4d%2d%2d%2d%2d%2d",
		      &Start.year, &Start.month, &Start.day,
		      &Start.hour, &Start.min,&Start.sec)){
		      

	Usage(-1);

      }
      uconvert_to_utime( &Start );

      if (6 != sscanf(argv[i+2],"%4d%2d%2d%2d%2d%2d",
		      &End.year, &End.month, &End.day,
		      &End.hour, &End.min,&End.sec)){
	Usage(-1);
      }

      uconvert_to_utime( &End );

    }



  }


}

///////////////////////////////////////////////////////////////////////

void Usage(int ExitCode)
{

  cout << "USAGE : Mdv2Axf -params paramfile [-archive YYYYMMDDhhmmss YYYYMMDDhhmmss]" << endl;
  exit(ExitCode);
 
}

//////////////////////////////////////////////////////////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{
  PMU_auto_unregister();
  exit(sig);
}       





