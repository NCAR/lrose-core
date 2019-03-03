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
 * @file FiltSpecial1.cc
 */
#include "Special1Filter.hh"
#include <radar/RadxAppRayLoopData.hh>
#include <toolsa/LogStream.hh>
#include <Radx/RayxData.hh>
// #include <toolsa/globals.h>
// #include <Radx/RadxVol.hh>
#include <cmath>

//------------------------------------------------------------------
bool Special1Filter::filter(const RayxData &width, double meanPrt,
			    double meanNsamples,  RadxAppRayLoopData *output)
{

  output->transferData(width);

  // "SD_VR = 0.107/(8*MeanPrt*MeanNSamples*sqrt(PI))*WIDTH0",

  double PI = 3.14159;
  double arg = 0.07/(8*meanPrt*meanNsamples*sqrt(PI));
  output->multiply(arg);
  return true;
}
