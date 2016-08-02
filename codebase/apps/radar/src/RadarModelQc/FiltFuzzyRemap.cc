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
 * @file FiltFuzzyRemap.cc
 */
#include "FiltFuzzyRemap.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
FiltFuzzyRemap::FiltFuzzyRemap(const Params::data_filter_t f,
			       const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  // go to the correct fuzzy function based on index
  switch (f.filter_index)
  {
  case 0:
    _build(P._fuzzy0, P.fuzzy0_n);
    break;
  case 1:
    _build(P._fuzzy1, P.fuzzy1_n);
    break;
  case 2:
    _build(P._fuzzy2, P.fuzzy2_n);
    break;
  case 3:
    _build(P._fuzzy3, P.fuzzy3_n);
    break;
  case 4:
    _build(P._fuzzy4, P.fuzzy4_n);
    break;
  case 5:
    _build(P._fuzzy5, P.fuzzy5_n);
    break;
  case 6:
    _build(P._fuzzy6, P.fuzzy6_n);
    break;
  case 7:
    _build(P._fuzzy7, P.fuzzy7_n);
    break;
  case 8:
    _build(P._fuzzy8, P.fuzzy8_n);
    break;
  case 9:
    _build(P._fuzzy9, P.fuzzy9_n);
    break;
  case 10:
    _build(P._fuzzy10, P.fuzzy10_n);
    break;
  case 11:
    _build(P._fuzzy11, P.fuzzy11_n);
    break;
  default:
    LOGF(LogMsg::ERROR, "Index out of range %d", f.filter_index);
    _ok = false;
  }
}

//------------------------------------------------------------------
FiltFuzzyRemap::~FiltFuzzyRemap()
{
}

//------------------------------------------------------------------
void FiltFuzzyRemap::filter_print(void) const
{
  LOG(LogMsg::DEBUG_VERBOSE, "filtering");
}

//------------------------------------------------------------------
bool FiltFuzzyRemap::filter(const time_t &t, const RadxRay &ray,
			    std::vector<RayData> &data)
{
  RayData r;
  if (!RadxFiltAlg::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }
  r.fuzzyRemap(_fuzzyf);
  RadxFiltAlg::modifyRayForOutput(r, _f.output_field, _f.output_units,
				  _f.output_missing);
  data.push_back(r);
  return true;
}

//------------------------------------------------------------------
void FiltFuzzyRemap::_build(const Params::Fuzzy_t *f, const int n)
{
  vector<pair<double,double> > vals;
  for (int i=0; i<n; ++i)
  {
    vals.push_back(pair<double,double>(f[i].x, f[i].y));
  }
  _fuzzyf = FuzzyF(vals);
}

