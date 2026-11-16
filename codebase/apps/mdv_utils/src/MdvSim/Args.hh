// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD license applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that the
// ** copyright notice, conditions, and disclaimer are retained.
// ** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
///////////////////////////////////////////////////////////////
//
// Args.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////

#ifndef MDV_SIM_ARGS_HH
#define MDV_SIM_ARGS_HH

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <tdrp/tdrp.h>

class Args {
public:
  Args();
  ~Args();

  int parse(int argc, char **argv, const std::string &progName);

  tdrp_override_t override;
  bool startTimeSet;
  bool endTimeSet;
  time_t startTime;
  time_t endTime;
  std::vector<std::string> inputFileList;

private:
  std::string _progName;
  void _usage(std::ostream &out);
};

#endif

