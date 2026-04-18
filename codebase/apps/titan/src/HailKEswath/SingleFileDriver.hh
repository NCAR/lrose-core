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
// SingleFileDriver.hh
//
// SingleFileDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2002
//
///////////////////////////////////////////////////////////////

#ifndef SingleFileDriver_HH
#define SingleFileDriver_HH

#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "MethodDriver.hh"
using namespace std;

class Trigger;

////////////////
// SingleFileDriver
//
// Derived class for running accumulation
//

class SingleFileDriver : public MethodDriver {
  
public:
  
  // constructor
  
  SingleFileDriver(const string &prog_name,
		   const Args &args, const Params &params);

  // destructor
  
  virtual ~SingleFileDriver();

  // override run method
  
  virtual int Run();

protected:
  
private:
  
  time_t _triggerTime;
  Trigger *_trigger;

  int _doAccum();

};

#endif
