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
#include <FiltAlg/FiltCombine.hh>
#include <FiltAlg/BasicInfo.hh>

//------------------------------------------------------------------
FiltCombine::FiltCombine(const FiltAlgParams::data_filter_t f, 
			 const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _weight0 = P._parm_combine[f.filter_index].input_weight;
  _comb = Comb(P, P._parm_combine[f.filter_index].combine_index);
  if (!_comb.ok())
  {
    _ok = false;
  }
}

//------------------------------------------------------------------
FiltCombine::~FiltCombine()
{
}

//------------------------------------------------------------------
void FiltCombine::initialize_output(const Data &inp,
				    const FiltAlgParams::data_filter_t f,
				    Data &g)
{
  // same type input and output is all that we know.
  g = Data(f.output_field, inp.get_type(), f.write_output_field);
}

//------------------------------------------------------------------
void FiltCombine::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltCombine::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltCombine::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  bool stat = true;

  if (inp.hasVlevels())
  {
    // filter depends on type of data (output = input)
    switch (inp.getDataOut()->get_type())
    {
    case Data::GRID3D:
    case Data::DATA2D:
      stat = _filter_slice(inp, o);
      break;
    case Data::DATA1D:
      LOG(ERROR) <<  "wrong method (1ddata, no slices)";
      stat = false;
      break;
    default:
      LOG(ERROR) << "unknown type,FiltCombine:filter() " 
		 << inp.getDataOut()->get_type();
      stat = false;
      break;
    }
  }
  else
  {
    if (inp.getInput1d()->get_type() != inp.getOutput1d()->get_type())
    {
      LOG(ERROR) <<  "1d filter, mismatch of types";
      stat = false;
    }
    else
    {
      if (inp.getOutput1d()->get_type() != Data::DATA1D)
      {
	LOG(ERROR) <<  "wrong filter method, want 1d data";
	stat = false;
      }
      else
      {
	stat = _filter_data1d(*inp.getInput1d(), *inp.getOutput1d(), o);
      }
    }
  }

  if (!stat)
  {
    o.setBad();
  }
  return stat;
}

// //------------------------------------------------------------------
// bool FiltCombine::filter(const Data &in, const Data &out, FiltInfo &o) const
// {
//   if (in.get_type() != out.get_type())
//   {
//     LOG(ERROR) <<  "1d filter, mismatch of types";
//     o.setBad();
//     return false;
//   }
//   if (out.get_type() != Data::DATA1D)
//   {
//     LOG(ERROR) <<  "wrong filter method, want 1d data";
//     o.setBad();
//     return false;
//   }
//   return _filter_data1d(in, out, o);
// }

//------------------------------------------------------------------
bool FiltCombine::create_inputs(const time_t &t, 
				const vector<Data> &input,
				const vector<Data> &output)
{
  return _comb.create_inputs(input, output);
}  

//------------------------------------------------------------------
void FiltCombine::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltCombine::store_outputs(const Data &o, Info *info,
				vector<FiltInfo> &extra,
				vector<Data> &output)
{
  // store to output
  output.push_back(o);

  Data2d d, vlevels;
  bool stat = true;

  switch (o.get_type())
  {
  case Data::GRID3D:
    // no additional info for 3d grids
    break;
  case Data::DATA2D:
    // build Data2d objects from Data for data and vlevels
    o.construct_data2d(d, vlevels);
    // add to info.
    info->add_data2d(d, vlevels);
    break;
  case Data::DATA1D:
    // add Data1d base class of output as Data1d to info.
    if (!info->add_data1d(o))
    {
      stat = false;
    }
    break;
  default:
    break;
  }
  return stat;
}

//------------------------------------------------------------------
void FiltCombine::set_info(Info **info) const
{
  // yes because we can write stuff to info (see above)
  if (*info == NULL)
  {
    BasicInfo *b = new BasicInfo();
    *info = (Info *)b;
  }

  // otherwise the base class is good enough
}

//------------------------------------------------------------------
void FiltCombine::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltCombine::_filter_slice(const FiltInfoInput &inp,
				FiltInfoOutput &o) const
{
  Data::Data_t type = inp.getDataOut()->get_type();

  // is all the input data of the same type?
  if (!_check_data(type, *inp.getSlice()))
  {
    return false;
  }

  // proceed
  VlevelSlice v(*inp.getSlice());

  double vlevel = inp.getVlevel();

  bool stat = true;
  switch (_f.filter)
  {
  case FiltAlgParams::MAX:
    stat = _comb.max(vlevel, v);
    break;
  case FiltAlgParams::AVERAGE:
    stat = _comb.average(vlevel, false, v);
    break;
  case FiltAlgParams::AVERAGE_ORIENTATION:
    stat = _comb.average(vlevel, true, v);
    break;
  case FiltAlgParams::PRODUCT:
    stat = _comb.product(vlevel, v);
    break;
  case FiltAlgParams::WEIGHTED_SUM:
    stat = _comb.weighted_sum(vlevel, _weight0, false, false, v);
    break;
  case FiltAlgParams::WEIGHTED_ORIENTATION_SUM:
    stat = _comb.weighted_sum(vlevel, _weight0, false, true, v);
    break;
  case FiltAlgParams::NORM_WEIGHTED_SUM:
    stat = _comb.weighted_sum(vlevel, _weight0, true, false, v);
    break;
  case FiltAlgParams::NORM_WEIGHTED_ORIENTATION_SUM:
    stat = _comb.weighted_sum(vlevel, _weight0, true, true, v);
    break;
  default:
    LOG(ERROR) <<  "wrong logic";
    stat = false;
  }

  if (stat)
  {
    // int index = in.get_vlevel_index();
    if (type == Data::GRID3D)
    {
      o = FiltInfoOutput(v, NULL);

      // // add the grid for this vlevel
      // out.add(o, vlevel, index, gp);
    }
    else
    {
      double val;
      if (v.get_1d_value(val))
      {

	o = FiltInfoOutput(val, NULL);

	// // add the data value for this vlevel
	// out.add(vlevel, index, v);
      }
      else
      {
	stat = false;
      }
    }
  }
  return stat;
}

//------------------------------------------------------------------
bool FiltCombine::_filter_data1d(const Data &in, const Data &out,
				 FiltInfoOutput &o) const
{
  // is all the data what we want?
  if (!_comb.check_data(Data::DATA1D))
  {
    o.setBad();
    return false;
  }
  
  // in this case want to use the Data1d base class of Data to get
  // numbers.
  double v;

  if (!in.get_1d_value(v))
  {
    return false;
  }
    
  bool status;
  switch (_f.filter)
  {
  case FiltAlgParams::MAX:
    status = _comb.max(v);
    break;
  case FiltAlgParams::AVERAGE:
    status = _comb.average(false, v);
    break;
  case FiltAlgParams::AVERAGE_ORIENTATION:
    status = _comb.average(true, v);
    break;
  case FiltAlgParams::PRODUCT:
    status = _comb.product(v);
    break;
  case FiltAlgParams::WEIGHTED_SUM:
    status = _comb.weighted_sum(_weight0, false, false, v);
    break;
  case FiltAlgParams::WEIGHTED_ORIENTATION_SUM:
    status = _comb.weighted_sum(_weight0, false, true, v);
    break;
  case FiltAlgParams::NORM_WEIGHTED_SUM:
    status = _comb.weighted_sum(_weight0, true, false, v);
    break;
  case FiltAlgParams::NORM_WEIGHTED_ORIENTATION_SUM:
    status = _comb.weighted_sum(_weight0, true, true, v);
    break;
  default:
    LOG(ERROR) <<  "wrong logic";
    o.setBad();
    return false;
  }
  if (!status)
  {
    o.setBad();
    return false;
  }
  else
  {
    o = FiltInfoOutput(v, NULL);
    return true;
  }
}

//------------------------------------------------------------------
bool FiltCombine::_check_data(const Data::Data_t type,
			      const VlevelSlice &data) const
{
  if (type == Data::GRID3D)
  {
    if (!data.is_grid())
    {
      LOG(ERROR) <<  "input not a grid whereas output is";
      return false;
    }
  }
  else if (type == Data::DATA2D)
  {
    if (data.is_grid())
    {
      LOG(ERROR) <<  "input a grid whereas output 2d Data";
      return false;
    }
  }
  return _comb.check_data(type);
}

