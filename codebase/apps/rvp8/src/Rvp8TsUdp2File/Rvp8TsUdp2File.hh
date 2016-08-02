// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:33:20 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Rvp8TsUdp2File.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// Rvp8TsUdp2File reads data from UDP in RVP8 TimeSeries format and
// writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef Rvp8TsUdp2File_H
#define Rvp8TsUdp2File_H

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

class Rvp8TsUdp2File {
  
public:

  // constructor

  Rvp8TsUdp2File(int argc, char **argv);

  // destructor
  
  ~Rvp8TsUdp2File();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:
  
  string _progName;
  Args _args;

  // aquisition mode - need new info?
  
  SINT4 _iAqModePrev;
  
  // sector information to determine if we need a new file

  bool _needNewFile;
  int _nPulsesFile; // number of pulses in current file
  double _sectorWidth; // width of sector (file)
  int _currentSector; // which sector are we in?

  // output file

  FILE *_out;
  bool _fileWritten;

  // Pulse info and header

  struct rvptsPulseInfo PulseInfo;
  struct rvptsPulseHdr PulseHeader ;

  // receiver state

  struct receiver_state
  {
    UINT4 iLastSeqNum;
    UINT4 iOutOfSeqCnt;
    UINT4 iHdrCnt;
    UINT4 iPInf1Cnt;
    UINT4 iPInf2Cnt;
    UINT4 iDataCnt;
    UINT4 iUnknownCnt;
    /* Because we need to assemble the data from many individual packets,
     * we keep a state variable showing where we are at.  Here are the
     * possible states:
     */
#define PS_WAITING       (0)  /* Waiting for an initial pulse info structure */
#define PS_PINFO_STARTED (1)  /* Got one or more PINFO packets */
#define PS_PINFO_DONE    (2)  /* Got all the PINFO packet */
#define PS_PHDR_STARTED  (3)  /* Got one or more pulse header packets */
#define PS_PHDR_DONE     (4)  /* Got all the pulse header packets */
#define PS_DATA_STARTED  (5)  /* Got one or more data packets */
#define PS_DATA_DONE     (6)  /* Got all the data packets */
    UINT4 iPulseState;
    UINT1 lNewPulseInfo;
    UINT1 lWasWaiting;
  };
  struct receiver_state State;

  // packet state

  struct packet_state
  {
    int iLastIndex;
    int iLastOffset;
  };
  struct packet_state PacketState;

  // keep track of pulse info while assembling packets
  
  struct rvptsPulseInfo lastPulseInfo;

  // Maximum size of data in a UDP packet
  static const int UDP_DATA_SIZE = 1472;
  static const int RECORD_SIZE = (2*RVPX_MAXBINS * 2 * sizeof(UINT2));

  // temporary packed IQ

  UINT1 *iTempPackedIQ;

  // debug print loop count

  UINT1 debugLoopCount;

  // functions

  int ProcessUdpPacket(const UINT1 Buffer_a[UDP_DATA_SIZE],
                       int iRecvBytes_a);
  
  void ProcessHeadPacket(const UINT1 Buffer_a[], int iRecvBytes_a);
  
  void ProcessInfoPacket(const UINT1 Buffer_a[], int iRecvBytes_a);
  
  int ProcessDataPacket(const UINT1 Buffer_a[], int iRecvBytes_a);
  
  int AssemblePacket(UINT1 iRecord_a[], int iMaxRecordSize_a,
                     const UINT1 iPacket_a[], int iPacketSize_a);

  int _checkNewFile();
  int _openNewFile();

  int _writePulseInfo();
  int _writePulseHeader();
  int _writePulseData();

  int _makedir(const char *path);
  int _makedir_recurse(const char *path);
  
};

#endif
