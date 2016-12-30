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
 * @file ThresholdBiasMapping.cc
 */

//------------------------------------------------------------------
#include <Spdb/ThresholdBiasMapping.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
static int _hms(const time_t &t)
{
  DateTime dt(t);
  int y, m, d;
  y = dt.getYear();
  m = dt.getMonth();
  d = dt.getDay();
  DateTime dt0(y, m, d, 0, 0, 0);
  time_t t0 = dt0.utime();
  int hms = t - t0;
  return hms;
}

//------------------------------------------------------------------
static int  _bestGuessHourMinSec(const time_t &gt, int hourMinSec0,
				 int deltaSec)
{
  // want the gen that is >= one of the target gens, and < the next one.
  int hms = _hms(gt);

  // so what is the biggest hms in a day?
  int k=0;
  while (hourMinSec0 + k*deltaSec < 24*3600)
  {
    k++;
  }
  k--;

  if (hms < hourMinSec0 + deltaSec)
  {
    // go with smallest one
    return hourMinSec0;
  }
  else if (hms >= hourMinSec0 + k*deltaSec)
  {
    return k*deltaSec;
  }
  else
  {
    for (int i=1; i<k; ++i)
    {
      if (hms >= hourMinSec0 + i*deltaSec && hms < hourMinSec0 + (i+1)*deltaSec)
      {
	return i*deltaSec;
      }
    }
    LOG(ERROR) << "Did not expect to be here";
    return -1;
  }
}


//------------------------------------------------------------------
static bool _bestGuessResolution(const std::vector<time_t> &times,
				 int &hourMinSec0, int &deltaSec)
{
  hourMinSec0 = -1;
  deltaSec = -1;
  if (times.empty())
  {
    return false;
  }
  else if (times.size() == 1)
  {
    // not setup to handle this case
    LOG(ERROR) << "One gen time in SPDB, not able to determine gen resolution";
    return false;
  }

  int numAtSame = 0;
  for (int i=0; i<(int)(times.size())-1; ++i)
  {
    int deltai = times[i+1] - times[i];
    if (i == 0)
    {
      deltaSec = deltai;
      numAtSame = 1;
    }
    else
    {
      if (deltai != deltaSec)
      {
	if (deltai < deltaSec)
	{
	  if (numAtSame > 1)
	  {
	    LOG(WARNING) << "Inconsistent SPDB time delta " << deltai << " " 
			 << deltaSec;
	  }
	  deltaSec = deltai;
	  numAtSame = 1;
	}
      }
      else 
      {
	numAtSame++;
      }
    }
  }

  // now that we have a delta, get an 'anchor' to get the hour/min/sec
  // values within each day.  Use times[0]
  int hms = _hms(times[0]);

  hourMinSec0 = hms;
  while (hourMinSec0 >= 0)
  {
    hourMinSec0 -= deltaSec;
  }
  hourMinSec0 += deltaSec;  // this is the earliest hour/min/sec in each day
  
  return true;
}

//------------------------------------------------------------------
ThresholdBiasMapping::ThresholdBiasMapping(void)
{
}

//------------------------------------------------------------------
ThresholdBiasMapping::ThresholdBiasMapping(const std::string &path)
{
  _url = path;
}

//------------------------------------------------------------------
ThresholdBiasMapping::~ThresholdBiasMapping()
{
}

//------------------------------------------------------------------
void ThresholdBiasMapping::store(int lt, double bias, double w)
{
  _biasMap[lt] = bias;
  _threshMap[lt] = w;
}

//------------------------------------------------------------------
void ThresholdBiasMapping::writeAndClear(const time_t &gt)
{
  string xml = _toXml();

  DsSpdb s;
  s.setPutMode(Spdb::putModeOver);
  
  s.clearPutChunks();
  s.clearUrls();
  s.addUrl(_url);

  MemBuf mem;
  mem.free();
  
  mem.add(xml.c_str(), xml.size() + 1);
  if (s.put(SPDB_XML_ID, SPDB_XML_LABEL, 1, gt, gt, mem.getLen(),
	    (void *)mem.getPtr()))
  {
    LOG(ERROR) << "problems writing out SPDB";
  }
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::readFirstBefore(const time_t &gt,
					   int maxSecondsBack)
{
  DsSpdb s;
  _threshMap.clear();
  _biasMap.clear();
  if (s.getFirstBefore(_url, gt-1, maxSecondsBack) != 0)
  {
    LOG(DEBUG) << "No SPDB data found in data base "
	       << _url << " before " << DateTime::strn(gt) 
	       << " , within " << maxSecondsBack << " seconds";
    return false;
  }
  else
  {
    return _load(s);
  }
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::readFirstAfter(const time_t &gt,
					  int maxSecondsAhead)
{
  DsSpdb s;
  _threshMap.clear();
  _biasMap.clear();
  if (s.getFirstAfter(_url, gt, maxSecondsAhead) != 0)
  {
    LOG(DEBUG) << "No SPDB data found in data base "
	       << _url << " after " << DateTime::strn(gt) 
	       << " , within " << maxSecondsAhead << " seconds";
    return false;
  }
  else
  {
    return _load(s);
  }
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::readNearest(const time_t &gt,
				       int maxSecondsBack,
				       int maxSecondsAhead)
{
  // want gen time in nearest day that is >= gt and < next gen time in
  // that nearest day
  DsSpdb S;
  if (S.compileTimeList(_url, gt - maxSecondsBack, gt + maxSecondsAhead) != 0)
  {
    LOG(ERROR) << "No SPDB data in data base " << _url << " from "
	       << DateTime::strn(gt - maxSecondsBack) << " to " 
	       << DateTime::strn(gt + maxSecondsAhead);
    return false;
  }

  vector<time_t> times = S.getTimeList();

  if (times.empty())
  {
    LOG(ERROR) << "No SPDB data in data base " << _url << " from "
	       << DateTime::strn(gt - maxSecondsBack) << " to " 
	       << DateTime::strn(gt + maxSecondsAhead);
    return false;
  }

  bool first = true;
  time_t best;
  if (times.size() == 1)
  {
    // this single time is it.
    first = false;
    best = times[0];
  }
  else
  {
    // size > 1
    // try to figure out the pattern of gen times (resolution, first gen in day)
    int hourMinSec0, deltaSec;
    if (!_bestGuessResolution(times, hourMinSec0, deltaSec))
    {
      return false;
    }

    // now figure out which gen to search for
    int hms= _bestGuessHourMinSec(gt, hourMinSec0, deltaSec);
    if (hms < 0)
    {
      return false;
    }

    // now find closest day to day of gt with this hour/min/sec
    double delta;
    for (size_t i=0; i<times.size(); ++i)
    {
      int hmsi = _hms(times[i]);
      if (hmsi == hms)
      {
	if (first)
	{
	  first = false;
	  delta = fabs(double(times[i] - gt));
	  best = times[i];
	}
	else
	{
	  double deltai = fabs(double(times[i] - gt));
	  if (deltai < delta)
	  {
	    delta = deltai;
	    best = times[i];
	  }
	}
      }
    }
  }
  if (first)
  {
    LOG(ERROR) << "No gen time with matching hour/min/sec found";
    return false;
  }
  else
  {
    return readExact(best);
  }
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::readExactPrevious(const time_t &gt, int maxDaysBack)
{
  _threshMap.clear();
  _biasMap.clear();
  for (int i=1; i<=maxDaysBack; ++i)
  {
    time_t ti = gt - i*3600*24;
    DsSpdb s;
    bool stat = true;
    if (s.getExact(_url, ti) != 0)
    {
      stat = false;
    }
    else
    {
      stat = _load(s);
    }
    if (stat)
    {
      return true;
    }
    else
    {
      //LOG(DEBUG) << "No SPDB data found in data base "
      // << _url << " at " << DateTime::strn(ti);
    }
  }
  //LOG(WARNING) << "No SPDB data up to maximum days back";
  return false;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::readExactPreviousOrCurrent(const time_t &gt,
						      int maxDaysBack)
{
  DsSpdb s;
  _threshMap.clear();
  _biasMap.clear();
  bool stat = true;
  if (s.getExact(_url, gt) != 0)
  {
    stat = false;
  }
  else
  {
    stat = _load(s);
  }
  if (!stat)
  {
    stat = readExactPrevious(gt, maxDaysBack);
  }
  return stat;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::readExact(const time_t &gt)
{
  DsSpdb s;
  _threshMap.clear();
  _biasMap.clear();
  bool stat = true;
  if (s.getExact(_url, gt) != 0)
  {
    stat = false;
  }
  else
  {
    stat = _load(s);
  }
  if (!stat)
  {
    LOG(WARNING) << "No SPDB data found in data base "
		 << _url << " at " << DateTime::strn(gt);
  }
  return stat;
}

//------------------------------------------------------------------
std::string ThresholdBiasMapping::setEnvString(void) const
{
  string ret = "";
  map<int,double>::const_iterator i;
  for (i=_threshMap.begin(); i!=_threshMap.end(); ++i)
  {
    char  buf[1000];
    sprintf(buf, "setenv THRESH_%d  %10.6lf\n", i->first, i->second);
    ret += buf;
  }
  return ret;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::getThreshold(int lt, double &thresh) const
{
  map<int, double>::const_iterator i;
  i = _threshMap.find(lt);
  if (i == _threshMap.end())
  // if (_threshMap.find(lt) == _threshMap.end() )
  {
    LOG(ERROR) << "Lead time " << lt << " not in map";
    return false;
  }
  else
  {
    thresh = i->second;
    // thresh = _threshMap.at(lt);
    return true;
  }
}

//------------------------------------------------------------------
bool
ThresholdBiasMapping::getLeadtimeInterpolatedThreshold(int lt,
						       double &thresh) const
{
  if (_threshMap.empty())
  {
    return false;
  }

  // pull out all the lead times
  map<int, double>::const_iterator i;
  vector<int> locLt;
  for (i=_threshMap.begin(); i!=_threshMap.end(); ++i)
  {
    if (lt == i->first)
    {
      thresh = i->second;
      return true;
    }
    locLt.push_back(i->first);
  }
  
  // now see where our lt fits in, since it was never an exact match
  // here we assume the leads are in ascending order, which is guaranteed
  // by the map class, so should be ok

  if (lt < locLt[0])
  {
    LOG(DEBUG_VERBOSE) << "Lead time " << lt << 
      " smaller than all leads in SPDB, use smallest lead times threshold";
    return getThreshold(locLt[0], thresh);
  }
  if (lt > *(locLt.rbegin()))
  {
    LOG(DEBUG_VERBOSE) << "Lead time " << lt << 
      " larger than all leads in SPDB, use largest lead times threshold";
    return getThreshold(*(locLt.rbegin()), thresh);
  }
  for (int i=1; i<static_cast<int>(locLt.size()); ++i)
  {
    if (lt > locLt[i-1] && lt < locLt[i])
    {
      // interpolate now
      double w0 = locLt[i] - lt;
      double w1 = lt - locLt[i-1];
      double w = locLt[i] - locLt[i-1];
      w0 /= w;
      w1 /= w;
      LOG(DEBUG_VERBOSE) << "Interpolating lead times " << locLt[i-1] 
			 << " " << locLt[i];

      LOG(DEBUG_VERBOSE) << "Interpolating weights are " << w0 << " " << w1;

      double t0, t1;
      if (!getThreshold(locLt[i-1], t0))
      {
	LOG(ERROR) << "Unexpected inability to get threshold at " << locLt[i-1];
	return false;
      }
      if (!getThreshold(locLt[i], t1))
      {
	LOG(ERROR) << "Unexpected inability to get threshold at " << locLt[i];
	return false;
      }
      LOG(DEBUG_VERBOSE) << "Interpolating thresholds are " << t0 << " " << t1;
      thresh = (w0*t0 + w1*t1);
      return true;
    }
  }
  LOG(ERROR) << "Unexpected lead time interpolation failed, no window";
  return false;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::getPartitionedThreshold(int lt,
						   double &thresh) const
{
  // produce the partition by pulling out all the lead times
  map<int, double>::const_iterator i;
  vector<int> locLt;
  for (i=_threshMap.begin(); i!=_threshMap.end(); ++i)
  {
    locLt.push_back(i->first);
  }
  
  int dt0 = 0;
  for (int j=0; j<static_cast<int>(locLt.size()); ++j)
  {
    if (j > 0)
    {
      int dt = locLt[j] - locLt[j-1];
      if (j == 1)
      {
	dt0 = dt;
      }
      else
      {
	if (dt != dt0)
	{
	  LOG(ERROR) << "Lead time partitioning uneven " << dt << " " << dt0;
	  return false;
	}
      }
    }
  }

  // so here the partitions are  [locLt[0]+i*dt0, locLt[0]+(i+1)*dt0),
  // i=0,1,...locLt.size()-1

  // so want lt >= locLt[0] + i*dt0
  //         lt < locLt[0] + (i+1)*dt0
  // I believe this works with truncation

  int j = (lt - locLt[0])/dt0;
  if (j >= static_cast<int>(locLt.size()))
  {
    j = static_cast<int>(locLt.size()) - 1;
  }
  if (j < 0)
  {
    j = 0;
  }

  int partitionLt = locLt[j];
  LOG(DEBUG) << "Partition chosen = [" 
	     << partitionLt << "," << partitionLt+dt0 << ")";

  return getThreshold(partitionLt, thresh);
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::getBias(int lt, double &bias) const
{
  map<int, double>::const_iterator i;
  i = _biasMap.find(lt);
  // if (_biasMap.find(lt) == _biasMap.end() )
  if (i == _biasMap.end())
  {
    LOG(ERROR) << "Lead time " << lt << " not in map";
    return false;
  }
  else
  {
    bias = i->second;
    // bias = _biasMap.at(lt);
    return true;
  }
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::compare(const ThresholdBiasMapping &inp,
				   double maxDiff,
				   std::string &comparisons) const
{
  bool status = true;
  std::map<int, double>::const_iterator i, iInp;

  comparisons = "";
  for (i = _threshMap.begin(); i != _threshMap.end(); ++i)
  {
    int lt = i->first;
    double thresh0 = i->second;
    iInp = inp._threshMap.find(lt);
    if (iInp == inp._threshMap.end())
    // if (inp._threshMap.find(lt) == inp._threshMap.end() )
    {
      LOG(ERROR) << "Lead time " << lt << " not in input map";
      status = false;
    }
    else
    {
      // double thresh1 = inp._threshMap.at(lt);
      double thresh1 = iInp->second;
      char buf[1000];
      sprintf(buf, "%6d   %6.4lf  %6.4lf   diff:%6.4lf\n",
	      lt, thresh0, thresh1, thresh1-thresh0);
      comparisons += buf;
      if (fabs(thresh1-thresh0) > maxDiff)
      {
	status = false;
      }
    }
  }

  for (i = inp._threshMap.begin(); i != inp._threshMap.end(); ++i)
  {
    int lt = i->first;
    if (_threshMap.find(lt) == _threshMap.end() )
    {
      LOG(ERROR) << "Lead time " << lt << " not in local map";
      status = false;
    }
  }
  return status;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::biasGood(double target, double tolerance) const
{
  std::map<int, double>::const_iterator i;
  for (i=_biasMap.begin(); i!=_biasMap.end(); ++i)
  {
    if (fabs(i->second - target) >= tolerance)
    {
      LOG(DEBUG) << "bias=" << i->second <<" at lead=" << i->first 
		 << " outside tolerance";
      return false;
    }
  }
  LOG(DEBUG) << "All bias values within " << tolerance << " of " << target;
  return true;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::biasGood(double target, double tolerance,
				    std::vector<int> leadSeconds) const
{
  if (leadSeconds.empty())
  {
    return biasGood(target, tolerance);
  }

  std::map<int, double>::const_iterator i;
  for (i=_biasMap.begin(); i!=_biasMap.end(); ++i)
  {
    if (find(leadSeconds.begin(), leadSeconds.end(),
	     i->first) != leadSeconds.end())
    {
      if (fabs(i->second - target) >= tolerance)
      {
	LOG(DEBUG) << "bias=" << i->second <<" at lead=" << i->first 
		   << " outside tolerance";
	return false;
      }
    }
  }
  LOG(DEBUG) << "All bias values within " << tolerance << " of " << target;
  return true;
}


//------------------------------------------------------------------
void ThresholdBiasMapping::copyData(const ThresholdBiasMapping &inp)
{
  _threshMap = inp._threshMap;
  _biasMap = inp._biasMap;
}

//------------------------------------------------------------------
std::string ThresholdBiasMapping::asciiTable(void) const
{
  std::string ret = "";

  // this assumes the same lead times for both mappings
  std::map<int,double>::const_iterator it, ib;
  for (it=_threshMap.begin(), ib=_biasMap.begin();
       it!=_threshMap.end() && ib != _biasMap.end(); ++it, ++ib)
  {
    int lt = it->first;
    int lt2 = ib->first;
    if (lt != lt2)
    {
      LOG(ERROR) << "Lead times do not match " << lt << " " << lt2;
    }
    else
    {
      double thresh = it->second;
      double bias = ib->second;
      char buf[1000];
      sprintf(buf, "%6d    %3.2lf   %3.2lf\n", lt, thresh, bias);
      ret += buf;
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::string ThresholdBiasMapping::asciiTable(const time_t &t) const
{
  std::string stime = DateTime::strs(t);
  
  std::string ret = "";

  // this assumes the same lead times for both mappings
  std::map<int,double>::const_iterator it, ib;
  for (it=_threshMap.begin(), ib=_biasMap.begin();
       it!=_threshMap.end() && ib != _biasMap.end(); ++it, ++ib)
  {
    int lt = it->first;
    int lt2 = ib->first;
    if (lt != lt2)
    {
      LOG(ERROR) << "Lead times do not match " << lt << " " << lt2;
    }
    else
    {
      double thresh = it->second;
      double bias = ib->second;
      char buf[1000];
      sprintf(buf, "%s %6d    %3.2lf   %3.2lf\n", stime.c_str(), lt,
	      thresh, bias);
      ret += buf;
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::string ThresholdBiasMapping::_toXml(void) const
{
  map<int,double>::const_iterator i;

  string s = TaXml::writeStartTag("Thresholds", 0);
  for (i=_threshMap.begin(); i!=_threshMap.end(); ++i)
  {
    int lt = i->first;
    double t = i->second;
    s += _appendXml("ThreshMap", "Thresh", lt, t);
  }
  s += TaXml::writeEndTag("Thresholds", 0);
  s += TaXml::writeStartTag("WeightedBias", 0);
  for (i=_biasMap.begin(); i!=_biasMap.end(); ++i)
  {
    int lt = i->first;
    double t = i->second;
    s += _appendXml("BiasMap", "Bias", lt, t);
  }
  s += TaXml::writeEndTag("WeightedBias", 0);
  return s;
}

//------------------------------------------------------------------
std::string ThresholdBiasMapping::_appendXml(const std::string &pairKey,
			      const std::string &valueKey, 
			      int lt, double w) const
{
  string s;
  s = TaXml::writeStartTag(pairKey, 0);
  s = s.substr(0, s.size()-1);
  s += TaXml::writeInt("Lead", 0, lt);
  s = s.substr(0, s.size()-1);
  s += TaXml::writeDouble(valueKey, 0, w, "%.10lf");
  s = s.substr(0, s.size()-1);
  s += TaXml::writeEndTag(pairKey, 0);
  return s;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::_fromXml(const std::string &xml)
{
  _threshMap.clear();
  _biasMap.clear();

  string s;
  if (TaXml::readString(xml, "Thresholds", s))
  {
    LOG(ERROR) << "No XML with key Thresholds";
    return false;
  }
  vector<string> m;
  if (TaXml::readStringArray(s, "ThreshMap", m))
  {
    LOG(ERROR) << "No XML array with key ThreshMap";
    return false;
  }
  for (size_t i=0; i<m.size(); ++i)
  {
    int lt;
    double w;

    if (TaXml::readInt(m[i], "Lead", lt))
    {
      LOG(ERROR) << "No Lead key in data";
      return false;
    }
    if (TaXml::readDouble(m[i], "Thresh", w))
    {
      LOG(ERROR) << "No Thresh key in data";
      return false;
    }
    _threshMap[lt] = w;
  }

  if (TaXml::readString(xml, "WeightedBias", s))
  {
    LOG(ERROR) << "No XML with key WeightedBias";
    return false;
  }
  m.clear();
  if (TaXml::readStringArray(s, "BiasMap", m))
  {
    LOG(ERROR) << "No XML array with key BiasMap";
    return false;
  }
  for (size_t i=0; i<m.size(); ++i)
  {
    int lt;
    double w;

    if (TaXml::readInt(m[i], "Lead", lt))
    {
      LOG(ERROR) << "No Lead key in data";
      return false;
    }
    if (TaXml::readDouble(m[i], "Bias", w))
    {
      LOG(ERROR) << "No Bias key in data";
      return false;
    }
    _biasMap[lt] = w;
  }
  return true;
}

//------------------------------------------------------------------
bool ThresholdBiasMapping::_load(DsSpdb &s)
{
  const vector<Spdb::chunk_t> &chunks = s.getChunks();
  int nChunks = static_cast<int>(chunks.size());
  if (nChunks <= 0)
  {
    //LOG(WARNING) << "No chunks";
    return false;
  }
  if (nChunks > 1)
  {
    LOG(WARNING) << "Too many chunks " << nChunks << " expected 1";
    return false;
  }
  vector<Spdb::chunk_t> lchunks;
  for (int i=0; i<nChunks; ++i)
  {
    lchunks.push_back(chunks[i]);
  }
  // Process out the results.
  for (int jj = 0; jj < nChunks; jj++)
  {
    const Spdb::chunk_t &chunk = lchunks[jj];
    
    LOG(DEBUG) << "Thresholds gen time[" << jj << "]=" 
	       << DateTime::strn(chunk.valid_time);
    LOG(DEBUG) << "Url=" << _url;
    // _print_chunk_hdr(chunk);
    
    // int data_len = chunk.len;
    void *chunk_data = chunk.data;
    if (s.getProdId() == SPDB_XML_ID)
    {
      string xml((char *)chunk_data);
      return _fromXml(xml);
    }
    else
    { 
      LOG(ERROR) << "spdb data is not XML data, want " << SPDB_XML_ID 
		 << " got " << s.getProdId();
      return false;
    }
  }
  return true;
}
