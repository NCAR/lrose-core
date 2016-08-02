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
// TTest.h
//
// TTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef TTest_H
#define TTest_H

#include <tdrp/tdrp.h>
#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"

class TTest {
  
public:

  // constructor

  TTest (int argc, char **argv);

  // destructor
  
  ~TTest();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;

  double _earthRadiusKm;
  double _effRadiusKm;
  double _effectiveMult;
  double _effectiveMultNex;
  double _h0;

  double _nexradHt(double rangeKm, double elevDeg);
  double _zrnicHt(double rangeKm, double elevDeg);
  double _cosineHt(double rangeKm, double elevDeg);
  double _zengHt(double rangeKm, double elevDeg);
  double _williamsHt(double rangeKm, double elevDeg);

};

#endif
