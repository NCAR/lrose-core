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
 * @file DsUrlTrigger.hh
 * @brief Triggering in one of several ways, supports subsampling.
 *
 * Note that in the case of SPDB data as forecasts, the lead time is
 * assumed in the 'data_type_2' fieldg
 */

#ifndef DS_URLTRIGGER_H
#define DS_URLTRIGGER_H

#include <string>
#include <vector>
#include <list>
#include <dsdata/DsFcstTime.hh>

class DsLdataTrigger;

#define _IN_DS_URLTRIGGER_H

#include <dsdata/DsUrlTriggerSubsample.hh>

/**
 * @class DsUrlTrigger
 * @brief Triggering in one of several ways, supports subsampling
 *
 * Subsampling is the ability to ignore a subset of the trigger events.
 */
class DsUrlTrigger : public DsUrlTriggerSubsample
{
public:

  /**
   * @enum Trigger_t
   * @brief Three types of triggering
   */
  typedef enum
  {
    OBS,       /**< Observation (non forecast) data */
    FCST_GEN,  /**< Trigger when a new forecast is complete (all lead times)*/
    FCST_LEAD  /**< Trigger when a new forecast occurs (one gen/lead time) */
  } Trigger_t;

  DsUrlTrigger(void);

  /**
   * realtime triggering of new forecast or obs data 
   *
   * @param[in] url   Location of triggering
   * @param[in] mode  OBS=_incoming observation data 
   *                  FCST_GEN=gen time when all lead times are present
   *                  FCST_LEAD=a seqeuence of forecast gen/lead times
   * @param[in] debug  True for debugging
   */
  DsUrlTrigger(const std::string &url, const Trigger_t mode,
	       const bool debug=false);

  /**
   * archive triggering of new forecast or obs data
   *
   * @param[in] t0   Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] trigger_url  Location of triggering
   * @param[in] mode  OBS=_incoming observation data 
   *                  FCST_GEN=gen time when all lead times are present
   *                  FCST_LEAD=a seqeuence of forecast gen/lead times
   * @param[in] debug  True for debugging
   * @param[in] isSpdb  True for SPDB data, false for gridded data
   *
   */
  DsUrlTrigger(const time_t &t0, const time_t &t1,
	       const std::string &trigger_url, 
	       const Trigger_t mode, const bool debug=false,
	       const bool isSpdb=false);

  /**
   * Realtime or Archive mode triggering of new forecast or obs data
   *
   * @param[in] argc  Number of command line args
   * @param[in] argv  The command line args
   * @param[in] url   Location of triggering
   * @param[in] mode  OBS=_incoming observation data 
   *                  FCST_GEN=gen time when all lead times are present
   *                  FCST_LEAD=a seqeuence of forecast gen/lead times
   * @param[in] debug  True for debugging
   * @param[in] isSpdb  True for SPDB data, false for gridded data
   *
   * If the command line args contain '-interval yyyymmddhhmmss yyyymmddhhmmss'
   * or '-start "yyyy mm dd hh mm ss"' and '-end "yyyy mm dd hh mm ss"' then
   * the triggering is set up for archive mode. If neither of these are
   * part of the command line args, the triggering is set up for real time mode
   */
  DsUrlTrigger(int argc, char **argv, const std::string &trigger_url, 
	       const Trigger_t mode, const bool debug=false,
	       const bool isSpdb=false);

  /**
   * Destructor
   */
  virtual ~DsUrlTrigger(void);

  /**
   * Set so there is no waiting for data.
   *
   * Calls to nextTime() will return immediately either true or false.
   *
   * @return false if internal mode = FCST_GEN as that makes no sense, or
   *         if archive mode as that also makes no sense
   *
   * If this method is not called the nextTime() call can wait indefinitly
   * for new data in real time mode.
   */
  bool setNowait(void);

  /**
   * Set the maximum age of data to trigger
   *
   * @param[in] max_valid_age
   *
   * @return false if in archive mode as that makes no sense
   */
  bool setMaxValidAge(const int max_valid_age);

  /**
   * Set debugging to input value true or false
   * @param[in] value
   */
  void setDebugging(const bool value);

  /**
   * Get the next triggering time when mode = FCST_GEN or mode = OBS
   *
   * @param[out] t next time, which is a gen time (FCST_GEN) or obs time (OBS)
   *
   * @return true if triggering happened and t is set, false if no more data
   * or an error occured.
   */
  bool nextTime(time_t &t);

  /**
   * Get the next triggering forecast time when mode = FCST_LEAD
   *
   * @param[out] t Triggering gen time
   * @param[out] lt Triggering lead time seconds
   * @return true if triggering happened and t/lt are set, false if no more
   * data or an error occured.
   */
  bool nextTime(time_t &t, int &lt);

  /**
   * Get the next triggering time for any mode and return the name of the
   * file that caused the triggering
   *
   * @param[out] t  Trigger time, which is gen time (FCST_GEN, FCST_LEAD) or
   * obs time (OBS)
   * @param[out] fname  Name of file that is new
   * @return true if t/fname are set, false if no more data or an error.
   */
  bool nextData(time_t &t, std::string &fname);

  /**
   * Rewind data so that the next call to nextTime() will return the earliest
   * time.  This makes sense only in archive mode.
   *
   * @return false if not in archive mode, or a problem occurs.
   */
  bool rewind(void);

  /**
   * @return the deafault value for the maximum valid age (seconds)
   */
  static int defaultMaxValidAge(void);

  /**
   * Check command line args for known archive mode specification
   *
   * @param[in] argc Number of args
   * @param[in] argv  Args
   * @param[out] t0  Start time if archive mode is seen in args
   * @param[out] t1  End time if archive mode is seen in args
   * @param[out] archive  True if archive mode was seen
   * @param[out] error  True if error occured in parsing args
   * 
   * @return true if args were parsed successfully and it was not a
   *         help request in the command args, false if args were not
   *         parsed successfully, or the args gave a help request
   *
   * help requests:
   *     -h,  --, -?
   * 
   * archive mode:
   *    -interval yyyymmddhhmmss yyyymmddhhmmss
   *    -start "yyyy mm dd hh mm ss" -end "yyyy mm dd hh mm ss"
   */
  static bool checkArgs(int argc, char **argv, time_t &t0, time_t &t1,
			bool &archive, bool &error);

  /**
   * Check command line args for help request
   *
   * @param[in] argc Number of args
   * @param[in] argv  Args
   * 
   * @return true if args contained a help request:   -h,  --, -?
   * 
   */
  static bool hasHelpArg(int argc, char **argv);

  /**
   * @return a string for the input mode
   * @param[in] t Mode
   */
  static std::string sprintMode(Trigger_t t);

protected:
private:

  #include <dsdata/DsUrlTriggerObject.hh>
  #include <dsdata/DsUrlTriggerObjectDerived.hh>

  Trigger_t _mode;               /**< The triggering mode */
  DsUrlTriggerObject *_trigger;  /**< one of several derived classes */

  void _initRealTime(const std::string &trigger_url);
  void _initArchive(const std::string &trigger_url,
		    const time_t &t0, const time_t &t1, const bool isSpdb);
};

#undef _IN_DS_URLTRIGGER_H


#endif
