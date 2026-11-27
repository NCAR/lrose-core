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
 * @file FrequencyCount.cc
 */
#include <cstdio>
#include <cmath>
#include "FrequencyCount.hh"
#include <Radx/RayxData.hh>

//------------------------------------------------------------------
FrequencyCount::FrequencyCount(const int nbins, const int nscan)
{
  _nscan = nscan;
  _nbin = static_cast<double>(nbins);
  _totalCount = _nbin;
  for (int i=0; i<nbins; ++i)
  {
    double f = static_cast<double>(i)/_nbin;
    _frequency.push_back(f);
    _counts.push_back(1.0);
  }
  
}

//------------------------------------------------------------------
FrequencyCount::~FrequencyCount()
{
}

//------------------------------------------------------------------
void FrequencyCount::update(const double count)
{
  double v = count/_nscan;
  int bin = static_cast<int>(v*static_cast<double>(_nbin));
  if (bin < 0)
  {
    bin = 0;
  }
  if (bin >= _nbin)
  {
    bin = _nbin - 1;
  }
  _counts[bin]++;
  ++_totalCount;
}

//------------------------------------------------------------------
void FrequencyCount::append(FILE *fp)
{
  for (int i=0; i<_nbin; ++i)
  {
    fprintf(fp, "%12.10lf ", _counts[i]/_totalCount);
  }
  fprintf(fp, "\n");
}

//------------------------------------------------------------------
void FrequencyCount::appendString(std::vector<std::string> &lines)
{
  string s = "";
  for (int i=0; i<_nbin; ++i)
  {
    char buf[1000];
    sprintf(buf, "%12.10lf ", _counts[i]/_totalCount);
    // fprintf(fp, "%12.10lf ", _counts[i]/_totalCount);
    s += buf;
  }
  // fprintf(fp, "\n");
  lines.push_back(s);
}
