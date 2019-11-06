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
// ReadEgm2008.h
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// ReadEgm2008 reads in Geoid corrections from Egm2008 files
//
////////////////////////////////////////////////////////////////

#ifndef ReadEgm2008_H
#define ReadEgm2008_H

#include <tdrp/tdrp.h>
#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"

class ReadEgm2008 {
  
public:

  // constructor

  ReadEgm2008 (int argc, char **argv);

  // destructor
  
  ~ReadEgm2008();

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

};

#endif
