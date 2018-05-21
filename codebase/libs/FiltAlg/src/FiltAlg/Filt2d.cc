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
 * @file Filt2d.cc
 */
#include <FiltAlg/Filt2d.hh>

//------------------------------------------------------------------
Filt2d::Filt2d(const FiltAlgParams::data_filter_t f, const FiltAlgParms &P) :
  Filter(f, P)
{
  _nr = _ntheta = 0;
  if (!ok())
  {
    return;
  }
  _nr = P._parm_2d[f.filter_index].nr;
  _ntheta = P._parm_2d[f.filter_index].ntheta;
}

//------------------------------------------------------------------
Filt2d::~Filt2d()
{
}

//------------------------------------------------------------------
void Filt2d::initialize_output(const Data &inp,
			       const FiltAlgParams::data_filter_t f, Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

//------------------------------------------------------------------
void Filt2d::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void Filt2d::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool Filt2d::filter(const FiltInfoInput &inp, FiltInfoOutput &out) const
{
  if (!createGridAtVlevel(inp, out))
  {
    return false;
  }

  switch (_f.filter)
  {
  case FiltAlgParams::ELLIP:
    out.smooth(_nr, _ntheta);
    break;
  case FiltAlgParams::DILATE:
    out.dilate(_nr, _ntheta);
    break;
  case FiltAlgParams::SDEV:
    out.sdev(_nr, _ntheta);
    break;
  case FiltAlgParams::TEXTURE_X:
    out.texture(_nr, _ntheta, true);
    break;
  case FiltAlgParams::TEXTURE_Y:
    out.texture(_nr, _ntheta, false);
    break;
  default:
    LOG(ERROR) << "wrong filter";
    out.setBad();
    return false;
  }

  return true;
}

//------------------------------------------------------------------
bool Filt2d::create_inputs(const time_t &t, 
			   const vector<Data> &input,
			   const vector<Data> &output)
{
  return true;
}

//------------------------------------------------------------------
void Filt2d::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool Filt2d::store_outputs(const Data &o, Info *info,
			   vector<FiltInfo> &extra,
			   vector<Data> &output)
{
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void Filt2d::set_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void Filt2d::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void Filt2d::vertical_level_change(void)
{
  // default is to do nothing
}


