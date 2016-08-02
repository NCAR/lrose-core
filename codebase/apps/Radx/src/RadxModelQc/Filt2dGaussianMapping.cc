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
 * @file Filt2dGaussianMapping.cc
 */
#include "Filt2dGaussianMapping.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Filt2dGaussianMapping::Filt2dGaussianMapping(const Params::data_filter_t f,
					     const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  
  _y_field_name = P._parm_2d_gaussian_mapping[f.filter_index].y_field_name;
  _x_factor = P._parm_2d_gaussian_mapping[f.filter_index].x_factor;
  _y_factor = P._parm_2d_gaussian_mapping[f.filter_index].y_factor;
  _x_is_absolute = P._parm_2d_gaussian_mapping[f.filter_index].x_is_absolute;
  _y_is_absolute = P._parm_2d_gaussian_mapping[f.filter_index].y_is_absolute;
  _scale = P._parm_2d_gaussian_mapping[f.filter_index].scale;
}

//------------------------------------------------------------------
Filt2dGaussianMapping::~Filt2dGaussianMapping()
{
}

//------------------------------------------------------------------
void Filt2dGaussianMapping::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
bool Filt2dGaussianMapping::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool Filt2dGaussianMapping::filter(const time_t &t, const RadxRay *ray0,
				   const RadxRay &ray, const RadxRay *ray1,
				   std::vector<RayxData> &data) const
{
  RayxData x, y;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, x))
  {
    return false;
  }
  if (!RadxApp::retrieveRay(_y_field_name, ray, data, y))
  {
    return false;
  }

  x.gaussian2dRemap(y, _x_factor, _y_factor, _x_is_absolute, _y_is_absolute,
		    _scale);

  RadxApp::modifyRayForOutput(x, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(x);
  return true;
}

//------------------------------------------------------------------
void Filt2dGaussianMapping::filterVolume(const RadxVol &vol)
{
}

//------------------------------------------------------------------
void Filt2dGaussianMapping::finish(void)
{
}

