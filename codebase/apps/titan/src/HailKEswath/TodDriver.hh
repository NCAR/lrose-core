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
// TodDriver.hh
//
// TodDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef TodDriver_H
#define TodDriver_H

#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "MethodDriver.hh"
#include "TodPeriod.hh"
using namespace std;

class MaxData;
class Trigger;

////////////
// TodDriver
//
// Derived class for Time-of-day accumulation
//

class TodDriver : public MethodDriver {
  
public:
  
  // constructor
  
  TodDriver(const string &prog_name, const Args &args, const Params &params);

  // destructor
  
  virtual ~TodDriver();

  // override run method
  
  virtual int Run();

protected:
  
private:

  time_t _triggerTime;
  time_t _startSearchNext;
  MaxData *_maxData;
  Trigger *_trigger;

  int _doAccum(TodPeriod &periods);

};

#endif


