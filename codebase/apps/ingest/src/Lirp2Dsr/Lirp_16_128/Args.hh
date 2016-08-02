/////////////////////////////////////////////////////////////
// Args.hh: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>
#include <iostream>
#include <tdrp/tdrp.h>
using namespace std;

class Args {
  
public:

  Args();
  ~Args();

  // parse

  int parse(int argc, char **argv, string &prog_name);

  // public data

  tdrp_override_t override;
  vector<string> inputFileList;

  void usage(string &prog_name, ostream &out);
  
protected:
  
private:

};

#endif
