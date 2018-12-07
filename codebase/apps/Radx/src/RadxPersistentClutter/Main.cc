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
 */

//////////////////////////////////////////////////////////////////////////
// Based on following paper:
// Lakshmanan V., J. Zhang, K. Hondl and C. Langston.
// A Statistical Approach to Mitigating Persistent Clutter in
// Radar Reflectivity Data.
// IEEE Journal of Selected Topics in Applied Earth Observations
// and Remote Sensing, Vol. 5, No. 2, April 2012.
///////////////////////////////////////////////////////////////////////////

#include "Parms.hh"
#include "Alg.hh"
#include "RayData.hh"
#include <radar/RadxAppParmsTemplate.hh>

#include "RadxPersistentClutterFirstPass.hh"
#include "RadxPersistentClutterSecondPass.hh"
#include <toolsa/LogStream.hh>

//--------------------------------------------------------------------
// tidy up on exit
static void cleanup(int sig)
{
  exit(sig);
}

// not used:
// //--------------------------------------------------------------------
// // Handle out-of-memory conditions
// static void out_of_store()
// {
//   exit(-1);
// }

//----------------------------------------------------------------------
int main(int argc, char **argv)

{
  Parms params;
  if (!parmAppInit(params, argc, argv))
  {
    exit(0);
  }

  Alg alg(params, cleanup);
  if (!alg.ok())
  {
    exit(1);
  }

  RayData volume(&params, argc, argv);
  string path;

  // two passes
  bool first = true;
  bool converged = false;
  RadxPersistentClutterFirstPass p1(params, cleanup);
  while (volume.triggerRadxVolume(path))
  {
    if (!p1.processVolume(&volume, first))
    {
      LOG(DEBUG) << "No convergence yet";
    }
    else
    {
      LOG(DEBUG) << "Converged";
      converged = true;
      break;
    }
  }
  if (!converged)
  {
    LOG(WARNING) << "Never converged";  
    p1.finishBad();
    return -1;
  }

  RayData vol2(&params, argc, argv);
  RadxPersistentClutterSecondPass p2(p1);
  first = true;
  while (vol2.triggerRadxVolume(path))
  {
    if (p2.processVolume(&vol2, first))
    {
      // done, all is well
      return 0;
    }
  }
  p2.finishBad();
  return -1;
}

