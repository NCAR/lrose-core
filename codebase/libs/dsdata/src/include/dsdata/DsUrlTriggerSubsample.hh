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
 * @file DsUrlTriggerSubsample.hh
 * @brief Controls the subsampling of data triggering
 * @class DsUrlTriggerSubsample
 * @brief Controls the subsampling of data triggering
 */

// This header file can only be included from DsUrlTrigger.hh
#ifdef _IN_DS_URLTRIGGER_H

class DsUrlTriggerSubsample
{
public:

  /**
   * Constructor
   */
  DsUrlTriggerSubsample(void);

  /**
   * Destructor
   */
  virtual ~DsUrlTriggerSubsample(void);

  /**
   * Turn on subsampling of gen times. Subsampling allows one to distinguish
   * a subset of times returned from "next_time" calls as the only ones that 
   * are wanted
   *
   * @param[in] wanted_minutes  The subset of gen time minutes
   */
  void  gentimeSubsamplingSet(const std::vector<int> &wanted_minutes);

  /**
   * Clear so there is no gentime subsampling
   */
  void  gentimeSubsamplingClear(void);

  /**
   * Turn on lead time subsampling (for modes FCST_SEQUENCE, FCST_ALL)
   * in FCST_SEQUENCE return only the subset of the lead times as indicated.
   * in FCST_ALL mode trigger as soon as the entire subset is all present.
   *
   * @param[in] lt0  Minimum lead time(minutes)
   * @param[in] lt1  Max lead time(minutes)
   * @param[in] dlt  Delta lead time (minutes)
   */
  void leadtimeSubsamplingActivate(const double lt0, const double lt1,
				   const double dlt);


  /**
   * Clear so there is no leadtime subsampling
   */
  void leadtimeSubsamplingDeactivate(void);

  /**
   * @return true if the input gen and lead time is within the selected subset
   * @param[in] t  Gen time
   * @param[in] lt  Lead time
   */
  bool timeOk(const time_t &t, const int lt) const;

  /**
   * @return true if the input gen time is within the selected subset
   * @param[in] t  Gen time
   */
  bool timeOk(const time_t &t) const;

  /**
   * @return true if lead time subsampling is activated
   */
  inline bool isLeadSubsampling(void) const
  {
    return !_lt_seconds.empty();
  }

  /**
   * @return true if a set of lead times contains all the selected lead times
   * @param[in] available_lt   The currently available lead times
   * @param[out] err  Error string if false is returned
   */
  bool leadtimesComplete(const std::vector<int> available_lt,
			 std::string &err) const;

protected:
private:

  /**
   * Gen time subsampling (empty for no subsampling)
   */
  std::vector<int> _trigger_minutes;

  /**
   * lead time subsampling (empty for no subsampling)
   */
  std::vector<int> _lt_seconds;
};

#endif
