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
 * @file FiltCombine.cc
 */
#include "FiltCombine.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
FiltCombine::FiltCombine(const Params::data_filter_t f,
			 const Params &P) :  Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _main_input_weight = P._parm_combine[f.filter_index].input_weight;
  _normalize = P._parm_combine[f.filter_index].normalize;
  _missing_ok = P._parm_combine[f.filter_index].missing_ok;
  switch (f.filter)
  {
  case Params::AVERAGE:
    _type = AVERAGE;
    break;
  case Params::MIN:
    _type = MIN;
    break;
  case Params::MAX:
    _type = MAX;
    break;
  case Params::PRODUCT:
    _type = PRODUCT;
    break;
  default:
    LOGF(LogMsg::ERROR, "Invalid filter type %d",
	    static_cast<int>(f.filter));
    _type = AVERAGE;
    _ok = false;
  }

  switch (P._parm_combine[f.filter_index].combine_index)
  {
  case 0:
    _build(P._combine0, P.combine0_n);
    break;
  case 1:
    _build(P._combine1, P.combine1_n);
    break;
  case 2:
    _build(P._combine2, P.combine2_n);
    break;
  case 3:
    _build(P._combine3, P.combine3_n);
    break;
  case 4:
    _build(P._combine4, P.combine4_n);
    break;
  case 5:
    _build(P._combine5, P.combine5_n);
    break;
  case 6:
    _build(P._combine6, P.combine6_n);
    break;
  case 7:
    _build(P._combine7, P.combine7_n);
    break;
  case 8:
    _build(P._combine8, P.combine8_n);
    break;
  case 9:
    _build(P._combine9, P.combine9_n);
    break;
  default:
    LOGF(LogMsg::ERROR, "Index out of range 0 to 9 (%d)", 
	    P._parm_combine[f.filter_index].combine_index);
    _ok = false;
  }    

}

//------------------------------------------------------------------
FiltCombine::~FiltCombine()
{
}

//------------------------------------------------------------------
void FiltCombine::filter_print(void) const
{
  LOG(LogMsg::DEBUG_VERBOSE, "filtering");
}


//------------------------------------------------------------------
bool FiltCombine::filter(const time_t &t, const RadxRay &ray,
			 std::vector<RayData> &data)
{
  RayData r;
  double sumWeight = 0;
  if (_type == AVERAGE)
  {
    r.multiply(_main_input_weight);
    sumWeight += _main_input_weight;
  }

  // get the main ray
  if (!RadxFiltAlg::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }

  // get the other rays, and combine as we go
  for (int i=0; i<static_cast<int>(_weight_and_names.size()); ++i)
  {
    RayData ri;
    if (!RadxFiltAlg::retrieveRay(_weight_and_names[i].second, ray, data, ri))
    {
      return false;
    }
    switch (_type)
    {
    case AVERAGE:
      ri.multiply(_weight_and_names[i].first);
      r.inc(ri, _missing_ok);
      sumWeight += _weight_and_names[i].first;
      break;
    case MIN:
      r.min(ri);
      break;
    case MAX:
      r.max(ri);
      break;
    case PRODUCT:
      r.multiply(ri, _missing_ok);
      break;
    default:
      LOGF(LogMsg::ERROR, "type %d out of range", static_cast<int>(_type));
      return false;
    }
  }
  if (_type == AVERAGE && _normalize)
  {
    r.divide(sumWeight);
  }

  RadxFiltAlg::modifyRayForOutput(r, _f.output_field, _f.output_units,
				  _f.output_missing);
  data.push_back(r);
  return true;
}

//------------------------------------------------------------------
void FiltCombine::_build(const Params::Combine_t *c, const int n)
{
  for (int i=0; i<n; ++i)
  {
    _weight_and_names.push_back(pair<double,string>(c[i].weight, c[i].name));
  }
}

