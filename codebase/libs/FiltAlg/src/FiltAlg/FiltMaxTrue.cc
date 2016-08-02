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
 * @file FiltMaxTrue.cc
 */
#include <FiltAlg/FiltMaxTrue.hh>
#include <FiltAlg/BasicInfo.hh>

//------------------------------------------------------------------
FiltMaxTrue::FiltMaxTrue(const FiltAlgParams::data_filter_t f, 
			 const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  // comb object because it already exists
  _comb = Comb(P, P._parm_max_true[f.filter_index].combine_index);
  if (!_comb.ok())
  {
    _ok = false;
  }
  _vlevel_tolerance = P.vlevel_tolerance;

  switch (P._parm_max_true[f.filter_index].strings_index)
  {
  case 0:
    _init(P._strings0, P.strings0_n);
    break;
  case 1:
    _init(P._strings1, P.strings1_n);
    break;
  case 2:
    _init(P._strings2, P.strings2_n);
    break;
  case 3:
    _init(P._strings3, P.strings3_n);
    break;
  case 4:
    _init(P._strings4, P.strings4_n);
    break;
  default:
    LOG(ERROR) << "strings index " 
	       <<  P._parm_max_true[f.filter_index].strings_index
	       <<  " out of range 0 to 4";
	 
    _ok = false;
  }

  if (_ok)
  {
    if (!_isConsistent())
    {
      _ok = false;
    }
  }
}

//------------------------------------------------------------------
FiltMaxTrue::~FiltMaxTrue()
{
}

//------------------------------------------------------------------
void FiltMaxTrue::initialize_output(const Data &inp,
				    const FiltAlgParams::data_filter_t f,
				    Data &g)
{
  g = Data(f.output_field, inp.get_type(), f.write_output_field);
}

//------------------------------------------------------------------
void FiltMaxTrue::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltMaxTrue::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltMaxTrue::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  if (!createGridAtVlevel(inp, o))
  {
    return false;
  }

  o.setAllMissing();

  double vlevel = inp.getVlevel();

  // at each point, do the thing
  for (int i=0; i< o.getNdata(); ++i)
  {
    double max = 0.0;
    bool first = true;
    for (size_t j=0; j<_rules.size(); ++j)
    {
      if (_rules[j].satisfiesCondition(i, vlevel, _vlevel_tolerance))
      {
	double v;
	if (_rules[j].getValue(i, vlevel, _vlevel_tolerance, v))
	{
	  if (first)
	  {
	    first = false;
	    max = v;
	  }
	  else
	  {
	    if (v > max)
	    {
	      max = v;
	    }
	  }
	}
      }
    }
    if (!first)
    {
      o.setValue(i, max);
    }
  }
  return true;
}

//------------------------------------------------------------------
bool FiltMaxTrue::create_inputs(const time_t &t, 
				const vector<Data> &input,
				const vector<Data> &output)
{
  if (!_comb.create_inputs(input, output))
  {
    return false;
  }

  bool stat = true;

  // now match up the data with the Find object
  for (size_t i=0; i<_rules.size(); ++i)
  {
    if (!_rules[i].setPointer(_comb))
    {
      stat = false;
    }
  }
  return stat;
}  

//------------------------------------------------------------------
void FiltMaxTrue::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltMaxTrue::store_outputs(const Data &o, Info *info,
				vector<FiltInfo> &extra,
				vector<Data> &output)
{
  // store to output
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void FiltMaxTrue::set_info(Info **info) const
{
}

//------------------------------------------------------------------
void FiltMaxTrue::set_input_info(Info **info) const
{
}

//------------------------------------------------------------------
void FiltMaxTrue::_init(char **strings, const int n)
{
  for (int i=0; i<n; ++i)
  {
    _init1(strings[i]);
  }
}

//------------------------------------------------------------------
void FiltMaxTrue::_init1(const std::string &s)
{
  Find F(s, _vlevel_tolerance);
  if (!F.ok())
  {
    _ok= false;
    return;
  }
  if (!F.is_single())
  {
    LOG(ERROR) << "Need only simple tests for this filter, not '" << s << "'";
    _ok = false;
    return;
  }
  _rules.push_back(F.get_single());
}

//------------------------------------------------------------------
bool FiltMaxTrue::_isConsistent(void) const
{
  bool stat = true;
  for (size_t i=0; i<_rules.size(); ++i)
  {
    if (!_rules[i].isConsistent(_comb))
    {
      stat = false;
    }
  }
  return stat;
}
