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
// CwbRadar2Mdv.hh
//
// CwbRadar2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#ifndef CwbRadar2Mdv_H
#define CwbRadar2Mdv_H

#include <toolsa/umisc.h>
#include <string>
#include <iostream>

#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvx.hh>
using namespace std;

class DsInputPath;

class CwbRadar2Mdv {
  
public:

  // constructor

  CwbRadar2Mdv (int argc, char **argv);

  // destructor
  
  ~CwbRadar2Mdv();

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
  DsInputPath *_input;

  int _processFile (const char *file_path);

};

#endif
