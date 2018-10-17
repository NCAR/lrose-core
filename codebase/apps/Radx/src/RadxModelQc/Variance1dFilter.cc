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
 * @file FiltVariance1d.cc
 */
#include "Variance1dFilter.hh"
#include "RayData.hh"
#include <toolsa/LogStream.hh>
#include <Radx/RayxData.hh>
#include <cmath>

//------------------------------------------------------------------
bool Variance1dFilter::filter(const RayxData &data,
			      RayLoopData *output)
{
  // copy contents of vel into output
  output->transferData(data);
  RayxData quality = *output;

  // no quality output just yet..note
  output->variance(_npt, _maxPctMissing);
  return true;

  // // "SD_VR = 0.107/(8*MeanPrt*MeanNSamples*sqrt(PI))*WIDTH0",

  // double PI = 3.14159;
  // double arg = 0.07/(8*meanPrt*meanNsamples*sqrt(PI));
  // rl->multiply(arg);
  // rl->invert();
  // rl->sqrt();
  // rl->inc(1.0);
  // rl->logBase10();
  // rl->multiply(10.0);
}
