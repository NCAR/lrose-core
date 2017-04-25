/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// ThreadTest.h
//
// ThreadTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef ThreadTest_H
#define ThreadTest_H

#include <tdrp/tdrp.h>
#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "MyThread.hh"

class ThreadTest {
  
public:

  // constructor

  ThreadTest (int argc, char **argv);

  // destructor
  
  ~ThreadTest();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;

  int _countTotal;
  double _sumTotal;

  vector<MyThread *> _myThreads;

  int _runThreadsOnce();
  int _runThreadPool();
  void _cleanUpThreads();
  

};

#endif
