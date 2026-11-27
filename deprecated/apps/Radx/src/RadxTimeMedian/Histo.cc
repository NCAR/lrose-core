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
  _nmissing(0),
  _minBin(0),
  _deltaBin(0),
  _maxBin(0)
{
}

//------------------------------------------------------------------
Histo::Histo(const double minBin, const double deltaBin, const double maxBin):
  _nmissing(0),
  _minBin(minBin),
  _deltaBin(deltaBin),
  _maxBin(maxBin)
{
}

//------------------------------------------------------------------
Histo::~Histo()
{
}

//------------------------------------------------------------------
void Histo::update(const double v)
{
  _data.push_back(v);
}

//------------------------------------------------------------------
void Histo::updateMissing(void)
{
  ++_nmissing;
}

//------------------------------------------------------------------
bool Histo::computeMedian(double &v) const
{
  if (_data.empty())
  {
    return false;
  }
  if (_nmissing >= static_cast<int>(_data.size())/2)
  {
    return false;
  }
  else
  {
    int nBin = 0;
    int npt = 0;
    std::vector<double> bin;
    for (double x=_minBin; x<=_maxBin; x+= _deltaBin)
    {
      bin.push_back(0);
      ++nBin;
    }
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      int ind;
      ind = static_cast<int>((_data[i] - _minBin)/_deltaBin + _deltaBin/2.0);
      if (ind < 0)
      {
	ind = 0;
      }
      else if (ind >= nBin)
      {
	ind = nBin-1;
      }
      bin[ind]++;
      npt++;
    }
    // start count out assuming missing values are 'minimum'
    int count = _nmissing;
    for (int j=0; j<nBin; ++j)
    {
      count += bin[j];
      if (count > npt/2)
      {
	v = _minBin + static_cast<double>(j)*_deltaBin;
	if (v > 30)
	{
	  v = v;
	}
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
void Histo::print(void) const
{
  printf("Nmissing:%d Npt:%d\n", _nmissing, static_cast<int>(_data.size()));
  int count = 0;
  for (size_t i=0; i<_data.size(); ++i)
  {
    printf("%5.2lf ", _data[i]);
    if (++count > 5)
    {
      count = 0;
      printf("\n");
    }
  }
  printf("\n");
}

//------------------------------------------------------------------
void Histo::print(const double x) const
{
  printf("x=%lf", x);
  print();
}
