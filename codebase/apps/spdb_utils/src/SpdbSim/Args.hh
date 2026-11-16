#ifndef SPDB_SIM_ARGS_HH
#define SPDB_SIM_ARGS_HH

#include <ctime>
#include <string>
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

private:
  std::string _progName;
  void _usage(std::ostream &out);
};

#endif

