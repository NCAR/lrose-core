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
 * @file FiltMask.cc
 */
#include "FiltMask.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
FiltMask::FiltMask(const Params::data_filter_t f,
		   const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _maskFieldName = P._parm_mask[f.filter_index].mask_field_name;
  _type = P._parm_mask[f.filter_index].type;
  _maskThreshold = P._parm_mask[f.filter_index].mask_threshold;
  _dataReplacementValue = P._parm_mask[f.filter_index].data_replacement_value;
  _replaceWithMissing = P._parm_thresh[f.filter_index].replace_with_missing;
}

//------------------------------------------------------------------
FiltMask::~FiltMask()
{
}
//------------------------------------------------------------------
void FiltMask::filter_print(void) const
{
  LOG(LogMsg::DEBUG_VERBOSE, "filtering");
}

//------------------------------------------------------------------
bool FiltMask::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltMask::filter(const time_t &t, const RadxRay *ray0, const RadxRay &ray,
		      const RadxRay *ray1, std::vector<RayxData> &data) const
{
  RayxData r, m;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }

  if (!RadxApp::retrieveRay(_maskFieldName, ray, data, m))
  {
    return false;
  }

  switch (_type)
  {
  case Params::LESS:
    r.modifyWhenMaskLessThan(m, _maskThreshold, _dataReplacementValue,
			     _replaceWithMissing);
    break;
  case Params::LESS_OR_EQUAL:
    r.modifyWhenMaskLessThanOrEqual(m, _maskThreshold, _dataReplacementValue,
				    _replaceWithMissing);
    break;
  case Params::GREATER:
    r.modifyWhenMaskGreaterThan(m, _maskThreshold, _dataReplacementValue,
				_replaceWithMissing);
    break;
  case Params::GREATER_OR_EQUAL:
    r.modifyWhenMaskGreaterThanOrEqual(m, _maskThreshold, _dataReplacementValue,
				     _replaceWithMissing);
    break;
  case Params::MISSING:
  default:
    r.modifyWhenMaskMissing(m, _dataReplacementValue, _replaceWithMissing);
    break;
  }
    
  RadxApp::modifyRayForOutput(r, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(r);
  return true;
}

//------------------------------------------------------------------
void FiltMask::filterVolume(const RadxVol &vol)
{
}


//------------------------------------------------------------------
void FiltMask::finish(void)
{
}

