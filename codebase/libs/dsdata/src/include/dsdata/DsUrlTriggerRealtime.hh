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
 * @file DsUrlTriggerRealtime.hh
 * @brief a sequence of triggering times is returned, realtime mode.
 * @class DsUrlTriggerRealtime
 * @brief a sequence of triggering times is returned, realtime mode.
 */

// This header file can only be included from DsUrlTriggerObject.hh
#ifdef _IN_DS_URLTRIGGER_OBJECT_H

class DsUrlTriggerRealtime 
{
public:

  DsUrlTriggerRealtime(void);

  /**
   * @param[in] url   Source of triggering events.
   */
  DsUrlTriggerRealtime(const std::string &url);

  /**
   * Copy constructor, needed because there is a pointer member
   * @param[in] u
   */
  DsUrlTriggerRealtime(const DsUrlTriggerRealtime &u);

  /**
   * operator=, needed because there is a pointer member
   * @param[in] u
   */
  DsUrlTriggerRealtime & operator=(const DsUrlTriggerRealtime &u);

  /**
   * Destructor
   */
  virtual ~DsUrlTriggerRealtime(void);

  /**
   * Set so there is no waiting for data.
   * Calls to next() will return immediately either true or false.
   */
  void setNowait(void);

  /**
   * Modify the max_valid_age param, used by the DsLdataTrigger member
   * @param[in] max_valid_age
   */
  void setMaxValidAge(const int max_valid_age);

  /**
   * next time, forecast or obs data. 
   * @param[in] s   Subsampling specification
   * @param[out] t  Next time, which is gen time if it is forecast data,
   *                or observation time if not.
   * @param[out] lt  Next lead time, which is lead time seconds, or 0 if
   *                 it is observations data.
   *
   * @return true if t/lt are set, false if no more data.
   */
  bool next(const DsUrlTriggerSubsample &s, time_t &t, int &lt);

  /**
   * Method that allows skipping of the trigger that occurs on startup.
   */
  void firstTrigger(void);

  /**
   * @return most recently triggered file name
   */
  std::string currentFilename(void) const;

  /**
   * @return default value for maximum valid age (seconds)
   */
  static int defaultMaxValidAge(void);

protected:
private:

  /**
   * @enum Read_status_t
   * @brief Status of reading data
   */
  typedef enum
  {
    ABORT = 0,     /**< Give up (end of data or error) */
    UNWANTED = 1,  /**< Triggred data is not part of subsampling */
    WANTED = 2     /**< Triggered data is good to go */
  } Read_status_t;

  std::string _url;        /**< Location of triggering */
  DsLdataTrigger *_LT;     /**< Triggering object */
  DsFcstTime _last_time;   /**< most recent triggered valid (gen+lead) time */
  std::string _last_fname; /**< most recent triggered file name */

  /**
   * _delay_msecs: polling delay in millisecs.
   *              The object will sleep for this time between polling attempts.
   *              If this value is < 0, the next() method won't block but
   *              will instead return the latest time in the ldata file.
   *
   */
  int _delay_msec;

  /**
   * _max_valid_age: the max valid age for data (secs)
   *                The object will not return data which has not arrived
   *                within this period. 
   */
  int _max_valid_age;

  void _init(void);
  Read_status_t _nextTimeSequence(const DsUrlTriggerSubsample &s,
				  time_t &t, int &lt);

};

#endif // #ifdef _IN_DS_URLTRIGGER_OBJECT_H

