#ifndef RADX_SIM_ARGS_HH
#define RADX_SIM_ARGS_HH

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

