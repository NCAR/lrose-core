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
 * @file TaThreadQue.hh
 * @brief Que or ques of pointers to TaThread, virtual base class
 *
 * @class TaThreadQue
 * @brief Que or ques of pointers to TaThread, virtual base class
 *
 * @note should work for threading, and also for the case of non-threaded
 *       (number of threads = 1) situations.
 */
#ifndef TaThreadQue_HH
#define TaThreadQue_HH

#include <pthread.h>
#include <string>

class TaThread;

class TaThreadQue
{
public:
  
  /**
   * Constructor, initializes the mutex members
   */
  TaThreadQue(void);

  /**
   * destroys mutex members
   */
  virtual ~TaThreadQue(void);

  /**
   * Release and destroy qued threads
   */
  virtual void releaseThreads(void) = 0;

  /**
   * Store the input thread to the que
   *
   * @param[in] thread
   */
  virtual void storeThread(TaThread *thread) = 0;

  /**
   * Create a new derived TaThread class object and return a
   * pointer to it down-cast to TaThread.
   *
   * The actual derived object is the kind of pointers that go into this que.
   *
   * @param[in] index  Index value to use if needed.
   */
  virtual TaThread *clone(int index) = 0;

  /**
   * Signal a thread to start work and do whatever else is needed
   *
   * @param[in] thread  Pointer to thread to signal
   *
   * The default behavior is to only call signalRunToStart() on the thread,
   */
  virtual void activate(TaThread *thread);

  /**
   * Initialize 
   *
   * @param[in] num_threads  Number of threads to maintain
   * @param[in] debug  True to turn on thread debugging
   *
   * The threads are created using clone(), pointers are put to
   * the local queue.
   *
   * If num_threads < 2, no threads get created..
   */
  void init(const int num_threads, const bool debug);

  /**
   * reinitialize after having already initialized
   *
   * @param[in] numThread  Number of threads, < 2 for no threading
   * @param[in] debug  True for debugging
   *
   * first releaseThreads(), then init()
   */
  void reinit(const int numThread, const bool debug);
  
  /**
   * Activate a thread in the que, or call '_threadMethod' directly if not
   * threaded.
   *
   * @param[in] index  Index value that may or may not be used to select a
   *                   thread
   * @param[in] info  Information which is put into TaThread, or used as an
   *                  argument to _threadMethod is there is no threading
   */
  void thread(int index, void *info);
  
  /**
   * If threading, lock this objects _inputOutputMutex
   */
  void lockForIO(void);

  /**
   * If threading, unlock unlock this objects _inputOutputMutex
   */
  void unlockAfterIO(void);

  /**
   * @return pointer to the debug print mutex
   */
  pthread_mutex_t *getDebugPrintMutex(void) { return &_debugPrintMutex; }

  /**
   * Set the _context in the input TaThread to this object.
   *
   * @param[in] t  Pointer to the TaThread
   */
  void setContext(TaThread *t);

protected:

  int _numThreads;                   /**< Number of threads total */
  pthread_mutex_t _debugPrintMutex;  /**< Mutex to lock when output log info*/
  pthread_mutex_t _inputOutputMutex; /**< A mutex to lock when doing i.o.*/

private:  

  /**
   * Return pointer to an available thread.
   * @param[in] index  Index value that may or may not be used to select a
   *                   thread
   */
  virtual TaThread *_grabThread(const int index) = 0;

};

#endif
