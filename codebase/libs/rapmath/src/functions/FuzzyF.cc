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
 * @file FuzzyF.cc
 */

//------------------------------------------------------------------
#include <rapmath/FuzzyF.hh>
#include <rapmath/ParamsFuzzyFunction.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <cstdio>

#define FUZZY_BAD -99.99

//------------------------------------------------------------------
// v = value, v0/f0  v1/f1 are interpolating endpoints
static double _fuzzyF(const double v, const double v0, const double f0,
		      const double v1, const double f1)
{
  // get range of x 
  double ax = fabs(v1-v0);
  if (ax < 1.0e-6)
  {
    // no range to speak of.. not good, can't have two x with same y
    LOG(ERROR) << "Unexpected range of values " << v0 << " " << v1;
    return FUZZY_BAD;
  }

  double slope, yint;

  // do the interpolation
  slope = (f1-f0)/(v1-v0);
  yint = f0 - slope*v0;
  return (slope*v + yint);
}

/*----------------------------------------------------------------*/
static double _fuzzyV(double x0, double x1, double y0, double y1,
		      double v)
{
  double x, slope, yint;

  x = (x1 - x0);
  if (fabs(x) < 1.0e-6)
    return (x0 + x1)/2.0;
    
  slope = (y1 - y0)/(x1 - x0);
  if (fabs(slope) < 1.0e-6)
    // assumes y0 and y1 both approximately equal to v
    return (x0 + x1)/2.0;
    
  // solve for x at y=v.
  yint = y0 - slope*x0;
  return (v - yint)/slope;
}

//------------------------------------------------------------------
FuzzyF::FuzzyF() : _ok(false)
{
}
  
//------------------------------------------------------------------
FuzzyF::FuzzyF(const vector<pair<double,double> > &f) : 
  _f(f), _ok(!_f.empty())
{
  // note none of the strings are set

  _checkContent();
}

//------------------------------------------------------------------
FuzzyF::FuzzyF(const std::vector<double> &x, const std::vector<double> &y) :
  _ok(true)
{
  if (x.size() != y.size())
  {
    LOG(ERROR) << "ERROR dimensions not same in constructor";
    _ok = false;
  }
  else
  {
    for (size_t i=0; i<x.size(); ++i)
    {
      _f.push_back(pair<double,double>(x[i], y[i]));
    }
    _checkContent();
  }

  // note none of the strings are set
}

//------------------------------------------------------------------
FuzzyF::FuzzyF(const int n, const double *x, const double *y) : _ok(true)
{
  // note none of the strings are set

  for (int i=0; i<n; ++i)
  {
    _f.push_back(pair<double,double>(x[i], y[i]));
  }
  _checkContent();
}

/*----------------------------------------------------------------*/
FuzzyF::FuzzyF(double angle, double x0, double x1) :  _ok(true)
{
  // note none of the strings are set

  double a;
  a = angle*3.14159/180.0;

  double x = x0 - (x1-x0)*0.000001;
  double y = 0.0;
  _f.push_back(pair<double,double>(x, y));

  x = x0;
  y = tan(a)*x0;
  _f.push_back(pair<double,double>(x, y));

  x = x1;
  y = tan(a)*x1;
  _f.push_back(pair<double,double>(x, y));
  
  x = x1 + (x1-x0)*0.000001;
  y = 0.0;
  _f.push_back(pair<double,double>(x, y));
  _checkContent();
}

//------------------------------------------------------------------
FuzzyF::FuzzyF(const std::string &paramfile) : _ok(true)
{
  ParamsFuzzyFunction p;
  if (p.load(paramfile.c_str(), NULL, true, false))
  {
    LOG(ERROR) << "loading param file " << paramfile;
    _ok = false;
  }
  else
  {
    for (int i=0; i<p.fuzzy_n; ++i)
    {
      _f.push_back(pair<double,double>(p._fuzzy[i].x, p._fuzzy[i].y));
    }
    _checkContent();
  }
  _xUnits = p.x_units;
  _yUnits = p.y_units;
  _title = p.identifier;
}

//------------------------------------------------------------------
FuzzyF::~FuzzyF()
{
}

//------------------------------------------------------------------
bool FuzzyF::operator==(const FuzzyF &f) const
{
  return _f == f._f;
}

//------------------------------------------------------------------
void FuzzyF::print(void) const
{
  if (!_title.empty())
  {
    printf("%s\n", _title.c_str());
  }
  if (!_xUnits.empty() && !_yUnits.empty())
  {
    printf("%s -> %s\n", _xUnits.c_str(), _yUnits.c_str());
  }

  for (size_t i=0; i<_f.size(); ++i)
  {
    printf("%10.5lf -> %10.5lf\n", _f[i].first, _f[i].second);
  }
}

//------------------------------------------------------------------
void FuzzyF::print(ostream &stream) const
{
  if (!_title.empty())
  {
    stream << "Fuzzy function " << _title << endl;
  }
  else
  {
    stream << "Fuzzy function" << endl;
  }
  if (!_xUnits.empty() && !_yUnits.empty())
  {
    stream << _xUnits << " -> " << _yUnits << endl;
  }
  for (size_t i=0; i<_f.size(); ++i)
  {
    stream << _f[i].first << " " << _f[i].second << endl;
  }
}

//------------------------------------------------------------------
double FuzzyF::apply(const double v) const
{
  if (!_ok)
  {
    return FUZZY_BAD;
  }

  int n = static_cast<int>(_f.size());
  if (n == 0)
  {
    // no points
    return FUZZY_BAD;
  }

  if (v <= _f[0].first)
  {
    // v < smallest x.. return smallest y
    return _f[0].second;
  }
  else if (v >= _f[n-1].first)
  {
    // v > largest x, return largest y
    return _f[n-1].second;
  }
  
  // find slot for input
  for (int i=0; i<n-1; ++i)
  {
    if (v >= _f[i].first && v <= _f[i+1].first)
    {
      // interpolate here.
      return _fuzzyF(v, _f[i].first, _f[i].second, _f[i+1].first,
		     _f[i+1].second);
    }
  }

  // just in case
  return _f[n-1].second;
}

//------------------------------------------------------------------
double FuzzyF::maxX(void) const
{
  double maxv = -99.99;
  for (size_t i=0; i<_f.size(); ++i)
  {
    if (i == 0)
    {
      maxv = _f[i].first;
    }
    else
    {
      if (_f[i].first > maxv)
      {
	maxv = _f[i].first;
      }
    }
  }
  return maxv;
}

//------------------------------------------------------------------
double FuzzyF::minX(void) const
{
  double minv = -99.99;
  for (size_t i=0; i<_f.size(); ++i)
  {
    if (i == 0)
    {
      minv = _f[i].first;
    }
    else
    {
      if (_f[i].first < minv)
      {
	minv = _f[i].first;
      }
    }
  }
  return minv;
}

/*----------------------------------------------------------------*/
bool FuzzyF::xRange(double &x0, double &x1) const
{
  if (_f.empty())
  {
    return false;
  }

  x0 = x1 = _f[0].first;
  for (size_t i=1; i<_f.size(); ++i)
  {
    double x = _f[i].first;
    if (x < x0)
      x0 = x;
    if (x  > x1)
      x1 = x;
  }
  return true;
}

/*----------------------------------------------------------------*/
bool FuzzyF::yRange(double &y0, double &y1) const
{
  if (_f.empty())
  {
    return false;
  }

  y0 = y1 = _f[0].second;
  for (size_t i=1; i<_f.size(); ++i)
  {
    double y = _f[i].second;
    if (y < y0)
      y0 = y;
    if (y  > y1)
      y1 = y;
  }
  return true;
}

/*----------------------------------------------------------------*/
bool FuzzyF::maxXAtGivenY(double v, double &max) const
{
  // get largest index where y >= v
  vector <pair<double,double> >::const_iterator iy;
  int i, imax;

  for (imax=-1,i=0,iy=_f.begin(); iy!=_f.end(); ++iy,++i)
  {
    if (iy->second >= v)
    {
      imax = i;
    }
  }

  if (imax == -1)
  {
    max = 0.0;
    return false;
  }
    
  // here y[imax] >= v and y[imax+1] < v, 
  // or   y[imax] >= v and imax is largest
  if (imax == static_cast<int>(_f.size()) - 1)
  {
    max = _f[imax].first;
  }
  else
  {
    max = _fuzzyV(_f[imax].first, _f[imax+1].first,
		  _f[imax].second, _f[imax+1].second, v);
  }
  return true;
}

/*----------------------------------------------------------------*/
bool FuzzyF::minXAtGivenY(double v, double &min) const
{
  // get smallest index where y >= v
  vector <pair<double,double> >::const_iterator iy;
  int i, imin;

  for (imin=-1,i=0,iy=_f.begin(); iy!=_f.end(); ++iy,++i)
  {
    if (iy->second >= v)
    {
      imin = i;
      break;
    }
  }

  if (imin == -1)
  {
    min = 0.0;
    return false;
  }
    
  // here y[imin] >= v and y[imin-1] < v, 
  // or   y[imin] >= v and imin=0
  if (imin == 0)
  {
    min = _f[imin].first;
  }
  else
  {
    min = _fuzzyV(_f[imin-1].first, _f[imin].first,
		  _f[imin-1].second, _f[imin].second, v);
  }
  return true;
}

//------------------------------------------------------------------
string FuzzyF::xmlContent(const string &name) const
{
  string ret0="", ret="";

  if (!_title.empty())
  {
    ret0 += TaXml::writeString("Title", 0, _title);
  }
  if (!_xUnits.empty())
  {
    ret0 += TaXml::writeString("Xunits", 0, _xUnits);
  }
  if (!_yUnits.empty())
  {
    ret0 += TaXml::writeString("Yunits", 0, _yUnits);
  }

  // for each pair
  for (size_t i=0; i<_f.size(); ++i)
  {
    // write x value as XML
    string s = TaXml::writeDouble("FzX", 0, _f[i].first, "%.2lf");

    // remove the \n if there is one.
    string::size_type ind = s.find("\n");
    if (ind != string::npos)
    {
      s = s.substr(0, ind);
    }
    
    // write y value as XML
    s += TaXml::writeDouble("FzY", 0, _f[i].second, "%.2lf");

    // remove \n if any
    ind = s.find("\n");
    if (ind != string::npos)
    {
      s = s.substr(0, ind);
    }

    // write all of this out as an xml string
    ret0 += TaXml::writeString("Fz1", 0, s);
  }

  // we now have an array of xml strings, each with two doubles.
  // write all of that out as the named string
  ret = TaXml::writeString(name, 0, ret0);
  return ret;
}

//------------------------------------------------------------------
bool FuzzyF::readXml(const string &s, const string &name)
{
  // initialize local state
  _f.clear();

  // try to get the string that has input name as the tag
  string content;
  if (TaXml::readString(s, name, content))
  {
    LOG(ERROR) << "Reading tag " << name;
    // didn't find it
    return false;
  }

  // the next 3 are optional, set empty if not found
  if (TaXml::readString(content, "Title", _title))
  {
    _title = "";
  }
  if (TaXml::readString(content, "Xunits", _xUnits))
  {
    _xUnits = "";
  }
  if (TaXml::readString(content, "Yunits", _yUnits))
  {
    _yUnits = "";
  }


  // now expect an array of strings
  vector<string> v;
  if (TaXml::readTagBufArray(content, "Fz1", v))
  {
    LOG(ERROR) << "ERROR reading array of strings tag=Fz1";
    // nope
    return false;
  }

  // each string should have two XML doubles
  bool stat = true;
  for (size_t i=0; i<v.size(); ++i)
  {
    double x, y;
    if (TaXml::readDouble(v[i], "FzX", x))
    {
      LOG(ERROR) << "ERROR reading tag FzX";
      stat = false;
      continue;
    }
    if (TaXml::readDouble(v[i], "FzY", y))
    {
      LOG(ERROR) << "ERROR reading tag FzY";
      stat = false;
      continue;
    }
    pair<double,double> p(x, y);
    _f.push_back(p);
  }
  return stat;
}

//------------------------------------------------------------------
void FuzzyF::rescaleXValues(const double scale)
{
  for (size_t i=0; i<_f.size(); ++i)
  {
    _f[i].first *= scale;
  }
}

//------------------------------------------------------------------
std::vector<double> FuzzyF::xValues(void) const
{
  std::vector<double> ret;
  for (size_t i=0; i<_f.size(); ++i)
  {
    ret.push_back(_f[i].first);
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<double> FuzzyF::yValues(void) const
{
  std::vector<double> ret;
  for (size_t i=0; i<_f.size(); ++i)
  {
    ret.push_back(_f[i].second);
  }
  return ret;
}

//------------------------------------------------------------------
bool FuzzyF::writeValues(const std::string &outputFile) const
{
  FILE *fp = fopen(outputFile.c_str(), "w");
  if (fp == NULL)
  {
    LOG(ERROR) << "Opening " << outputFile;
    return false;
  }
  for (size_t i=0; i<_f.size(); ++i)
  {
    fprintf(fp, "%.10lf %.10lf\n", _f[i].first, _f[i].second);
  }
  fclose(fp);
  return true;
}


//------------------------------------------------------------------
double FuzzyF::bad(void)
{
  return FUZZY_BAD;
}

//------------------------------------------------------------------
void FuzzyF::_checkContent(void)
{
  if (_f.empty())
  {
    LOG(WARNING) << "Empty fuzzy function";
    _ok = false;
    return;
  }
  if (_f.size() == 1)
  {
    LOG(WARNING) << "Singleton fuzzy function";
    _ok = false;
    return;
  }

  // see if things are either strictly increasing or strictly decreasing
  if (_f[1].first == _f[0].first)
  {
    LOG(ERROR) << "succesive input x values cannot be equal"
	       << _f[1].first;
    _ok = false;
    return;
  }
  bool increasing = _f[1].first > _f[0].first;
  for (size_t i=2; i<_f.size(); ++i)
  {
    if (_f[i].first == _f[i-1].first)
    {
      LOG(ERROR) << "succesive input x values cannot be equal"
		 << _f[i].first;
      _ok = false;
    }
    bool increasingi = _f[i] > _f[i-1];
    if ((increasingi && !increasing) || (increasing && !increasingi))
    {
      LOG(ERROR) << "X values not strictly increasing or decreasing";
      _ok = false;
    }
  }
  if (_ok && !increasing)
  {
    // reverse the order so that other methods will work
    std::vector<std::pair<double,double> > mapping;
    std::vector<std::pair<double,double> >::reverse_iterator i;
    for (i=_f.rbegin(); i!=_f.rend(); ++i)
    {
      mapping.push_back(*i);
    }
    _f = mapping;
  }
}
