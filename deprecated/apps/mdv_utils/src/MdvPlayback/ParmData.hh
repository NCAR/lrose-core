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
#include <copyright.h>

/**
 * @file ParmData.hh
 * @brief  Parameters for playback for one data
 * @class ParmData
 * @brief Parameters for playback for one data
 *
 * The parameters are intentionally public as it is a stateless 'struct-like'
 * class.
 */

# ifndef    PARM_DATA_HH
# define    PARM_DATA_HH

#include <string>

//------------------------------------------------------------------
class ParmData
{
public:

  /**
   * Default constructor
   * 
   * @param[in] url  Input URL
   * @param[in] isObs  True if data is flat, false if forecast
   * @param[in] useDataTime  True to ignore time_written in playback and 
   *                         instead use data_time
   * @param[in] latencyHoursMax  Maximum expected latency between time_written
   *                             and data_time (positive if
   *                             data_time < time_written)
   */
  ParmData(const std::string &url, const bool isObs,
	   const bool useDataTime,  const double latencyHoursMax);

  /**
   * Destructor
   */
  virtual ~ParmData(void);

  std::string pUrl;      /**< Input URL */
  bool pIsObs;           /**< True if data is flat, false if forecast */

  /**
   * True to ignore time_written in playback and  instead use data_time
   */
  bool pUseDataTime;     
  
  /**
   * Maximum expected latency between time_written and data_time
   * (positive if data_time < time_written)
   */
  int pLatencySecondsMax;

protected:
private:  

};

# endif
