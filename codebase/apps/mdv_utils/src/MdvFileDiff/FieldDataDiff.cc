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
 * @file FieldDataDiff.cc
 */
#include "FieldDataDiff.hh"
#include <cstdio>

FieldDataDiff::FieldDataDiff(void) :
  _name("unknown"),
  _ntotal(0), _ndiff(0), _nOnlyOne(0), _minDiff(0), _maxDiff(0), _aveDiff(0),
  _xAtMaxDiff(0), _yAtMaxDiff(0), _zAtMaxDiff(0)
{

}

FieldDataDiff::FieldDataDiff(const std::string &name) :
  _name(name), 
  _ntotal(0), _ndiff(0), _nOnlyOne(0), _minDiff(0), _maxDiff(0), _aveDiff(0),
  _xAtMaxDiff(0), _yAtMaxDiff(0), _zAtMaxDiff(0)
{

}

FieldDataDiff::~FieldDataDiff(void)
{
}

void FieldDataDiff::print(const double minDiff) const
{
  printf("  Field:%s\n", _name.c_str());
  if (_ndiff == 0 && _nOnlyOne == 0)
  {
    printf("   No diffs\n");
    return;
  }

  // do some computations
  double ave = _aveDiff/static_cast<double>(_ntotal);
  double pct = 
    static_cast<double>(_ndiff + _nOnlyOne)/static_cast<double>(_ntotal);

  if (pct < minDiff)
  {
    return;
  }

  printf("   minD:%lf  maxD:%lf  aveD:%lf  pctD:%lf  nd:%d  nOne:%d  nT:%d  maxDAt=(%d,%d,%d)\n",
	 _minDiff, _maxDiff, ave, pct, _ndiff, _nOnlyOne,
	 _ntotal, _xAtMaxDiff, _yAtMaxDiff, _zAtMaxDiff);
}

void FieldDataDiff::dataSame(void)
{
  _ntotal ++;
}

void FieldDataDiff::dataDiff(const double diff, const int x, const int y, 
			     const int z)
{
  _ntotal ++;
  if (_ndiff == 0)
  {
    _minDiff = _maxDiff = _aveDiff = diff;
    _xAtMaxDiff = x;
    _yAtMaxDiff = y;
    _zAtMaxDiff = z;
  }
  else
  {
    if (diff < _minDiff)
    {
      _minDiff = diff;
    }
    if (diff > _maxDiff)
    {
      _maxDiff = diff;
      _xAtMaxDiff = x;
      _yAtMaxDiff = y;
      _zAtMaxDiff = z;
    }
    _aveDiff += diff;
  }
  _ndiff ++;
}

void FieldDataDiff::dataOnlyOne(void)
{
  _ntotal ++;
  _nOnlyOne ++;
}

bool FieldDataDiff::isDifferent(const double minPct) const
{
  double pct = 
    static_cast<double>(_ndiff + _nOnlyOne)/static_cast<double>(_ntotal);
  return (pct > 0.0 && pct >= minPct);
}

