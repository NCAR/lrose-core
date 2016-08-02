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

#include <signal.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>

#include "Args.hh"
#include "Params.hh"
#include "Waiter.hh"
#include "FilesMgr.hh"
using namespace std;

// File scope.
void tidy_and_exit(int sig);
FilesMgr *M;

int main(int argc, char *argv[])
{

  // get command line args

  string progName("LdmDynamic2Static");

  Args args;
  if (args.parse(argc, argv, progName)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with command line args" << endl;
    exit(-1);
  }

  // Grab TDRP arguments.

  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,args.override.list,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  }

  // Make sure the output directory exists.
  // Exit if not able to create.
  //
  if (ta_makedir_recurse(P->OutDir)){
    fprintf(stderr, "Could not create output directory %s\n",
	    P->OutDir);
    exit(-1);
  }

  // Trap signals.

   PORTsignal( SIGINT,  tidy_and_exit );
   PORTsignal( SIGTERM, tidy_and_exit );
   PORTsignal( SIGQUIT, tidy_and_exit );
   PORTsignal( SIGKILL, tidy_and_exit );

   // PMU Checkin.                        
   PMU_auto_init("LdmDynamic2Static",
		 P->Instance,PROCMAP_REGISTER_INTERVAL);
  

   // Infinite loop.
   if (P->Debug) fprintf(stderr,"Main(): Initialising.\n");
   M = new FilesMgr(*P);

   int i=0;
   while (1){

     PMU_auto_register("Operating.");

     if (P->Debug) 
       fprintf(stderr,"Main(): Starting pass %d.\n",i);

      FilesMgr::Status_t searchStatus = M->newSearch();
      
      if (searchStatus == FilesMgr::FILESMGR_SUCCESS)
      {
	int noUpdateCount = 0;
	
	while ( noUpdateCount < P->MaxFails)
	{
	  if (P->Debug) 
	    fprintf(stderr,"Main(): Delaying %d secs.\n", P->Delay);

	  //
	  // Sleep
	  //
	  Waiter *W = new Waiter(P->Delay);
	  
	  delete W;

	  FilesMgr::Status_t updateStatus =  M->update();
	
	  if (updateStatus == FilesMgr::FILESMGR_FAILURE)
	    noUpdateCount++;
	}

	if (P->Debug) 
	  fprintf(stderr,"Main(): Delaying %d minutes.\n",
		    P->Delay/60);
	
	//
	// Sleep, then we will start over with the search
	//
	Waiter *W = new Waiter(P->Delay);
	
	delete W;
	
      } else {
        // sleep a second
	Waiter W(1);
      }
   }
      return 0; // Avoid compiler warnings.
}

/////////////////////////////////////////////

void tidy_and_exit(int sig)
{

  PMU_auto_unregister();
  delete M;
  exit(sig);
}
