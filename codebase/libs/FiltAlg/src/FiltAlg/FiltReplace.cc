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
 * @file FiltReplace.cc
 */
#include <FiltAlg/FiltReplace.hh>
#include <FiltAlg/BasicInfo.hh>

//------------------------------------------------------------------
FiltReplace::FiltReplace(const FiltAlgParams::data_filter_t f, 
			 const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  _set_initial_value = P._parm_replace[f.filter_index].set_initial_value;
  _initial_value = P._parm_replace[f.filter_index].initial_value;
  _replacement_value = P._parm_replace[f.filter_index].replacement_value;
  _vlevel_tolerance = P.vlevel_tolerance;

  // comb object because it already exists
  _comb = Comb(P, P._parm_replace[f.filter_index].combine_index);
  if (!_comb.ok())
  {
    _ok = false;
  }

  // parse the string to get the logical tests
  _find = Find(P._parm_replace[f.filter_index].logical_command,
	       P.vlevel_tolerance);
  if (!_find.ok())
  {
    _ok = false;
  }

  if (!_find.isConsistent(_comb))
  {
    _ok = false;
  }
}

//------------------------------------------------------------------
FiltReplace::~FiltReplace()
{
}

//------------------------------------------------------------------
void FiltReplace::initialize_output(const Data &inp,
				    const FiltAlgParams::data_filter_t f,
				    Data &g)
{
  g = Data(f.output_field, inp.get_type(), f.write_output_field);
}

//------------------------------------------------------------------
void FiltReplace::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltReplace::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltReplace::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  if (!createGridAtVlevel(inp, o))
  {
    return false;
  }
  if (_set_initial_value)
  {
    // note we are not even using the input
    o.setAllToValue(_initial_value);
  }

  // at each point, get a true false from the 
  for (int i=0; i< o.getNdata(); ++i)
  {
    if (_find.satisfiesConditions(i, inp.getVlevel()))
    {
      o.setValue(i, _replacement_value);
    }
  }

  return true;
}

//------------------------------------------------------------------
bool FiltReplace::create_inputs(const time_t &t, 
				const vector<Data> &input,
				const vector<Data> &output)
{
  if (!_comb.create_inputs(input, output))
  {
    return false;
  }

  // now match up the data with the Find object
  return _find.setPointers(_comb);
}  

//------------------------------------------------------------------
void FiltReplace::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltReplace::store_outputs(const Data &o, Info *info,
				vector<FiltInfo> &extra,
				vector<Data> &output)
{
  // store to output
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void FiltReplace::set_info(Info **info) const
{
}

//------------------------------------------------------------------
void FiltReplace::set_input_info(Info **info) const
{
}

