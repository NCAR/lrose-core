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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Args {
  
public:

  // members

  bool debug;
  bool verbose;
  string xmlPath;
  string host;
  int port;
  bool noServer;
  bool noDsp;
  bool sendCommands;
  bool queryCommands;
  bool getStatus;
  bool regWithProcmap;
  string instance;

  // constructor

  Args ();

  // destructor

  virtual ~Args();

  // parse command line

  int parse (int argc, char **argv, string &prog_name);

protected:
  
private:

  void _usage(const string &prog_name, ostream &out);
  
  // data
  
};

#endif

