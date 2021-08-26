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
 * @file SFuzzyF.cc
 */

//------------------------------------------------------------------
#include <rapmath/SFuzzyF.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <cstdio>

//------------------------------------------------------------------
SFuzzyF::SFuzzyF() : _ok(false), _a(0), _b(0)
{
}
  
//------------------------------------------------------------------
SFuzzyF::SFuzzyF(double a, double b) :
  _ok(true), _a(a), _b(b)
{
  if (a >= b)
  {
    LOG(ERROR) << " arguments a,b must be ascending, not "
	       << a << "," << b;
    _ok = false;
  }
}

//------------------------------------------------------------------
SFuzzyF::~SFuzzyF()
{
}

//------------------------------------------------------------------
bool SFuzzyF::operator==(const SFuzzyF &f) const
{
  return _a == f._a && _b == f._b;
}

//------------------------------------------------------------------
void SFuzzyF::print(void) const
{
  printf("(a,b):(%10.5lf,%10.5lf)\n", _a, _b);
}

//------------------------------------------------------------------
void SFuzzyF::print(ostream &stream) const
{
  stream << "(a,b)(" << _a << "," << _b << ")" << endl;
}

//------------------------------------------------------------------
double SFuzzyF::apply(double x) const
{
  if (x <= _a)
  {
    return 0.0;
  }
  else if (x >= _a && x <= (_a+_b)/2.0)
  {
    return 2.0*(x-_a)*(x-_a)/((_b-_a)*(_b-_a));
  }
  else if (x >= (_a+_b)/2.0 && x <= _b)
  {
    return 1.0 - 2.0*(x-_b)*(x-_b)/((_b-_a)*(_b-_a));
  }
  else if (x >= _b)
  {
    return 1.0;
  }
  else
  {
    return 0.0;
  }
}

//------------------------------------------------------------------
string SFuzzyF::xmlContent(const string &name) const
{
  // write values as XML
  string s = TaXml::writeDouble("S_Shaped_A", 0, _a, "%.5lf");
  s += TaXml::writeDouble("S_Shaped_B", 0, _b, "%.5lf");

  // write all of that out as the named string
  string ret = TaXml::writeString(name, 0, s);
  return ret;
}

//------------------------------------------------------------------
bool SFuzzyF::readXml(const string &s, const string &name)
{
  // try to get the string that has input name as the tag
  string content;
  if (TaXml::readString(s, name, content))
  {
    LOG(ERROR) << "Reading tag " << name;
    // didn't find it
    return false;
  }

  // now expect to find the fixed tags
  if (TaXml::readDouble(content, "S_Shaped_A", _a))
  {
    LOG(ERROR) << "Reading tag S_Shaped_A";
    return false;
  }
  if (TaXml::readDouble(content, "S_Shaped_B", _b))
  {
    LOG(ERROR) << "Reading tag S_Shaped_B";
    return false;
  }
  return true;
}
