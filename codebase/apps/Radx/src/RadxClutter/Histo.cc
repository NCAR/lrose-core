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
#include <cstdio>
#include "Histo.hh"

//------------------------------------------------------------------
Histo::Histo() :
  _nbin(0),
  _deltaBin(0),
  _minBin(0),
  _maxBin(0),
  _nmissing(0),
  _countBelowMin(0),
  _ntotal(0)
{
}

//------------------------------------------------------------------
Histo::Histo(const double minBin, const double deltaBin,
	     const double maxBin):
  _nbin(0),
  _deltaBin(deltaBin),
  _minBin(minBin),
  _maxBin(maxBin),
  _nmissing(0),
  _countBelowMin(0),
  _ntotal(0)
{
  _nbin = (_maxBin-_minBin)/_deltaBin + 1;
  _counts.resize(_nbin);
  for (int i=0; i<_nbin; ++i)
  {
    _counts[i] = 0;
  }
}

//------------------------------------------------------------------
Histo::~Histo()
{
}

//------------------------------------------------------------------
void Histo::init(const double minBin, const double deltaBin,
		 const double maxBin)
{
  _nbin = 0;
  _deltaBin = deltaBin;
  _minBin = minBin;
  _maxBin = maxBin;
  _nmissing = 0;
  _countBelowMin = 0;
  _ntotal = 0;

  _nbin = (_maxBin-_minBin)/_deltaBin + 1;
  _counts.resize(_nbin);
  for (int i=0; i<_nbin; ++i)
  {
    _counts[i] = 0;
  }
}

//------------------------------------------------------------------
void Histo::update(const double v)
{
  _ntotal++;

  // _data.push_back(v);
  if (v < _minBin)
  {
    ++_countBelowMin;
    return;
  }

  int bin = (int) (((v - _minBin) / _deltaBin) + 0.5);
  if (bin < 0)
  {
    bin = 0;
  }
  if (bin >= _nbin)
  {
    bin = _nbin - 1;
  }
  _counts[bin] ++;
}

//------------------------------------------------------------------
void Histo::updateMissing()
{
  ++_nmissing;
  ++_ntotal;
}

//------------------------------------------------------------------
bool Histo::computePercentile(const double pct, double &v) const
{
  double pctC = static_cast<double>(_ntotal)*pct;
  int pctCount = static_cast<int>(pctC);
  if (_nmissing >= pctCount)
  {
    return false;
  }
  else
  {
    // start count out assuming missing values are 'minimum'
    int count = _nmissing;
    count += _countBelowMin;
    if (count >= pctCount)
    {
      return false;
    }
    for (int j=0; j<_nbin; ++j)
    {
      count += _counts[j];
      if (count >= pctCount)
      {
	v = static_cast<double>(j)*_deltaBin + _minBin;
	return true;
      }
    }
    if (count == 0)
    {
      return false;
    }
    else
    {
      v = _maxBin;
      return true;
    }
  }
}

//------------------------------------------------------------------
void Histo::print(FILE *out) const
{
  fprintf(out, "Nmissing:%d NbelowMin:%d, Ntotal:%d\n",
          _nmissing, _countBelowMin,
          _ntotal);
  for (int i=0; i<_nbin; ++i) {
    fprintf(out, "bin[%lf]:%d\n",
            _minBin+_deltaBin*static_cast<double>(i),
            _counts[i]);
  }
}

//------------------------------------------------------------------
void Histo::print(FILE *out, const double x) const
{
  fprintf(out, "x=%lf ", x);
  print(out);
}
