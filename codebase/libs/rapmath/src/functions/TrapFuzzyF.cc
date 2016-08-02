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
 * @file TrapFuzzyF.cc
 */

//------------------------------------------------------------------
#include <rapmath/TrapFuzzyF.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <cstdio>

//------------------------------------------------------------------
TrapFuzzyF::TrapFuzzyF() : _ok(false), _a(0), _b(0), _c(0), _d(0)
{
}
  
//------------------------------------------------------------------
TrapFuzzyF::TrapFuzzyF(double a, double b, double c, double d) :
  _ok(true), _a(a), _b(b), _c(c), _d(d)
{
  if (a >= b || b >= c || c >= d)
  {
    LOG(ERROR) << " arguments a,b,c,d must be ascending, not "
	       << a << "," << b << "," << c << "," << d;
    _ok = false;
  }
}

//------------------------------------------------------------------
TrapFuzzyF::~TrapFuzzyF()
{
}

//------------------------------------------------------------------
bool TrapFuzzyF::operator==(const TrapFuzzyF &f) const
{
  return _a == f._a && _b == f._b && _c == f._c && _d == f._d;
}

//------------------------------------------------------------------
void TrapFuzzyF::print(void) const
{
  printf("(a,b,c,d):(%10.5lf,%10.5lf,%10.5lf,%10.5lf)\n", _a, _b, _c, _d);
}

//------------------------------------------------------------------
void TrapFuzzyF::print(ostream &stream) const
{
  stream << "(a,b,c,d)(" << _a << "," << _b << "," << _c << "," 
	 << _d << ")" << endl;
}

//------------------------------------------------------------------
double TrapFuzzyF::apply(double x) const
{
  if (x <= _a)
  {
    return 0.0;
  }
  else if (x >= _a && x <= _b)
  {
    return (x-_a)/(_b-_a);
  }
  else if (x >= _b && x <= _c)
  {
    return 1.0;
  }
  else if (x >= _c && x <= _d)
  {
    return (_d-x)/(_d-_c);
  }
  else
  {
    return 0.0;
  }
}

//------------------------------------------------------------------
string TrapFuzzyF::xmlContent(const string &name) const
{
  // write values as XML
  string s = TaXml::writeDouble("TrapA", 0, _a, "%.5lf");
  s += TaXml::writeDouble("TrapB", 0, _b, "%.5lf");
  s += TaXml::writeDouble("TrapC", 0, _b, "%.5lf");
  s += TaXml::writeDouble("TrapD", 0, _b, "%.5lf");

  // write all of that out as the named string
  string ret = TaXml::writeString(name, 0, s);
  return ret;
}

//------------------------------------------------------------------
bool TrapFuzzyF::readXml(const string &s, const string &name)
{
  // try to get the string that has input name as the tag
  string content;
  if (TaXml::readString(s, name, content))
  {
    LOG(ERROR) << "Reading tag " << name;
    // didn't find it
    return false;
  }

  // now expect to find the 4 fixed tags
  if (TaXml::readDouble(content, "TrapA", _a))
  {
    LOG(ERROR) << "Reading tag TrapA";
    return false;
  }
  if (TaXml::readDouble(content, "TrapB", _b))
  {
    LOG(ERROR) << "Reading tag TrapB";
    return false;
  }
  if (TaXml::readDouble(content, "TrapC", _c))
  {
    LOG(ERROR) << "Reading tag TrapC";
    return false;
  }
  if (TaXml::readDouble(content, "TrapD", _d))
  {
    LOG(ERROR) << "Reading tag TrapD";
    return false;
  }
  return true;
}
