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

// RAP Include Files
#include <cstdio>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>

// Local Include Files
#include "Gini2Mdv.hh"

using namespace std;

static void tidy_and_exit (int sig);
static Gini2Mdv *gini2Mdv = 0;

int main(int argc, char *argv[])
   {
   // Create main object
   gini2Mdv =  new Gini2Mdv(argc, argv);
   if (!gini2Mdv->isOK())
      {
      return(-1);
      }
 
   // Trap.
   PORTsignal(SIGINT, tidy_and_exit);
   PORTsignal(SIGHUP, tidy_and_exit);
   PORTsignal(SIGTERM, tidy_and_exit);

   // Run Gini2Mdv 
   int status = gini2Mdv->run();
   if (status != 0)
      {
      cerr << gini2Mdv->getErrStr() << endl;
      }

   // clean up and exit
   tidy_and_exit(status);
   }

/////////////////////////////////////////////

static void tidy_and_exit(int sig)
   {
   PMU_auto_unregister();
   exit(sig);
   }
