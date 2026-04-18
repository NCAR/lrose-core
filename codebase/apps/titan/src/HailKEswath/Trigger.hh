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

#include "HailKEswath.hh"
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
using namespace std;

////////////////////////////////
// Trigger - abstract base class

class Trigger {
  
public:

  // constructor

  Trigger(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Trigger();

  // get the next trigger time - this must be overloaded by the
  // derived classes

  virtual time_t next() = 0;

protected:
  
  const string &_progName;
  const Params &_params;

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

  ArchiveTimeTrigger(const string &prog_name, const Params &params,
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

////////////////////////////////////////////////////////
// ArchiveFileTrigger
//
// Derived class for archive mode trigger based on files
// in input directory
//

class ArchiveFileTrigger : public Trigger {
  
public:

  // constructor

  ArchiveFileTrigger(const string &prog_name, const Params &params,
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

////////////////////////////////////////////////////////
// RealltimeTimeTrigger
//
// Derived class for realltime mode trigger based on time
//

class RealtimeTimeTrigger : public Trigger {
  
public:

  // constructor
  
  RealtimeTimeTrigger(const string &prog_name, const Params &params);

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

////////////////////////////////////////////////////////
// RealtimeFileTrigger
//
// Derived class for realtime mode trigger based on files
// in input directory
//

class RealtimeFileTrigger : public Trigger {
  
public:
  
  // constructor

  RealtimeFileTrigger(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~RealtimeFileTrigger();

  // get the next trigger time - this overloads the function in the
  // abstract base class

  time_t next();

protected:
  
private:
  
  LdataInfo _ldata;

};

#endif

