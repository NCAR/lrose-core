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
 * @file FiltPassThrough.cc
 */
#include <FiltAlg/FiltPassThrough.hh>

//------------------------------------------------------------------
FiltPassThrough::FiltPassThrough(const FiltAlgParams::data_filter_t f,
				 const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
}

//------------------------------------------------------------------
FiltPassThrough::~FiltPassThrough()
{
}

//------------------------------------------------------------------
void FiltPassThrough::initialize_output(const Data &inp,
					const FiltAlgParams::data_filter_t f,
					Data &g)
{
  Data::Data_t type = inp.get_type();
  string field = inp.get_name();

  // same type and name for input and output
  g = Data(field, type, f.write_output_field);
}

//------------------------------------------------------------------
void FiltPassThrough::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltPassThrough::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltPassThrough::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  bool stat = true;

  if (inp.hasVlevels())
  {
    const VlevelSlice *in = inp.getSlice();

    // no filtering needed, create the proper FiltInfo
    if (in->is_grid())
    {
      o = FiltInfoOutput(*in, NULL);
    }
    else
    {
      double v;
      if (in->get_1d_value(v))
      {
	o = FiltInfoOutput(v, NULL);
      }
      else
      {
	o.setBad();
	stat = false;
      }
    }
  }
  else
  {
    // no filtering needed
    double v;
    if (inp.getInput1d()->get_1d_value(v))
    {
      o = FiltInfoOutput(v, NULL);
    }
    else
    {
      o.setBad();
      stat = false;
    }
  }
  return stat;
}

//------------------------------------------------------------------
bool FiltPassThrough::create_inputs(const time_t &t, 
				    const vector<Data> &input,
				    const vector<Data> &output)
{
  return true;
}

//------------------------------------------------------------------
void FiltPassThrough::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltPassThrough::store_outputs(const Data &o, Info *info,
				    vector<FiltInfo> &extra,
				    vector<Data> &output)
{
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void FiltPassThrough::set_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltPassThrough::set_input_info(Info **info) const
{
  return;
}
