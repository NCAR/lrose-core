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
// main for MomentsCompute
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// MomentsCompute reads pulse data, forms the pulses into beams
// and computes the moments.
//
////////////////////////////////////////////////////////////////

#include "MomentsCompute.hh"
#include <radar/IwrfTsReader.hh>
#include <signal.h>
#include <new>
#include "Args.hh"
using namespace std;

// main

int main(int argc, char **argv)

{

  string progName = "MomentsCompute";

  // get command line args

  Args args;
  if (args.parse(argc, argv)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with command line args" << endl;
    return -1;
  }
 
  // create Moments compute

  Params params;
  MomentsCompute compute(params);
  if (!compute.isOK) {
    return(-1);
  }
  if (args.debugExtraVerbose) {
    compute.setDebugExtraVerbose();
  } else if (args.debugVerbose) {
    compute.setDebugVerbose();
  } else if (args.debug) {
    compute.setDebug();
  }


  IwrfTsReaderFile reader(args.inputFileList);
  const IwrfTsInfo opsInfo;

  // loop through the input files

  int iret = 0;
  while (true) {

    const IwrfTsPulse *pulse = reader.getNextPulse(true);
    if (pulse == NULL) {
      break;
    }

    // process this pulse
    
    if (compute.processPulse((float *) pulse->getIq0(),
                             (float *) pulse->getIq1(),
                             pulse->getNGates(),
                             pulse->getPrt(),
                             pulse->getEl(),
                             pulse->getAz(),
                             pulse->getSeqNum(),
                             true)) {
        iret = -1;
      }
      
  } // while
  
  return iret;
  
}

