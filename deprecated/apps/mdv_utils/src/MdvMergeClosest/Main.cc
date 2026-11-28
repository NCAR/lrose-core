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
#include <toolsa/copyright.h>
/**
 * @mainpage MdvMergeClosest
 * @file MdvMergeClosest.cc
 */

#include "Parms.hh"
#include "MdvMergeClosestMgr.hh"
#include <toolsa/pmu.h>
#include <toolsa/LogMsg.hh>
#include <cstdlib>

static void tidyAndExit(int i);

/**
 * Create algorithm manager and run algorithm
 * @param[in] argc
 * @param[in] argv  Typical:
 *                  'MdvMergeClosest -params MdvMergeClosest.params'
 *
 * @return integer status
 */
int main(int argc, char **argv)
{
  // Read in parameters
  Parms P(argc, argv);
  if (!P.isOk())
  {
    tidyAndExit(1);
  }

  // initialize process registration and logging using param
  PMU_auto_init(argv[0], P._instance.c_str(), P._registerSeconds);
  LOG_INIT(P._debug, P._debugVerbose, true, true);

  // create a manager
  MdvMergeClosestMgr mgr(P, argv[0], tidyAndExit);

  // run the app
  int status = mgr.run();
  
  // done
  tidyAndExit(status);
}

//----------------------------------------------------------------
static void tidyAndExit(int i)
{
  PMU_auto_unregister();
  exit(i);
}

