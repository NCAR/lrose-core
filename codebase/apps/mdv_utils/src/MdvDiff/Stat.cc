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
#include <toolsa/copyright.h>

//   File: $RCSfile: Stat.cc,v $
//   Version: $Revision: 1.8 $  Dated: $Date: 2016/03/04 02:22:11 $

/**
 * @file Stat.cc
 */

#include "Stat.hh"
#include <cstdarg>
#include <string>
#include <vector>
using namespace std;

/*----------------------------------------------------------------*/
Stat::Stat(const string &name)
{
  _name = name;

  _min_diff=0.0;
  _max_diff=0.0;
  _ave_diff=0.0;

  _nread_error = 0;
  _nfieldhdr_error = 0;
  _nmissing_field = 0;

  _ncompare=0.0;
  _ndiff = 0;
  _nmissing_only_one=0;
  _ntotal = 0;
  _max_x =  _max_y = 0;
}

/*----------------------------------------------------------------*/
Stat::~Stat()
{
}

/*----------------------------------------------------------------*/
void Stat::process_diff(const int x, const int y, const double ldiff)
{
  if (_ncompare == 0.0)
    _min_diff = _max_diff = _ave_diff = ldiff;
  else
  {
    if (ldiff < _min_diff)
      _min_diff = ldiff;
    if (ldiff > _max_diff)
    {
      _max_diff = ldiff;
      _max_x = x;
      _max_y = y;
    }
    _ave_diff += ldiff;
  }
  ++_ncompare;
  ++_ndiff;
}

/*----------------------------------------------------------------*/
void Stat::inc_one_missing(void)
{
  ++_nmissing_only_one;
  ++_ndiff;
}

/*----------------------------------------------------------------*/
void Stat::addInfo(const string &format, ...)
{
  va_list args;
  va_start( args, format );
  char buf[1000];
  vsprintf( buf, format.c_str(), args );
  va_end( args );
  string s = buf;
  _info.push_back(s);
}

/*----------------------------------------------------------------*/
void Stat::print_final(const int nc) const
{
  string s = _printNameWithPadding(nc);
  printf("%s", s.c_str());
  if (_ncompare == 0)
  {
    if (_nread_error == 0 && _nmissing_field == 0)
      printf("Perfect grid direct comparisons\n");
    else
      printf("\n");
  }
  else
    printf("min:%7.4lf max:%8.4lf ave:%8.4lf ndiff:%d n_miss_diff:%d tot:%d\n",
	   _min_diff, _max_diff, _ave_diff/_ncompare, _ndiff,
	   _nmissing_only_one, _ntotal);

  printf("%s%d read errors\n", s.c_str(), _nread_error);
  printf("%s%d field header differences\n", s.c_str(), _nfieldhdr_error);
  printf("%s%d missing fields in one file\n", s.c_str(), _nmissing_field);
}

/*----------------------------------------------------------------*/
void Stat::print_one(const MdvxProj &proj, const bool show_all,
		     const bool show_info, const int nc) const
{
  string s = _printNameWithPadding(nc);

  string err = "";
  string err2 = "";
  if (_nread_error != 0)
    err += "read error ";
  if (_nfieldhdr_error != 0)
    err += "field header diff ";
  if (_nmissing_field != 0)
    err += "missing field";
  if (_ndiff == 0)
  {
    if (show_all && _nmissing_field == 0 && _nread_error == 0)
    {
      err2 = "MATCH PERFECT";
    }
  }
  else
  {
    double lat, lon;
    proj.xyIndex2latlon(_max_x, _max_y, lat, lon);
    char buf[1000];
    sprintf(buf, "min:%7.4lf max:%8.4lf ave:%8.4lf ndiff:%d n_missing_diff:%d maxx,maxy(%d,%d) = lat/lon(%.3lf, %.3lf)",
	    _min_diff, _max_diff, _ave_diff/_ncompare, _ndiff, _nmissing_only_one, _max_x, _max_y, lat, lon);
    err2 = buf;
  }
  if (!err.empty() || !err2.empty())
    printf("%s%s %s\n", s.c_str(), err.c_str(), err2.c_str());
  if (show_info)
  {
    for (int i=0; i<(int)_info.size(); ++i)
    {
      printf("%s%s\n", s.c_str(),_info[i].c_str());
    }
  }
}

string Stat::_printNameWithPadding(const int nc) const
{
  string ret = "";
  int l = (int)_name.length();
  for (int i=0; i<nc-l; ++i)
    ret += " ";
  ret += _name;
  ret += ":";
  return ret;
}
