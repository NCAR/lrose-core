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
 * @file Histo.cc
 */

#include <FiltAlgVirtVol/Histo.hh>
#include <toolsa/LogStream.hh>
using std::vector;


//----------------------------------------------------------------
Histo::Histo(const double res, const double min, const double max)
{
  _binMin = min;
  _binMax = max;
  _binDelta = res;
  _nbin = static_cast<int>((_binMax-_binMin)/_binDelta) + 1;
  for (int i=0; i<_nbin; ++i)
  {
    double v = _binMin + _binDelta*i;
    _bin.push_back(v);
    _counts.push_back(0.0);
  }
  _nc = 0;
}

//----------------------------------------------------------------
Histo::~Histo()
{
}

//----------------------------------------------------------------
void Histo::clear(void)
{
  for (int i=0; i<_nbin; ++i)
  {
    _counts[i] = 0;
  }
  _nc = 0;
}

//----------------------------------------------------------------
void Histo::addValue(double d)
{
  // find bin
  int index = static_cast<int>((d-_binMin)/_binDelta);
  if (index < 0)
  {
    index = 0;
  }
  if (index >= _nbin)
  {
    index = _nbin-1;
  }
  _counts[index] ++;
  ++_nc;
}

//----------------------------------------------------------------
bool Histo::getMedian(double &m) const
{
  return getPercentile(0.5, m);
}

//----------------------------------------------------------------
bool Histo::getPercentile(double pct, double &m) const
{
  if (_nc < 1)
  {
    return false;
  }
  else
  {
    return _pcntile(pct, m);
  }
}

//----------------------------------------------------------------
bool Histo::_pcntile(double pct, double &m) const
{
  double fpt = pct*static_cast<double>(_nc);
  int ipt = static_cast<int>(fpt);
  int count=0;
  for (int i=0; i<_nbin; ++i)
  {
    count += static_cast<int>(_counts[i]);
    if (count >= ipt)
    {
      m = _bin[i];
      return true;
    }
  }
  LOG(ERROR) << "getting percentile " << pct;
  return false;
}

