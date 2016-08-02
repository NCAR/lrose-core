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
 * @file Data2d.cc
 */
#include <FiltAlg/Data2d.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
static bool _scan_double_array(const string &s, const string &tag,
			       const string &inner_tag, vector<double> &ret)
{
  string locs;
  ret.clear();
  if (TaXml::readString(s, tag, locs) != 0)
  {
    LOG(ERROR) << "reading tag " << tag;
    return false;
  }
  vector<string> v;
  if (TaXml::readStringArray(locs, inner_tag, v) != 0)
  {
    LOG(ERROR) << "reading tag " << inner_tag;
    return false;
  }

  bool status = true;
  for (int i=0; i<static_cast<int>(v.size()); ++i)
  {
    double vi;
    if (sscanf(v[i].c_str(), "%lf", &vi) != 1)
    {
      LOG(ERROR) << "scanning '" << v[i] << "' as a double";
      status = false;
    }
    else
    {
      ret.push_back(vi);
    }
  }
  return status;
}

//------------------------------------------------------------------
Data2d::Data2d()
{
  _name = bad_2d_name();
}

//------------------------------------------------------------------
Data2d::Data2d(const string &name)
{
  _name = name;
}

//------------------------------------------------------------------
Data2d::~Data2d()
{
}

//------------------------------------------------------------------
void Data2d::print_2d(void) const
{
  printf("%s:\n", _name.c_str());
  for (int i=0; i<static_cast<int>(_v.size()); ++i)
  {
    printf("%lf ", _v[i]);
  }
  printf("\n");
}

//------------------------------------------------------------------
void Data2d::add(const double value)
{
  _v.push_back(value);
}

//------------------------------------------------------------------
string Data2d::write_xml(void) const
{
  string s = TaXml::writeStartTag("Data2d", 0);
  s += TaXml::writeString("Data2dName", 0, _name);
  s += TaXml::writeStartTag("Data2dValues", 0);
  for (int i=0; i<static_cast<int>(_v.size()); ++i)
  {
    s += TaXml::writeDouble("Data2dValue", 0, _v[i], "%.5lf");
  }
  s += TaXml::writeEndTag("Data2dValues", 0);
  s += TaXml::writeEndTag("Data2d", 0);
  return s;
}

//------------------------------------------------------------------
bool Data2d::set_from_xml(const string &s)
{
  bool stat = true;
  if (TaXml::readString(s, "Data2dName", _name) != 0)
  {
    LOG(ERROR) << "reading tag Data2dName";
    stat = false;
  }
  if (!_scan_double_array(s, "Data2dValues", "Data2dValue", _v))
  {
    stat = false;
  }
  return stat;
}
