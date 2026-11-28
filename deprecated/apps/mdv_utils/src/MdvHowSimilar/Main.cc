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

/**
 * @file Main.cc
 *
 * A simple app to do a quick comparison between two Mdv files.
 * This app is based on MdvQuickCompare & MdvBlender, but is used for ACES, where MdvQuickCompare is used by the IFI test harness.
 # MdvQuickCompare is entirely command line based, but this app will take a param file.
 *
 * @author P J Prestopnik
 * @version $Id $
 */

// SYSTEM INCLUDES

#include <iostream>
#include <cstdio>
#include <math.h>
#include <cstdlib>

// PROJECT INCLUDES
#include <toolsa/umisc.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>    


// LOCAL INCLUDES
#include "MdvHowSimilar.hh"

using namespace std;
static MdvHowSimilar *mainObj = 0;

void tidy_and_exit(int signal)
{
   // Remove the top-level application class
   delete mainObj;
   printf("Exiting %d\n", signal);
   exit(signal);
}


void no_memory() {
  cerr << "Caught bad_alloc exception (Failed to allocate memory!)\n";
  tidy_and_exit(1);
}

int main(int argc, char **argv)
{

  // create main object
  mainObj = MdvHowSimilar::instance(argc, argv);
  if(!mainObj->isOK()) {
    tidy_and_exit(-1);
  } 
  // set signal handling
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);
  set_new_handler(no_memory);


  // run it
  int iret = mainObj->run();

  // clean up and exit
  tidy_and_exit(iret);
}


