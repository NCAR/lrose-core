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
// RunningDriver.hh
//
// RunningDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef RunningDriver_HH
#define RunningDriver_HH

#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "MethodDriver.hh"
using namespace std;

class Trigger;

////////////////
// RunningDriver
//
// Derived class for running accumulation
//

class RunningDriver : public MethodDriver {
  
public:
  
  // constructor
  
  RunningDriver(const string &prog_name, const Args &args, const Params &params);

  // destructor
  
  virtual ~RunningDriver();

  // override run method
  
  virtual int Run();

protected:
  
private:
  
  time_t _triggerTime;
  Trigger *_trigger;

  int _doAccum();

};

#endif
