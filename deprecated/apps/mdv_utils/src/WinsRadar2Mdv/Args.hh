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
// Args.h: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <iostream>
#include <tdrp/tdrp.h>
#include <string>
#include <vector>
using namespace std;

class Args {
  
public:

  // constructor

  Args ();

  // destructor

  ~Args ();

  // parse

  int parse(int argc, char **argv, const string &prog_name);

   // public data

  tdrp_override_t override;
  vector<string> filePaths;

protected:
  
private:

  void _usage(const string &prog_name, ostream &out);
  
};

#endif
