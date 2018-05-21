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
 * @file FiltClump.cc
 */
#include <FiltAlg/FiltClump.hh>
#include <euclid/Grid2dClump.hh>

//------------------------------------------------------------------
FiltClump::FiltClump(const FiltAlgParams::data_filter_t f,
		     const FiltAlgParms &P) :
  Filter(f, P)
{
  _thresh = 0;
  if (!ok())
  {
    return;
  }
  _thresh = P._parm_clump[f.filter_index].threshold;
}

//------------------------------------------------------------------
FiltClump::~FiltClump()
{
}

//------------------------------------------------------------------
void FiltClump::initialize_output(const Data &inp,
			       const FiltAlgParams::data_filter_t f, Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

//------------------------------------------------------------------
void FiltClump::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltClump::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltClump::filter(const FiltInfoInput &inp, FiltInfoOutput &out) const
{
  if (!createGridAtVlevel(inp, out))
  {
    return false;
  }

  out.belowThresholdToMissing(_thresh);

  Grid2dClump C(out);
  vector<clump::Region_t> r = C.buildRegions();

  out.setAllMissing();
  for (size_t i=0; i<r.size(); ++i)
  {
    for (size_t j=0; j<r[i].size(); ++j)
    {
      out.setValue(r[i][j].first, r[i][j].second, 1.0);
    }
  }
  return true;
}

//------------------------------------------------------------------
bool FiltClump::create_inputs(const time_t &t, 
			      const vector<Data> &input,
			      const vector<Data> &output)
{
  return true;
}

//------------------------------------------------------------------
void FiltClump::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltClump::store_outputs(const Data &o, Info *info,
			   vector<FiltInfo> &extra,
			   vector<Data> &output)
{
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void FiltClump::set_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltClump::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltClump::vertical_level_change(void)
{
  // default is to do nothing
}

