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

#ifndef TotalDriver_HH
#define TotalDriver_HH

#include "MethodDriver.hh"
using namespace std;

////////////////////////////////////////
// TotalDriver
//
// Derived class for total accumulation
//

class TotalDriver : public MethodDriver {
  
public:
  
  // constructor
  
  TotalDriver(const string &prog_name,
	      const Args &args, const Params &params);

  // destructor
  
  virtual ~TotalDriver();

  // override run method
  
  virtual int Run();

protected:
  
private:
  
};

#endif


