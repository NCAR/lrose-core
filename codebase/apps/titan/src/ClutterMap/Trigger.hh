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
#include <didss/LdataInfo.hh>
#include <string>
#include "Params.hh"

using namespace std;

////////////////////////////////
// Trigger - abstract base class

class Trigger {
  
public:

  // constructor

  Trigger(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Trigger();

  // get the next trigger times - this must be overloaded by the
  // derived classes

  virtual int next(time_t &data_start, time_t &data_end) = 0;

protected:
  
  const string &_progName;
  const Params &_params;

private:

};

////////////////////////////////////////////////////////
// RealtimeTrigger
//
// Derived class for realtime mode trigger
//

class RealtimeTrigger : public Trigger {
  
public:

  // constructor
  
  RealtimeTrigger(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~RealtimeTrigger();

  // get start and end times for next data
  // returns 0 on success, -1 on failure

  int next(time_t &data_start, time_t &data_end);

protected:
  
private:
  
  time_t _prevTime;
  time_t _nextTime;

};

////////////////////////////////////////////////////////
// ArchiveTrigger
//
// Derived class for archive mode trigger
//

class ArchiveTrigger : public Trigger {
  
public:

  // constructor

  ArchiveTrigger(const string &prog_name, const Params &params,
		 time_t start_time, time_t end_time);

  // destructor
  
  virtual ~ArchiveTrigger();

  // get start and end times for next data
  // returns 0 on success, -1 on failure

  int next(time_t &data_start, time_t &data_end);

protected:
  
private:
  
  time_t _startTime;
  time_t _endTime;
  time_t _prevTime;

};

////////////////////////////////////////////////////////
// ArchiveTrigger
//
// Derived class for archive mode trigger
//

class IntervalTrigger : public Trigger {
  
public:

  // constructor

  IntervalTrigger(const string &prog_name, const Params &params,
		 time_t start_time, time_t end_time);

  // destructor
  
  virtual ~IntervalTrigger();

  // get start and end times for next data
  // returns 0 on success, -1 on failure

  int next(time_t &data_start, time_t &data_end);

protected:
  
private:
  
  time_t _startTime;
  time_t _endTime;
  bool _done;

};

#endif

