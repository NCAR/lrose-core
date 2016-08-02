// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:31:59 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// Rvp8TsTcpServer.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
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

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <ctime>
#include "Rvp8TsTcpServer.hh"
#include "Args.hh"
#include <radar/iwrf_functions.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaXml.hh>
using namespace std;

// Constructor

Rvp8TsTcpServer::Rvp8TsTcpServer(int argc, char **argv)

{

  OK = true;

  _pulseBuf = NULL;
  _iqPacked = NULL;
  _iqFloat = NULL;

  _debugLoopCount = 0;
  _timeLastMeta = 0;
  _packetSeqNum = 0;
  
  // set programe name
  
  _progName = "Rvp8TsTcpServer";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // allocate array for packed data

  _pulseBuf = new ui08[MAX_BUF];
  _iqPacked = new ui16[MAX_IQ];
  _iqFloat = new fl32[MAX_IQ];

  // compute lookup table

  _computeFloatLut();

  // movement check - initialization

  _isMoving = true;
  _moveCheckTime = -1;
  _moveCheckAz = -999;
  _moveCheckEl = -999;

  // stowed check - initialization

  _isStowed = false;
  _stowedCheckTime = -1;
  _stowedCheckAz = -999;
  _stowedCheckEl = -999;

  // init process mapper registration
  
  if (_args.regWithProcmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _args.instance.c_str(),
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;

}

// destructor

Rvp8TsTcpServer::~Rvp8TsTcpServer()

{

  if (_iqPacked) {
    delete[] _iqPacked;
  }
  if (_pulseBuf) {
    delete[] _pulseBuf;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Rvp8TsTcpServer::Run()
{
  
  PMU_auto_register("Run");
  
  int iret = 0;

  // run forever
  
  while (true) {

    PMU_auto_register("Opening server");

    ServerSocket server;
    if (server.openServer(_args.port)) {
      cerr << "ERROR - Rvp8TsTcpServer" << endl;
      cerr << "  Cannot open server, port: " << _args.port << endl;
      cerr << "  " << server.getErrStr() << endl;
      umsleep(1000);
      continue;
    }

    if (_args.debug) {
      cerr << "INFO - listening on port: " << _args.port << endl;
    }

    while (true) {

      PMU_auto_register("Getting client");

      // get a client
      
      Socket *sock = NULL;
      while (sock == NULL) {
        if (_args.debug) {
          cerr << "INFO - waiting for client ..." << endl;
        }
        sock = server.getClient(10000);
	PMU_auto_register("Listening ...");
      }

      // handle the client

      if (_handleClient(*sock)) {
        umsleep(1000);
        iret = -1;
      }
        
      // close connection

      sock->close();
      delete sock;
      
    } // while
      
  } // while

  return iret;
  
}

//////////////////////////////////////////////////
// handle client
//
// Returns 0 on success, -1 on failure

int Rvp8TsTcpServer::_handleClient(Socket &sock)
{

  PMU_auto_register("Handle client");
  _debugLoopCount = 0;

  // attach to RVP8

  _iAqModePrev = -1;
  MESSAGE status;
#ifdef RVP8_LEGACY_V8
  RvpTS *ts = rvp8tsAttach_(&status,
			    RVP8TS_UNIT_MAIN,
			    RVP8TS_CLIENT_READER);
#else
  RvpTS *ts = rvptsAttach(&status,
			  RVPTS_UNIT_MAIN,
			  RVPTS_CLIENT_READER, 0);
#endif
  
  
  if (ts == NULL) {
    cerr << "ERROR - Ts2File::Run" << endl;
    cerr << "  Could not attach to RVP8 TS interface" << endl;
    sig_signal(status);
    return -1;
  }
  
  // get an initial sequence number into the pulse sequence
  
  int seqNum = rvptsCurrentSeqNum(ts);

  while (true) {
    
    PMU_auto_register("Waiting for data");

    // wait until data is available
    
    _waitAvailable(ts, seqNum);
    
    // read available pulses, up to number expected
    
    PMU_auto_register("Reading pulses");

    if (_readPulses(ts, seqNum, sock)) {
      return -1;
    }

  } // while
  

  return 0;

}

//////////////////////////////////////////////////
// wait for available ts data from the RVP8

void Rvp8TsTcpServer::_waitAvailable(RvpTS *ts, int &seqNum)
  
{
  
  while (!rvptsSeqNumOkay(ts, seqNum)) {
    
    if (rvptsSeqNumOkay(ts, seqNum-1)) {
      
      /* If the sequence number is not okay but the previous one is,
       * then we have completely caught up with the RVP8's supply of
       * (I,Q) data.  There's no particular rush to resume, so sleep
       * long enough for others to get some work done.
       */
      
      sig_microSleep(100000) ;
      
    } else {
  
      /* If our current sequence number no longer represents a valid
       * window into the timeseries data, then print a message and
       * get a fresh sequence number with which to resume.
       */
      
      if (_args.debug) {
        cerr << "Expired seqNum: " << seqNum << endl;
        cerr << "  Resetting ..." << endl;
      }
      seqNum = rvptsCurrentSeqNum(ts) ;

    }

  } // while

  /* We now have at least one valid pulse to read.  Check that it
   * would actually be worthwhile to run right now, i.e., we have
   * either a minimum number of pulses, or the data are starting
   * to get too old.  If neither of these apply, then wait a
   * little while.
   */
  
  while (true) {
    
    int iFwdPulses = rvptsNumPulsesFwd(ts, seqNum);
    unsigned iAgeMS =  rvptsRealTimeAge(ts, seqNum);
    
    if((iFwdPulses < 25) && (iAgeMS < 250)) {
      sig_microSleep(20000) ;
    } else {
      break;
    }

  } // while
    
}
    
//////////////////////////////////////////////////
// read available ts data from the RVP8
//
// Returns the number of samples read

int Rvp8TsTcpServer::_readPulses(RvpTS *ts, int &seqNum, Socket &sock)
  
{
  
  // get as many pulses as we can
  // up to the maximum imposed by our local buffer size
  
  const struct rvptsPulseHdr *pHdrs[MaxPulses];
  int nPulsesAvail = rvptsGetPulses(ts, pHdrs, seqNum, MaxPulses);
  if (nPulsesAvail == 0) {
    return 0;
  }
  
  // If the acquisition mode has changed, save the pulse info
  
  if(_iAqModePrev != pHdrs[0]->iAqMode) {
    if (_args.debug) {
      cerr << "\nNew (I,Q) acquisition mode: " << (int) pHdrs[0]->iAqMode << endl;
    }
    _iAqModePrev = pHdrs[0]->iAqMode;
    _rvptsInfo = *(rvptsGetPulseInfo(ts, pHdrs[0]));
    _updateCal(_rvptsInfo);
    // convert RVP8 info to IWRF info
    _convertRvp8Info2Iwrf(_rvptsInfo);
    // write info
    if (_writeInfo(_rvptsInfo, sock)) {
      return -1;
    }
    _timeLastMeta = time(NULL);
  }

  // loop through the pulses
  
  for (int ipulse = 0; ipulse < nPulsesAvail; ipulse++, seqNum++) {
    
    // send meta data evey 5 secs

    time_t now = time(NULL);
    if (now - _timeLastMeta > 5) {
      if (_writeInfo(_rvptsInfo, sock)) {
	return -1;
      }
      _timeLastMeta = now;
    }

    // pulse header

    struct rvptsPulseHdr rvp8Hdr = *(pHdrs[ipulse]);
    
    // check the sequence number
    
    if (seqNum != rvp8Hdr.iSeqNum) {
      if (_args.debug) {
        cerr << "ERROR in sequence number, resetting" << endl;
        cerr << "  Expecting: " << seqNum << endl;
        cerr << "  Found: " << rvp8Hdr.iSeqNum << endl;
      }
      seqNum = rvp8Hdr.iSeqNum;
    }

 
    // if required, invert the sense on the HV flag
    
    if (_args.invertHvFlag) {
      rvp8Hdr.iPolarBits = !rvp8Hdr.iPolarBits;
    }
    
    // write the pulse

    bool doWrite = true;
    if (_args.checkMove && !_moving(rvp8Hdr)) {
      if (_args.verbose) {
	cerr << "-->> Antenna is stopped, will not write pulse" << endl;
      }
      doWrite = false;
    }
    if (_args.checkStowed && _stowed(rvp8Hdr)) {
      if (_args.verbose) {
	cerr << "-->> Antenna is stowed, will not write pulse" << endl;
      }
      doWrite = false;
    }

    if (doWrite) {
      if (_writePulse(ts, _rvptsInfo, rvp8Hdr, sock)) {
        return -1;
      }
    }

    // check that this sequence number is still OK, it may have been
    // overwritten by the RVP8 if we are too slow
    
    if (!rvptsSeqNumOkay(ts, seqNum)) {
      if (_args.debug) {
        cerr << "WARNING - sequence number expired: " << seqNum << endl;
        cerr << "  Getting new sequence number" << endl;
      }
      seqNum = rvptsCurrentSeqNum(ts);
      return 0;
    }
    
  } // ipulse
  
  return 0;

}

/////////////////////////////////////
// write the pulse info
// returns 0 on success, -1 on error

int Rvp8TsTcpServer::_writeInfo(const struct rvptsPulseInfo &rvp8Info,
                                Socket &sock)

{

  PMU_auto_register("Writing message");

  // convert RVP8 info to IWRF info
  
  _convertRvp8Info2Iwrf(rvp8Info);

  // write individual messages for each struct

  if (sock.writeBuffer(&_radar, sizeof(_radar))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_RADAR_INFO" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (sock.writeBuffer(&_proc, sizeof(_proc))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_TS_PROCESSING" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (sock.writeBuffer(&_xmitInfo, sizeof(_xmitInfo))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_XMIT_INFO" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (sock.writeBuffer(&_calib, sizeof(_calib))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_CALIBRATION" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (sock.writeBuffer(&_scan, sizeof(_scan))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_SCAN_SEGMENT" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (sock.writeBuffer(&_rvp8Info, sizeof(_rvp8Info))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_RVP8_OPS_INFO" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (sock.writeBuffer(_statusXmlBuf.getPtr(), _statusXmlBuf.getLen())) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfInfo" << endl;
      cerr << "  Writing IWRF_STATUS_XML" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (_args.verbose) {
    cerr << "Successfully wrote iwrf info to client" << endl;
  }

  return 0;

}

//////////////////////////
// write the pulse header
// returns 0 on success, -1 on error

int Rvp8TsTcpServer::_writePulse(RvpTS *ts,
                                 const struct rvptsPulseInfo &rvp8Info,
                                 const struct rvptsPulseHdr &rvp8Hdr,
                                 Socket &sock)
  
{

  // convert RVP8 struct into IWRF structs

  _convertRvp8PulseHdr2Iwrf(rvp8Info, rvp8Hdr);
  
  // start of data is beyond the pulse header

  int dataLen = sizeof(_pulse);

  // loop through channels, copy in data
  
  int nChannels = rvp8Hdr.iVIQPerBin;
  int nBins = rvp8Hdr.iNumVecs;
  int nChanBins = nBins * nChannels;
  int nIQ = nChanBins * 2;
  double satMult = pow(10.0, rvp8Info.fSaturationDBM / 20.0);

  // get floating point IQ data from RVP8
  
  for (int ichan = 0; ichan < nChannels; ichan++) {
    const fl32 *rvp8Iq = rvptsIQDataFLT4(ts, &rvp8Hdr, ichan);
    memcpy(_iqFloat + ichan * nBins * 2,  rvp8Iq, nBins * 2 * sizeof(fl32));
  }
    
  if (_args.outputPacking == Args::IWRF_FL32) {
      
    // first adjust floats for saturation value
    
    for (int ii = 0; ii < nIQ; ii++) {
      _iqFloat[ii] *= satMult;
    }

    // copy float data into buffer

    int nBytes = nIQ * sizeof(fl32);
    memcpy(_pulseBuf + dataLen, _iqFloat, nBytes);
    dataLen += nBytes;

    // set scale and offset in pulse header
    
    _pulse.iq_encoding = IWRF_IQ_ENCODING_FL32;    
    _pulse.scale = 1.0;
    _pulse.offset = 0.0;
    
  } else if (_args.outputPacking == Args::IWRF_SI16) {
    
    // first adjust floats for saturation value
    
    for (int ii = 0; ii < nIQ; ii++) {
      _iqFloat[ii] *= satMult;
    }

    // compute max absolute val
    
    double maxAbsVal = -1.0e99;
    for (int ii = 0; ii < nIQ; ii++) {
      double absVal = fabs(_iqFloat[ii]);
      if (absVal > maxAbsVal) maxAbsVal = absVal;
    }

    // compute scale and offset

    double scale = maxAbsVal / 32768.0;
    double offset = 0;
    
    // load scaled data
    
    for (int ii = 0; ii < nIQ; ii++) {
      int scaledVal =
        (int) floor(_iqFloat[ii] / scale + 0.5);
      if (scaledVal < -32767) {
        scaledVal = -32767;
      } else if (scaledVal > 32767) {
        scaledVal = 32767;
      }
      _iqPacked[ii] = (si16) scaledVal;
    }
    
    // copy packed data into buffer
    
    int nBytes = nIQ * sizeof(si16);
    memcpy(_pulseBuf + dataLen, _iqPacked, nBytes);
    dataLen += nBytes;

    // set scale and offset in pulse header
    
    _pulse.scale = scale;
    _pulse.offset = offset;
    _pulse.iq_encoding = IWRF_IQ_ENCODING_SCALED_SI16;    

  } else {

    // SIGMET packing
    
    vecPackIQFromFloatIQ_(_iqPacked, _iqFloat,
                          nIQ,  PACKIQ_HIGHSNR);
    
    // copy packed data into buffer
    
    int nBytes = nIQ * sizeof(si16);
    memcpy(_pulseBuf + dataLen, _iqPacked, nBytes);
    dataLen += nBytes;

    _pulse.scale = 1.0;
    _pulse.offset = 0.0;
    _pulse.iq_encoding = IWRF_IQ_ENCODING_SIGMET_FL16;    

    // now adjust floats for saturation value so that burst values
    // will be correct
    
    for (int ii = 0; ii < nIQ; ii++) {
      _iqFloat[ii] *= satMult;
    }

  } // if (_args.outputPacking == Args::IWRF_FL32)

  // set packet length
  
  _pulse.packet.len_bytes = dataLen;

  // set burst info
  
  _setBurstInfo(rvp8Info,  rvp8Hdr, _iqFloat, _pulse);

  // copy pulse header into buffer

  memcpy(_pulseBuf, &_pulse, sizeof(_pulse));

  // write pulse to client

  if (sock.writeBuffer(_pulseBuf, dataLen)) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfPulse" << endl;
      cerr << "  Writing IWRF pulse" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }

  // write RVP8 pulse header to client

  if (sock.writeBuffer(&_rvp8Pulse, sizeof(_rvp8Pulse))) {
    if (_args.debug) {
      cerr << "ERROR - Rvp8TsTcpServer::_writeIwrfPulse" << endl;
      cerr << "  Writing rvp8 pulse header" << endl;
      cerr << "  " << sock.getErrStr() << endl;
    }
    return -1;
  }
  
  if (_args.verbose) {
    _debugLoopCount++;
    if (_debugLoopCount % 1000 == 0) {
      cerr << "Wrote " << _debugLoopCount << " pulses to client" << endl;
    }
  }

  return 0;

}

///////////////////////////////
// convert RVP8 info to IWRF

void
Rvp8TsTcpServer::_convertRvp8Info2Iwrf(const struct rvptsPulseInfo &rvp8Info)
  
{

  // radar info

  iwrf_radar_info_init(_radar);
  _radar.packet.seq_num = _packetSeqNum++;
  _radar.wavelength_cm = rvp8Info.fWavelengthCM;
  STRncopy(_radar.site_name, rvp8Info.sSiteName, IWRF_MAX_SITE_NAME);

  // processing info

  iwrf_ts_processing_init(_proc);
  _proc.packet.seq_num = _packetSeqNum++;
  if (rvp8Info.iPhaseModSeq == PHSEQ_FIXED) {
    _proc.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_FIXED;
  } else if (rvp8Info.iPhaseModSeq == PHSEQ_RANDOM) {
    _proc.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_RANDOM;
  } else if (rvp8Info.iPhaseModSeq == PHSEQ_SZ8_64) {
    _proc.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_SZ864;
  }

  _proc.pulse_width_us = rvp8Info.fPWidthUSec;
  double startRangeM, gateSpacingM;
  _computeRangeInfo(rvp8Info, startRangeM, gateSpacingM);
  _proc.start_range_m = startRangeM;
  _proc.gate_spacing_m = gateSpacingM;
  _proc.integration_cycle_pulses = rvp8Info.iSampleSize;
  if (rvp8Info.iPolarization == POL_HORIZ_FIX) {
    _proc.pol_mode = IWRF_POL_MODE_H;
  } else if (rvp8Info.iPolarization == POL_VERT_FIX) {
    _proc.pol_mode = IWRF_POL_MODE_V;
  } else if (rvp8Info.iPolarization == POL_ALTERNATING) {
    _proc.pol_mode = IWRF_POL_MODE_HV_ALT;
  } else if (rvp8Info.iPolarization == POL_SIMULTANEOUS) {
    _proc.pol_mode = IWRF_POL_MODE_HV_SIM;
  }

  // xmit info

  iwrf_xmit_info_init(_xmitInfo);
  _xmitInfo.packet.seq_num = _packetSeqNum++;
  if (rvp8Info.iPhaseModSeq == PHSEQ_FIXED) {
    _xmitInfo.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_FIXED;
  } else if (rvp8Info.iPhaseModSeq == PHSEQ_RANDOM) {
    _xmitInfo.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_RANDOM;
  } else if (rvp8Info.iPhaseModSeq == PHSEQ_SZ8_64) {
    _xmitInfo.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_SZ864;
  }

  if (rvp8Info.iPolarization == POL_HORIZ_FIX) {
    _xmitInfo.pol_mode = IWRF_POL_MODE_H;
  } else if (rvp8Info.iPolarization == POL_VERT_FIX) {
    _xmitInfo.pol_mode = IWRF_POL_MODE_V;
  } else if (rvp8Info.iPolarization == POL_ALTERNATING) {
    _xmitInfo.pol_mode = IWRF_POL_MODE_HV_ALT;
  } else if (rvp8Info.iPolarization == POL_SIMULTANEOUS) {
    _xmitInfo.pol_mode = IWRF_POL_MODE_HV_SIM;
  }

  // calibration

  iwrf_calibration_init(_calib);
  _calib.packet.seq_num = _packetSeqNum++;
  _calib.wavelength_cm = rvp8Info.fWavelengthCM;
  _calib.noise_dbm_hc = rvp8Info.fNoiseDBm[0];
  _calib.noise_dbm_hx = rvp8Info.fNoiseDBm[0];
  _calib.noise_dbm_vc = rvp8Info.fNoiseDBm[1];
  _calib.noise_dbm_vx = rvp8Info.fNoiseDBm[1];
  _calib.base_dbz_1km_hc = rvp8Info.fDBzCalib;
  _calib.base_dbz_1km_hx = rvp8Info.fDBzCalib;
  _calib.base_dbz_1km_vc = rvp8Info.fDBzCalib;
  _calib.base_dbz_1km_vx = rvp8Info.fDBzCalib;

  // scan info
  
  iwrf_scan_segment_init(_scan);
  _scan.packet.seq_num = _packetSeqNum++;
  if (rvp8Info.taskID.iScanType == TASK_SCAN_PPI) {
    _scan.scan_mode = IWRF_SCAN_MODE_SECTOR;
  } else if (rvp8Info.taskID.iScanType == TASK_SCAN_RHI) {
    _scan.scan_mode = IWRF_SCAN_MODE_RHI;
  } else if (rvp8Info.taskID.iScanType == TASK_SCAN_PPIFULL) {
    _scan.scan_mode = IWRF_SCAN_MODE_AZ_SUR_360;
  }
  _scan.volume_num = rvp8Info.iAqMode;

  // RVP8-specific
  
  iwrf_rvp8_ops_info_init(_rvp8Info);
  _rvp8Info.packet.seq_num = _packetSeqNum++;
  _rvp8Info.i_version = rvp8Info.iVersion;
  _rvp8Info.i_major_mode = rvp8Info.iMajorMode;
  _rvp8Info.i_polarization = rvp8Info.iPolarization;
  _rvp8Info.i_phase_mode_seq = rvp8Info.iPhaseModSeq;
  _rvp8Info.i_task_sweep = rvp8Info.taskID.iSweep;
  _rvp8Info.i_task_aux_num = rvp8Info.taskID.iAuxNum;
  _rvp8Info.i_task_scan_type = rvp8Info.taskID.iScanType;
  memcpy(_rvp8Info.s_task_name, rvp8Info.taskID.sTaskName,
         sizeof(_rvp8Info.s_task_name));
  memcpy(_rvp8Info.s_site_name, rvp8Info.sSiteName,
         sizeof(_rvp8Info.s_site_name));
  _rvp8Info.i_aq_mode = rvp8Info.iAqMode;
  _rvp8Info.i_unfold_mode = rvp8Info.iUnfoldMode;
  _rvp8Info.i_pwidth_code = rvp8Info.iPWidthCode;
  _rvp8Info.f_pwidth_usec = rvp8Info.fPWidthUSec;
  _rvp8Info.f_dbz_calib = rvp8Info.fDBzCalib;
  _rvp8Info.i_sample_size = rvp8Info.iSampleSize;
  _rvp8Info.i_mean_angle_sync = rvp8Info.iMeanAngleSync;
  _rvp8Info.i_flags = rvp8Info.iFlags;
  _rvp8Info.i_playback_version = rvp8Info.iPlaybackVersion;
  _rvp8Info.f_sy_clk_mhz = rvp8Info.fSyClkMHz;
  _rvp8Info.f_wavelength_cm = rvp8Info.fWavelengthCM;
  _rvp8Info.f_saturation_dbm = rvp8Info.fSaturationDBM;
  _rvp8Info.f_range_mask_res = rvp8Info.fRangeMaskRes;
  for (int ii = 0; ii < 512; ii++) {
    _rvp8Info.i_range_mask[ii] = rvp8Info.iRangeMask[ii];
  }
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    _rvp8Info.f_noise_dbm[ii] = rvp8Info.fNoiseDBm[ii];
    _rvp8Info.f_noise_stdv_db[ii] = rvp8Info.fNoiseStdvDB[ii];
  }
  _rvp8Info.f_noise_range_km = rvp8Info.fNoiseRangeKM;
  _rvp8Info.f_noise_prf_hz = rvp8Info.fNoisePRFHz;
  for (int ii = 0; ii < 2; ii++) {
    _rvp8Info.i_gparm_latch_sts[ii] = rvp8Info.iGparmLatchSts[ii];
  }
  for (int ii = 0; ii < 6; ii++) {
    _rvp8Info.i_gparm_immed_sts[ii] = rvp8Info.iGparmImmedSts[ii];
  }
  for (int ii = 0; ii < 4; ii++) {
    _rvp8Info.i_gparm_diag_bits[ii] = rvp8Info.iGparmDiagBits[ii];
  }
  memcpy(_rvp8Info.s_version_string, rvp8Info.sVersionString,
         sizeof(_rvp8Info.s_version_string));

  // status XML

  iwrf_status_xml_init(_statusXmlHdr);

  _statusXmlHdr.packet.seq_num = _packetSeqNum++;
  _statusXmlStr.clear();
  time_t now = time(NULL);

  char version[16];
  memset(version, 16, 0);
  memcpy(version, _rvp8Info.s_version_string, 12);

  _statusXmlStr += TaXml::writeStartTag("Rvp8Status", 0);
  _statusXmlStr += TaXml::writeTime("Time", 1, now);
  _statusXmlStr += TaXml::writeDouble("PulseWidthUs", 1, _rvp8Info.f_pwidth_usec);
  _statusXmlStr += TaXml::writeDouble("DbzCalib", 1, _rvp8Info.f_dbz_calib);
  _statusXmlStr += TaXml::writeDouble("SyClkMhz", 1, _rvp8Info.f_sy_clk_mhz);
  _statusXmlStr += TaXml::writeDouble("SaturationDbm", 1, _rvp8Info.f_saturation_dbm);
  _statusXmlStr += TaXml::writeDouble("RangeMaskRes", 1, _rvp8Info.f_range_mask_res);
  _statusXmlStr += TaXml::writeDouble("NoiseDbmChan0", 1, _rvp8Info.f_noise_dbm[0]);
  _statusXmlStr += TaXml::writeDouble("NoiseDbmChan1", 1, _rvp8Info.f_noise_dbm[1]);
  _statusXmlStr += TaXml::writeString("Version", 1, version);
  _statusXmlStr += TaXml::writeEndTag("Rvp8Status", 0);

  int xmlLen = _statusXmlStr.size() + 1;
  _statusXmlHdr.packet.len_bytes = sizeof(iwrf_status_xml_t) + xmlLen;
  _statusXmlHdr.xml_len = xmlLen;

  _statusXmlBuf.clear();
  _statusXmlBuf.add(&_statusXmlHdr, sizeof(iwrf_status_xml_t));
  _statusXmlBuf.add(_statusXmlStr.c_str(), xmlLen);

}
  
/////////////////////////////////////////
// convert RVP8 pulse hdr to IWRF pulse

void Rvp8TsTcpServer::_convertRvp8PulseHdr2Iwrf
  (const struct rvptsPulseInfo &rvp8Info,
   const struct rvptsPulseHdr &rvp8Hdr)
  
{

  iwrf_pulse_header_init(_pulse);
  iwrf_rvp8_pulse_header_init(_rvp8Pulse);

  _pulse.packet.seq_num = _packetSeqNum++;
  _pulse.packet.time_secs_utc = rvp8Hdr.iTimeUTC; 
  _pulse.packet.time_nano_secs = (ui32) rvp8Hdr.iMSecUTC * 1000000;

  _pulse.pulse_seq_num = rvp8Hdr.iSeqNum;
  _pulse.volume_num = rvp8Hdr.iAqMode;

  // scan info

  if (rvp8Info.taskID.iScanType == TASK_SCAN_PPI) {
    _pulse.scan_mode = IWRF_SCAN_MODE_SECTOR;
  } else if (rvp8Info.taskID.iScanType == TASK_SCAN_RHI) {
    _pulse.scan_mode = IWRF_SCAN_MODE_RHI;
  } else if (rvp8Info.taskID.iScanType == TASK_SCAN_PPIFULL) {
    _pulse.scan_mode = IWRF_SCAN_MODE_AZ_SUR_360;
  }
  
  _pulse.elevation = (rvp8Hdr.iEl / 65535.0) * 360.0;
  _pulse.azimuth = (rvp8Hdr.iAz / 65535.0) * 360.0;

  if (rvp8Hdr.iPrevPRT == 0) {
    _pulse.prt = 0.001;
  } else {
    _pulse.prt = ((double) rvp8Hdr.iPrevPRT / rvp8Info.fSyClkMHz) / 1.0e6;
  }

  if (rvp8Hdr.iNextPRT == 0) {
    _pulse.prt_next = 0.001;
  } else {
    _pulse.prt_next = ((double) rvp8Hdr.iNextPRT / rvp8Info.fSyClkMHz) / 1.0e6;
  }

  _pulse.pulse_width_us = rvp8Info.fPWidthUSec;

  _pulse.n_gates = rvp8Hdr.iNumVecs - RVP8_NGATES_BURST;

  _pulse.n_channels = rvp8Hdr.iVIQPerBin;   
  _pulse.n_data = rvp8Hdr.iNumVecs *  rvp8Hdr.iVIQPerBin * 2;

  _pulse.hv_flag = rvp8Hdr.iPolarBits;      

  _pulse.antenna_transition = 0;
  _pulse.phase_cohered = 1;
  
  double mult = pow(10.0, rvp8Info.fSaturationDBM / 10.0);

  _pulse.status = 0;
  _pulse.n_gates_burst = RVP8_NGATES_BURST;

  int startIndex0 = RVP8_NGATES_BURST * 2;
  _pulse.iq_offset[0] = startIndex0;
  if (_pulse.n_channels > 1) {
    int startIndex1 = startIndex0 + rvp8Hdr.iNumVecs * 2;
    _pulse.iq_offset[1] = startIndex1;
  }
  
  // RVP8-specific support

  _rvp8Pulse.packet.seq_num = _packetSeqNum++;
  _rvp8Pulse.i_flags = rvp8Hdr.iFlags;
  _rvp8Pulse.i_aq_mode = rvp8Hdr.iAqMode;
  _rvp8Pulse.i_polar_bits = rvp8Hdr.iPolarBits;
  _rvp8Pulse.i_viq_per_bin = rvp8Hdr.iVIQPerBin;
  _rvp8Pulse.i_tg_bank = rvp8Hdr.iTgBank;
  _rvp8Pulse.i_tx_phase = rvp8Hdr.iTxPhase;
  _rvp8Pulse.i_az = rvp8Hdr.iAz;
  _rvp8Pulse.i_el = rvp8Hdr.iEl;
  _rvp8Pulse.i_num_vecs = rvp8Hdr.iNumVecs;
  _rvp8Pulse.i_max_vecs = rvp8Hdr.iMaxVecs;
  _rvp8Pulse.i_tg_wave = rvp8Hdr.iTgWave;
  _rvp8Pulse.i_btime_api = rvp8Hdr.iBtimeAPI;
  _rvp8Pulse.i_sys_time = rvp8Hdr.iSysTime;
  _rvp8Pulse.i_prev_prt = rvp8Hdr.iPrevPRT;
  _rvp8Pulse.i_next_prt = rvp8Hdr.iNextPRT;

  _rvp8Pulse.uiq_perm[0] = rvp8Hdr.uiqPerm.iLong[0];
  _rvp8Pulse.uiq_perm[1] = rvp8Hdr.uiqPerm.iLong[1];
  _rvp8Pulse.uiq_once[0] = rvp8Hdr.uiqOnce.iLong[0];
  _rvp8Pulse.uiq_once[1] = rvp8Hdr.uiqOnce.iLong[1];

  _rvp8Pulse.i_data_off[0] = rvp8Hdr.Rx[0].iDataOff;
  _rvp8Pulse.i_data_off[1] = rvp8Hdr.Rx[0].iDataOff;

  _rvp8Pulse.f_burst_mag[0] = rvp8Hdr.Rx[0].fBurstMag;
  _rvp8Pulse.f_burst_mag[1] = rvp8Hdr.Rx[0].fBurstMag;

  _rvp8Pulse.i_burst_arg[0] = rvp8Hdr.Rx[0].iBurstArg;
  _rvp8Pulse.i_burst_arg[1] = rvp8Hdr.Rx[0].iBurstArg;

  _rvp8Pulse.i_wrap_iq[0] = rvp8Hdr.Rx[1].iWrapIQ;
  _rvp8Pulse.i_wrap_iq[1] = rvp8Hdr.Rx[1].iWrapIQ;

}

////////////////////////////////////
// set burst info in IWRF pulse header
// burst pulse is in gates 0 and 1
// so the IQ data starts at gate 2, which is index 4
  

void Rvp8TsTcpServer::_setBurstInfo(const struct rvptsPulseInfo &rvp8Info,
                                    const struct rvptsPulseHdr &rvp8Hdr,
                                    const fl32 *floats,
                                    iwrf_pulse_header_t &pulse)
  
{
  
  int startIndex0 = pulse.iq_offset[0];
  pulse.burst_arg_diff[0] = (rvp8Hdr.Rx[0].iBurstArg / 65536.0) * 360.0;
  fl32 iBurst0 = floats[startIndex0];
  fl32 qBurst0 = floats[startIndex0+1];
  pulse.burst_mag[0] = sqrt(iBurst0 * iBurst0 + qBurst0 * qBurst0);
  pulse.burst_arg[0] = atan2(qBurst0, iBurst0) * RAD_TO_DEG;
  
  if (pulse.n_channels > 1) {
    pulse.burst_arg_diff[1] = (rvp8Hdr.Rx[1].iBurstArg / 65536.0) * 360.0;
    int startIndex1 = pulse.iq_offset[1];
    fl32 iBurst1 = floats[startIndex1];
    fl32 qBurst1 = floats[startIndex1+1];
    pulse.burst_mag[1] = sqrt(iBurst1 * iBurst1 + qBurst1 * qBurst1);
    pulse.burst_arg[1] = atan2(qBurst1, iBurst1) * RAD_TO_DEG;
  }

}

/////////////////////////////////
// compute range information

void Rvp8TsTcpServer::_computeRangeInfo(const struct rvptsPulseInfo &rvp8Info,
                                        double &startRangeM,
                                        double &gateSpacingM)

{

  // Find first, last, and total number of range bins in the mask
  // Based on SIGMET code in tsview.c

  int binCount = 0;
  int binStart = 0;
  int binEnd = 0;
  
  for (int ii = 0; ii < 512; ii++) {
    ui16 mask = rvp8Info.iRangeMask[ii];
    if (mask) {
      for (int iBit = 0; iBit < 16; iBit++) {
        if (1 & (mask >> iBit)) {
          int iBin = iBit + (16*ii);
          if (binCount == 0) {
            binStart = iBin;
          }
          binEnd = iBin;
          binCount++;
        }
      }
    }
  }
  
  // range computations
  
  startRangeM = binStart * rvp8Info.fRangeMaskRes;
  double maxRangeM = binEnd * rvp8Info.fRangeMaskRes;
  gateSpacingM = (maxRangeM - startRangeM) / (binCount - 1.0);

  // pulse centered on PRT boundary, and first gate holds burst

  startRangeM += gateSpacingM / 2.0; 

}

//////////////////////////
// is the antenna moving?

bool Rvp8TsTcpServer::_moving(const struct rvptsPulseHdr &rvp8Hdr)
  
{

  time_t now = time(NULL);
  if (now == _moveCheckTime) {
    return _isMoving;
  }
  
  double az = (rvp8Hdr.iAz / 65535.0) * 360.0;
  double el = (rvp8Hdr.iEl / 65535.0) * 360.0;

  if (fabs(az - _moveCheckAz) > 0.1 ||
      fabs(el - _moveCheckEl) > 0.1) {
    _isMoving = true;
  } else {
    _isMoving = false;
  }
  
  _moveCheckAz = az;
  _moveCheckEl = el;
  _moveCheckTime = now;
  
  return _isMoving;

}

//////////////////////////
// is the antenna stowed?

bool Rvp8TsTcpServer::_stowed(const struct rvptsPulseHdr &rvp8Hdr)
  
{

  time_t now = time(NULL);
  if (now == _stowedCheckTime) {
    return _isStowed;
  }
  
  double az = (rvp8Hdr.iAz / 65535.0) * 360.0;
  double el = (rvp8Hdr.iEl / 65535.0) * 360.0;

  _isStowed = true;

  // check for movement

  if (fabs(az - _stowedCheckAz) > 0.1 ||
      fabs(el - _stowedCheckEl) > 0.1) {
    _isStowed = false;
  }

  // check for position

  if (fabs(el) < 0.001 || fabs(el) < 0.001) {
    // power off
    _isStowed = true;
  } else if (fabs(el - 90.0) > 1) {
    _isStowed = false;
  }
  
  _stowedCheckAz = az;
  _stowedCheckEl = el;
  _stowedCheckTime = now;
  
  return _isStowed;

}

void Rvp8TsTcpServer::_updateCal(struct rvptsPulseInfo &rvp8Info)
  
{

  // open cal file

  FILE *cal;
  char *calFilePath = "/usr/sigmet/config/zcalib.conf";
  if ((cal = fopen(calFilePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Rvp8TsTcpServer::_updateCal" << endl;
    cerr << "  Cannot open file: " << calFilePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    cerr << "  Cal will not be used to update time series" << endl;
    return;
  }

  if (_args.debug) {
    cerr << "Reading cal file: " << calFilePath << endl;
  }

  // get pulse width index

  int pulseWidthIndex = rvp8Info.iPWidthCode;

  // compute search strings

  char horizFzCalStr[128];
  char horizFNoiseLevelStr[128];
  char vertFzCalStr[128];
  char vertFNoiseLevelStr[128];

  sprintf(horizFzCalStr, "Stored.Horiz.PW[%d].fZCal",
          pulseWidthIndex);
  sprintf(horizFNoiseLevelStr, "Stored.Horiz.PW[%d].fNoiseLevel",
          pulseWidthIndex);
  sprintf(vertFzCalStr, "Stored.Vert.PW[%d].fZCal",
          pulseWidthIndex);
  sprintf(vertFNoiseLevelStr, "Stored.Vert.PW[%d].fNoiseLevel",
          pulseWidthIndex);
  
  // read through file

  while (!feof(cal)) {

    char line[1024];
    if (fgets(line, 1024, cal) == NULL) {
      break;
    }
    
    // horizontal fzCal

    if (strstr(line, horizFzCalStr)) {
      char *equals = strstr(line, "=");
      if (equals != NULL) {
        char *start = equals + 1;
        double fzCal;
        if (sscanf(start, "%lg", &fzCal) == 1) {
          rvp8Info.fNoiseStdvDB[0] = fzCal;
        } else {
          cerr << "ERROR - Rvp8TsTcpServer::_updateCal" << endl;
          cerr << "  Cannot read: " << horizFzCalStr << endl;
          cerr << "  Pulse width index: " << pulseWidthIndex << endl;
        }
      }
    }

    // horizontal noiseLevel

    if (strstr(line, horizFNoiseLevelStr)) {
      char *equals = strstr(line, "=");
      if (equals != NULL) {
        char *start = equals + 1;
        double fNoiseLevel;
        if (sscanf(start, "%lg", &fNoiseLevel) == 1) {
          rvp8Info.fNoiseDBm[0] = fNoiseLevel;
        } else {
          cerr << "ERROR - Rvp8TsTcpServer::_updateCal" << endl;
          cerr << "  Cannot read: " << horizFNoiseLevelStr << endl;
          cerr << "  Pulse width index: " << pulseWidthIndex << endl;
        }
      }
    }

    // vertical fzCal

    if (strstr(line, vertFzCalStr)) {
      char *equals = strstr(line, "=");
      if (equals != NULL) {
        char *start = equals + 1;
        double fzCal;
        if (sscanf(start, "%lg", &fzCal) == 1) {
          rvp8Info.fNoiseStdvDB[1] = fzCal;
        } else {
          cerr << "ERROR - Rvp8TsTcpServer::_updateCal" << endl;
          cerr << "  Cannot read: " << vertFzCalStr << endl;
          cerr << "  Pulse width index: " << pulseWidthIndex << endl;
        }
      }
    }

    // vertical noiseLevel

    if (strstr(line, vertFNoiseLevelStr)) {
      char *equals = strstr(line, "=");
      if (equals != NULL) {
        char *start = equals + 1;
        double fNoiseLevel;
        if (sscanf(start, "%lg", &fNoiseLevel) == 1) {
          rvp8Info.fNoiseDBm[1] = fNoiseLevel;
        } else {
          cerr << "ERROR - Rvp8TsTcpServer::_updateCal" << endl;
          cerr << "  Cannot read: " << vertFNoiseLevelStr << endl;
          cerr << "  Pulse width index: " << pulseWidthIndex << endl;
        }
      }
    }

  } // while (!feof ...

  fclose(cal);

  if (_args.debug) {
    cerr << "Pulse width index: " << pulseWidthIndex << endl;
    cerr << "  Horiz fzCal: " << rvp8Info.fNoiseStdvDB[0] << endl;
    cerr << "  Horiz noise: " << rvp8Info.fNoiseDBm[0] << endl;
    cerr << "  Vert fzCal: " << rvp8Info.fNoiseStdvDB[1] << endl;
    cerr << "  Vert noise: " << rvp8Info.fNoiseDBm[1] << endl;
  }

}


// compute lookup table for converting packed shorts to floats

void Rvp8TsTcpServer::_computeFloatLut()

{

  ui16 *packed = new ui16[65536];
  for (int ii = 0; ii < 65536; ii++) {
    packed[ii] = ii;
  }
  _vecFloatIQFromPackIQ(_floatLut, packed, 65536);
  delete[] packed;

}

/* ======================================================================
 * Convert a normalized floating "I" or "Q" value from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 *
 * From Sigmet lib.
 */

void Rvp8TsTcpServer::_vecFloatIQFromPackIQ
( volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
  si32 iCount_a)
{

  si32 iCount ; volatile const ui16 *iCodes = iCodes_a ;
  volatile fl32 *fIQVals = fIQVals_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode = *iCodes++ ; fl32 fVal = 0.0 ;
    
    if( iCode & 0xF000 ) {
      si32 iMan =  iCode        & 0x7FF ;
      si32 iExp = (iCode >> 12) & 0x00F ;
      
      if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
      else                 iMan |= 0x00000800 ;
      
      fVal = ((fl32)iMan) * ((fl32)(ui32)(1 << iExp)) / 3.355443E7 ;
    }
    else {
      fVal = ( (fl32)(((si32)iCode) << 20) ) / 1.759218E13 ;
    }
    *fIQVals++ = fVal ;
  }
}

