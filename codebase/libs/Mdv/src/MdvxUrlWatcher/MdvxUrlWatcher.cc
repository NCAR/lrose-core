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

/************************************************************************

Module:	MdvxUrlWatcher.cc
Author:	Niles Oien, Dave Albo
Ported: to Mdvx, Mike Dixon

************************************************************************/

/* System include files / Local include files */
#include <iostream>
#include <cstdlib>
#include <Mdv/MdvxUrlWatcher.hh>
#include <toolsa/pmu.h>
using namespace std;


/*----------------------------------------------------------------*/
MdvxUrlWatcher::MdvxUrlWatcher(const char *url,
                               time_t start_time,
                               time_t end_time,
                               const bool debug)

{

  _debug = false;
  _really_bad = false;
  _url = url;
  _is_ldata = false;
  _max_valid = 0;
  _is_fcst = false;
  _is_fast = false;
  _fcst_dt0 = _fcst_dt = _nfcst = 0;
  _good = true;
  _archive_mode = true;
  _wait_for_data = false;
  _debug = debug;

  if (_in_mdv_times.setArchive(url, start_time, end_time))
  {
    _logError("MdvxUrlWatcher", "Failed to set URL", url);
    _good = false;
  }
  _time = time(NULL);
}

/*----------------------------------------------------------------*/
MdvxUrlWatcher::MdvxUrlWatcher(const char *url,
                               time_t start_time,
                               time_t end_time,
                               const bool gen_times,
                               const bool debug)

{
  _debug = false;
  _really_bad = false;
  _url = url;
  _is_ldata = false;
  _max_valid = 0;
  _is_fcst = false;
  _is_fast = false;
  _fcst_dt0 = _fcst_dt = _nfcst = 0;
  _good = true;
  _archive_mode = true;
  _wait_for_data = false;
  _debug = debug;

  if (gen_times)
  {
    if (_in_mdv_times.setArchiveGen(url, start_time, end_time))
    {
      _logError("MdvxUrlWatcher", "Failed to set URL", url);
      _good = false;
    }
  }
  else
  {
    if (_in_mdv_times.setArchive(url, start_time, end_time))
    {
      _logError("MdvxUrlWatcher", "Failed to set URL", url);
      _good = false;
    }
  }
  _time = time(NULL);
}

/*----------------------------------------------------------------*/
MdvxUrlWatcher::MdvxUrlWatcher(const char *url, const int max_valid,
		       const int delay_msecs, const bool ldata,
		       const bool debug)
{
  _debug = false;
  _really_bad = false;
  _url = url;
  _is_ldata = ldata;
  _max_valid = max_valid;
  _is_fcst = false;
  _is_fast = false;
  _fcst_dt0 = _fcst_dt = _nfcst = 0;
  _wait_for_data = true;
  _archive_mode = false;
  _debug = debug;
  _good = true;
  if (_is_ldata)
  {
    _ldata.setDir(url);
    _ldata.setDebug();
  }
  else
  {
    if (_in_mdv_times.setRealtime(url, max_valid, PMU_auto_register,
				  delay_msecs))
    {
      _logError("MdvxUrlWatcher", "Failed to set URL", url);
      _good = false;
    }
  }
  _time = time(NULL);
}

/*----------------------------------------------------------------*/
MdvxUrlWatcher::MdvxUrlWatcher(const char *url, const int max_valid,
		       const bool ldata, const bool debug)
{
  _debug = false;
  _really_bad = false;
  _url = url;
  _is_ldata = ldata;
  _max_valid = max_valid;
  _is_fcst = false;
  _is_fast = false;
  _fcst_dt0 = _fcst_dt = _nfcst = 0;
  _wait_for_data = false;
  _archive_mode = false;
  _debug = debug;
  _good = true;
  if (_is_ldata)
  {
    _ldata.setDir(url);
    _ldata.setDebug();
  }
  else
  {
    if (_in_mdv_times.setRealtime(url, max_valid, PMU_auto_register, -1))
    {
      _logError("MdvxUrlWatcher", "Failed to set URL", url);
      _good = false;
    }
  }
  _time = time(NULL);
}

/*----------------------------------------------------------------*/
MdvxUrlWatcher::MdvxUrlWatcher(const char *url, const int max_valid,
                               const int min_valid_dt_seconds, const int nfcst,
                               const int fcst_dt_seconds, const bool fast,
                               const bool debug)
{
  _debug = false;
  _really_bad = false;
  _url = url;
  _is_ldata = false;
  _max_valid = max_valid;
  _is_fcst = true;
  _is_fast = fast;
  _fcst_dt0 = min_valid_dt_seconds;
  _fcst_dt = fcst_dt_seconds;
  _nfcst = nfcst;
  _archive_mode = false;
  _wait_for_data = true;
  _debug = debug;
  if (_in_mdv_times.setRealtime(url, max_valid, PMU_auto_register))
  {
    _logError("MdvxUrlWatcher", "Failed to set URL", url);
    _good = false;
  }
  if (_is_fast)
    _fast_init_gentime();
  else
    _time = time(NULL);
}

/*----------------------------------------------------------------*/
MdvxUrlWatcher::MdvxUrlWatcher(const string &url, const int max_valid,
                               const vector<double> &lt_hours, const bool fast,
                               const bool debug)
{
  _debug = false;
  _really_bad = false;
  _url = url;
  _is_ldata = false;
  _max_valid = max_valid;
  _is_fcst = true;
  _is_fast = fast;
  _fcst_dt0 = (int)(lt_hours[0]*3600);
  _fcst_dt = (int)((lt_hours[1]-lt_hours[0])*3600);
  _nfcst = (int)lt_hours.size();
  _archive_mode = false;
  _wait_for_data = true;
  _debug = debug;
  if (_in_mdv_times.setRealtime(url, max_valid, PMU_auto_register))
  {
    _logError("MdvxUrlWatcher", "Failed to set URL", url);
    _good = false;
  }
  if (_is_fast)
    _fast_init_gentime();
  else
    _time = time(NULL);
}

/*----------------------------------------------------------------*/
MdvxUrlWatcher::~MdvxUrlWatcher()
{
}

/*----------------------------------------------------------------*/
void MdvxUrlWatcher::set_debugging(const bool v)
{
  _debug = v;
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::get_data(void)
{
  static const string method = "get_data";
  if (_is_ldata)
  {
    while (true)
    {
      PMU_auto_register(method.c_str());
      if (_ldata.read(_max_valid) == 0)
      {
	_time = _ldata.getLatestTime();
	if (_debug)
	  _ldata.printAsXml(cout);
	return true;
      }
      else
      {
	if (_wait_for_data)
	{
	  umsleep(1000);
          if (_debug) {
            cerr << "DEBUG: MdvxUrlWatcher::get_data: waiting" << endl;
          }
        }
	else
	  return false;
      }
    }
  }
  else
  {
    if (_is_fcst)
      return _fcst_getdata();
    else
      return _getdata();
  }
}

/*----------------------------------------------------------------*/
void MdvxUrlWatcher::getArchiveList(vector<time_t> &t)
{
  t.clear();
  if (!_archive_mode)
    return;
  if (_is_ldata)
    return;
  t = _in_mdv_times.getArchiveList();
}

/*----------------------------------------------------------------*/
void MdvxUrlWatcher::get_fcst_lt(int &dt0, int &dt, int &nfcst) const
{
  dt0 = _fcst_dt0;
  dt = _fcst_dt;
  nfcst = _nfcst;
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::_fcst_getdata(void)
{
  static const string method = "fcst_getdata";
  if (!_wait_for_data)
  {
    _logError(method, "!_wait_for_data", "not implemented");
    return false;
  }

//   if (_is_fast)
    return _fast_fcst_getdata();
//   else
//     return _slow_fcst_getdata();
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::_slow_fcst_getdata(void)
{
  // expect to get _nfcst forecasts _fcst_dt apart. 
  static const string method = "_slow_fcst_getdata";
  time_t *ti = new time_t[_nfcst];
  if ( !ti) {
    _logDebug(method, "Memory allocation failure");
    return false;
  }
  int i=0;
  while (_getdata())
  {
    ti[i] = _time;
    _logDebug(method, "Valid time", i, DateTime::strm(ti[i]));
    if (i > 0)
    {
      if (ti[i] - ti[i-1] > _fcst_dt)
      {
        _logDebug(method, "Out of synch due to big time jump, resynching");
	ti[0] = ti[i];
	i = 0;
        _logDebug(method, "Valid time", i, DateTime::strm(ti[i]));
      }
      else
      {
	if (ti[i] - ti[i-1] < 0)
	{
          _logDebug(method,
                    "Out of synch due to neg. Resyncing, time jump",
                    ti[i] - ti[i-1]);
	  ti[0] = ti[i];
	  i = 0;
          _logDebug(method, "Valid time", i, DateTime::strm(ti[i]));
	}
	else if (ti[i] - ti[i-1] > 0 && ti[i] - ti[i-1] < _fcst_dt)
          _logDebug(method, "Too small a time gap",
                    ti[i] - ti[i-1]);
      }
    }
    if (i == _nfcst-1)
    {
      // set the actual 'gen' time to the oldest time
      _time = ti[0] - _fcst_dt0;
      _logDebug(method, "Got all forecasts");
      delete [] ti;
      return true;
    }
    ++i;
  }
  _logDebug(method, "No more data in fcst get");
  delete [] ti;
  return false;
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::_fast_fcst_getdata(void)
{
  int nv;
  static const string method = "fast_fcst_getdata";

  _get_new_gentime(true);
  int num_really_bad = 0;
  while (true)
  {
    PMU_auto_register(method.c_str());
    if (_get_forecasts(_time, nv))
      return true;
    if (_debug) {
      cerr << "DEBUG - MdvxUrlWatcher::_fast_fcst_getdata" << endl
           << "  Not all the expected valid times, want: " << _nfcst
           << ",  got: " << nv << endl;
    }
    // try for 30 seconds before giving up.
    for (int i=0; i<15; ++i)
    {
      PMU_auto_register("MdvxUrlWatcher::fast_fcst_getdata");
      sleep(2);
      if (_debug) {
        cerr << "DEBUG - MdvxUrlWatcher::_fast_fcst_getdata" << endl
             << "  Trying to get forecasts: " << i+1
             << " of 15 attempt, gen = "
             << DateTime::strn(_time) << endl;
      }
      if (_get_forecasts(_time, nv))
	return true;
      if (_debug) {
        cerr << "DEBUG - MdvxUrlWatcher::_fast_fcst_getdata" << endl
             << "  Not all the expected valid times, want: " << _nfcst
             << ",  got: " << nv << endl;
      }
      if (_really_bad)
      {
	_logError(method, "SEVERE - Really bad situation");
	_really_bad = false;
	if (++num_really_bad > 2)
	{
          _logError(method, "FATAL - GIVING UP");
	  exit(0);
	}
      }
    }      

    // now check for a newer gen time just in case. (but don't require it)
    _get_new_gentime(false);
  }
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::_getdata(void)
{
  static const string method = "_getdata";
  while (true)
  {
    PMU_auto_register(method.c_str());
    int ret = _in_mdv_times.getNext(_time);
    if (_time != time(NULL) && ret != -1)
      return true;
    if (!_wait_for_data)
      return false;
    sleep(1);
    _logDebug(method, "trying");
  }      
}

/*----------------------------------------------------------------*/
void MdvxUrlWatcher::_fast_init_gentime(void)
{
  int nv, counter=0;
  time_t gentime, gentime0 = -1;
  static const string method = "_fast_init_gentime";

  while (true)
  {
    PMU_auto_register(method.c_str());
    _get_initial_gen_time(gentime);
    if (_get_forecasts(gentime, nv))
    {
      _time = gentime;
      return;
    }
    if (gentime != gentime0)
    {
      gentime0 = gentime;
      ++counter;
      if (counter > 5)
      {
        cerr << "FATAL - MdvxUrlWatcher::_fast_init_gentime" << endl
             << "  Number of valid times: " << nv
             << " appears to be wrong, wanted: " << _nfcst << endl;
	exit(-1);
      }
    }
    sleep(10);
  }
}

/*----------------------------------------------------------------*/

bool MdvxUrlWatcher::_get_newest_gen_time(const string &url, const time_t t0,
                                          const time_t t1, time_t &gentime)
{

  DsMdvx d;
  PMU_auto_register("MdvxUrlWatcher::_get_newest_gen_time");
  d.setTimeListModeGen(url, t0, t1);
  d.compileTimeList();

  vector<time_t> gt;
  gt = d.getTimeList();
  int n = (int)gt.size();
  if (n < 1)
  {
    _logDebug("MdvxUrlWatcher::_get_newest_gen_time", "None in range");
    return false;
  }
  else
  {
    gentime = gt[n-1];
    return true;
  }
}

/*----------------------------------------------------------------*/
void MdvxUrlWatcher::_get_initial_gen_time(time_t &gentime)
{
  time_t t0 = -1;
  time_t t;
  static const string method = "_get_initial_gen_time";
  while (true)
  {
    PMU_auto_register(method.c_str());
    _logDebug(method, "Looking for new data");
    if (_in_mdv_times.getNew(t0, t) != 0)
    {
      _logDebug(method, "waiting for initial time");
      sleep(1);
      continue;
    }
    _logDebug(method,
              "Looking for gentime <= initial valid time",
              DateTime::strn(t));

    // see what we are starting with..expect it to be ANY of:
    // gen time+0,  gen_time+dt0
    // gen_time+dt0+dt, ..., gen_time+dt0+(nt-1)*dt
    // meaning the gen time lags behind by at most this much
    if (_get_newest_gen_time(_url, t-_fcst_dt0-_nfcst*_fcst_dt, t, gentime))
      return;
  }
}

/*----------------------------------------------------------------*/
void MdvxUrlWatcher::_get_new_gentime(const bool must_be_new)
{
  while (!_trigger_then_find_gentime(must_be_new))
  {
    sleep(10);
    PMU_auto_register("MdvxUrlWatcher::_get_new_gentime");
  }
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::_trigger_then_find_gentime(const bool must_be_new)
{
  // wait for new trigger of any sort
  time_t t;
  static const string method = "_trigger then find gentime";
  while (true)
  {
    PMU_auto_register(method.c_str());
    _in_mdv_times.getNext(t);
    if (t != time(NULL))
      break;
  }
  _logDebug(method, "Got new data valid time:", DateTime::strn(t));
  time_t gentime1;
  if (_get_newest_gen_time(_url, t-_fcst_dt0-_nfcst*_fcst_dt, t-_fcst_dt0,
			   gentime1))
  {
    if (_time == gentime1 && must_be_new)
    {
      _logDebug(method, "Same gen time as before:", DateTime::strn(_time));
      return false;
    }
    else
    {
      _time = gentime1;
      return true;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
bool MdvxUrlWatcher::_get_forecasts(const time_t gt, int &nv)
{
  vector<time_t> vt;
  DsMdvx d;
  static const string method = "_get_forecasts";

  // get all the forecasts for gen time
  PMU_auto_register(method.c_str());
  d.setTimeListModeForecast(_url, gt);
  d.compileTimeList();
  vt = d.getValidTimes();
  nv = vt.size();
  time_t t0 = time(0);
  _logDebug(method, "Realtime:", DateTime::strn(t0));
  vector<time_t>::iterator i;
  if (_debug) {
    cerr << "DEBUG - MdvxUrlWatcher::_get_forecasts" << endl;
    cerr << "  Using gentime: " << DateTime::strn(gt) << endl;
    cerr << "  Got validtimes:" << endl;
    for (i=vt.begin(); i!=vt.end();++i) {
      cerr << "    " << DateTime::strn(*i) << endl;
    }
  }

  if (nv != _nfcst)
   {
     _logDebug(method, "Not the expected number of valid times");
    return false;
  }
  else
  {
    bool stat = true;
    // do we have all the right fcst times?
    for (int j=0; j<_nfcst; ++j)
      if (vt[j] != gt + _fcst_dt0 + j*_fcst_dt)
      {
        cerr << "ERROR - " << method << endl;
        cerr << "  valid time not as expected" << endl;
        cerr << "    want: " << DateTime::strn(gt+_fcst_dt0+j*_fcst_dt) << endl;
        cerr << "    got : " << DateTime::strn(vt[j]) << endl;
	stat = false;
      }
    if (!stat)
    {
      _logError(method, "SEVERE ERROR in configuration");
      _really_bad = true;
      return false;
    }
    else
      return true;
  }
}

// logging messages

void MdvxUrlWatcher::_logError(const string &method,
                               const string &label,
                               const string &val)

{
  cerr << "ERROR - " << method << endl;
  cerr << "  " << label;
  if (val.size() > 0) {
    cerr << ": " << val;
  }
  cerr << endl;
}

void MdvxUrlWatcher::_logDebug(const string &method,
                               const string &label,
                               const string &val)

{
  cerr << "DEBUG - " << method << endl;
  cerr << "  " << label;
  if (val.size() > 0) {
    cerr << ": " << val;
  }
  cerr << endl;
}

void MdvxUrlWatcher::_logDebug(const string &method,
                               const string &label,
                               int num,
                               const string &val)

{
  cerr << "DEBUG - " << method << endl;
  cerr << "  " << label << "[" << num << "]";
  if (val.size() > 0) {
    cerr << ": " << val;
  }
  cerr << endl;
}

