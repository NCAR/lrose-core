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
#include <FiltAlg/FiltMask.hh>

//------------------------------------------------------------------
FiltMask::FiltMask(const FiltAlgParams::data_filter_t f,
		   const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  _mask = NULL;
  _mask_name = P._parm_mask[f.filter_index].mask_name;
  _mask_is_input = P._parm_mask[f.filter_index].mask_is_input;
  int ind = P._parm_mask[f.filter_index].mask_list_index;
  switch (ind)
  {
  case 0:
    _set_ranges(P.mask_range0_n, P._mask_range0);
    break;
  case 1:
    _set_ranges(P.mask_range1_n, P._mask_range1);
    break;
  case 2:
    _set_ranges(P.mask_range2_n, P._mask_range2);
    break;
  default:
    LOG(ERROR) << "mask_list_index must be between 0 and 2";
    _ok = false;
  }
}

//------------------------------------------------------------------
FiltMask::~FiltMask()
{
}

//------------------------------------------------------------------
void FiltMask::initialize_output(const Data &inp,
				 const FiltAlgParams::data_filter_t f, Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

//------------------------------------------------------------------
void FiltMask::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltMask::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltMask::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  if (!createGridAtVlevel(inp, o))
  {
    return false;
  }

  // point to the 2d slice of mask data 
  const VlevelSlice *mask = _mask->matching_vlevel(inp.getVlevel(),
						   _vlevel_tolerance);

  for (int i=0; i<static_cast<int>(_range.size()); ++i)
  {
    o.maskRange(*mask, _range[i].first, _range[i].second);
  }

  return true;
}

//------------------------------------------------------------------
bool FiltMask::create_inputs(const time_t &t, 
			     const vector<Data> &input,
			     const vector<Data> &output)
{
  _mask = Filter::set_data(_mask_is_input, _mask_name.c_str(), input, output);
  if (_mask == NULL)
  {
    LOG(ERROR) << "input " << _mask_name << " never found";
    return false;
  }
  if (!_mask->is_grid3d())
  {
    LOG(ERROR) << "input " << _mask_name << " not a 3d grid";
    return false;
  }
  return true;
}

//------------------------------------------------------------------
void FiltMask::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltMask::store_outputs(const Data &o, Info *info,
			     vector<FiltInfo> &extra,
			     vector<Data> &output)
{
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void FiltMask::set_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltMask::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltMask::vertical_level_change(void)
{
  // default is to do nothing
}

//------------------------------------------------------------------
void FiltMask::_set_ranges(const int n, const FiltAlgParams::mask_range_t *r)
{
  _range.clear();
  for (int i=0; i<n; ++i)
  {
    pair<double,double> p(r[i].mask_min, r[i].mask_max);
    _range.push_back(p);
  }
}
