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
 * @file Sync.hh
 * @brief Support synchronization between playback and downstream processing, 
 *        base class for two synchronizing mode derived classes.
 * @class Sync
 * @brief Support synchronization between playback and downstream processing,
 *        base class for two synchronizing mode derived classes.
 *
 * Contains the data time which is what is synchronized to.
 */

# ifndef    SYNC_H
# define    SYNC_H

#include "ParmsMdvPlayback.hh"
class MdvPlaybackData;

//----------------------------------------------------------------
class Sync
{
public:

  /**
   * Default constructor
   *
   * @param[in]  obs True for flat obs synchronizing, false for forecasts
   * @param[in] parms  Parameters
   */
  Sync(const bool obs, const ParmsMdvPlayback &parms);
  
  /**
   *  Destructor
   */
  virtual ~Sync(void);

  /**
   * Initialize synchronization state using the first data that is played back
   *
   * @param[in] data  The first data to playback after program start
   */
  void init(const MdvPlaybackData &data);

  /**
   * Do the synchronization algorithm.
   *
   * @param[in] outUrls  The URL(s) to synchronize to
   *
   * Each URL is queried until it has data at the synchronization time,
   * or until too much time has elapsed based on parameter settings
   */
  void synchronize(const std::vector<std::string> &outUrls);

  /**
   * Increment the synchronization times to match input playback data.
   *
   * @param[in] data Data who's times are used to set the synchronization
   *                 time state
   *
   * @note This is called after synchronize() completes
   */
  void incrementSyncTime(const MdvPlaybackData &data);

  /**
   * Update synchronization data time values to agree with input
   *
   * @param[in] data  Playback data from the synchronization input URL
   */
  void updateInput(const MdvPlaybackData &data);


  /**
   * Pure virtual method, 
   * Initialize synchronization state using the first data that is played back
   *
   * @param[in] data  The first data after program start
   */
  virtual void initDerived(const MdvPlaybackData &data) = 0;


  /**
   * Pure virtual method to check for the synchronization event.
   *
   * @return true if it is now time to start synchronizing
   *
   * @param[in] data  Data who's time values are used to decide.
   */
  virtual bool timeToSync(const MdvPlaybackData &data) const = 0;

  /**
   * Pure virtual method to increment synchronization times 
   */
  virtual void incrementSyncDerived(void) = 0;


protected:

  /**
   * Parameters
   */
  ParmsMdvPlayback pParms;

  /**
   * The synchronization input data newest gen (data) time
   */
  time_t pSyncGenTime;

  /**
   * The synchronization input data newest lead seconds (for forecast data)
   */
  int pSyncLeadSeconds;

private:  

  /**
   * True if the data used for synchronization is flat (obs) data
   */
  bool pSyncObs;

  bool pSyncDataExists(const std::string &url) const;
};

# endif 
