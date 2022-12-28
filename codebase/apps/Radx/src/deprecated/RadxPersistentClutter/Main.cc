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
#include "Volume.hh"
#include <radar/RadxAppParmsTemplate.hh>
#include <toolsa/LogStream.hh>

//--------------------------------------------------------------------
// tidy up on exit
static void cleanup(int sig)
{
  exit(sig);
}

//----------------------------------------------------------------------
int main(int argc, char **argv)

{
  Parms params;
  if (!parmAppInit(params, argc, argv))
  {
    exit(0);
  }

  // create the Alg
  Alg alg(params, cleanup);
  if (!alg.ok())
  {
    exit(1);
  }

  // create a Volume of data
  Volume volume(&params, argc, argv);
  string path;

  // Repeat:  
  while (volume.triggerRadxVolume(path))
  {
    // Read in a volume and run the algorithm
    if (!alg.run(&volume))
    {
      LOG(ERROR) << "Processing this volume";
    }
    else
    {
      // check out the status for actions 
      if (volume.doWrite())
      {
	// write
	alg.write(&volume);
      }
      if (volume.done())
      {
	// all done
	break;
      }
    }
  }

  // If the algorithm converged, write out the final results
  if (volume.converged())
  {
    alg.write(&volume, params.final_output_url);
  }
  LOG(DEBUG) << "Done";
  return 0;
}
