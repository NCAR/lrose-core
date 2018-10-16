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
 * @file MdvxUrlWatcher.hh
 * @brief watches a url for new triggering.
 *
 * @class MdvxUrlWatcher
 * @brief watches a url for new triggering.
 *
 * Tells caller when new or next data at the url through call to get_data.
 *
 * Gives caller access to the time.
 * 
 */

#ifndef MDVX_URL_WATCHER_H
#define MDVX_URL_WATCHER_H

#include <didss/LdataInfo.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/DsMdvx.hh>
using namespace std;

/*----------------------------------------------------------------*/
class MdvxUrlWatcher
{

public:

  /**
   * archive constructor - non forecast directory structure MDV data.
   *
   * In archive mode, returns all times between begin and in through successive
   * calls to get_data. After that calls to get_data return false.
   *
   * @param[in] a  contains information about archive range.
   * @param[in] url 
   *
   * @note if url has forecast dir data, get_data will return valid times.
   */

  MdvxUrlWatcher(const char *url,
                 time_t start_time, time_t end_time,
                 const bool debug=false);

  /**
   * archive constructor - forecast directory structure MDV data.
   *
   * In archive mode, returns all times between begin and in through successive
   * calls to get_data. After that calls to get_data return false.
   *
   * @param[in] a  contains information about archive range.
   * @param[in] url 
   * @param[in] gen_times  true to get fcst dir. structure gen_times.
   *            if false return valid times.
   *
   * @note if url has non-forecast dir data and gen_times is true, get_data
   * will return nothing, if gen_times is false, get_data returns data times.
   */

  MdvxUrlWatcher(const char *url,
                 time_t start_time, time_t end_time,
                 const bool gen_times,
                 const bool debug=false);
  /**
   * realtime constructor with blocking i.o.
   *
   * @param[in] url
   * @param[in] max_valid  the max valid age for data (secs)
   *           The object will not return data which has not arrived
   *           within this period.
   * @param[in] delay_msecs  if positive, blocking i.o. with this polling
   *            at the Ds level. If negative, repeated read attempts
   *            (non-blocking) with one second delays between them.
   * @param[in] ldata true for latest data info files, requires url to be a 
   *            path on the local host. 
   *
   * @note if ldata is false, url must be MDV.
   *
   * @note calls to get_data will never return until there is data.
   */
  
  MdvxUrlWatcher(const char *url, const int max_valid, const int delay_msecs,
                 const bool ldata, const bool debug=false);
  
  /**
   * realtime constructor with non-blocking i.o.
   *
   * @param[in] url
   * @param[in] max_valid  the max valid age for data (secs)
   *           The object will not return data which has not arrived
   *           within this period.
   * @param[in] ldata true for latest data info files, requires url to be a 
   *            path on the local host
   *
   * @note if ldata is false, url must be MDV.
   *
   * @note calls to get_data return immediatly if no new data is available.
   */

  MdvxUrlWatcher(const char *url, const int max_valid, const bool ldata,
                 const bool debug=false);
  
  /** 
   * Realtime constructor, forecast MDV data in forecast directory structure.
   *
   * @note returns from get_data only when all the forecasts have triggered
   * @param[in] url
   * @param[in] max_valid  the max valid age for data (secs)
   *           The object will not return data which has not arrived
   *           within this period.
   * @param[in] min_dt_seconds forecast time (sec) of earliest forecast
   * @param[in] nfcst number of forecats
   * @param[in] fcst_dt_seconds time between forecasts (seconds)
   * @param[in] fast true if data will come in quickly, false if slowly
   *
   * @note at some remove slow option (slow is an older vsn)
   *
   * @note at gen time=T let T0=T+min_dt_seconds. We expect forecasts
   * for Ti = T0 + i*fcst_dt_seconds, i=0,...,nfcst-1
   */
  MdvxUrlWatcher(const char *url, const int max_valid, const int min_dt_seconds,
                 const int nfcst, const int fcst_dt_seconds, const bool fast,
                 const bool debug=false);

  MdvxUrlWatcher(const string &url, const int max_valid,
                 const vector<double> &lt_hours, const bool fast,
                 const bool debug=false);

  virtual ~MdvxUrlWatcher();

  /**
   * @return true if constructor had problems.
   */
  inline bool is_bad(void) const {return !_good;}

  /**
   * Set debugging to input value.
   */
  void set_debugging(const bool value);

  /**
   * @return true if triggered, otherwise there is no more data or no data now.
   */
  bool get_data(void);

  /**
   * @return current time that get_data triggered.
   */
  inline time_t get_time(void) const {return _time;}
  
  /**
   * @param[out] archive times in list, archive mode only.
   */
  void getArchiveList(vector<time_t> &t);

  inline string get_url(void) const {return _url;}
  inline int get_max_valid(void) const {return _max_valid;}
  inline bool is_fcst(void) const {return _is_fcst;}
  inline bool is_fast(void) const {return _is_fast;}
  void get_fcst_lt(int &dt0, int &dt, int &nfcst) const;

private :

  bool _debug;
  bool _really_bad;

  //! the url
  string _url;

  bool _good;            //!< false if constructor failed.
  bool _archive_mode;    //!< true in archive mode.
  bool _wait_for_data;   //!< realtime state
  bool _is_fast;         //! true if 'fast' forecast directory realtime url.

  //! most recent time gotten, or (time_t)NULL for none.
  time_t _time;

  //! information for directory forecasts
  bool _is_fcst;
  //! information for directory forecasts
  int _fcst_dt0;
  //! information for directory forecasts
  int _fcst_dt;
  //! information for directory forecasts
  int _nfcst;

  //! Mechanism for watching the URL, MDV data.
  DsMdvxTimes      _in_mdv_times;
  //! Mechanism for watching the URL, MDV data.
  LdataInfo        _ldata;
  //! Mechanism for watching the URL, MDV data.
  bool             _is_ldata;
  //! Mechanism for watching the URL, MDV data.
  int              _max_valid;

  bool _fcst_getdata(void);
  bool _slow_fcst_getdata(void);
  bool _fast_fcst_getdata(void);
  bool _getdata(void);
  void _fast_init_gentime(void);
  bool _get_newest_gen_time(const string &url, const time_t t0,
                            const time_t t1, time_t &gentime);
  void _get_initial_gen_time(time_t &gentime);
  void _get_new_gentime(const bool must_be_new);
  bool _trigger_then_find_gentime(const bool must_be_new);
  bool _get_forecasts(const time_t gt, int &nv);

  void _logError(const string &method,
                 const string &label,
                 const string &val = "");
 
  void _logDebug(const string &method,
                 const string &label,
                 const string &val = "");
 
  void _logDebug(const string &method,
                 const string &label,
                 int num,
                 const string &val = "");
 
};

#endif
