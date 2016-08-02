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
 * @file DsUrlTriggerArchive.hh
 * @brief a sequence of triggering times is returned, archive mode.
 * @class DsUrlTriggerArchive
 * @brief a sequence of triggering times is returned, archive mode.
 */

// This header file can only be included from DsUrlTriggerObject.hh
#ifdef _IN_DS_URLTRIGGER_OBJECT_H

class DsUrlTriggerArchive
{
public:

  /**
   * Empty constructor
   */
  DsUrlTriggerArchive(void);

  /**
   * @param[in] url  Location of triggering data
   * @param[in] isSpdb  True for SPDB data, false for gridded
   */
  DsUrlTriggerArchive(const std::string &url, const bool isSpdb);

  /**
   * Destructor
   */
  virtual ~DsUrlTriggerArchive(void);

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
   * Init archive mode for obs data
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   */
  void initObs(const time_t &t0, const time_t &t1);

  /**
   * Init archive mode for forecast gen data
   * @param[in] t0  Earliest wanted gen time.
   * @param[in] t1  Latest wanted gen time.
   */
  void initFcstGen(const time_t &t0, const time_t &t1);

  /**
   * Init archive mode for forecast gen/lead data
   * @param[in] t0  Earliest wanted gen time.
   * @param[in] t1  Latest wanted gen time.
   */
  void initFcstLead(const time_t &t0, const time_t &t1);

  /**
   * rewind data so that a call to next() will return earliest data
   *
   * @return false if a problem
   */
  bool rewind(void);

protected:
private:

  std::string _url;   /**< Location of triggering data */

  bool _isSpdb;       /**< True for SPDB */

  /**
   * all the times in range, in order
   */
  std::list <DsFcstTime> _times;

  /**
   * most recently used index into the times
   */
  std::list <DsFcstTime>::iterator _times_index;


  void _initObsMdv(const time_t &t0, const time_t &t1);
  void _initFcstLeadMdv(const time_t &t0, const time_t &t1);
  void _initObsSpdb(const time_t &t0, const time_t &t1);
  void _initFcstLeadSpdb(const time_t &t0, const time_t &t1);

};


#endif  // #ifdef _IN_DS_URLTRIGGER_OBJECT_H
