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
 * @file DsUrlTriggerObjectDerived.hh
 * @brief Derived classes of the DsUrlTriggerObject virtual base class
 */

// can only be included from the DsUrlTrigger include file
#ifdef _IN_DS_URLTRIGGER_H

/**
 * @class UrlTriggerObs
 * @brief triggering incoming obs data
 */
class DsUrlTriggerObs : public DsUrlTriggerObject
{
public:

  DsUrlTriggerObs(void);

  /**
   * realtime triggering of new data 
   * @param[in] url   url from which to get times in real time
   */
  DsUrlTriggerObs(const std::string &url);

  /**
   * archive triggering of new data
   * @param[in] t0 = earliest wanted time.
   * @param[in] t1 = latest wanted time.
   * @param[in] trigger_url   url from which to get times
   * @param[in] isSpdb   True if SPDB data, false if gridded
   */
  DsUrlTriggerObs(const time_t &t0, const time_t &t1,
		  const std::string &trigger_url,
		  bool isSpdb);


  virtual ~DsUrlTriggerObs(void);

  #include <dsdata/DsUrlTriggerObjectVirtualMethods.hh>

protected:
private:

};

/**
 * @class DsUrlTriggerFcstGen
 * @brief triggering incoming forecast data when one gen time has all leads
 */

class DsUrlTriggerFcstGen : public DsUrlTriggerObject
{
public:

  DsUrlTriggerFcstGen(void);

  /**
   * realtime triggering of new data 
   * @param[in] url   url from which to get times in real time
   */
  DsUrlTriggerFcstGen(const std::string &url);

  /**
   * archive triggering of new data
   * @param[in] t0 = earliest wanted time.
   * @param[in] t1 = latest wanted time.
   * @param[in] trigger_url   url from which to get times in real time
   * @param[in] isSpdb        true for SPDB data false for gridded
   */
  DsUrlTriggerFcstGen(const time_t &t0, const time_t &t1,
		      const std::string &trigger_url, bool isSpdb);

  virtual ~DsUrlTriggerFcstGen(void);

  #include <dsdata/DsUrlTriggerObjectVirtualMethods.hh>

protected:
private:

  // full set of lead times in realtime mode
  std::vector<int> _fcst_lt;

  // last gen time returned as a trigger time
  time_t _last_gt;

  // current gen time being analyzed
  time_t _current_gt;

  // used when checking for gentime_is_complete
  std::string _error_message;

  void _initFcstAll(void);
  bool _checkTime(const DsUrlTriggerSubsample &sub,
		  const time_t &t, const int lt,
		  std::vector<std::pair<time_t,int> > &gt_nlt,
		  time_t &tret, bool &keep_first);
  bool
  _adjustLeadtimesIfStable(const std::vector<std::pair<time_t,int> > &gt_nlt);
  bool _setLeadTimesFromData(void);
  bool _setLeadTimesFromData(const std::vector<int> &lt, const time_t &gt);
  bool _gentimeIsComplete(const DsUrlTriggerSubsample &s, const time_t &t);
};


/**
 * @class DsUrlTriggerFcstLead
 * @brief triggering a sequence of forecast gen/lead times.
 */
class DsUrlTriggerFcstLead : public DsUrlTriggerObject
{
public:

  DsUrlTriggerFcstLead(void);

  /**
   * realtime triggering of new data 
   * @param[in] url   url from which to get times in real time
   */
  DsUrlTriggerFcstLead(const std::string &url);

  /**
   * archive triggering of new data
   * @param[in] t0 = earliest wanted time.
   * @param[in] t1 = latest wanted time.
   * @param[in] trigger_url   url from which to get times in real time
   * @param[in] isSpdb        true for SPDB data false for gridded
   */
  DsUrlTriggerFcstLead(const time_t &t0, const time_t &t1,
		       const std::string &trigger_url,
		       bool isSpdb);

  virtual ~DsUrlTriggerFcstLead(void);

  #include <dsdata/DsUrlTriggerObjectVirtualMethods.hh>

protected:
private:

};

#endif
