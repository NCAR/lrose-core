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
#include <FiltAlg/Filter.hh>

//------------------------------------------------------------------
Filter::Filter(const FiltAlgParams::data_filter_t f, const FiltAlgParms &P)
{
  _f = f;
  int max_f = max_elem_for_filter(f, P);
  _vlevel_tolerance = P.vlevel_tolerance;
  if (f.filter_index >= max_f)
  {
    LOG(ERROR) << filter_string(f) << " index " << f.filter_index 
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
void Filter::vertical_level_change(void)
{
  // default is to do nothing
}

//------------------------------------------------------------------
void Filter::printInputOutput(void) const
{
  LOG(PRINT) << "--- Filter   " 
	     << Filter::filter_string(_f) 
	     << "["  << _f.field  << "] = "
	     << _f.output_field << " ---";
}

//------------------------------------------------------------------
std::string Filter::sprintInputOutput(void) const
{
  string ret = "--- Filter   ";
  ret += filter_string(_f);
  ret += "[";
  ret += _f.field;
  ret += "] = ";
  ret += _f.output_field;
  ret += " ---";
  return ret;
}

//------------------------------------------------------------------
const Data *Filter::create_input_output(const vector<Data> &in,
					const vector<Data> &out,
					Data &gout)
{
  // find the input to this filter using inputs
  const Data *gin = Filter::set_data(_f.is_input_field, _f.field, in, out);
  if (gin == NULL)
  {
    LOG(ERROR) << "field " << _f.field << " never found";
  }

  // create a 3d output object empty for return (depends on particular filter)
  initialize_output(*gin, _f, gout);

  // return that pointer
  return gin;
}

//------------------------------------------------------------------
string Filter::filter_string(const FiltAlgParams::data_filter_t f)
{
  string ret;
  switch (f.filter)
  {
  case FiltAlgParams::CLUMP:
    ret = "CLUMP";
    break;
  case FiltAlgParams::ELLIP:
    ret = "ELLIP";
    break;
  case FiltAlgParams::DILATE:
    ret = "DILATE";
    break;
  case FiltAlgParams::MEDIAN:
    ret = "MEDIAN";
    break;
  case FiltAlgParams::SDEV:
    ret = "SDEV";
    break;
  case FiltAlgParams::MEDIAN_NO_OVERLAP:
    ret = "MEDIAN_NO_OVERLAP";
    break;
  case FiltAlgParams::SDEV_NO_OVERLAP:
    ret = "SDEV_NO_OVERLAP";
    break;
  case FiltAlgParams::TEXTURE_X:
    ret = "TEXTURE_X";
    break;
  case FiltAlgParams::TEXTURE_Y:
    ret = "TEXTURE_Y";
    break;
  case FiltAlgParams::REMAP:
    ret = "REMAP";
    break;
  case FiltAlgParams::TRAPEZOID_REMAP:
    ret = "TRAPEZOID_REMAP";
    break;
  case FiltAlgParams::S_REMAP:
    ret = "S_REMAP";
    break;
  case FiltAlgParams::RESCALE:
    ret = "RESCALE";
    break;
  case FiltAlgParams::REPLACE:
    ret = "REPLACE";
    break;
  case FiltAlgParams::MAX:
    ret = "MAX";
    break;
  case FiltAlgParams::MAX_TRUE:
    ret = "MAX_TRUE";
    break;
  case FiltAlgParams::AVERAGE:
    ret = "AVERAGE";
    break;
  case FiltAlgParams::AVERAGE_ORIENTATION:
    ret = "AVERAGE_ORIENTATION";
    break;
  case FiltAlgParams::PRODUCT:
    ret = "PRODUCT";
    break;
  case FiltAlgParams::WEIGHTED_SUM:
    ret = "WEIGHTED_SUM";
    break;
  case FiltAlgParams::WEIGHTED_ORIENTATION_SUM:
    ret = "WEIGHTED_SUM";
    break;
  case FiltAlgParams::NORM_WEIGHTED_SUM:
    ret = "NORM_WEIGHTED_SUM";
    break;
  case FiltAlgParams::NORM_WEIGHTED_ORIENTATION_SUM:
    ret = "NORM_WEIGHTED_SUM";
    break;
  case FiltAlgParams::FULL_MEAN:
    ret = "FULL_MEAN";
    break;
  case FiltAlgParams::FULL_SDEV:
    ret = "FULL_SDEV";
    break;
  case FiltAlgParams::FULL_MEDIAN:
    ret = "FULL_MEDIAN";
    break;
  case FiltAlgParams::VERT_AVERAGE:
    ret = "VERT_AVERAGE";
    break;
  case FiltAlgParams::VERT_MAX:
    ret = "VERT_MAX";
    break;
  case FiltAlgParams::VERT_PRODUCT:
    ret = "VERT_PRODUCT";
    break;
  case FiltAlgParams::MASK:
    ret = "MASK";
    break;
  case FiltAlgParams::DB2LINEAR:
    ret = "DB2LINEAR";
    break;
  case FiltAlgParams::LINEAR2DB:
    ret = "LINEAR2DB";
    break;
  case FiltAlgParams::PASSTHROUGH:
    ret = "PASSTHROUGH";
    break;
  case FiltAlgParams::APPFILTER:
    ret = f.app_filter_name;
    break;
  default:
    ret = "Unknown";
    break;
  }
  return ret;
}

//------------------------------------------------------------------
int Filter::max_elem_for_filter(const FiltAlgParams::data_filter_t f, 
				const FiltAlgParms &P)
{
  int ret;
  switch (f.filter)
  {
  case FiltAlgParams::CLUMP:
    ret = P.parm_clump_n;
    break;
  case FiltAlgParams::ELLIP:
  case FiltAlgParams::DILATE:
  case FiltAlgParams::SDEV:
  case FiltAlgParams::SDEV_NO_OVERLAP:
  case FiltAlgParams::TEXTURE_X:
  case FiltAlgParams::TEXTURE_Y:
    ret = P.parm_2d_n;
    break;
  case FiltAlgParams::MAX_TRUE:
    ret = P.parm_max_true_n;
    break;
  case FiltAlgParams::MEDIAN:
  case FiltAlgParams::MEDIAN_NO_OVERLAP:
    ret = P.parm_2d_median_n;
    break;
  case FiltAlgParams::REMAP:
    ret = 12;
    break;
  case FiltAlgParams::TRAPEZOID_REMAP:
    ret = P.parm_trap_remap_n;
    break;
  case FiltAlgParams::S_REMAP:
    ret = P.parm_s_remap_n;
    break;
  case FiltAlgParams::RESCALE:
    ret = P.parm_rescale_n;
    break;
  case FiltAlgParams::REPLACE:
    ret = P.parm_replace_n;
    break;
  case FiltAlgParams::MAX:
  case FiltAlgParams::AVERAGE:
  case FiltAlgParams::AVERAGE_ORIENTATION:
  case FiltAlgParams::PRODUCT:
  case FiltAlgParams::WEIGHTED_SUM:
  case FiltAlgParams::WEIGHTED_ORIENTATION_SUM:
  case FiltAlgParams::NORM_WEIGHTED_SUM:
  case FiltAlgParams::NORM_WEIGHTED_ORIENTATION_SUM:
    ret = 10;
    break;
  case FiltAlgParams::FULL_MEAN:
  case FiltAlgParams::FULL_SDEV:
  case FiltAlgParams::FULL_MEDIAN:
    ret = P.parm_Scalar_n;
    break;
  case FiltAlgParams::MASK:
    ret = P.parm_mask_n;
    break;
  case FiltAlgParams::VERT_AVERAGE:
  case FiltAlgParams::VERT_MAX:
  case FiltAlgParams::VERT_PRODUCT:
  case FiltAlgParams::DB2LINEAR:
  case FiltAlgParams::LINEAR2DB:
  case FiltAlgParams::PASSTHROUGH:
    ret = P.parm_dummy_n;
    break;
  case FiltAlgParams::APPFILTER:
    ret = P.app_max_elem_for_filter(f);
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

//------------------------------------------------------------------
const Data *Filter::set_data(const bool is_input_field, const char *name,
			     const vector<Data> &in, const vector<Data> &out) 
{
  if (is_input_field)
  {
    for (int i=0; i<static_cast<int>(in.size()); ++i)
    {
      if (in[i].name_equals(name))
      {
	return &in[i];
      }
    }
  }
  else
  {
    for (int i=0; i<static_cast<int>(out.size()); ++i)
    {
      if (out[i].name_equals(name))
      {
	return &out[i];
      }
    }
  }
  return NULL;
}

//------------------------------------------------------------------
bool Filter::createGridAtVlevel(const FiltInfoInput &inp,
				FiltInfoOutput &o)
{
  if (!inp.hasVlevels())
  {
    LOG(ERROR) << "can't filter 1 data value";
    o.setBad();
    return false;
  }
  if (!inp.getSlice()->is_grid())
  {
    LOG(ERROR) << "input not a grid";
    o.setBad();
    return false;
  }
  o = FiltInfoOutput(*inp.getSlice(), o.getExtra());
  return true;
}
