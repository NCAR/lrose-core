#ifndef SPDB_SIM_HH
#define SPDB_SIM_HH

#include "Args.hh"
#include "Params.hh"

#include <Spdb/DsSpdb.hh>
#include <csignal>
#include <string>
#include <vector>

class SpdbSim {
public:
  SpdbSim(int argc, char **argv);
  ~SpdbSim();

  int Run();
  void requestStop() { _stopRequested = 1; }

  int OK;

private:
  struct ReplayChunk {
    Spdb::chunk_t chunk;
  };

  std::string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  volatile sig_atomic_t _stopRequested;
  bool _procmapRegistered;
  // chunk_t owns no payload memory.  Keep the reader alive for the full replay
  // because every ReplayChunk below points into its returned chunk storage.
  DsSpdb _input;
  std::vector<ReplayChunk> _chunks;
  int _productId;
  std::string _productLabel;
  time_t _archiveStart;
  time_t _archiveEnd;

  int _validateParams() const;
  int _readAndPreflight();
  int _preflightChunk(const Spdb::chunk_t &chunk) const;
  int _preflightCycle(time_t dataCycleStart) const;
  int _runCycles();
  int _runCycle(time_t dataCycleStart, double deadlineCycleStart);
  int _publishGroup(size_t begin, size_t end, time_t dataCycleStart);
  int _waitUntil(double deadline, const std::string &activity);
  time_t _mappedValid(const Spdb::chunk_t &chunk,
                      time_t dataCycleStart) const;
  Spdb::compression_t _outputCompression(
      const Spdb::chunk_t &chunk) const;

  static bool _chunkLess(const ReplayChunk &lhs, const ReplayChunk &rhs);
  static bool _fitsSpdbTime(int64_t value);
  static double _monotonicNow();
};

#endif

