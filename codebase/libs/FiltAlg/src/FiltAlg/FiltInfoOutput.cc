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
 * @file FiltInfo.cc
 */
#include <FiltAlg/FiltInfoOutput.hh>
#include <FiltAlg/Data.hh>
#include <FiltAlg/GridProj.hh>

//------------------------------------------------------------------
FiltInfoOutput::FiltInfoOutput(void *extra) :
  GridAlgs(), _value(-1), _type(NONE), _status(false), _extra(extra)
{
  
}

//------------------------------------------------------------------
FiltInfoOutput::FiltInfoOutput(const Grid2d &g, void *extra) :
  GridAlgs(), _value(-1), _type(GRID), _status(false), _extra(extra)
{
  GridAlgs::operator=(GridAlgs::promote(g));
}

//------------------------------------------------------------------
FiltInfoOutput::FiltInfoOutput(const double v, void *extra) : 
  GridAlgs(), _value(v), _type(VALUE), _status(false), _extra(extra)
{
  
}

//------------------------------------------------------------------
FiltInfoOutput::~FiltInfoOutput()
{
}

//------------------------------------------------------------------
bool FiltInfoOutput::getFiltInfoValue(double &v) const
{
  if (_type == VALUE)
  {
    v = _value;
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool FiltInfoOutput::storeSlice(const double vlevel, const int vlevel_index,
				const GridProj &gp, Data &out) const
{
  bool stat = true;
  if (_type == NONE)
  {
    LOG(DEBUG_VERBOSE) << "Filter with no output, hopefully";
    return true;
  }
  switch (out.get_type())
  {
  case Data::GRID3D:
    if (_type == GRID)
    {
      stat = out.add(*this, vlevel, vlevel_index, gp);
    }
    else
    {
      LOG(ERROR) << "Adding " << sType(_type) << " data, expected grid";
      stat = false;
    }
    break;
  case Data::DATA2D:
    if (_type == VALUE)
    {
      stat = out.add(vlevel, vlevel_index, _value);
    }
    else
    {
      LOG(ERROR) << "Adding to DATA2D slice, wrong type = " << sType(_type);
      stat = false;
    }
    break;
  case Data::DATA1D:
  default:
    LOG(ERROR) << "Storing to slice,local data = DATA1D or not set";
    stat = false;
    break;
  }
  return stat;
}

//------------------------------------------------------------------
bool FiltInfoOutput::store1dValue(Data &out) const
{
  bool stat = true;
  switch (_type)
  {
  case VALUE:
    out.set_1d_value(_value);
    break;
  case NONE:
    LOG(DEBUG) << "Filter with no output, hopefully";
    break;
  case GRID:
  default:
    LOG(ERROR) << "cannot store 1d value, type=" << sType(_type);
    stat = false;
  }
  return stat;
}    

//------------------------------------------------------------------
void FiltInfoOutput::setBad(void)
{
  _status = false;
}

//------------------------------------------------------------------
void FiltInfoOutput::setNoOutput(void)
{
  _status = NONE;
}

//------------------------------------------------------------------
string FiltInfoOutput::sType(FiltInfoOutput::Info_t type)
{
  string s;
  switch (type)
  {
  case VALUE:
    s ="VALUE";
    break;
  case GRID:
    s = "GRID";
    break;
  case NONE:
    s = "NONE";
    break;
  default:
    s = "UNKNOWN";
    break;
  }
  return s;
}

