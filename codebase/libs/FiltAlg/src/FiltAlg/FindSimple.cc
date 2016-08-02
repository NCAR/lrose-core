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
 * @file FindSimple.cc
 */
#include <FiltAlg/FindSimple.hh>
#include <toolsa/LogStream.hh>
using std::string;

//------------------------------------------------------------------
FindSimple::FindSimple(void)
{
  _ok = false;
}

//------------------------------------------------------------------
FindSimple::FindSimple(const string &name, const string &comp,
		       const string &value)
{
  _ok = true;
  _data_name = name;
  _data = NULL;
  if (sscanf(value.c_str(), "%lf", &_value) != 1)
  {
    LOG(ERROR) << "scanning '" << value << "' as a numerical value";
    _value = 0.0;
    _ok = false;
  }
  if (comp == ">")
  {
    _test = GT;
  }
  else if (comp == ">=")
  {
    _test = GE;
  }
  else if (comp == "<")
  {
    _test = LT;
  }
  else if (comp == "<=")
  {
    _test = LE;
  }
  else if (comp == "=" || comp == "==")
  {
    _test = EQ;
  }
  else
  {
    _ok = false;
    LOG(ERROR) << "unknown comparison operator " << comp;
    _test = GT;
  }
}

//------------------------------------------------------------------
FindSimple::~FindSimple(void)
{
}

//------------------------------------------------------------------
void FindSimple::print(void) const
{
  printf("%s %s %.5lf", _data_name.c_str(), comparisonString(_test).c_str(),
	 _value);
}

//------------------------------------------------------------------
bool FindSimple::isConsistent(const Comb &comb) const
{
  if (comb.hasNamedData(_data_name))
  {
    return true;
  }
  else
  {
    LOG(ERROR) << _data_name << " not found in Combine data";
    return false;
  }
}

//------------------------------------------------------------------
bool FindSimple::setPointer(const Comb &comb)
{
  _data = comb.dataPointer(_data_name);
  if (_data != NULL)
  {
    return true;
  }
  else
  {
    LOG(ERROR) << "pointer not found in Combine data " << _data_name;
    return false;
  }
}

//------------------------------------------------------------------
bool FindSimple::getValue(const int ipt, 
			  const double vlevel,
			  const double vlevel_tolerance,
			  double &value) const
{
  if (_data == NULL)
  {
    LOG(DEBUG) << "No data";
    return false;
  }
  const VlevelSlice *gi = _data->matching_vlevel(vlevel, vlevel_tolerance);
  if (gi == NULL)
  {
    LOG(DEBUG_VERBOSE) << "No slice at " << vlevel;
    if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
    {
      _data->print();
    }
    return false;
  }
  if (!gi->is_grid())
  {
    LOG(DEBUG) << "No grid";
    return false;
  }
  double v;
  if (!gi->getValue(ipt, v))
  {
    return false;
  }
  else
  {
    value = v;
    return true;
  }
}

//------------------------------------------------------------------
bool FindSimple::satisfiesCondition(const int ipt,
				    const double vlevel,
				    const double vlevel_tolerance) const
{
  double v;
  if (!getValue(ipt, vlevel, vlevel_tolerance, v))
  {
    return false;
  }

  bool stat;
  switch (_test)
  {
  case GT:
    stat = v > _value;
    break;
  case GE:
    stat = v >= _value;
    break;
  case LT:
    stat = v < _value;
    break;
  case LE:
    stat = v <= _value;
    break;
  case EQ:
    stat = v == _value;
    break;
  default:
    stat = false;
    break;
  }
  return stat;
}

//------------------------------------------------------------------
string FindSimple::comparisonString(const Compare_t &c)
{
  string ret;
  switch (c)
  {
  case GT:
    ret = ">";
    break;
  case GE:
    ret = ">=";
    break;
  case EQ:
    ret = "=";
    break;
  case LE:
    ret = "<=";
    break;
  case LT:
    ret = "<";
    break;
  default:
    ret = "?";
    break;
  }
  return ret;
}
