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
 * @file TaThreadDoubleQue.hh
 * @brief Two ques of pointers to TaThread, available and active.
 *
 * @class TaThreadDoubleQue
 * @brief Two ques of pointers to TaThread, available and active.
 *
 */
#ifndef TaThreadDoubleQue_HH
#define TaThreadDoubleQue_HH

#include <toolsa/TaThreadQue.hh>
#include <deque>

class TaThread;

class TaThreadDoubleQue : public TaThreadQue
{
public:
  
  /**
   * Constructor
   */
  TaThreadDoubleQue(void);

  /**
   * calls releaseThreads
   */
  virtual ~TaThreadDoubleQue(void);

  /**
   * Release and destroy local threads, destroy local mutexes
   */
  virtual void releaseThreads(void);

  /**
   * Store input thread to local ques
   * @param[in] thread
   */
  virtual void storeThread(TaThread *thread);

  /**
   * Signal a TaThread to start work, and push it to the _active que.
   *
   * @param[in] t  Pointer to TaThread to use
   */
  virtual void activate(TaThread *t);

  /**
   * Do not return until all _active Threads have completed and
   * been moved to the _available que
   */
  void waitForThreads(void);

protected:

  /**
   * Return pointer to the next available thread.  The returned pointer is not
   * in the active or available que when returned, it is a 'free agent'.
   *
   * If any _available threads exist, the front one is taken out of that que 
   * and returned.
   * Otherwise, the first _active thread is taken from that que and returned,
   * once its work is complete.
   *
   * @param[in] i  Not used in this method
   */
  TaThread *_grabThread(const int i);

private:

  std::deque<TaThread *> _available; /**< Currently available threads */
  std::deque<TaThread *> _active;    /**< Currently active threads */


};

#endif
