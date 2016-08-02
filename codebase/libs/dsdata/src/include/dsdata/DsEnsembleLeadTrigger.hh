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
 * @file DsEnsembleLeadTrigger.hh
 * @brief Trigger a return when all ensemble members for a lead time are
 *        present, or there is a timeout.
 * @class DsEnsembleLeadTrigger
 * @brief Trigger a return when all ensemble members for a lead time are
 *        present, or there is a timeout.
 *
 * The base class handles the actual triggering of new input from multiple
 * URLs
 */

#ifndef DsEnsembleLeadTrigger_H
#define DsEnsembleLeadTrigger_H

#include <dsdata/DsEnsembleAnyTrigger.hh>
#include <vector>
#include <string>
#include <deque>
#include <map>

namespace ensembleLeadTrigger
{
  /**
   * Default timeout seconds
   */
  const int timeout_seconds = 300;

  /**
   * Default disabling timeout seconds
   */
  const int disable_seconds = 600;

  /**
   * Persistant disable default status
   */
  const bool persistant_disable = true;
}

class LeadTimeState;

class DsEnsembleLeadTrigger : public DsEnsembleAnyTrigger
{
public:

  /**
   * Empty constructor
   */
  DsEnsembleLeadTrigger(void);

  /**
   * Realtime triggering constructor
   *
   * @param[in] url  The ensemble model member Urls
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored).
   *
   * The set of lead times is also needed to manage timeouts.
   */
  DsEnsembleLeadTrigger(const std::vector<std::string> &url,
			const std::vector<int> &leadSeconds);


  /**
   * Archive triggering constructor
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] url  The ensemble model member Urls
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored).
   *
   * The set of lead times is also needed to manage timeouts.
   */
  DsEnsembleLeadTrigger(const time_t &t0, const time_t &t1,
			const std::vector<std::string> &url,
			const std::vector<int> &leadSeconds);

  /**
   * Destructor
   */
  virtual ~DsEnsembleLeadTrigger(void);

  /**
   * Set maximum seconds during which no new inputs arrive for a particular
   * URL compared to other URLs during a particular gen time before declaring
   * it timed out. The default is ensembleLeadTrigger::timeout_seconds
   * 
   * @param[in] maxSeconds
   */
  void setMaxSecondsBeforeTimeout(const int maxSeconds);

  /**
   * Set maximum seconds for no inputs at all from a particular URL compared to
   * other URLs during a particular gen time before disabling it. It becomes
   * re-enabled when it starts triggering. The default is
   * ensembleLeadTrigger::disable_seconds 
   * 
   * @param[in] maxSeconds
   */
  void setMaxSecondsBeforeDisable(const int maxSeconds);

  /**
   * Set a 'persistant disable' status flag. 
   *
   * @param[in] status true or false
   *
   * If true, once a URl has become disabled it must remain disabled for the
   * duration of that gen time no matter what.
   * If false, a URL can become disabled, and triggering can occur for some
   * lead times without this URL, but then it can become enabled and contribute
   * to triggering at other lead times, all within a single gen time.
   *
   * The default is ensembleLeadTrigger::persistant_disable
   *
   * This flag should be set to true when your app needs a consistent set of
   * URl's across all lead times for an individual gen time.
   */
  void setPersistantDisable(const bool status);

  /**
   * Return with a triggering generation time/lead time/ set of URLs.
   * This is the main triggering method.
   *
   * @param[out] gt  Returned generation time
   * @param[out] lt  Returned lead time
   * @param[out] url  List of available URLs at this gen time/lead time
   * @param[out] complete  True if the list of URLs is all of them,
   *                       false if it was a timeout, or another gen time
   *                       has started triggering prior to completion.
   *
   * @return true if values are set, false if there is no more data.
   */
  bool nextTrigger(time_t &gt, int &lt, std::vector<std::string> &url,
		   bool &complete);

protected:
private:

  //----------- Params (don't change typically) ------------------------
  /**
   * The full set of URLS
   */
  std::vector<std::string> _urls;

  /**
   * The full set of lead times
   */
  std::vector<int> _lead_times;

  /**
   * Seconds before disabling a URL after a gen time starts triggering due to
   * no input, real time mode. The default is
   * ensembleLeadTrigger::disable_seconds
   */
  int _max_seconds_before_disable;

  /**
   * Seconds before declaring a URL timed out at a gen time (partial complete),
   * real time mode. The default is ensembleLeadTrigger::timeout_seconds
   */
  int _max_seconds_before_timeout;

  /**
   * True to force a URL to stay disabled once it is declared disabled for
   * the remainder of that gen time.  False to allow changing from disabled
   * back to 'abled' for lead times that have not yet completed, if data
   * for that URL recovers and begins to trigger.  Default = true,
   * realtime mode
   */
  bool _persistant_disable;

  // bool _archive_mode;  /**< True for archive mode, false for real time */
  // time_t _archive_t0;  /**< Earliest time (archive mode) */
  // time_t _archive_t1;  /**< Latest time (archive mode) */

  //----------- State (do change often) ------------------------

  /**
   * Information returned with each triggering event
   */
  struct Trigger_t
  {
    time_t t;  /**< Gen time */
    int lt;    /**< Lead seconds */
    std::vector<std::string> url;  /**< The URls that have triggered */
    bool complete;  /**< True if all URL's have triggered */
    Trigger_t() : t(0), lt(0), url(), complete(false) {}
  };

  /**
   * Que of triggering events ready to be returned, oldest thing at front
   */
  std::deque<Trigger_t> _triggered_que;

  /**
   * Current availability for each lead time at the current gen time
   */
  std::vector<LeadTimeState> _lead_time_state;

  /**
   * Current gen time
   */
  time_t _gen_time;

  /**
   * Real time of the first trigger at current _gen_time
   */
  time_t _real_time_0;

  /**
   * Current timed out URL's at current gen time
   * Timeout happens if the time between the most recent lead time
   * for the URL and the current time exceeds a maximum
   */
  std::vector<std::string> _timeout_urls;

  /**
   * Current disabled URls. Disabled URLs have not come in at all for the
   * current gen time and the real elapsed time exceeds a maximum.
   */
  std::vector<std::string> _disabled_urls;

  /**
   * Map from URL to real time of last trigger for that URL, time=-1 for none
   */
  std::map<std::string, time_t> _last_trigger_time;


  // //------------------------- archive mode state ----------------------------

  // std::vector<time_t> _archive_gen_times;  /**< Set of archive mode gen times*/
  // int _gen_time_index;   /**< Index to Last gen time returned in archivemode */
  // int _lead_time_index;  /**< Index to Last lead time returned in archivemode */

  //------------------------- methods  --------------------------------------

  bool _realtime_next(time_t &t, int &lt, std::vector<std::string> &url,
  		      bool &complete);
  // bool _archive_next(time_t &t, int &lt, std::vector<std::string> &url,
  // 		     bool &complete);
  bool _nextQuedTrigger(time_t &t, int &lt, std::vector<std::string> &url,
			bool &complete);
  void _process(const time_t &t, const int lt, const std::string &url,
	       const bool &hasData);
  void _process_data(const time_t &t, const int lt, const std::string &url);
  void _process_timeout(const std::string &url);
  void _add_to_que(void);
  void _push_new_trigger(const int i);
  void _start_new_gen_time(const time_t &t);
  void _disable_urls(void);
  // bool _archive_increment(time_t &t, int &lt);
  // void _fill_url_list(const time_t &t, const int lt,
  // 		      std::vector<std::string> &url, bool &complete) const;

};

/*
 * @class LeadTimeState
 * @brief a set of URL's for which data has triggered at a lead time,
 * for the current gen time.
 *
 */
class LeadTimeState
{
public:

  /**
   * Default constructor
   * @param[in]   The lead time (seconds)
   */
  LeadTimeState(const int lead_seconds);
  
  /**
   *  Destructor
   */
  virtual ~LeadTimeState(void);

  /**
   * Re-initialize for a new gen time (no urls yet, not processed )
   */
  void init(void);

  /**
   * Add the input URL to the vector of URLs which have triggered at gen time
   * @param[in] url
   */
  void update(const std::string &url);

  /**
   * @return True if state indicates it is time to send out the results for this
   * gen time
   * @param[in] urls  The full set of URLs
   * @param[in] timeout_urls  Those URls that have timed out
   * @param[in] disabled_urls  Those URls that are disabled.
   */
  bool should_process(const std::vector<std::string> &urls,
		      const std::vector<std::string> &timeout_urls,
		      const std::vector<std::string> &disabled_urls) const;


  /**
   * Set the _processed status to true
   */
  inline void setProcessed(void) { _processed = true;}

  /**
   * @return true if a URL has triggered based on internal state
   * @param[in] url
   */
  bool hasUrl(const std::string &url) const;

  int _lead_seconds;              /**< Current lead time */
  std::vector<std::string> _urls; /**< URLs with triggered data */
  bool _processed;                /**< True if this lead time has been completed
				   *   (led to a trigger) at current gen time */

protected:
private:  

};

#endif
