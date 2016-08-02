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
 * @file FiltCreate.cc
 */
#include "FiltCreate.hh"
#include "Params.hh"
#include "FiltAzGradient.hh"
#include "FiltClutter2dQual.hh"
#include "FiltComment.hh"
#include "Filt2dGaussianMapping.hh"
#include "FiltGriddedMath.hh"
#include "FiltMath.hh"
#include "FiltMask.hh"
#include "FiltPassThrough.hh"
#include "FiltQscale.hh"
#include "FiltSpectrumWidthNorm.hh"
#include "FiltThresh.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Filter *modelQc::filtCreate(const Params::data_filter_t f, const Params &P)
{
  Filter *filt = NULL;
  switch (f.filter)
  {
  case Params::AZ_GRADIENT:
    filt = new FiltAzGradient(f, P);
    break;
  case Params::CLUTTER_2D_QUAL:
    filt = new FiltClutter2dQual(f, P);
    break;
  case Params::COMMENT:
    filt = new FiltComment(f, P);
    break;
  case Params::GAUSSIAN_2D_REMAP:
    filt = new Filt2dGaussianMapping(f, P);
    break;
  case Params::GRIDDED_MATH:
    filt = new FiltGriddedMath(f, P);
    break;
  case Params::MATH:
    filt = new FiltMath(f, P);
    break;
  case Params::MASK:
    filt = new FiltMask(f, P);
    break;
  case Params::PASSTHROUGH:
    filt = new FiltPassThrough(f, P);
    break;
  case Params::QSCALE:
    filt = new FiltQscale(f, P);
    break;
  case Params::SW_NORM:
    filt = new FiltSpectrumWidthNorm(f, P);
    break;
  case Params::THRESH:
    filt = new FiltThresh(f, P);
    break;
  default:
    LOG(ERROR) << "unknown filter "<< f.filter;
    filt = NULL;
    break;
  }
  return filt; 
}
