/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 13:58:59
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// Args.hh: Command line object
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <tdrp/tdrp.h>
#include <iostream>
#include <string>
using namespace std;

class Args {
  
public:

  // constructor

  Args (const string &prog_name);

  // destructor

  ~Args ();

  // parse command line
  // Returns 0 on success, -1 on failure

  int parse (const int argc, const char **argv);

  // public data

  tdrp_override_t override;

protected:
  
private:

  string _progName;
  void _usage(ostream &out);
  
};

#endif
