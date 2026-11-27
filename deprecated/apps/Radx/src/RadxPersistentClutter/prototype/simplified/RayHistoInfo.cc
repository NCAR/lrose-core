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
 * @file RayHistoInfo.cc
 */
#include <cstdio>
#include "RayHistoInfo.hh"
#include "Params.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RayHistoInfo::RayHistoInfo(void) : RayClutterInfo()
{
}

//------------------------------------------------------------------
RayHistoInfo::RayHistoInfo(const RayClutterInfo &r, const Params &p) :
  RayClutterInfo(r)
{
  // loop through all points
  for (int i=0; i<_nx; ++i)
  {
    double v;
    if (_clutter.getV(i, v))
    {
      if (v == 1.0)
      {
	// it is a clutter point, so initialize a new object in the map
	_histograms[i].init(p.threshold, p.histogram_resolution,
			    p.histogram_max);
      }
    }
  }
}

//------------------------------------------------------------------
RayHistoInfo::~RayHistoInfo()
{
}

//-----------------------------------------------------------------
bool RayHistoInfo::updateSecondPass(const RayData &r)
{
  int n = _initNpt(r);
  if (n > 0)
  {
    // for each point
    for (int i=0; i<n; ++i)
    {
      double v;
      if (_clutter.getV(i, v))
      {
	if (v == 1.0)
	{
	  // a clutter point
	  if (r.getV(i, v))
	  {
	    // update the appropriate histogram with the value
	    _histograms[i].update(v);
	  }
	}
	else
	{
	  // update the appropriate histogram for a missing data 
	  _histograms[i].updateMissing();
	}
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}


//------------------------------------------------------------------
bool RayHistoInfo::setClutter(RayData &r, const double percentile,
			      const double missingClutterValue) const
{
  int n = _initNpt(r);
  double missv = r.getMissing();
  if (n > 0)
  {
    // for reach point
    for (int i=0; i<n; ++i)
    {
      double outv = _outputValue(i, percentile, missingClutterValue, missv);
      r.setV(i, outv);
    }
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
void RayHistoInfo::print(void) const
{
  RayClutterInfo::print();
  std::map<int, Histo>::const_iterator j;
  for (j=_histograms.begin(); j!=_histograms.end(); ++j)
  {
    j->second.print(_x0 + _dx*static_cast<double>(j->first));
  }
}

//------------------------------------------------------------------
const Histo *RayHistoInfo::_histoForIndex(const int i) const
{
  std::map<int, Histo>::const_iterator j;
  for (j=_histograms.begin(); j!=_histograms.end(); ++j)
  {
    if (j->first == i)
    {
      return &j->second;
    }
  }
  LOG(LogStream::ERROR) << "Index " << i << " never found in map";
  return NULL;
}
      
//------------------------------------------------------------------
double RayHistoInfo::_outputValue(const int i, const double percentile, 
				   const double missingClutterValue,
				   const double missingValue) const
{
  double c, outv=missingValue;
  if (_clutter.getV(i, c))
  {
    if (c == 1.0)
    {
      // a clutter point, point to its histogram
      const Histo *h = _histoForIndex(i);
      if (h != NULL)
      {
	// get the percentile histogram value
	double value;
	if (h->computePercentile(percentile, value))
	{
	  outv = value;
	}
	else
	{
	  outv = missingClutterValue;
	}
      }
    }
  }
  return outv;
}
