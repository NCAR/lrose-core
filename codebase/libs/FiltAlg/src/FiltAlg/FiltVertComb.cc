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
 * @file FiltVertComb.cc
 */
#include <FiltAlg/FiltVertComb.hh>
#include <FiltAlg/BasicInfo.hh>

//------------------------------------------------------------------
FiltVertComb::FiltVertComb(const FiltAlgParams::data_filter_t f,
			   const FiltAlgParms &P) : Filter(f, P)
{
}

//------------------------------------------------------------------
FiltVertComb::~FiltVertComb()
{
}

//------------------------------------------------------------------
void FiltVertComb::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltVertComb::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
void FiltVertComb::initialize_output(const Data &inp,
				     const FiltAlgParams::data_filter_t f,
				     Data &g)
{
  // output is a single #.
  g = Data(f.output_field, Data::DATA1D, f.write_output_field);
}

//------------------------------------------------------------------
bool FiltVertComb::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  if (!inp.hasVlevels())
  {
    LOG(ERROR) << "can't filter 1d input";
    o.setBad();
    return false;
  }
  if (inp.getDataOut()->get_type() != Data::DATA1D)
  {
    LOG(ERROR) << "wrong output (want data1d)";
    o.setBad();
    return false;
  }
  if (inp.getSlice()->is_grid())
  {
    LOG(ERROR) << "input slice is grid, want data1d";
    o.setBad();
    return false;
  }

  double v;
  if (!inp.getSlice()->get_1d_value(v))
  {
    LOG(ERROR) << "retrieving input 1d value";
    o.setBad();
    return false;
  }

  VertCombExtra *e = (VertCombExtra *)o.getExtra();
  if (e == NULL)
  {
    LOG(ERROR) << "No pointer to ver comb extra";
    o.setBad();
    return false;
  }
  e->_v = v;
  o.setNoOutput();
  return true;

}

//------------------------------------------------------------------
bool FiltVertComb::create_inputs(const time_t &t, 
				 const vector<Data> &input,
				 const vector<Data> &output)
{
  return true;
}  

//------------------------------------------------------------------
void FiltVertComb::create_extra(FiltInfo &info) const
{
  VertCombExtra *e = new VertCombExtra();
  info.getOutput().storeExtra((void *)e);
}

//------------------------------------------------------------------
bool FiltVertComb::store_outputs(const Data &o, Info *info,
				 vector<FiltInfo> &extra,
				 vector<Data> &output)
{
  double v = 0.0, count=0.0;
  for (size_t i = 0; i<extra.size(); ++i)
  {
    VertCombExtra *e = (VertCombExtra *)extra[i].getOutput().getExtra();

    switch (_f.filter)
    {
    case FiltAlgParams::VERT_AVERAGE:
      v += e->_v;
      count += 1.0;
      break;
    case FiltAlgParams::VERT_MAX:
      if (count == 0.0)
      {
	v = e->_v;
	count = 1;
      }
      else
      {
	if (e->_v > v)
	{
	  v = e->_v;
	}
      }
      break;
    case FiltAlgParams::VERT_PRODUCT:
      if (count == 0)
      {
	v = e->_v;
	count = 1;
      }
      else
      {
	v *= e->_v;
      }
      break;
    default:
      LOG(ERROR) << "bad logic";
      return false;
      break;
    }

    delete e;
  }
  
  bool stat = true;

  // store final value to copy of output
  Data tmp(o);
  if (!tmp.set_1d_value(v))
  {
    stat = false;
  }

  // store the copy of output to output
  output.push_back(tmp);

  // store Data1D base class of copy of output to info.
  if (!info->add_data1d(tmp))
  {
    stat = false;
  }

  return stat;
}

//------------------------------------------------------------------
void FiltVertComb::set_info(Info **info) const
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
void FiltVertComb::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltVertComb::vertical_level_change(void)
{
  // default is to do nothing
}

