// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:31:59 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Args.hh: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <iostream>
#include <string>
using namespace std;

class Args {
  
public:

  // constructor

  Args ();

  // destructor

  virtual ~Args();

  // parse command line

  int parse (int argc, char **argv, string &prog_name);

  // data
  
  bool debug;
  bool verbose;
  int port;

  bool regWithProcmap;
  string instance;

  bool checkMove;
  bool checkStowed;

  // option to invert the HV flag data (iPolarBits)
  // because the input HV flag signal is inverted
  
  bool invertHvFlag;

  // output packing

  typedef enum {
    SIGMET_FL16,
    IWRF_FL32,
    IWRF_SI16
  } packing_t;

  packing_t outputPacking;

protected:
  
private:

  void _usage(const string &prog_name, ostream &out);
  void _print(const string &prog_name, ostream &out);
  
};

#endif

