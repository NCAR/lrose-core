// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
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
#include <vector>
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

  typedef enum {
    PRINT_MODE,
    CAL_MODE,
    SERVER_MODE,
    ASCOPE_MODE
  } run_mode_t;
  
  typedef enum {
    FILE_INPUT_MODE,
    TS_API_INPUT_MODE
  } input_mode_t;
  
  typedef enum {
    PRINT_SUMMARY,
    PRINT_FULL
  } print_mode_t;
  
  bool debug;
  bool verbose;

  run_mode_t runMode;
  input_mode_t inputMode;
  int serverPort;
  bool invertHvFlag;
  
  int nSamples;
  int startGate;
  int nGates;
  bool fastAlternating;
  bool dualChannel;

  print_mode_t printMode;
  int labelInterval;
  bool onceOnly;
  bool printHvFlag;
  bool printBinAngles;
  
  bool regWithProcmap;
  string instance;

  vector<string> inputFileList;

  void usage(string &prog_name, ostream &out);
  
protected:
  
private:

};

#endif
