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
// Trigger.hh
//
// Trigger mechanism 
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#ifndef Trigger_HH
#define Trigger_HH

#include <didss/DsInputPath.hh>
#include <toolsa/ldata_info.h>
#include <cstdlib>

#include "Params.hh"
using namespace std;

////////////////////////////////
// Trigger - abstract base class

class Trigger {
  
public:

  // constructor

  Trigger(const char *prog_name, Params *P);

  // destructor
  
  virtual ~Trigger();

  int OK;

  // get the next trigger time - this must be overloaded by the
  // derived classes

  virtual time_t next() = 0;

protected:
  
  char *_progName;
  Params *_params;

private:

};

////////////////////////////////////////////////////////
// ArchiveTimeTrigger
//
// Derived class for archive mode trigger based on time
//

class ArchiveTimeTrigger : public Trigger {
  
public:

  // constructor

  ArchiveTimeTrigger(const char *prog_name, Params *params,
		     time_t start_time, time_t end_time);

  // destructor
  
  virtual ~ArchiveTimeTrigger();

  // get the next trigger time - this overloads the function in the
  // abstract base class

  time_t next();

protected:
  
private:
  
  time_t _startTime;
  time_t _endTime;
  time_t _prevTime;

};



/* The FileTrigger classes are not used in this program.

////////////////////////////////////////////////////////
// ArchiveFileTrigger
//
// Derived class for archive mode trigger based on files
// in input directory
//

class ArchiveFileTrigger : public Trigger {
  
public:

  // constructor

  ArchiveFileTrigger(const char *prog_name, Params *params,
		     time_t start_time, time_t end_time);

  // destructor
  
  virtual ~ArchiveFileTrigger();

  // get the next trigger time - this overloads the function in the
  // abstract base class

  time_t next();

protected:
  
private:
  
  DsInputPath *_inputPath;

};

*/

////////////////////////////////////////////////////////
// RealltimeTimeTrigger
//
// Derived class for realltime mode trigger based on time
//

class RealtimeTimeTrigger : public Trigger {
  
public:

  // constructor
  
  RealtimeTimeTrigger(const char *prog_name, Params *params);

  // destructor
  
  virtual ~RealtimeTimeTrigger();

  // get the next trigger time - this overloads the function in the
  // abstract base class

  time_t next();
  
protected:
  
private:
  
  time_t _prevTime;
  time_t _nextTime;

};


/* The FileTrigger classes are not needed here.

////////////////////////////////////////////////////////
// RealtimeFileTrigger
//
// Derived class for realtime mode trigger based on files
// in input directory
//

class RealtimeFileTrigger : public Trigger {
  
public:
  
  // constructor

  RealtimeFileTrigger(const char *prog_name, Params *params);

  // destructor
  
  virtual ~RealtimeFileTrigger();

  // get the next trigger time - this overloads the function in the
  // abstract base class

  time_t next();

protected:
  
private:
  
  LDATA_handle_t _lData;

};

*/

#endif

