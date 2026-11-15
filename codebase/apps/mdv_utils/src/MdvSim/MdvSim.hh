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
// MdvSim.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// MdvSim is intended to simulate MDV files in a realtime
// scenario.
//
// MdvSim loops through a set of MDV files,
// modifies the header times to wall-clock time, and writes the
// modified files to an output dir.
//
///////////////////////////////////////////////////////////////

#ifndef MDV_SIM_HH
#define MDV_SIM_HH

#include "Args.hh"
#include "Params.hh"

#include <csignal>
#include <string>
#include <vector>

class DsMdvx;

class MdvSim {
public:
  MdvSim(int argc, char **argv);
  ~MdvSim();

  int Run();
  void requestStop() { _stopRequested = 1; }

  int OK;

private:
  struct InputFile {
    enum Selector {
      PATH,
      VALID_TIME,
      FORECAST_TIME
    } selector;

    std::string path;
    std::string tieKey;
    time_t validTime;
    time_t genTime;
    int leadTime;
    time_t scheduleTime;
    int forecastDelta;
    int sourceFormat;
  };

  std::string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  volatile sig_atomic_t _stopRequested;
  bool _procmapRegistered;
  std::vector<InputFile> _inputs;
  time_t _archiveStart;
  time_t _archiveEnd;

  int _validateParams() const;
  int _resolveInputs();
  int _collectCandidates(std::vector<InputFile> &candidates) const;
  int _inspectInput(InputFile &input) const;
  int _readInput(const InputFile &input, DsMdvx &mdvx) const;
  int _preflightCycle(time_t dataCycleStart) const;
  int _runCycles();
  int _runCycle(time_t dataCycleStart, double deadlineCycleStart);
  int _retime(DsMdvx &mdvx, time_t mappedSchedule) const;
  int _waitUntil(double deadline, const std::string &activity);
  int _publish(DsMdvx &mdvx, const InputFile &input);
  time_t _mappedSchedule(const InputFile &input,
                         time_t dataCycleStart) const;

  static double _monotonicNow();
  static std::string _canonicalPath(const std::string &path);
  static bool _inputLess(const InputFile &lhs, const InputFile &rhs);
};

#endif

