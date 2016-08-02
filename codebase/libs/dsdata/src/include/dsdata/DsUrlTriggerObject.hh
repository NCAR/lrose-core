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
 * @file DsUrlTriggerObject.hh
 * @brief virtual base class for triggering
 * @class DsUrlTriggerObject
 * @brief virtual base class for triggering
 */

// This header file can only be included from DsUrlTrigger.hh
#ifdef _IN_DS_URLTRIGGER_H

#define _IN_DS_URLTRIGGER_OBJECT_H

class DsUrlTriggerObject
{
public:

  /**
   * Empty Constructor
   */
  DsUrlTriggerObject(void);

  /**
   * Realtime triggering of new data 
   *
   * @param[in] url  Location of triggering data
   */
  DsUrlTriggerObject(const std::string &url);

  /**
   * Archive triggering of new data
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] url  Location of triggering data
   * @param[in] isSpdb  True for SPDB data, false for gridded
   */
  DsUrlTriggerObject(const time_t &t0, const time_t &t1,
		     const std::string &url, bool isSpdb);

  /**
   * Destructor
   */
  virtual ~DsUrlTriggerObject(void);

  /**
   * Set so there is no waiting for data.
   *
   * Calls to nextTime() will return immediately either true or false.
   *
   * @return false if archive mode as that makes no sense
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
   * Get the next triggering time and return the name of the file that caused
   * the triggering
   *
   * @param[in] sub  Subsampling specification
   * @param[out] t  Trigger time, which is gen time (FCST_GEN, FCST_LEAD) or
   *                obs time (OBS)
   * @param[out] fname  Name of file that is new
   * @return true if t/fname are set, false if no more data or an error.
   *
   * @note only implemented for real time mode
   */
  bool nextData(const DsUrlTriggerSubsample &sub, time_t &t,
		std::string &fname);

  /**
   * Rewind data so that the next call to nextTime() will return the earliest
   * time.  This makes sense only in archive mode.
   *
   * @return false if not in archive mode, or a problem occurs.
   */
  bool rewind(void);

  #define DS_URLTRIGGER_BASE
  #include <dsdata/DsUrlTriggerObjectVirtualMethods.hh>
  #undef DS_URLTRIGGER_BASE

  /**
   * @return the maximum valid age default value (seconds)
   */
  inline static int defaultMaxValidAge(void)
  {
    return DsUrlTriggerRealtime::defaultMaxValidAge();
  }

protected:

  #include <dsdata/DsUrlTriggerArchive.hh>
  #include <dsdata/DsUrlTriggerRealtime.hh>

  bool _is_realtime;
  std::string _url;
  DsUrlTriggerArchive _archive;
  DsUrlTriggerRealtime _realtime;

private:

};

#undef _IN_DS_URLTRIGGER_OBJECT_H

#endif  // #ifdef _IN_DS_URLTRIGGER_H

