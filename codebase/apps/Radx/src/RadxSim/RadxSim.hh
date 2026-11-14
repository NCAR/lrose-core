#ifndef RADX_SIM_HH
#define RADX_SIM_HH

#include "Args.hh"
#include "Params.hh"

#include <csignal>
#include <string>
#include <vector>

class RadxVol;

class RadxSim {
public:
  RadxSim(int argc, char **argv);
  ~RadxSim();

  int Run();
  void requestStop() { _stopRequested = 1; }

  int OK;

private:
  struct InputFile {
    std::string path;
    double startTime;
    double endTime;
  };

  std::string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  volatile sig_atomic_t _stopRequested;
  bool _procmapRegistered;
  std::vector<InputFile> _inputs;
  double _archiveStart;
  double _archiveEnd;
  unsigned long long _publicationSequence;

  int _validateParams() const;
  int _resolveInputs();
  int _collectCandidatePaths(std::vector<std::string> &paths) const;
  int _inspectInput(const std::string &path, InputFile &input) const;
  int _runCycles();
  int _runCycle(double dataCycleStart, double monotonicCycleStart);
  int _readVolume(const std::string &path, RadxVol &vol) const;
  int _retimeVolume(RadxVol &vol, double dataCycleStart) const;
  int _waitUntil(double deadline, const std::string &activity);
  int _publishVolume(RadxVol &vol);

  static double _wallClockNow();
  static double _monotonicNow();
  static std::string _canonicalPath(const std::string &path);
  static bool _inputLess(const InputFile &lhs, const InputFile &rhs);
};

#endif

