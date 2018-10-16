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
 * @file DsEnsembleAnyTrigger.hh
 * @brief Triggering incoming forecast data whenever a new forecast arrives,
 *        (new gen and/or lead time) and there are multiple input URLs
 * @class DsEnsembleAnyTrigger
 * @brief triggering incoming forecast data whenever a new forecast arrives,
 *        (new gen and/or lead time) and there are multiple input URLs
 *
 * Uses threads for each URL, designed for triggering ensemble model inputs
 * for the case where the trigger is for generation/lead time and should 
 * return when any one model input triggers arrival of new data.
 *
 * A timeout parameter causes the triggering method to return if a sufficient
 * time with no new data transpires.  One such return will happen per URL.
 *
 * This is a real time only class.
 */

#ifndef DsEnsembleAnyTrigger_H
#define DsEnsembleAnyTrigger_H

#include <string>
#include <vector>
#include <toolsa/TaThreadPollingQue.hh>

//////////////////////////////////////////////////////////////////////////
namespace ensembleAnyTrigger
{
  /**
   * Default number of seconds to sleep in polling loops
   */
  const int defaultSleepSeconds = 5;

  /**
   * The maximum age in seconds of data to be considered for triggering
   */
  const int maxValidAgeSeconds = 12*3600;

  /**
   * The maximum seconds to wait for new triggering before returning status
   * that nothing new has triggered
   */
  const int maxWaitSeconds = 60;
}


//////////////////////////////////////////////////////////////////////////
class DsEnsembleAnyTrigger;
class DsEnsembleAnyTrigger1;
class DsLdataTrigger;
class AnyTriggerData;

//////////////////////////////////////////////////////////////////////////
/**
 * @class ThreadAny
 * @brief Instantiates the TaThreadPollingQue by implementing clone()
 */
class ThreadAny : public TaThreadPollingQue
{
public:
  /**
   * Empty constructor
   */
  inline ThreadAny(void) : TaThreadPollingQue(), _context(NULL) {}
  /**
   * Empty destructor
   */
  inline virtual ~ThreadAny(void) {}
  /**
   * Set context member pointer to input
   * @param[in] c
   */
  inline void setContext(DsEnsembleAnyTrigger *c) {_context = c;}

  /**
   * Clone an object, returning a TaThread pointer
   * @param[in] i Index
   *
   * @return pointer
   */
  virtual TaThread *clone(int i);
private:

  /**
   * Pointer to the context in which the thread exists
   */
  DsEnsembleAnyTrigger *_context;
};    

//////////////////////////////////////////////////////////////////////////
class DsEnsembleAnyTrigger : public ThreadAny
{
public:

  /**
   * Empty constructor
   */
  DsEnsembleAnyTrigger(void);

  /**
   * Main constructor - real time
   *
   * @param[in] url  The set of Urls to watch
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * The set of lead times is also needed to manage timeouts.
   */
  DsEnsembleAnyTrigger(const std::vector<std::string> &url,
		       const std::vector<int> &leadSeconds);

  /**
   * Main constructor - archive mode
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] url  The set of Urls to watch
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * The set of lead times is also needed to manage timeouts.
   */
  DsEnsembleAnyTrigger(const time_t &t0, const time_t &t1,
		       const std::vector<std::string> &url,
		       const std::vector<int> &leadSeconds);

  /**
   * Destructor
   */
  virtual ~DsEnsembleAnyTrigger(void);

  /**
   * Enable debugging.
   */
  void setDebug(void);

  /**
   * Filter out a substring from all URL's for use in debug output messages.
   *
   * @param[in] remove  Each URL has this substring removed from it when
   *                    used in debug output messages
   *
   * As an example, if a member URL = "mdvp:://localhost::PROJECT/data0"
   * and remove = "mdvp:://localhost::PROJECT/", the output messages will
   * have only "data0", not the full URL.
   */
  void filterNames(const std::string &remove);

  /**
   * Set sleep seconds when polling a thread. The default is
   * ensembleAnyTrigger::defaultSleepSeconds
   * 
   * @param[in] seconds
   */
  void setSleepSeconds(const int seconds);

  /**
   * Set maximum age of data to be considered, the default is
   * ensembleAnyTrigger::maxValidAgeSeconds
   * default = xxx
   *
   * @param[in] maxSeconds
   */
  void setMaximumAgeSeconds(const int maxSeconds);

  /**
   * Set maximum seconds to wait for  any one thread to trigger before
   * giving up and reporting back that it has not triggered,
   * the default is ensembleAnyTrigger::maxWaitSeconds
   *
   * @param[in] maxSeconds
   */
  void setTriggerMaxWaitSeconds(const int maxSeconds);

  /**
   * Compute method needed for threading
   *
   * @param[in] i  Information passed in and out 
   */
  static void compute(void *i);

  /**
   * Return the next complete set of urls for a given gen/lead time
   */
  bool archiveNextGenLeadTime(time_t &t, int &lt, 
			      std::vector<std::string> &url,
			      bool &complete);

  /**
   * Return with the next triggering generation time/lead time and URL.
   * This is the main triggering method.
   *
   * @param[out] gt  Returned generation time, if hasData = true
   * @param[out] lt  Returned lead time, if hasData = true
   * @param[out] url  Returned URL
   * @param[out] hasData Returned status, true if gen and lead times are
   *                     set, false for what is most likely a timeout.
   *
   * @return true if the method succesfully either triggered or timed out,
   * false for error.
   *
   */
  bool nextTime(time_t &gt, int &lt, std::string &url, bool &hasData);

  inline bool isArchiveMode(void) const {return _archive_mode;}


protected:
private:

  /**
   * the triggering handler for each URL which is basically the information that
   * goes into each thread, set as pointers to help it be thread safe.
   */
  std::vector<DsEnsembleAnyTrigger1 *> _elem;

  /**
   * Seconds to sleep in polling loops
   */
  int _sleepSeconds;

  //------------------------- archive mode state ----------------------------

  bool _archive_mode;
  std::vector<AnyTriggerData> _archive_events; 
  int _archive_index;   /**< Index to Last gen time returned in archivemode */
  std::vector<std::string> _urls;


  bool _nextArchiveTime(time_t &t, int &lt, std::string &url, bool &hasData);
  bool _nextRealTime(time_t &t, int &lt, std::string &url, bool &hasData);
};


//////////////////////////////////////////////////////////////////////////
/**
 * @class DsEnsembleAnyTrigger1
 * @brief triggering incoming forecast data off of gen/lead for multiple input
 *        URLs, and this is one URL, for use in real time processing only.
 */
class DsEnsembleAnyTrigger1
{
public:

  /**
   * Empty constructor
   */
  DsEnsembleAnyTrigger1(void);

  /**
   * Copy constructor
   * @param[in] u
   */
  DsEnsembleAnyTrigger1(const DsEnsembleAnyTrigger1 &u);

  /**
   * Real time constructor
   *
   * @param[in] url  The URL
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] alg  The context in which this object exists
   * 
   */
  DsEnsembleAnyTrigger1(const std::string &url, 
			const std::vector<int> &leadSeconds,
			DsEnsembleAnyTrigger *alg);
  /**
   * Destructor
   */
  virtual ~DsEnsembleAnyTrigger1(void);

  /**
   * Operator=
   * @param[in] u
   */
  DsEnsembleAnyTrigger1 & operator=(const DsEnsembleAnyTrigger1 &u);

  /**
   * Operator==
   * @param[in] u
   */
  bool operator==(const DsEnsembleAnyTrigger1 &u) const;

  /**
   * Print out internal state with a label and return that string
   *
   * @return string state information.
   */
  std::string sprintState(void) const;

  /**
   * Filter out a substring from the URL, for use in debug output messages.
   *
   * @param[in] remove  The URL has this substring removed from it in
   *                    debug output messages
   *
   */
  void filterName(const std::string &remove);

  /**
   * Set sleep seconds when polling value,  the default is
   * ensembleAnyTrigger::defaultSleepSeconds
   *
   * @param[in] seconds
   */
  void setSleepSeconds(const int seconds);

  /**
   * Set maximum age of data to be considered, the default is
   * ensembleAnyTrigger::maxValidAgeSeconds
   *
   * @param[in] maxSeconds
   */
  void setMaximumAgeSeconds(const int maxSeconds);

  /**
   * Set timeout value before returning and indicating nothing happened,
   * the default is ensembleAnyTrigger::maxWaitSeconds
   *
   * @param[in] maxSeconds
   */
  void setTriggerMaxWaitSeconds(const int maxSeconds);

  /**
   * The main method that waits until a trigger happens, then sets state.
   */
  void process(void);


  /**
   * @return true if the object has valid data
   */
  inline bool hasData(void) const {return _has_data;}

  /**
   * @return the currently triggered gen time
   */
  inline time_t getTime(void) const
  {
    if (_has_data)
      return _gt;
    else
      return -1;
  }

  /**
   * @return the currently triggered lead time
   */
  inline int getLead(void) const
  {
    if (_has_data)
      return _lt;
    else
      return -1;
  }

  /**
   * @return the URL
   */
  inline std::string getUrl(void) const {return _url;}

  /**
   * @return the name
   */
  inline std::string getName(void) const {return _name;}

  /**
   * Copy from input to output
   * @param[in] input  Void pointer to DsEnsembleAnyTrigger1 object 
   * @param[in] output Void pointer to DsEnsembleAnyTrigger1 object 
   */
  static void copy(void *input, void *output);

protected:
private:

  std::string _url;           /**< The data location */
  std::string _name;          /**< Name used in debug output */
  bool _has_data;             /**< State: True if gt/lt set */
  time_t _gt;                 /**< State: Current gen time */
  int _lt;                    /**< State: Current lead time */


  int _maxAgeSeconds;                /**< Maximum age seconds for data */
  int _sleepSeconds;                 /**< Tick between various actions param */
  int _maxWaitSeconds;               /**< Timeout seconds for triggering */
  std::vector<int> _leadTimeSeconds; /**< Configured leads */

  DsLdataTrigger *_trigger;       /**< real time triggering object pointer */
  DsEnsembleAnyTrigger *_alg;     /**< Enclosing context */

  std::string _sprintState(void) const;
  bool _next_time_sequence(void);
};

/**
 * @class AnyTriggerData
 * @brief representation of one 'any' trigger event
 */
class AnyTriggerData
{
public:
  /**
   * Empty constructor
   */
  AnyTriggerData(void);

  /**
   * Full constructor
   * @param[in] url  URL
   * @param[in] gt  Gen time 
   * @param[in] lt  Lead time seconds
   */
  AnyTriggerData(const std::string &url, const time_t &gt, int lt);

  /**
   * Destructor
   */
  virtual ~AnyTriggerData(void);

  void getValues(time_t &t, int &lt, std::string &url) const;

  static bool lessThan(const AnyTriggerData &d0, const AnyTriggerData &d1);

protected:
private:

  std::string _url;  /**< Data location */
  time_t _gt;        /**< Gen time */
  int _lt;           /**< Lead time seconds */
};

#endif


