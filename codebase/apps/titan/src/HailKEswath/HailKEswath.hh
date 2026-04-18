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
// HailKEswath.hh
//
// HailKEswath object
//
// RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#ifndef HailKEswath_H
#define HailKEswath_H

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>

#include "Args.hh"
#include "Params.hh"
#include <string>
using namespace std;

class HailKEswath {
  
public:

  // constructor

  HailKEswath (int argc, char **argv);

  // destructor
  
  ~HailKEswath();

  // run 

  int Run();

  // data members

  int isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

};

#endif

