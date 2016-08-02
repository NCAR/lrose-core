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
 * @file TaThreadPollingQue.hh
 * @brief One que of pointers to TaThread, used for polling applications
 *
 * @class TaThreadPollingQue
 * @brief One que of pointers to TaThread, used for polling applications
 */
#ifndef TaThreadPollingQue_HH
#define TaThreadPollingQue_HH

#include <toolsa/TaThreadQue.hh>
#include <deque>

class TaThread;

class TaThreadPollingQue  : public TaThreadQue
{
public:
  
  /**
   * Constructor, initializes the mutex members
   */
  TaThreadPollingQue(void);

  /**
   * calls releaseThreads, and destroys local mutex members
   */
  virtual ~TaThreadPollingQue(void);

  /**
   * Release and destroy local threads.
   */
  virtual void releaseThreads(void);

  /**
   * Store input thread pointer to the que
   *
   * @param[in] thread
   */
  virtual void storeThread(TaThread *thread);

  /**
   * Wait for any one thread in the que to indicate it is complete,
   * then get information from it and return that information.
   *
   * This is where the polling occurs.
   *
   * @param[in] info  Pointer to information object be copied into
   *                  from the completed TaThreads _info pointer
   *
   * @param[in] copy  Method that copies infomation, assuming the correct
   *                  info pointers are set.
   *
   * The information is copied so that the completed thread can get back
   * to work immediately.
   */
  bool waitForAnyOneThread(void *info, void copy(void *from, void *to));

  /**
   * The threads in the que will make this call when complete.
   *
   * @param[in] index  The index value of the completed thread
   */
  void signalFromThreadWhenComplete(int index);

  /**
   * The threads in the que make this call at a point where
   * they should wait for the TaThreadPollingQue to enter a 'waiting'
   * state before proceeding. This prevents (hopefully) any problems.
   */
  void waitForQueToWait(void);

protected:

  /**
   * Return pointer to an available thread.
   *
   * @param[in] index  May be used to access a particular thread
   */
  virtual TaThread *_grabThread(const int index);

private:


  std::deque<TaThread *> _thread;    /**< All the threads */
  pthread_cond_t _pollingCond;  /**< signaler = tread, for completion status */
  pthread_mutex_t _pollingMutex; /**< signaler = thread, for completion status*/

  pthread_cond_t _waitingCond;   /**< signaler = que, 'waiting state' status */
  pthread_mutex_t _waitingMutex; /**< signaler = que, 'waiting state' status */

  bool _complete;    /**< set by thread when complete */
  int _thread_index; /**< Set by thread indicating its index number */
  int _waiting;      /**< Set by queue indicating it is in 'waiting' state */

  /**
   * @return Pointer to any TaThread in the queue that has completed.  
   *
   * The first TaThread that completes is returned, once its work is complete.
   *
   * NULL is returned if there are no threads on entry
   */
  TaThread *_grabCompletedThread(void);

  /**
   * Wait for a signal from any TaThread that indicates it is complete.
   * @param[out] index  Index as set by the TaThread
   */
  void _waitForThreadToComplete(int &index);

  /**
   * Let all Threads know this que is now 'waiting'
   */
  void _signalThreadQueIsWaiting(void);


};

#endif
