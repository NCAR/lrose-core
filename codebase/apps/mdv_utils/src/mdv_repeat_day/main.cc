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

#define MAX_NUM_TIMES 300

#include <cstdio>
#include <unistd.h>

#include "scan_input_dir.hh"
#include "process_file.hh"
#include <toolsa/pmu.h>

#include "Params.hh"
using namespace std;

static void tidy_and_exit (int sig);

int main(int argc, char *argv[])
{

  date_time_t FileTimes[MAX_NUM_TIMES];
  int NumFileTimes,itime;
  int DelayOn;

  // Load up from param file.
  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    return(-1);
  }

  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
     

  PMU_auto_init("mdv_repeat_day",P->instance,PROCMAP_REGISTER_INTERVAL);

  //
  // Get the array of times from the MDV filenames.
  //
  if (scan_input_dir(P->InDir, FileTimes,
		     MAX_NUM_TIMES, &NumFileTimes)){

    fprintf(stderr,"More that %d times in directory %s\n",
	    MAX_NUM_TIMES,P->InDir);

    return(-1);
  }

  //
  // Print these times if debugging.
  //
  if (P->debug){
    for(int i=0;i<NumFileTimes;i++){
      fprintf(stdout,"%d : %02d:%02d:%02d\n",i+1,
	      FileTimes[i].hour,
	      FileTimes[i].min,
	      FileTimes[i].sec);
    }
  }

  if (NumFileTimes==0){
    fprintf(stderr,"No mdv files found in %s\n",P->InDir);
    return(-1);
  }

  //
  // Go into the main loop.
  //
  int LastDay,processing;
  itime=0; DelayOn=0; LastDay=-1; processing=1;

  do{
    PMU_auto_register("Running...");
    date_time_t Now,FileTime;
    ugmtime(&Now);
    //
    // See if the day has rolled over, if it has,
    // reset itime
    //
    if (LastDay == -1) LastDay=Now.day; // First pass
    //
    if (Now.day != LastDay){
      LastDay = Now.day;
      itime=0; // Go back to start of array.
      processing=1; // Turn processing back on.
    }
    //
    // Take the time from the file time, the date from Now.
    //
    FileTime=FileTimes[itime];
    FileTime.year=Now.year;
    FileTime.month=Now.month;
    FileTime.day=Now.day;
    FileTime.unix_time=uconvert_to_utime(&FileTime);

    if ((FileTime.unix_time < Now.unix_time + P->DelT) || (P->single_pass)){

      if (P->debug) fprintf(stdout,"%d : %02d:%02d:%02d\n",itime+1,
			 FileTime.hour,
			 FileTime.min,
			 FileTime.sec);

      if (processing) process_file(P->InDir,P->OutDir,FileTime);

      itime++;
      if (itime>=NumFileTimes){
	itime=NumFileTimes-1; // Stay here until day rolls over.
	processing=0; // Turn processing off until day rolls over.
	if (P->single_pass){
	  tidy_and_exit(0);
	}
      }

    } else { // Time check failed, turn on sleep delay.
      DelayOn=1;
    }

    //
    // Delay, if it isn't a single pass, and if the delay has
    // been turned on by a failed check.
    //
    if ((!(P->single_pass)) && (DelayOn)){
      for(int i=0;i<P->Delay;i++){
	sleep(1);
	PMU_auto_register("Sleeping between checks.");
      }
    }

  }while(1);

  return 0;


}


static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}
 




