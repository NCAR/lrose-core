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
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Filter::Filter(const Params::data_filter_t f, const Params &p)
{
  _f = f;
  int max_f = max_elem_for_filter(f, p);
  _doPrintInputOutput = true; // default

  if (f.filter_index >= max_f)
  {
    LOG(ERROR) <<  filter_string(f) << " index " << f.filter_index 
	       << " out of range [0," << max_f << "]";
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
  if (_doPrintInputOutput)
  {
    LOG(PRINT) << "--- Filter   " << filter_string(_f) << "[" << _f.input_field 
	       << "] = " << _f.output_field << " ---";
  }
  else
  {
    filter_print();
  }
}

//------------------------------------------------------------------
string Filter::filter_string(const Params::data_filter_t f)
{
  string ret;
  switch (f.filter)
  {
  case Params::AZ_GRADIENT:
    ret = "AZ_GRADIENT";
    break;
  case Params::CLUTTER_2D_QUAL:
    ret = "CLUTTER_2D_QUAL";
    break;
  case Params::COMMENT:
    ret = "";
    break;
  case Params::GAUSSIAN_2D_REMAP:
    ret = "GAUSSIAN_2D_REMAP";
    break;
  case Params::GRIDDED_MATH:
    ret = "GRIDDED_MATH";
    break;
  case Params::MATH:
    ret = "MATH";
    break;
  case Params::MASK:
    ret = "MASK";
    break;
  case Params::PASSTHROUGH:
    ret = "PASSTHROUGH";
    break;
  case Params::QSCALE:
    ret = "QSCALE";
    break;
  case Params::SW_NORM:
    ret = "SW_NORM";
    break;
  case Params::THRESH:
    ret = "THRESH";
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
  case Params::AZ_GRADIENT:
    ret = P.parm_math_n;
    break;
  case Params::CLUTTER_2D_QUAL:
    ret = P.parm_clutter_2d_qual_n;
    break;
  case Params::COMMENT:
    ret = P.parm_comment_n;
    break;
  case Params::GAUSSIAN_2D_REMAP:
    ret = P.parm_2d_gaussian_mapping_n;
    break;
  case Params::GRIDDED_MATH:
    ret = P.parm_gridded_math_n;
    break;
  case Params::MATH:
    ret = P.parm_math_n;
    break;
  case Params::MASK:
    ret = P.parm_mask_n;
    break;
  case Params::PASSTHROUGH:
    ret = P.parm_dummy_n;
    break;
  case Params::QSCALE:
    ret = P.parm_qscale_n;
    break;
  case Params::SW_NORM:
    ret = P.parm_sw_norm_n;
    break;
  case Params::THRESH:
    ret = P.parm_thresh_n;
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

