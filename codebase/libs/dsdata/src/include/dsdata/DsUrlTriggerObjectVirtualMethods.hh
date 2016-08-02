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
 * @file DsUrlTriggerObjectVirtualMethods.hh
 * @brief A file included within class definition for DsUrlTriggerObject
 *        or derived classes
 */

/**
 * virtual next time method. Returns next time in all modes. When mode
 * is FCST_GEN it is a new gen time, when mode = OBS it is the next obs time,
 * and when mode = FCST_LEAD it is the next triggering 
 * gen time (which might be same as previously triggered gen time).
 *
 * @param[in] sub  Subsampling state.
 * @param[out] t  Next time, which is a gen time or obs time
 * @return true if t is set, false if no more data or error.
 */
virtual bool nextTime(const DsUrlTriggerSubsample &sub, time_t &t)
#ifdef DS_URLTRIGGER_BASE
  = 0;
#else
  ;
#endif

/**
 * virtual next time method. Returns next forecast time when mode = FCST_LEAD.
 * For all other modes returns false.
 *
 * @param[in] sub  Subsampling state.
 * @param[out] t  Next time, which is a gen time
 * @param[out] lt next lead time seconds.
 * @return true if t/lt are set, false if no more data.
 */
virtual bool nextTime(const DsUrlTriggerSubsample &sub, time_t &t,  int &lt)
#ifdef DS_URLTRIGGER_BASE
  = 0;
#else
  ;
#endif

/**
 * Virtual method to initialize archive list of gen/lead values
 * @param[in] t0  Earliest wanted time
 * @param[in] t1  Latest wanted time
 */
virtual void archiveInit(const time_t &t0, const time_t &t1)
#ifdef DS_URLTRIGGER_BASE
  = 0;
#else
  ;
#endif

