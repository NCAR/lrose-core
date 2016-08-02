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
///////////////////////////////////////////////////////////////
//
// main for TrecGauge2Spdb
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1997
//
///////////////////////////////////////////////////////////////
//
// TrecGauge2Spdb takes the following inputs:
//  (a) trec output files
//  (b) a list of precip gauge positions
//
// For each trec output file and each gauge position, the following
// are computed:
//  (a) the mean trec motion vector for the gauge position
//  (b) an array of mean reflectivity values upwind of the
//      gauge position. The intention here is that the
//      reflectivity array represents a series of forecast
//      values, since the reflectivity should drift over
//      the gauge roughly using the mean trec motion vector.
//
// The output data are stored in SPDB files. See the include
// file <rapformats/trec_gauge.h> for the format of the
// output data.
//
///////////////////////////////////////////////////////////////

#include "TrecGauge2Spdb.hh"
#include <toolsa/str.h>
#include <signal.h>
using namespace std;

// file scope

static void tidy_and_exit (int sig);
static TrecGauge2Spdb *Prog;

// main

int main(int argc, char **argv)

{

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // create program object

  Prog = new TrecGauge2Spdb(argc, argv);
  if (!Prog->OK) {
    return(-1);
  }

  // run it

  
  Prog->Run();

  // clean up

  tidy_and_exit(0);
  return (0);
  
}

// tidy up on exit

static void tidy_and_exit (int sig)

{
  delete(Prog);
  exit(sig);
}

