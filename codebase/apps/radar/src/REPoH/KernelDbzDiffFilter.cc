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
 * @file KernelDbzDiffFilter.cc
 */
#include "KernelDbzDiffFilter.hh"
#include <cstdio>

/*----------------------------------------------------------------*/
KernelDbzDiffFilter::KernelDbzDiffFilter(const bool debug)
{
  _i_min = 0;
  _i_max = 0;
  _min = 0;
  _max = 0;
  _mean = 0;
  _num = 0;
  _first = true;

  _debug = debug;
}

/*----------------------------------------------------------------*/
KernelDbzDiffFilter::~KernelDbzDiffFilter()
{
}

/*----------------------------------------------------------------*/
void KernelDbzDiffFilter::inc(const double v, const int i)
{
  _mean += v;
  _num ++;
  if (_first)
  {
    _first = false;
    _i_min = _i_max = i;
    _min = _max = v;
  }
  else
  {
    if (v < _min)
    {
      _i_min = i;
      _min = v;
    }
    if (v > _max)
    {
      _i_max = i;
      _max = v;
    }
  }
}

/*----------------------------------------------------------------*/
bool KernelDbzDiffFilter::finish(const double diff_threshold)
{
  if (_first)
  {
    // no data
    if (_debug)
      printf("No Points at all to filter\n");
    return true;
  }
  if (_max - _min < diff_threshold)
  {
    // data is all within the tolerated differences range
    if (_debug)
      printf("difference within tolerence max=%lf min=%lf\n", _max, _min);
    return true;
  }

  // compute the mean from what was accumulated for later
  _mean /= _num;
  return false;
}  

/*----------------------------------------------------------------*/
int KernelDbzDiffFilter::choose_remove_index(void) const
{
  if (_max - _mean > _mean - _min)
  {
    // remove max
    if (_debug) printf("Removing MAX range:%lf min:%lf max:%lf mean:%lf\n",
		      _max - _min, _min, _max, _mean);
    return _i_max;
  }
  else
  {
    // remove min
    if (_debug) printf("Removing MIN range:%lf min:%lf max:%lf mean:%lf\n",
		      _max - _min, _min, _max, _mean);
    return _i_min;
  }
}
