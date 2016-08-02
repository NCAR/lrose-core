// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:31:59 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Rvp8TsTcpServer.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Rvp8TsTcpServer serves out RVP8 time-series data under the 
// TCP/IP protocol.
//
// Rvp8TsTcpServer listens on the specified port for a connection
// from a client. When a client connects, the server starts
// sending time-series data to the client, using the message
// format from the Socket class.
//
// Only a single client may connect at a time.
//
///////////////////////////////////////////////////////////////

#ifndef Rvp8TsTcpServer_H
#define Rvp8TsTcpServer_H

#include "Args.hh"
#include <radar/iwrf_data.h>
#include <toolsa/Socket.hh>
#include <string>

// rvp8 includes

#include <rvp8_rap/Rvp8Legacy.hh>
#define RVP8_NGATES_BURST 2

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

class Rvp8TsTcpServer {
  
public:

  // constructor

  Rvp8TsTcpServer(int argc, char **argv);
  
  // destructor
  
  ~Rvp8TsTcpServer();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  static const int MaxPulses = 500;
  static const int MAX_IQ = (8 * RVPX_MAXBINS);
  static const int MAX_BUF =
  (sizeof(struct rvptsPulseHdr) + MAX_IQ * sizeof(ui16) * 8);
  
  string _progName;
  Args _args;

  // aquisition mode - need new info?

  int _iAqModePrev;

  // RVPTS ops info

  struct rvptsPulseInfo _rvptsInfo;
 
  // arrays
  
  ui08 *_pulseBuf;
  ui16 *_packedIQ;
  
  fl32 *_iqFloat;
  double *_powerDbmArray;
  double *_phaseDegArray;

  // lookup table for converting 16-bit floats to 32-bit floats

  fl32 _floatLut[65536];

  // debug print loop count

  int _debugLoopCount;

  // movement check

  bool _isMoving;
  double _moveCheckAz;
  double _moveCheckEl;
  time_t _moveCheckTime;

  // stowed check

  bool _isStowed;
  double _stowedCheckAz;
  double _stowedCheckEl;
  time_t _stowedCheckTime;
  
  // metadata - to be sent every 10 secs or so

  time_t _timeLastMeta;

  iwrf_radar_info_t _radar;
  iwrf_ts_processing_t _proc;
  iwrf_xmit_info _xmit;
  iwrf_calibration_t _calib;
  iwrf_scan_segment_t _scan;
  iwrf_rvp8_ops_info_t _rvp8Info;

  si64 _packetSeqNum;

  // pulse headers

  iwrf_pulse_header_t _pulse;
  iwrf_rvp8_pulse_header _rvp8Pulse;
  
  // private functions

  int _handleClient(Socket &sock);
  
  void _waitAvailable(RvpTS *ts, int &seqNum);
  
  int _readPulses(RvpTS *ts, int &seqNum,
                  Socket &sock);
  
  int _writeInfo(const struct rvptsPulseInfo &rvp8Info,
                 Socket &sock);

  int _writePulse(RvpTS *ts,
                  const struct rvptsPulseInfo &rvp8Info,
                  const struct rvptsPulseHdr &rvp8Hdr,
                  Socket &sock);
  
  void _convertRvp8Info2Iwrf(const struct rvptsPulseInfo &rvp8Info);
  
  void _convertRvp8PulseHdr2Iwrf(const struct rvptsPulseInfo &rvp8Info,
				 const struct rvptsPulseHdr &rvp8Hdr);
  
  void _setBurstInfo(const struct rvptsPulseInfo &rvp8Info,
		     const struct rvptsPulseHdr &rvp8Hdr,
		     const ui16 *packed,
		     iwrf_pulse_header_t &pulse);

  void _setBurstInfo(const struct rvptsPulseInfo &rvp8Info,
		     const struct rvptsPulseHdr &rvp8Hdr,
		     const fl32 *floats,
		     iwrf_pulse_header_t &pulse);

  void _computeRangeInfo(const struct rvptsPulseInfo &rvp8Info,
                         double &startRangeM,
                         double &gateSpacingM);
  
  bool _moving(const struct rvptsPulseHdr &rvp8Hdr);

  bool _stowed(const struct rvptsPulseHdr &rvp8Hdr);

  void _updateCal(struct rvptsPulseInfo &rvp8Info);

  void _computeFloatLut();

  void _vecFloatIQFromPackIQ
  (volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
   si32 iCount_a);

};

#endif
