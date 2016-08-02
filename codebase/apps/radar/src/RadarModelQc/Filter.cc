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
 * @file Filter.cc
 */
#include "Filter.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
Filter::Filter(const Params::data_filter_t f, const Params &p)
{
  _f = f;
  int max_f = max_elem_for_filter(f, p);

  if (f.filter_index >= max_f)
  {
    LOGF(LogMsg::ERROR, "%s index %d out of range [0,%d]",
	    filter_string(f).c_str(), f.filter_index, max_f);
    _ok = false;
  }
  else
  {
    _ok = true;
  }
}

//------------------------------------------------------------------
Filter::~Filter()
{
}

//------------------------------------------------------------------
void Filter::printInputOutput(void) const
{
  LOGF(LogMsg::PRINT, "--- Filter   %s[%s] = %s ---",
	  Filter::filter_string(_f).c_str(), _f.input_field,
	  _f.output_field);
}

//------------------------------------------------------------------
string Filter::filter_string(const Params::data_filter_t f)
{
  string ret;
  switch (f.filter)
  {
  case Params::AVERAGE:
    ret = "AVERAGE";
    break;
  case Params::MIN:
    ret = "MIN";
    break;
  case Params::MAX:
    ret = "MAX";
    break;
  case Params::PRODUCT:
    ret = "PRODUCT";
    break;
  case Params::DIFFERENCE:
    ret = "DIFFERENCE";
    break;
  case Params::DB2LINEAR:
    ret = "DB2LINEAR";
    break;
  case Params::LINEAR2DB:
    ret = "LINEAR2DB";
    break;
  case Params::FUZZY_REMAP:
    ret = "FUZZY_REMAP";
    break;
  case Params::FUZZY_2D_REMAP:
    ret = "FUZZY_2D_REMAP";
    break;
  case Params::GAUSSIAN_2D_REMAP:
    ret = "GAUSSIAN_2D_REMAP";
    break;
  case Params::DIVIDE:
    ret = "DIVIDE";
    break;
  case Params::MASK:
    ret = "MASK";
    break;
  case Params::PASSTHROUGH:
    ret = "PASSTHROUGH";
    break;
  case Params::REMAP:
    ret = "REMAP";
    break;
  case Params::RESTRICT:
    ret = "RESTRICT";
    break;
  case Params::SPECKLE:
    ret = "SPECKLE";
    break;
  default:
    ret = "Unknown";
    break;
  }
  return ret;
}

//------------------------------------------------------------------
int Filter::max_elem_for_filter(const Params::data_filter_t f, 
				const Params &P)
{
  int ret;
  switch (f.filter)
  {
  case Params::DIFFERENCE:
    ret = P.parm_difference_n;
    break;
  case Params::FUZZY_REMAP:
    ret = 12;
    break;
  case Params::FUZZY_2D_REMAP:
    ret = P.parm_fuzzy2d_n;
    break;
  case Params::GAUSSIAN_2D_REMAP:
    ret = P.parm_2d_gaussian_mapping_n;
    break;
  case Params::AVERAGE:
  case Params::MIN:
  case Params::MAX:
  case Params::PRODUCT:
    ret = P.parm_combine_n;
    break;
  case Params::DIVIDE:
    ret = P.parm_divide_n;
    break;
  case Params::MASK:
    ret = P.parm_mask_n;
    break;
  case Params::PASSTHROUGH:
  case Params::LINEAR2DB:
  case Params::DB2LINEAR:
    ret = P.parm_dummy_n;
    break;
  case Params::REMAP:
    ret = P.parm_remap_n;
    break;
  case Params::RESTRICT:
    ret = P.parm_restrict_n;
    break;
  case Params::SPECKLE:
    ret = P.parm_speckle_n;
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

