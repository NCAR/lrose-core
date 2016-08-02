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
#include "FiltCombine.hh"
#include "FiltDB.hh"
#include "FiltMask.hh"
#include "FiltFuzzyRemap.hh"
#include "FiltFuzzy2dRemap.hh"
#include "Filt2dGaussianMapping.hh"
#include "FiltPassThrough.hh"
#include "FiltDivide.hh"
#include "FiltDifference.hh"
#include "FiltRemap.hh"
#include "FiltRestrict.hh"
#include "FiltSpeckle.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
Filter *modelQc::filtCreate(const Params::data_filter_t f, const Params &P)
{
  Filter *filt = NULL;
  switch (f.filter)
  {
  case Params::AVERAGE:
  case Params::MIN:
  case Params::MAX:
  case Params::PRODUCT:
    filt = new FiltCombine(f, P);
    break;
  case Params::DB2LINEAR:
  case Params::LINEAR2DB:
    filt = new FiltDB(f, P);
    break;
  case Params::DIFFERENCE:
    filt = new FiltDifference(f, P);
    break;
  case Params::DIVIDE:
    filt = new FiltDivide(f, P);
    break;
  case Params::FUZZY_REMAP:
    filt = new FiltFuzzyRemap(f, P);
    break;
  case Params::FUZZY_2D_REMAP:
    filt = new FiltFuzzy2dRemap(f, P);
    break;
  case Params::GAUSSIAN_2D_REMAP:
    filt = new Filt2dGaussianMapping(f, P);
    break;
  case Params::MASK:
    filt = new FiltMask(f, P);
    break;
  case Params::PASSTHROUGH:
    filt = new FiltPassThrough(f, P);
    break;
  case Params::REMAP:
    filt = new FiltRemap(f, P);
    break;
  case Params::RESTRICT:
    filt = new FiltRestrict(f, P);
    break;
  case Params::SPECKLE:
    filt = new FiltSpeckle(f, P);
    break;
  default:
    LOGF(LogMsg::ERROR, "unknown filter %d", f.filter);
    filt = NULL;
    break;
  }
  return filt;
}
