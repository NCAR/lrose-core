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
/////////////////////////////////////////////////////////////
// Thread.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Handling compute threads
//
///////////////////////////////////////////////////////////////

#ifndef Threads_hh
#define Threads_hh

#include <pthread.h>
class Radx2Grid;
class RadxRay;
class Interp;
class CartInterp;
class PpiInterp;
class PolarInterp;
class PrevReorderInterp;
class ReorderInterp;
class SatInterp;

using namespace std;

////////////////////////////
// Generic thread base class

class Thread {
public:
  
  Thread();
  virtual ~Thread();

  // thread details

  inline void setThreadId(pthread_t val) { _thread = val; }
  
  // return code
  
  inline void setReturnCode(int val) { _returnCode = val; }
  inline int getReturnCode() const { return _returnCode; }

  //////////////////////////////////////////////////////////////
  // Mutex handling for communication between caller and thread
  
  // Parent signals thread to start work
  
  void signalWorkToStart();
  
  // Thread waits for parent to signal start

  void waitForStartSignal();

  // Thread signals parent it is complete

  void signalParentWorkIsComplete();

  // Parent waits for thread to be complete

  void waitForWorkToComplete();

  // Mark thread as available

  void markAsAvailable();

  // Wait for thread to be available

  void waitToBeAvailable();
  
  // get flag indicating thread is available

  bool getAvailFlag();

  // set flag to tell thread to exit

  void setExitFlag(bool val);

  // get flag indicating thread should exit

  bool getExitFlag();

protected:

  pthread_t _thread;

  pthread_mutex_t _startMutex;
  pthread_mutex_t _completeMutex;
  pthread_mutex_t _availMutex;
  pthread_mutex_t _exitMutex;

  pthread_cond_t _startCond;
  pthread_cond_t _completeCond;
  pthread_cond_t _availCond;

  bool _startFlag;
  bool _completeFlag;
  bool _availFlag;
  bool _exitFlag;

  // return code
  
  int _returnCode;

private:

};

//////////////////////////////////
// Threads that need app context

class AppThread : public Thread 

{

public:

  AppThread();
  virtual ~AppThread();

  // application context
  
  inline void setApp(Radx2Grid *val) { _app = val; }
  inline Radx2Grid *getApp() const { return _app; }

protected:

  // application context

  Radx2Grid *_app;

};

///////////////////////////////////////
// Computation thread for cartesian data

class CartThread : public Thread 

{

public:
  
  typedef enum {
    GRID_LOC, INTERP
  } cart_task_t;

  CartThread();
  virtual ~CartThread();

  // computation context
  
  inline void setTask(cart_task_t task) { _task = task; }
  inline void setYIndex(int yIndex) { _yIndex = yIndex; }
  inline void setZIndex(int zIndex) { _zIndex = zIndex; }
  
  inline cart_task_t getTask() const { return _task; }
  inline int getYIndex() const { return _yIndex; }
  inline int getZIndex() const { return _zIndex; }
  
  // object context
  
  inline void setContext(CartInterp *val) { _context = val; }
  inline CartInterp *getContext() const { return _context; }

private:

  // object context
  
  CartInterp *_context;

  // computation context

  cart_task_t _task;
  int _yIndex;
  int _zIndex;

};

////////////////////////////////////////////
// Computation thread for ppi cartesian data

class PpiThread : public Thread 

{

public:
  
  typedef enum {
    GRID_LOC, INTERP
  } ppi_task_t;

  PpiThread();
  virtual ~PpiThread();

  // computation context
  
  inline void setTask(ppi_task_t task) { _task = task; }
  inline void setYIndex(int yIndex) { _yIndex = yIndex; }
  inline void setZIndex(int zIndex) { _zIndex = zIndex; }
  
  inline ppi_task_t getTask() const { return _task; }
  inline int getYIndex() const { return _yIndex; }
  inline int getZIndex() const { return _zIndex; }
  
  // object context
  
  inline void setContext(PpiInterp *val) { _context = val; }
  inline PpiInterp *getContext() const { return _context; }

private:

  // object context
  
  PpiInterp *_context;

  // computation context

  ppi_task_t _task;
  int _yIndex;
  int _zIndex;

};

///////////////////////////////////////////////
// Computation thread for polar regular grid

class PolarThread : public Thread 

{

public:
  
  PolarThread();
  virtual ~PolarThread();

  // computation context
  
  inline void setYIndex(int yIndex) { _yIndex = yIndex; }
  inline void setZIndex(int zIndex) { _zIndex = zIndex; }
  
  inline int getYIndex() const { return _yIndex; }
  inline int getZIndex() const { return _zIndex; }
  
  // object context
  
  inline void setContext(PolarInterp *val) { _context = val; }
  inline PolarInterp *getContext() const { return _context; }

private:

  // object context
  
  PolarInterp *_context;

  // computation context

  int _yIndex;
  int _zIndex;

};

////////////////////////////////////////////////
// Computation thread for reorder interpolation

class ReorderThread : public Thread 

{

public:
  
  typedef enum {
    GRID_LOC, INTERP
  } reorder_task_t;

  ReorderThread();
  virtual ~ReorderThread();

  // computation context
  
  inline void setTask(reorder_task_t task) { _task = task; }
  inline void setXIndex(int xIndex) { _xIndex = xIndex; }
  inline void setYIndex(int yIndex) { _yIndex = yIndex; }
  inline void setZIndex(int zIndex) { _zIndex = zIndex; }
  inline void setGridLoc(void ***_gridPlaneLoc);  // GridLoc information
  
  inline reorder_task_t getTask() const { return _task; }
  inline int getXIndex() const { return _xIndex; }
  inline int getYIndex() const { return _yIndex; }
  inline int getZIndex() const { return _zIndex; }
  
  // object context
  
  inline void setContext(ReorderInterp *val) { _context = val; }
  inline ReorderInterp *getContext() const { return _context; }

private:

  // object context
  
  ReorderInterp *_context;

  // computation context

  reorder_task_t _task;
  int _xIndex;
  int _yIndex;
  int _zIndex;

};

////////////////////////////////////////////////
// Computation thread for reorder interpolation

class PrevReorderThread : public Thread 

{

public:
  
  typedef enum {
    GRID_LOC, INTERP
  } reorder_task_t;

  PrevReorderThread();
  virtual ~PrevReorderThread();

  // computation context
  
  inline void setTask(reorder_task_t task) { _task = task; }
  inline void setXIndex(int xIndex) { _xIndex = xIndex; }
  inline void setYIndex(int yIndex) { _yIndex = yIndex; }
  inline void setZIndex(int zIndex) { _zIndex = zIndex; }
  
  inline reorder_task_t getTask() const { return _task; }
  inline int getXIndex() const { return _xIndex; }
  inline int getYIndex() const { return _yIndex; }
  inline int getZIndex() const { return _zIndex; }
  
  // object context
  
  inline void setContext(PrevReorderInterp *val) { _context = val; }
  inline PrevReorderInterp *getContext() const { return _context; }

private:

  // object context
  
  PrevReorderInterp *_context;

  // computation context

  reorder_task_t _task;
  int _xIndex;
  int _yIndex;
  int _zIndex;

};

////////////////////////////////////////////////
// Computation thread for reorder interpolation

class SatThread : public Thread 

{

public:
  
  typedef enum {
    GRID_LOC, INTERP
  } reorder_task_t;

  SatThread();
  virtual ~SatThread();

  // computation context
  
  inline void setTask(reorder_task_t task) { _task = task; }
  inline void setXIndex(int xIndex) { _xIndex = xIndex; }
  inline void setYIndex(int yIndex) { _yIndex = yIndex; }
  inline void setZIndex(int zIndex) { _zIndex = zIndex; }
  
  inline reorder_task_t getTask() const { return _task; }
  inline int getXIndex() const { return _xIndex; }
  inline int getYIndex() const { return _yIndex; }
  inline int getZIndex() const { return _zIndex; }
  
  // object context
  
  inline void setContext(SatInterp *val) { _context = val; }
  inline SatInterp *getContext() const { return _context; }

private:

  // object context
  
  SatInterp *_context;

  // computation context

  reorder_task_t _task;
  int _xIndex;
  int _yIndex;
  int _zIndex;

};

#endif

