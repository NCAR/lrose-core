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
 * @file FiltSRemap.cc
 */
#include <FiltAlg/FiltSRemap.hh>

//------------------------------------------------------------------
FiltSRemap::FiltSRemap(const FiltAlgParams::data_filter_t f,
			     const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  double a = P._parm_s_remap[f.filter_index].a;
  double b = P._parm_s_remap[f.filter_index].b;

  _f = SFuzzyF(a, b);
  if (!_f.ok())
  {
    _ok = false;
    return;
  }
}

//------------------------------------------------------------------
FiltSRemap::~FiltSRemap()
{
}

//------------------------------------------------------------------
void FiltSRemap::initialize_output(const Data &inp,
				      const FiltAlgParams::data_filter_t f,
				      Data &g)
{
  Data::Data_t type = inp.get_type();

  // same type for input and output, new name
  g = Data(f.output_field, type, f.write_output_field);
}

//------------------------------------------------------------------
void FiltSRemap::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltSRemap::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltSRemap::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  // possibilities:
  // input = grid slice, output = grid slice
  // input = single value at this vlevel, output = single value at this vlevel
  bool stat = true;
  double v;

  if (inp.hasVlevels())
  {
    const VlevelSlice *in = inp.getSlice();
    switch (inp.getDataOut()->get_type())
    {
    case Data::GRID3D:
      o = FiltInfoOutput(*in, NULL);
      if (_remap(o))
      {
	stat = true;
      }
      else
      {
	stat = false;
      }
      break;
    case Data::DATA2D:
      if (in->get_1d_value(v))
      {
	o = FiltInfoOutput(v, NULL);
	stat = _remap(o);
      }
      else
      {
	stat = false;
      }
      break;
    case Data::DATA1D:
      LOG(ERROR) << "wrong filter method called (1d)";
      stat = false;
      break;
    default:
      LOG(ERROR) << "unknown type in FiltSRemap: " 
		 << inp.getDataOut()->get_type();
      stat = false;
      break;
    }
  }
  else
  {
    switch (inp.getOutput1d()->get_type())
    {
    case Data::GRID3D:
    case Data::DATA2D:
      LOG(ERROR) << "wrong filter method called (not 1d)";
      stat = false;
      break;
    case Data::DATA1D:
      if (inp.getInput1d()->get_1d_value(v))
      {
	o = FiltInfoOutput(v, NULL);
	stat = _remap(o);
      }
      else
      {
	stat = false;
      }
      break;
    default:
      LOG(ERROR) << "unknown type in FiltSRemap: "
		 << inp.getOutput1d()->get_type();
      stat = false;
      break;
    }
  }

  if (!stat)
  {
    o.setBad();
  }    
  return stat;
}

//------------------------------------------------------------------
bool FiltSRemap::create_inputs(const time_t &t, 
			      const vector<Data> &input,
			      const vector<Data> &output)
{
  return true;
}

//------------------------------------------------------------------
void FiltSRemap::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltSRemap::store_outputs(const Data &o, Info *info,
				  vector<FiltInfo> &extra,
				  vector<Data> &output)
{
  output.push_back(o);
  return true;
}


//------------------------------------------------------------------
void FiltSRemap::set_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltSRemap::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltSRemap::vertical_level_change(void)
{
  // default is to do nothing
}

//------------------------------------------------------------------
bool FiltSRemap::_remap(FiltInfoOutput &o) const
{
  if (o.isGrid())
  {
    for (int iy=0; iy<o.getNy(); ++iy)
    {
      for (int ix=0; ix<o.getNx(); ++ix)
      {
	double v;
	if (o.getValue(ix, iy, v))
	{
	  v = _f.apply(v);
	  o.setValue(ix, iy, v);
	}
      }
    }
    return true;
  }
  else
  {
    // get input value and convert into output value
    double v;
    if (o.getFiltInfoValue(v))
    {
      v = _f.apply(v);
      o = FiltInfoOutput(v, NULL);
      return true;
    }
    else
    {
      return false;
    }
  }
}

