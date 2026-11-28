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

//   File: $RCSfile: Stats.cc,v $
//   Version: $Revision: 1.9 $  Dated: $Date: 2016/03/04 02:22:11 $

/**
 * @file Stats.cc
 */

#include "Stats.hh"
#include <cstdarg>
#include <cmath>
#include <string>
#include <vector>
#include <toolsa/DateTime.hh>
using namespace std;

/*----------------------------------------------------------------*/
Stats::Stats(void)
{
  _nread_error = 0;
  _nmhdr_error = 0;
  _nmetadata_diff = 0;
  _name = "unknown";
  _t = 0;
  _lt = 0;
  _current = NULL;
  _min_diff_threshold = 0.0;
  _show_all = false;
  _show_info = false;
}

/*----------------------------------------------------------------*/
Stats::Stats(const time_t &t, const int lt, const string &name,
	     const double min_diff_thresh, const bool show_all,
	     const bool show_info)
{
  _nread_error = 0;
  _nmhdr_error = 0;
  _nmetadata_diff = 0;
  _name = name;
  _t = t;
  _lt = lt;
  _current = NULL;
  _min_diff_threshold = min_diff_thresh;
  _show_all = show_all;
  _show_info = show_info;
}

/*----------------------------------------------------------------*/
Stats::Stats(const time_t &t, const string &name, const double min_diff_thresh,
	     const bool show_all, const bool show_info)
{
  _nread_error = 0;
  _nmhdr_error = 0;
  _nmetadata_diff = 0;
  _name = name;
  _t = t;
  _lt = -1;
  _current = NULL;
  _min_diff_threshold = min_diff_thresh;
  _show_all = show_all;
  _show_info = show_info;
}

/*----------------------------------------------------------------*/
Stats::~Stats()
{
}

/*----------------------------------------------------------------*/
void Stats::setField(const std::string &fname)
{
  for (int i=0; i<(int)_stat.size(); ++i)
  {
    if (_stat[i]._name == fname)
    {
      _current = &_stat[i];
      return;
    }
  }
  Stat s(fname);
  _stat.push_back(s);
  _current = &_stat[(int)_stat.size()-1];
}

/*----------------------------------------------------------------*/
void Stats::fieldMissingInOne(void)
{
  _current->_nmissing_field++;
}

/*----------------------------------------------------------------*/
void Stats::fieldHdrError(void)
{
  _current->_nfieldhdr_error++;
}

/*----------------------------------------------------------------*/
void Stats::fieldReadError(void)
{
  _current->_nread_error++;
}

/*----------------------------------------------------------------*/
void Stats::inc_one_missing(void)
{
  _current->_nmissing_only_one++;
  _current->_ndiff++;
}

/*----------------------------------------------------------------*/
bool Stats::process_diff(const int x, const int y, const double diff)
{
  double ldiff = fabs(diff);
  if (ldiff < _min_diff_threshold)
    // who cares
    return false;
  _current->process_diff(x, y, ldiff);
  return true;
}

/*----------------------------------------------------------------*/
void Stats::addInfo(const string &format, ...)
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
void Stats::print_final(void) const
{
  int nc = 0;
  for (int i=0; i<(int)_stat.size(); ++i)
  {
    if ((int)_stat[i]._name.size() > nc)
    {
      nc = (int)_stat[i]._name.size();
    }
  }

  for (int i=0; i<(int)_stat.size(); ++i)
  {
    _stat[i].print_final(nc);
  }
  printf("%d read errors\n", _nread_error);
  printf("%d nmetadata differences\n", _nmetadata_diff);
  printf("%d master header differences\n", _nmhdr_error);
}

/*----------------------------------------------------------------*/
void Stats::print_one(const MdvxProj &proj) const
{
  int nc = 0;
  for (int i=0; i<(int)_stat.size(); ++i)
  {
    if ((int)_stat[i]._name.size() > nc)
    {
      nc = (int)_stat[i]._name.size();
    }
  }

  string err = "";
  string err2 = "";
  if (_nread_error != 0)
    err += "ReadErrors ";
  if (_nmetadata_diff != 0)
    err += "Metadata differences ";
  if (_nmhdr_error != 0)
    err += "Master header differences ";
  if (_lt != -1)
    printf("%s %s+%d:", _name.c_str(), DateTime::strn(_t).c_str(), _lt);
  else
    printf("%s %s:", _name.c_str(), DateTime::strn(_t).c_str()); 
  printf("%s\n", err.c_str());
  if (_show_info)
  {
    for (int i=0; i<(int)_info.size(); ++i)
    {
      printf("%s\n", _info[i].c_str());
    }
  }

  for (int i=0; i<_stat.size(); ++i)
    _stat[i].print_one(proj, _show_all, _show_info, nc);
}

