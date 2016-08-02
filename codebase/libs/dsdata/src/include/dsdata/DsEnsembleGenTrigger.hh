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
 * @file DsEnsembleGenTrigger.hh
 * @brief Trigger a return when all forecasts for all ensemble members for a
 *        gen time are present, or there is a timeout.
 * @class DsEnsembleGenTrigger
 * @brief Trigger a return when all forecasts for all ensemble members for a
 *        gen time are present, or there is a timeout.
 *
 * Uses threads for each URL, designed for triggering ensemble model inputs
 * for the case where the trigger is for generation time. A thread will complete
 * when all the model inputs for all ensembles members for a gen time are
 * present.. within reason.  Each thread can time out independently of others.
 */

#ifndef DsEnsembleGenTrigger_H
#define DsEnsembleGenTrigger_H

#include <string>
#include <vector>
#include <toolsa/TaThreadDoubleQue.hh>

namespace ensembleGenTrigger
{
  /**
   * Default number of seconds to sleep in polling loops
   */
  const int defaultSleepSeconds = 5;

  /**
   * The maximum age in seconds of data to be considered for triggering
   */
  const int maxValidAgeSeconds = 12*3600;

}

class DsEnsembleGenTrigger;
class DsEnsembleGenTrigger1;
class DsLdataTrigger;

/**
 * @class ThreadAll
 * @brief Instantiates the TaThreadDoubleQue by implementing clone()
 */
class ThreadAll : public TaThreadDoubleQue
{
public:
  /**
   * Empty constructor
   */
  inline ThreadAll(void) : TaThreadDoubleQue(), _context(NULL) {}
  /**
   * Empty destructor
   */
  inline virtual ~ThreadAll(void) {}

  /**
   * Set local context member to input
   * @param[in] c
   */
  inline void setContext(DsEnsembleGenTrigger *c) {_context = c;}

  /**
   * Clone and return pointer to a TaThread
   * @param[in] i  Index
   */
  virtual TaThread *clone(int i);
private:

  /**
   * Pointer to context in which this object exists
   */
  DsEnsembleGenTrigger *_context;
};    


class DsEnsembleGenTrigger : public ThreadAll
{
public:

  /**
   * Empty constructor
   */
  DsEnsembleGenTrigger(void);

  /**
   * Realtime constructor
   *
   * @param[in] url  The set of Urls to watch
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored), also needed to manage timeouts.
   *
   * @param[in] maxSecondsDuringGenTime  Maximum seconds allowed to
   *                                     totally process a gen time. Once
   *                                     a gen time begins showing up, the
   *                                     trigger will happen after this much
   *                                     elapsed real time, no matter how much
   *                                     data has come in.
   * @param[in] maxSecondsAfterThreadComplete  Maximum seconds to wait
   *                                           after any one thread completes
   *                                           before timing out and triggering.
   *                                           A thread completes when all its
   *                                           lead times have arrived.
   *
   * @param[in] orderedLeadTimes  True if input data from each URL is expected
   *                     to arrive in increasing lead time order. If this is
   *                     true, can decide a gen time is complete whenever the
   *                     last lead time has triggered.
   *
   *
   */
  DsEnsembleGenTrigger(const std::vector<std::string> &url,
			 const std::vector<int> &leadSeconds,
			 const int maxSecondsDuringGenTime,
			 const int maxSecondsAfterThreadComplete,
			 const bool orderedLeadTimes);


  /**
   * Archive mode constructor
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] url  The set of Urls to watch
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   */
  DsEnsembleGenTrigger(const time_t &t0, const time_t &t1,
			 const std::vector<std::string> &url,
			 const std::vector<int> &leadSeconds);
  /**
   * Destructor
   */
  virtual ~DsEnsembleGenTrigger(void);

  /**
   * Enable debugging of the triggering
   */
  void setDebug(void);

  /**
   * Filter out a substring from all URL's for use in debug output messages.
   *
   * @param[in] remove  Each URL has this substring removed from it when
   *                    used in debug output messages
   */
  void filterNames(const std::string &remove);

  /**
   * Set sleep seconds when polling. The default is
   * 
   * @param[in] seconds
   */
  void setSleepSeconds(const int seconds);

  /**
   * Set maximum age of data to be considered, The default is
   *
   * @param[in] maxSeconds
   */
  void setMaximumAgeSeconds(const int maxSeconds);

  /**
   * Compute method needed for threading
   *
   * @param[in] i  Information passed in and out
   */
  static void compute(void *i);

  /**
   * Return with the next triggering generation time,
   * This is the main triggering method.
   *
   * @param[out] t  Returned generation time
   *
   * @return true if t is set, false if not
   *
   * @note does best job to return when all data at this gen time is available
   *       for all URL's.
   */
  bool nextTime(time_t &t);

  /**
   * Set the _threadCompleteTime shared thread member if not yet set
   *
   */
  void setThreadComplete(void);

  /**
   * @return true if _threadCompleteTime shared thread member is set
   *
   * @param[out] doneTime  The value of _threadCompleteTime when return is true
   */
  bool threadComplete(time_t &doneTime);

  /**
   * @return the currently triggered gen time for i'th URL
   * @param[in] i  Index into URLs
   */
  time_t currentGen(const int i) const;

  /**
   * @return the currently triggered lead times at the current gen time for
   * the ith element
   * @param[in] i  Index into URLs
   */
  std::vector<int> currentLeads(const int i) const;

protected:
private:


  /**
   * the triggering for one input, which is basically the information that
   * goes into that thread 
   */
  std::vector<DsEnsembleGenTrigger1> _elem;

  /**
   * Value shared by all threads, set to true when by the first thread 
   * that completes a gen time.
   */
  bool _threadComplete;

  /**
   * Value shared by all threads, set to the time the first thread completed.
   * It has a valid value only when _threadComplete = true
   */
  time_t _threadCompleteTime;


  /**
   * True if in archive mode
   */
  bool _archiveMode;

  /**
   * Seconds to sleep in polling loops
   */
  int _sleepSeconds;

  bool _initializeGenTime(time_t &t);
  bool _initializeGenTimeArchive(time_t &t);
  bool _initializeGenTimeRealtime(time_t &t);
  void _evaluate(DsEnsembleGenTrigger1 &e, size_t i, time_t &t);

};

/**
 * @class DsEnsembleGenTrigger1
 *
 * @brief triggering incoming forecast data whenever a generation time has 
 *        all of its forecasts for multiple input URLs, and this is one of
 *        the URLs
 */
class DsEnsembleGenTrigger1
{
public:

  /**
   * Empty constructor
   */
  DsEnsembleGenTrigger1(void);

  /**
   * Copy constructor
   * @param[in] u
   */
  DsEnsembleGenTrigger1(const DsEnsembleGenTrigger1 &u);

  /**
   * Real time constructor
   *
   * @param[in] url  The URL
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] maxSecondsDuringGenTime  Maximum seconds to totally process
   *                                     a gen time.
   * @param[in] maxSecondsAfterThreadComplete  Maximum seconds to wait
   *                                           after any one thread completes.
   * @param[in] orderedLeadTimes  True if input data from the URL is expected
   *                     to arrive in increasing lead time order. If this is
   *                     true, can decide a gen time is complete whenever the
   *                     last lead time has triggered.
   * @param[in] alg  Context in which this object exists
   */
  DsEnsembleGenTrigger1(const std::string &url, 
			const std::vector<int> &leadSeconds,
			const int maxSecondsDuringGenTime,
			const int maxSecondsAfterThreadComplete,
			const bool orderedLeadTimes,
			DsEnsembleGenTrigger *alg);


  /**
   * Archive mode constructor
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] url  The URL
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] alg  Context in which this object exists
   */
  DsEnsembleGenTrigger1(const time_t &t0, const time_t &t1,
			  const std::string &url,
			  const std::vector<int> &leadSeconds,
			  DsEnsembleGenTrigger *alg);

  /**
   * Destructor
   */
  virtual ~DsEnsembleGenTrigger1(void);

  /**
   * Operator=
   * @param[in] u
   */
  DsEnsembleGenTrigger1 & operator=(const DsEnsembleGenTrigger1 &u);

  /**
   * Operator==
   * @param[in] u
   */
  bool operator==(const DsEnsembleGenTrigger1 &u) const;

  // inline void setAlg(DsEnsembleGenTrigger *alg) { _alg = alg;}

  /**
   * Print out internal state with a label and return that string
   *
   * @return string state inforomation.
   */
  std::string sprintState(void) const;

  /**
   * Filter out a substring from the URL, for use in debug output messages.
   *
   * @param[in] remove  The URL has this substring removed from it in
   *                    debug output messages
   */
  void filterName(const std::string &remove);

  /**
   * Set sleep seconds when polling value, 
   * default = urlTriggerMultiFcstGen::defaultSleepSeconds
   *
   * @param[in] seconds
   */
  void setSleepSeconds(const int seconds);

  /**
   * Set maximum age of data to be considered,
   * default = urlTriggerMultiFcstGen::maxValidAgeSeconds
   *
   * @param[in] maxSeconds
   */
  void setMaximumAgeSeconds(const int maxSeconds);

  /**
   * Method called outside of threading, returns most recent gen
   * time that has not yet been triggered.
   *
   * @param[out] gt  The returned gen time
   *
   * @return true if gen time was set
   */
  bool startGenTime(time_t &gt);

  /**
   * Set local state value for _wantedGt
   *
   * @param[in] gt    Value to use
   */
  void setWantedGenTime(const time_t &gt);

  /**
   * The main method that waits and accumulates lead times at a gen time
   * until all lead times are present, or some kind of timeout happens
   */
  void processGenTime(void);

  /**
   * Clear the state so there is no data at the gen time.
   *
   * @note gen time is not modified
   */
  void clear(void);

  /**
   * @return true if local state indicates data is present for a gen time
   */
  inline bool hasData(void) const {return _hasData;}

  /**
   * @return the currently triggered gen time (makes sense only when hasData())
   */
  inline time_t currentGen(void) const {return _gt;}

  /**
   * @return the currently triggered lead times at the current gen time
   */
  inline std::vector<int> currentLeads(void) const {return _availableLtSeconds;}

  /**
   * @return true if no more data can possibly be triggered
   */
  inline bool noMoreData(void) const { return _noMoreData; }

protected:
private:

  /**
   * Status values when triggering attempted
   */
  typedef enum
  {
    END_OF_DATA = 0,  /**< no more data is possible */
    NO_NEW_DATA = 1,  /**< No new data has arrived */
    GOT_DATA = 2      /**< New data is now available */
  } Trigger_t;

  std::string _url;                     /**< param */
  std::string _name;                    /**< Name used in debug output */
  std::vector<int> _leadTimeSeconds;    /**< param */
  bool _orderedLeadTimes;               /**< param */
  int _maxAgeSeconds;                   /**< param */
  int _maxSecondsDuringGenTime;       /**< param */
  int _maxSecondsAfterThreadComplete; /**< param */
  int _sleepSeconds;                  /**< param */

  time_t _wantedGt;    /**< State: The gen time to try and trigger to */
  bool _isForwardGt;   /**< State: True if a gen time is present to use */
  time_t _forwardGt;   /**< state: Gen time to ue when _isForwardGt */

  time_t _gt;                           /**< State: Current gen time */
  std::vector<int> _availableLtSeconds; /**< State: lead times available */

  bool _archive;            /**< param: Archive mode */
  std::vector<time_t> _archiveGenTimesInRange; /**< Archive gen times (all) */
  int _nextArchiveIndex;      /**< current index into archive gen times */

  DsLdataTrigger *_trigger;  /**< real time triggering object pointer */

  bool _noMoreData; /**< Set to true when no more data is possible */
  bool _hasData;    /**< Set to true if at least one lead time exists at the
		     *   current gen time */

  time_t _threadStartTime; /**< The time this object (thread) began to gather
			    *   lead times */

  DsEnsembleGenTrigger *_alg; /**< The calling algorithm */

  void _nextArchive(void);
  void _nextRealtime(void);
  bool _startGenTimeArchive(time_t &gt);
  bool _startGenTimeRealtime(time_t &gt);
  Trigger_t _doTrigger(time_t &gt, int &lt);
  void _initForWantedGenTime(void);
  bool _nextRealtimeInit(bool &done, bool &doSleep);
  bool _canAddToGenTime(bool &doSleep);
  bool _genIsComplete(void) const;
  bool _shouldGiveUp(void);
  void _printState(void) const;
  std::string _sprintState(void) const;
};

#endif

