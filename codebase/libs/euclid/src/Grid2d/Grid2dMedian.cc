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
 * @file Grid2dMedian.cc
 */

#include <euclid/Grid2dMedian.hh>
#include <toolsa/LogStream.hh>
using std::vector;
using std::pair;


//----------------------------------------------------------------
Grid2dMedian::Grid2dMedian(const Grid2d &g, const int xw, const int yw,
			   const double res, const double min,
			   const double max) : Grid2d(g)
{
  _xw = xw;
  _yw = yw;

  _binMin = min;
  _binMax = max;
  _binDelta = res;
  _nbin = static_cast<int>((_binMax-_binMin)/_binDelta) + 1;
  // _bin = new double[_nbin];
  // _counts = new double[_nbin];
  for (int i=0; i<_nbin; ++i)
  {
    double v = _binMin + _binDelta*i;
    _bin.push_back(v);
    _counts.push_back(0.0);
  }
  _nc = 0;
}

//----------------------------------------------------------------
Grid2dMedian::~Grid2dMedian()
{
  // delete [] _bin;
  // delete [] _counts;
}

//----------------------------------------------------------------
void Grid2dMedian::clear(void)
{
  for (int i=0; i<_nbin; ++i)
  {
    _counts[i] = 0;
  }
  _nc = 0;
}

//----------------------------------------------------------------
void Grid2dMedian::update(const vector<pair<int,int> > &newv,
			  const vector<pair<int,int> > &oldv)
{
  for (size_t i=0; i<oldv.size(); ++i)
  {
    _subtract(oldv[i].first, oldv[i].second);
  }
  for (size_t i=0; i<newv.size(); ++i)
  {
    _add(newv[i].first, newv[i].second);
  }
}

//----------------------------------------------------------------
void Grid2dMedian::addValue(double d)
{
  _addValue(d);
}

//----------------------------------------------------------------
double Grid2dMedian::getMedian(void) const
{
  return getPercentile(0.5);
}

//----------------------------------------------------------------
double Grid2dMedian::getPercentile(double pct) const
{
  if (_nc < _xw*_yw/2)
  {
    return _missing;
  }
  else
  {
    return _pcntile(pct);
  }
}

//----------------------------------------------------------------
double Grid2dMedian::getCount(double pct) const
{
  if (_nc < _xw*_yw/2)
  {
    return _missing;
  }
  else
  {
    return _count(pct);
  }
}

//----------------------------------------------------------------
double Grid2dMedian::getMedianAllData(void) const
{
  return getPercentileAllData(0.5);
}

//----------------------------------------------------------------
double Grid2dMedian::getPercentileAllData(double pct) const
{
  if (_nc < 1)
  {
    return _missing;
  }
  else
  {
    return _pcntile(pct);
  }
}

//----------------------------------------------------------------
double Grid2dMedian::getCountAllData(double pct) const
{
  if (_nc < 1)
  {
    return _missing;
  }
  else
  {
    return _count(pct);
  }
}

//----------------------------------------------------------------
double Grid2dMedian::smallestBinWithCount(void) const
{
  if (_nc < 1)
  {
    return _missing;
  }
  for (int i=0; i<_nbin; ++i)
  {
    if (_counts[i] > 0)
    {
      return _bin[i];
    }
  }
  return _missing;
}

//----------------------------------------------------------------
double Grid2dMedian::largestBinWithCount(void) const
{
  if (_nc < 1)
  {
    return _missing;
  }
  for (int i=_nbin-1; i>=0; --i)
  {
    if (_counts[i] > 0)
    {
      return _bin[i];
    }
  }
  return _missing;
}

//----------------------------------------------------------------
void Grid2dMedian::_subtract(const int x, const int y)
{
  double v;
  if (getValue(x, y, v))
  {
    _removeValue(v);
  }
}

//----------------------------------------------------------------
void Grid2dMedian::_add(const int x, const int y)
{
  double v;
  if (getValue(x, y, v))
  {
    _addValue(v);
  }
}

//----------------------------------------------------------------
void Grid2dMedian::_removeValue(const double d)
{
  _nc--;
  if (_nc < 0)
  {
    LOG(ERROR) << d << " remove led to negative total count";
  }

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
  _counts[index]--;
  if (_counts[index] < 0)
  {
    LOG(ERROR) << "negative count of bin " << _bin[index];
  }
}

//----------------------------------------------------------------
void Grid2dMedian::_addValue(double d)
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
double Grid2dMedian::_pcntile(double pct) const
{
  double fpt = pct*static_cast<double>(_nc);
  int ipt = static_cast<int>(fpt);
  int count=0;
  for (int i=0; i<_nbin; ++i)
  {
    count += static_cast<int>(_counts[i]);
    if (count >= ipt)
    {
      return _bin[i];
    }
  }
  LOG(ERROR) << "getting percentile " << pct;
  return _missing;
}


//----------------------------------------------------------------
double Grid2dMedian::_count(double pct) const
{
  double fpt = pct*static_cast<double>(_nc);
  int ipt = static_cast<int>(fpt);
  int count=0;
  for (int i=0; i<_nbin; ++i)
  {
    count += static_cast<int>(_counts[i]);
    if (count >= ipt)
    {
      return _counts[i];
    }
  }
  LOG(ERROR) << "getting percentile " << pct;
  return _missing;
}
