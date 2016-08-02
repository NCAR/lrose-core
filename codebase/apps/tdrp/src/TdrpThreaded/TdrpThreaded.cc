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
///////////////////////////////////////////////////////////////
// TdrpThreaded.cc
//
// TdrpThreaded object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// TdrpThreaded tests the C++ functionality of TDRP
//
///////////////////////////////////////////////////////////////

#include "TdrpThreaded.hh"
#include <string>
#include <stdlib.h>

// Constructor

TdrpThreaded::TdrpThreaded(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = strdup("TdrpThreaded");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  _threadCount = 0;
  pthread_mutex_init(&_countMutex, NULL);

  return;

}

// destructor

TdrpThreaded::~TdrpThreaded()

{

  // free up

  delete(_params);
  delete(_args);
  free(_progName);
  
}

//////////////////////////////////////////////////
// Run

int TdrpThreaded::Run()
{

  cerr << "Run: Params file: " <<  _paramsPath << endl;

  // create 50 threads, read in file, print out

  for (int i = 0; i < 50; i++) {

    int iret;
    pthread_t thread;
    if ((iret = pthread_create(&thread, NULL,
			       _loadParams, this))) {
      cerr << "ERROR - " << _progName << ":Run()." << endl;
      cerr << "  Cannot create thread." << endl;
      cerr << "  " << strerror(iret) << endl;
      return (-1);
    }
    pthread_detach(thread);

  }

  return 0;

}

void *TdrpThreaded::_loadParams(void *args)
  
{

  TdrpThreaded *obj = (TdrpThreaded *) args;

  // set thread number

  int tnum;
  pthread_mutex_lock(&obj->_countMutex);
  tnum = obj->_threadCount;
  obj->_threadCount++;
  string paramsPath = obj->_paramsPath;
  pthread_mutex_unlock(&obj->_countMutex);
  
  cerr << "_loadParams: thread number: " <<  tnum << endl;
  cerr << "_loadParams: Params file: " <<  paramsPath << endl;

  Params params;
  
  if (params.load((char *) paramsPath.c_str(), NULL, false, false)) {
    cerr << "  Thread number:" << tnum << endl;
    cerr << "  Cannot load params file." << paramsPath << endl;
    return NULL;
  }
    
  // print out the parameters
  
  cerr << "  Thread number:" << tnum << endl;
  params.print(stderr);

  return NULL;

}

