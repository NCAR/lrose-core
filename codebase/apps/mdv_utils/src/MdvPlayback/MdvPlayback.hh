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
 * @file MdvPlayback.hh
 * @brief The MdvPlayback algorithm.
 * @class MdvPlayback
 * @brief The MdvPlayback algorithm.
 *
 */

# ifndef    MDV_PLAYBACK_H
# define    MDV_PLAYBACK_H

#include "ParmsMdvPlayback.hh"
#include "MdvPlaybackData.hh"
#include "Sync.hh"
#include <vector>
#include <toolsa/TaThreadDoubleQue.hh>

//----------------------------------------------------------------
class MdvPlayback
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   * @param[in] tidyAndExit  Cleanup function to call at exit
   *
   * Prepare the class for a call to the run() method
   */
  MdvPlayback(const ParmsMdvPlayback &p, void tidyAndExit(int));
  
  /**
   *  Destructor
   */
  virtual ~MdvPlayback(void);

  /**
   * Run the app (playback)
   */
  void run(void);

  /**
   * Method needed for threading
   * @param[in] i Information pointer
   */
  static void compute(void *i);

protected:
private:  

  /**
   * @class ThreadAlg
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   *        clone method
   */
  class ThreadAlg : public TaThreadDoubleQue
  {
  public:
    inline ThreadAlg() : TaThreadDoubleQue() {}
    inline virtual ~ThreadAlg() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index
     */
    TaThread *clone(const int index);
  };

  /**
   * The Algorithm parameters, kept as internal state
   */
  ParmsMdvPlayback pParms;

  /**
   * The Playback data
   */
  std::vector<MdvPlaybackData> pData;

  /**
   * Pointer to Sync class object, which will be one of two derived classes,
   * or NULL for no synchronization
   */
  Sync *pSync;

  /**
   * Actual real time written for most recent write
   */
  time_t pRealTimeWritten;

  /**
   * Threading object
   */
  ThreadAlg pThread;

  void pInit(void);
  bool pInitSync(void);
  void pSetState(const ParmData &P);
  bool pSetRange(const ParmData &P, time_t &lt0, time_t &lt1);
  void pSetObsState(const ParmData &P, const time_t &lt0, const time_t &lt1);
  void pSetFcstState(const ParmData &P, const time_t &lt0, const time_t &lt1);
  void pProcess(const int i);
  void pPlaybackRealtime(MdvPlaybackData *data);
  void pPlaybackSync(const MdvPlaybackData *data);
};

# endif 
