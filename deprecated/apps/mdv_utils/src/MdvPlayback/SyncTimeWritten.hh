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
 * @file SyncTimeWritten.hh
 * @brief Synchronization between playback and downstream processing, when
 *        time written is used as synchronizing time
 * @class SyncTimeWritten
 * @brief Synchronization between playback and downstream processing, when
 *        time written is used as synchronizing time
 */

# ifndef    SYNC_TIME_WRITTEN_H
# define    SYNC_TIME_WRITTEN_H

#include "ParmsMdvPlayback.hh"
#include "Sync.hh"
#include <vector>
#include <string>
class MdvPlaybackData;

//----------------------------------------------------------------
class SyncTimeWritten  : public Sync
{
public:

  /**
   * Default constructor
   *
   * @param[in]  obs True for flat obs synchronizing, false for forecasts
   * @param[in] parms  Parameters
   *
   */
  SyncTimeWritten(const bool obs, const ParmsMdvPlayback &parms);
  
  /**
   *  Destructor
   */
  virtual ~SyncTimeWritten(void);

  /**
   * Initialize synchronization state using the first data that is played back
   *
   * @param[in] data  The first data after program start
   */
  virtual void initDerived(const MdvPlaybackData &data);

  /**
   * Check for the synchronization event.
   *
   * @return true if it is now time to start synchronizing
   *
   * @param[in] data  Data who's time values are used to decide.
   */
  virtual bool timeToSync(const MdvPlaybackData &data) const;

  /**
   * Increment synchronization times 
   */
  virtual void incrementSyncDerived(void);

protected:
private:  

  /**
   * Resolution of synchronization intervals (seconds)
   */
  int pResSeconds; 

  /**
   * The most recent synchronization interval start time
   */
  time_t pSyncTwritten;

  /**
   * The expected next synchronization interval start time
   */
  time_t pSyncNextTwritten;

};

# endif 
