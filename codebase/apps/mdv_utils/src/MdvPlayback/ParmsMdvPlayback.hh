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
 * @file ParmsMdvPlayback.hh
 * @brief All the algorithm parameters for MdvPlayback
 * @class ParmsMdvPlayback
 * @brief All the algorithm parameters for MdvPlayback
 *
 * The parameters are intentionally public as it is a stateless 'struct-like'
 * class.
 */

# ifndef    PARMS_MDV_PLAYBACK_HH
# define    PARMS_MDV_PLAYBACK_HH

#include <vector>
#include <string>
#include "ParmData.hh"

//------------------------------------------------------------------
class ParmsMdvPlayback
{
public:

  /**
   * @enum Mode_t
   * @brief Modes of playback
   */
  typedef enum
  {
    PRE_PLAYBACK, /**< write latest_data_info of data prior to playback start*/
    PLAYBACK      /**< Normal mode */
  } Mode_t;

  /**
   * Default constructor (sets members to default values)
   */
  ParmsMdvPlayback(void);

  /**
   * Default constructor, gives parameters values by reading in a parameter
   * file, using input command line arguments
   * 
   * @param[in] argc  Number of command line arguments
   * @param[in] argv  args.
   */
  ParmsMdvPlayback(int argc, char **argv);
  
  /**
   * Destructor
   */
  virtual ~ParmsMdvPlayback(void);

  /**
   * The data to play back
   */
  std::vector<ParmData> pInput; 

  /**
   * The strings used to convert input URL's to output URL's
   */
  std::vector<std::pair<std::string,std::string> > pInputOutputPath;

  Mode_t pMode;      /**< The playback mode*/
  time_t pTime0;     /**< Start time of playback */
  time_t pTime1;     /**< End time of playback */
  bool pDebug;       /**< True for debugging */
  double pSpeedup;   /**< Speedup factor */
  bool pShowRealtime;/**< True to print out real time in output messages*/
  int pSecondsDelayBeforeExit; /**< Seconds to wait before exit */
  std::string pInputSyncUrl; /**< Synchronization input URL */

  /**
   * Synchronization output URLs
   */
  std::vector<std::string >  pOutputSyncUrl; 


  int pMaxWaitSeconds;     /**< Maximum seconds to wait for synchronization */

  /**
   * True to synchronize to time_written, false to synchronize to data_time
   */
  bool pSyncToTimeWritten; 

  /**
   * Resolution of playback intervals when pSyncToTimeWritten=true
   */
  double pResolutionMinutes;

  bool pDebugSync;   /**< True to print debug output for synchronization */

  int pNumThreads;
  bool pThreadDebug;

protected:
private:  

};

# endif
