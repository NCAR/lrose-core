// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:29:54 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Rvp8Ts2File.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Rvp8Ts2File reads data from the RVP8 TimeSeries API and writes 
// it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef Rvp8Ts2File_H
#define Rvp8Ts2File_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"

// rvp8 includes

#include <rvp8_rap/Rvp8Legacy.hh>

extern "C" {
#include "sigtypes.h"
#include "dsp.h"
#ifdef RVP8_LEGACY_V8
#include "rvp8.h"
#else
#include "rvpts.h"
#endif
#include "user_lib.h"
#include "dsp_lib.h"
#include "rdasubs_lib.h"
}

using namespace std;

////////////////////////
// This class

class Rvp8Ts2File {
  
public:

  // constructor

  Rvp8Ts2File(int argc, char **argv);

  // destructor
  
  ~Rvp8Ts2File();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const int MaxPulses = 500;
  static const int RECORD_SIZE = (2*RVPX_MAXBINS * 2 * sizeof(UINT2));

  string _progName;
  Args _args;
  
  SINT4 _iAqModePrev; // aquisition mode - need new info?
  
  // sector information to determine if we need a new file

  bool _needNewFile;
  int _nPulsesFile; // number of pulses in current file
  double _sectorWidth; // width of sector (file)
  int _currentSector; // which sector are we in?

  // output file

  FILE *_out;
  bool _fileWritten;
  
  // pulse information

  rvptsPulseInfo _pulseInfo;

  // temporary packed IQ

  UINT2 *iTempPackedIQ;

  // functions

  void _waitAvailable(RvpTS *ts, int &seqNum);

  int _readPulses(RvpTS *ts, int &seqNum);

  int _openNewFile(const struct rvptsPulseHdr &pulseHdr);

  int _writePulseInfo();

  int _writePulseHeader(const rvptsPulseHdr &pulseHdr);

  int _writePulseData(RvpTS *ts,
                      const rvptsPulseHdr &pulseHdr);

  int _makedir(const char *path);
  int _makedir_recurse(const char *path);

};

#endif
