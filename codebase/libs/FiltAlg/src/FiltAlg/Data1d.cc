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
 * @file Data1d.cc
 */
#include <FiltAlg/Data1d.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Data1d::Data1d(void)
{
  _name = bad_1d_name();
  _value = 0;

}

//------------------------------------------------------------------
Data1d::Data1d(const string &name)
{
  _name = name;
  _value = 0;

}

//------------------------------------------------------------------
Data1d::Data1d(const string &name, const double value)
{
  _name = name;
  _value = value;
}

//------------------------------------------------------------------
Data1d::~Data1d(void)
{
}

//------------------------------------------------------------------
void Data1d::print_1d(void) const
{
  printf("%s[%6.2lf]\n", _name.c_str(), _value);
}

//------------------------------------------------------------------
string Data1d::write_xml(void) const
{
  string s = TaXml::writeStartTag("Data1d", 0);
  s += TaXml::writeString("Data1dName", 0, _name);
  s += TaXml::writeDouble("Data1dValue", 0, _value, "%.5lf");
  s += TaXml::writeEndTag("Data1d", 0);
  return s;
}

//------------------------------------------------------------------
bool Data1d::set_from_xml(const string &s)
{
  bool stat = true;
  if (TaXml::readString(s, "Data1dName", _name) != 0)
  {
    LOG(ERROR) << "reading tag Data1dName";
    stat = false;
  }
  if (TaXml::readDouble(s, "Data1dValue", _value) != 0)
  {
    LOG(ERROR) << "reading tag Data1dValue";
    stat = false;
  }
  return stat;
}

//------------------------------------------------------------------
bool Data1d::get_1d_value(double &v) const
{
  if (_name == bad_1d_name())
  {
    v = 0;
    LOG(ERROR) << "getting 1d value, not set";
    return false;
  }
  else
  {
    v = _value;
    return true;
  }
}

//------------------------------------------------------------------
void Data1d::set_1d_name(const string &n)
{
  _name = n;
}

//------------------------------------------------------------------
bool Data1d::set_1d_value(const double v)
{
  if (_name == bad_1d_name())
  {
    LOG(ERROR) << "setting 1d value, name not set";
    _value = 0;
    return false;
  }
  else
  {
    _value = v;
    return true;
  }
}
  
//------------------------------------------------------------------
bool Data1d::add_1d_value(const Data1d &d)
{
  if (_name == bad_1d_name())
  {
    LOG(ERROR) << "name not set";
    return false;
  }
  else
  {
    if (d._name == bad_1d_name())
    {
      LOG(ERROR) << "name not set";
      return false;
    }
    else
    {
      _value += d._value;
      return true;
    }
  }
}  

//------------------------------------------------------------------
bool Data1d::inc_1d_value(const double v)
{
  if (_name == bad_1d_name())
  {
    LOG(ERROR) << "name not set";
    return false;
  }
  else
  {
    _value += v;
    return true;
  }
}  

//------------------------------------------------------------------
bool Data1d::multiply_1d_value(const Data1d &d)
{
  if (_name == bad_1d_name())
  {
    LOG(ERROR) << "name not set";
    return false;
  }
  else
  {
    if (d._name == bad_1d_name())
    {
      LOG(ERROR) << "name not set";
      return false;
    }
    else
    {
      _value *= d._value;
      return true;
    }
  }
}  

//------------------------------------------------------------------
bool Data1d::prod_1d_value(const double v)
{
  if (_name == bad_1d_name())
  {
    LOG(ERROR) << "name not set";
    return false;
  }
  else
  {
    _value *= v;
    return true;
  }
}  

