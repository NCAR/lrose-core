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
// Shmem.hh
//
// Shared memory object for communication with storm track program
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Shmem_HH
#define Shmem_HH

#include "Params.hh"
#include <titan/storm.h>
using namespace std;

////////////////////////////////
// Shmem

class Shmem {
  
public:

  // constructor

  Shmem(char *prog_name, Params *params);

  // destructor
  
  virtual ~Shmem();

  int OK;

  // Create semaphores and shared memory
  int create();

  // initialize the shared mem
  // void init(char *header_file_path);


  // Signal storm tracking program to do tracking
  int performTracking(char *header_file_path, int tracking_mode);

  // Signal to storm tracking program that StormIdent is exiting.
  // Wait to storm tracking to exit.
  void signalExit(int sig);

protected:
  
private:

  char *_progName;
  Params *_params;

  int _shmemAttached;
  int _semsAttached;

  int _semId;

  // pointer to shared mem

  storm_tracking_shmem_t *_shmem;

};

#endif


