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
 * @file FiltFuzzy2dRemap.cc
 */
#include "FiltFuzzy2dRemap.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
FiltFuzzy2dRemap::FiltFuzzy2dRemap(const Params::data_filter_t f,
			       const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  
  _y_field_name = P._parm_fuzzy2d[f.filter_index].y_field_name;
  if (!_fuzzyf.readParmFile(P._parm_fuzzy2d[f.filter_index].paramfile_name))
  {
    _fuzzyf = Fuzzy2d();
  }
}

//------------------------------------------------------------------
FiltFuzzy2dRemap::~FiltFuzzy2dRemap()
{
}

//------------------------------------------------------------------
void FiltFuzzy2dRemap::filter_print(void) const
{
  LOG(LogMsg::DEBUG_VERBOSE, "filtering");
}

//------------------------------------------------------------------
bool FiltFuzzy2dRemap::filter(const time_t &t, const RadxRay &ray,
			    std::vector<RayData> &data)
{
  RayData x, y;
  if (!RadxFiltAlg::retrieveRay(_f.input_field, ray, data, x))
  {
    return false;
  }
  if (!RadxFiltAlg::retrieveRay(_y_field_name, ray, data, y))
  {
    return false;
  }

  x.fuzzy2dRemap(_fuzzyf, y);

  RadxFiltAlg::modifyRayForOutput(x, _f.output_field, _f.output_units,
				  _f.output_missing);
  data.push_back(x);
  return true;
}

