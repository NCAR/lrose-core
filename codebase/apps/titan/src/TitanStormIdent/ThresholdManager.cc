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
/////////////////////////////////////////////////////////////
// ThresholdManager.cc
//
// ThresholdManager class
//
// Dave Albo
//
// August 2014
//
///////////////////////////////////////////////////////////////

#include "ThresholdManager.hh"
#include <Spdb/ThresholdBiasMapping.hh>
#include <toolsa/DateTime.hh>

//////////////
// constructor
//

ThresholdManager::ThresholdManager(const string &prog_name,
				   const Params &params) :
  Worker(prog_name, params),
  _threshold_low(params.low_dbz_threshold),
  _threshold_high(params.high_dbz_threshold),
  _is_archive_rerun(false),
  _archive_t1(0)
{
  _set_hist_intervals();
}

/////////////
// destructor
//

ThresholdManager::~ThresholdManager()

{
}


void ThresholdManager::set_to_defaults(void)
{
  bool changed = (_threshold_low != _params.low_dbz_threshold ||
		  _threshold_high != _params.high_dbz_threshold);
  if (changed)
  {
    _threshold_low = _params.low_dbz_threshold;
    _threshold_high = _params.high_dbz_threshold;
    _set_hist_intervals();
  }
}

void ThresholdManager::set_archive_rerun(const time_t &archive_t1)
{
  _is_archive_rerun = true;
  _archive_t1 = archive_t1;
}

void ThresholdManager::set_dynamically(const time_t &gt, int lt)
{
  if (strlen(_params.low_dbz_threshold_spdb_url) == 0)
  {
    set_to_defaults();
    return;
  }

  // try to acquire a threshold
  ThresholdBiasMapping A(_params.low_dbz_threshold_spdb_url);
  bool good = false;
  double threshold = _params.low_dbz_threshold;

  time_t target_time;
  if (_is_archive_rerun)
  {
    // take archive t1, and in that day go with same hour/min/sec as target
    DateTime adt(_archive_t1);
    int y, m, d;
    y = adt.getYear();
    m = adt.getMonth();
    d = adt.getDay();

    DateTime dt(gt);
    int h, min, s;
    h = dt.getHour();
    min = dt.getMin();
    s = dt.getSec();

    cerr << "archive rerun set target time to " << y << " " 
	 << m << " " << d << " " << h << " " << min << " " << s << endl;
    DateTime tt(y, m, d, h, min, s);
    target_time = tt.utime();
  }
  else
  {
    target_time = gt;
  }
  if (A.readNearest(target_time,
		    _params.low_dbz_threshold_spdb_lookback_seconds,
		    _params.low_dbz_threshold_spdb_lookahead_seconds))
  {
    switch (_params.low_dbz_threshold_spdb_lookup_mode)
    {
    case Params::EXACT:
      good = A.getThreshold(lt, threshold);
      break;
    case Params::PARTITION:
      good = A.getPartitionedThreshold(lt, threshold);
      break;
    case Params::INTERPOLATE:
      good = A.getLeadtimeInterpolatedThreshold(lt, threshold);
      break;
    default:
      cerr << "Bad lookup mode " << _params.low_dbz_threshold_spdb_lookup_mode;
      good = false;
      break;
    }
    if (!good)
    {
      cerr << "No threshold data for lt=" << lt;
    }
  }
  if (good)
  {
    bool changed = (_threshold_low != threshold ||
		    _threshold_high != _params.high_dbz_threshold);
    if (changed)
    {
      _threshold_low = threshold;
      _threshold_high = _params.high_dbz_threshold;
      _set_hist_intervals();
      cerr << "Low Dbz Threshold set dynamically to " << _threshold_low << endl;
    }
  }
  else
  {
    set_to_defaults();
  }
}

int ThresholdManager::num_intervals(void) const
{
  return (int)
    ((_threshold_high - _threshold_low) / (_hist_interval) + 1);
}
  

void ThresholdManager::_set_hist_intervals(void)
{
  // check dbz histogram interval is rational

  _hist_interval = _params.dbz_hist_interval;

  int nIntervals = (int)
	      ((_threshold_high - _threshold_low) / (_hist_interval) + 1);
  if (nIntervals > 100)
  {
    double new_hist_interval = (_threshold_high - _threshold_low) / 100.0;
    if (_params.debug >= Params::DEBUG_EXTRA)
    {
      cerr << "WARNING - _set_hist_intervals" << endl;
      cerr << "  Too many dbz intervals: " << nIntervals << endl;
      cerr << "  Requested dbz_hist_interval: "
         << _hist_interval << endl;
      cerr << "  Resetting dbz_hist_interval to: " << new_hist_interval << endl;
    }
    _hist_interval = new_hist_interval;
  }
}
