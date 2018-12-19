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
 * @file FiltSpecial0.cc
 */
#include "Special0Filter.hh"
#include "RayData.hh"
#include <toolsa/LogStream.hh>
#include <Radx/RayxData.hh>
#include <cmath>

//------------------------------------------------------------------
bool Special0Filter::filter(const RayxData &width, double meanPrt,
			    double meanNsamples,
			    RayLoopData *output)
{
  // copy contents into output
  output->transferData(width);

  
  //"SD_DBZ = 10*log10(1 + sqrt(1.0/(WIDTH0*(4.0*sqrt(PI)*MeanPrt*MeanNSamples/0.10))))",
  double PI = 3.14159;
  double arg = 4.0*sqrt(PI)*meanPrt*meanNsamples/0.10;

  output->multiply(arg);
  output->invert();
  output->squareRoot();
  output->inc(1.0);
  output->logBase10();
  output->multiply(10.0);
  return true;
}
