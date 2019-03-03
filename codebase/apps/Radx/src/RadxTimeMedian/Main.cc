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
#include "Parms.hh"
#include "Alg.hh"
#include "Volume.hh"
#include <radar/RadxAppParmsTemplate.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
static void tidy_and_exit (int sig)
{
  exit(sig);
}

// //------------------------------------------------------------------
// static void out_of_store(void)
// {
//   LOG(FATAL) << "Operator new failed - out of store";
//   exit(-1);
// }

//------------------------------------------------------------------
int main(int argc, char **argv)

{
  Parms params;
  if (!parmAppInit(params, argc, argv))
  {
    exit(-1);
  }
  
  Alg alg(params, tidy_and_exit);
  if (!alg.ok())
  {
    exit(-1);
  }

  Volume volume(&params, argc, argv);
  string path;
  while (volume.triggerRadxVolume(path))
  {
    alg.processVolume(&volume);
  }
  alg.processLast(&volume);

  // clean up
  tidy_and_exit(0);
  return (0);
}

