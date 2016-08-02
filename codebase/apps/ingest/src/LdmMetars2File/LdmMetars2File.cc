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

#include "Params.hh"
#include "Waiter.hh"
#include "MetarFiles.hh"
using namespace std;

// File scope.
void tidy_and_exit(int sig);
MetarFiles *M;

int main(int argc, char *argv[])
{

  // Grab TDRP arguments.

  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  }


  // Trap signals.

   PORTsignal( SIGINT,  tidy_and_exit );
   PORTsignal( SIGTERM, tidy_and_exit );
   PORTsignal( SIGQUIT, tidy_and_exit );
   PORTsignal( SIGKILL, tidy_and_exit );

   // PMU Checkin.                        
   PMU_auto_init("LdmMetars2File",
		 P->Instance,PROCMAP_REGISTER_INTERVAL);
  

   // Infinite loop.
   if (P->Debug) fprintf(stderr,"Initialising.\n");
   M = new MetarFiles(P->InDir,P->Debug,P->MaxGap,P->MaxFails,
		      P->MinSize, P->OutDir, P->WriteLdataFile);

   int i=0;
   while (1){

     i++;
     PMU_auto_register("Operating.");
     if (P->Debug) fprintf(stderr,"Starting pass %d.\n",i);

     if (M->Update(P->Debug, P->MinSize)){ // re-start.
       delete M;
       M = new MetarFiles(P->InDir,P->Debug,P->MaxGap,P->MaxFails,
			  P->MinSize, P->OutDir, P->WriteLdataFile);
     }

     if (P->Debug) fprintf(stderr,"Delaying.\n");
     Waiter *W = new Waiter(P->Delay);
     delete W;

     if (i>9) i=0;
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
