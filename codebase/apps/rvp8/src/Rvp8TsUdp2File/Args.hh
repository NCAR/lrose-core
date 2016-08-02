// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:33:20 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Args.hh
//
// Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <iostream>
using namespace std;

#define TRUE 1
#define FALSE 0

class Args {
  
public:

  Args();
  ~Args();

  // parse
  
  int parse(int argc, char **argv, string &prog_name);
  
  // public data
  
  bool debug;
  bool verbose;
  bool oneFile;
  bool regWithProcmap;
  int maxDegrees;
  int maxPulses;
  string outDir;
  string instance;
  int port;
  
  void usage(string &prog_name, ostream &out);
  
protected:
  
private:

};

#endif
