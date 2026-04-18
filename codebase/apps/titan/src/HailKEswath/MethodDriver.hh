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
// MethodDriver.hh
//
// MethodDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef MethodDriver_H
#define MethodDriver_H

#include "Args.hh"
#include "Params.hh"
#include <string>
using namespace std;

//////////////////////////////////////////////////
// MethodDriver class - abstract base class for
// the classes for the accumulation methods
//
//

class MethodDriver {
  
public:

  // constructor
  
  MethodDriver (const string &prog_name,
		const Args &args, const Params &params);

  // destructor
  
  virtual ~MethodDriver();

  // run
  
  virtual int Run() = 0;

protected:
  
  const string &_progName;
  const Args &_args;
  const Params &_params;

private:

};

#endif


