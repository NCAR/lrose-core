// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// ChillSdb2Dsr.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// ChillSdb2Dsr reads data from a legacy CHILL server in TCP,
// and writes it out to an FMQ.
//
// Also optionally computes calibration from SDB packets, and
// writes calibration XML files to a specified directory.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctime>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/chill_to_iwrf.hh>
#include "ChillSdb2Dsr.hh"

using namespace std;

const double ChillSdb2Dsr::piCubed = pow(M_PI, 3.0);
const double ChillSdb2Dsr::lightSpeed = 299792458.0;
const double ChillSdb2Dsr::kSquared = 0.93;

// Constructor

ChillSdb2Dsr::ChillSdb2Dsr(int argc, char **argv)
  
{

  // initialize

  isOK = true;
  _rQueue = NULL;
  _reconnectTime = 0;
  
  MEM_zero(_radarInfo);
  MEM_zero(_scanSeg);
  MEM_zero(_procInfo);
  MEM_zero(_powerUpdate);
  MEM_zero(_eventNotice);
  MEM_zero(_calTerms);
  MEM_zero(_xmitInfo);
  MEM_zero(_antCorr);
  MEM_zero(_xmitSample);
  MEM_zero(_phaseCode);
  MEM_zero(_version);
  MEM_zero(_track);
  MEM_zero(_rayHdr);

  _radarInfoAvail = false;
  _scanSegAvail = false;
  _procInfoAvail = false;
  _powerUpdateAvail = false;
  _eventNoticeAvail = false;
  _calTermsAvail = false;
  _xmitInfoAvail = false;
  _antCorrAvail = false;
  _xmitSampleAvail = false;
  _phaseCodeAvail = false;
  _versionAvail = false;
  _trackAvail = false;
  _paramsPending = false;
  _calibPending = false;
  _calibTime = 0;
  _prevCalibXmlWriteTime = 0;

  _nBeamsSinceParams = 0;
  _nBeamsWritten = 0;
  _volNum = 0;
  _tiltNum = -99;

  _prevAz = -9999;
  _prevEl = -9999;
  _az = 0;
  _el = 0;
  
  // set programe name
 
  _progName = "ChillSdb2Dsr";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }
  
  // initialize the output FMQ

  if (_openFmq()) {
    isOK = false;
    return;
  }

  // initialize message
  
  _msg.clearAll();
  _msg.setType(0);

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

//////////////////////////////////////////////////
// destructor

ChillSdb2Dsr::~ChillSdb2Dsr()

{

  if (_rQueue) {
    delete _rQueue;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ChillSdb2Dsr::Run ()
{

  PMU_auto_register("Run");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running ChillSdb2Dsr - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running ChillSdb2Dsr - debug mode" << endl;
  }
  if (_params.debug) {
    cerr << "  FMQ: " << _params.output_fmq_url << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
  }

  int iret = 0;
  int errCount = 0;
  while (true) {
    
    PMU_auto_register("Opening socket");
    
    // open socket to server
    
    if (_sock.open(_params.sdb_server_host,
                   _params.sdb_server_port,
                   10000)) {

      if (_sock.getErrNum() == Socket::TIMED_OUT) {
	if (_params.debug) {
	  cerr << "  Waiting for server to come up ..." << endl;
	}
      } else {
	if (_params.debug) {
	  cerr << "ERROR - ChillSdb2Dsr::Run" << endl;
	  cerr << "  Connecting to server" << endl;
	  cerr << "  " << _sock.getErrStr() << endl;
	}
        iret = -1;
        _sock.close();
      }
      errCount++;
      _sleepBeforeReconnect(errCount);
      continue;

    } else {

      // read from the server
      
      if (_readFromServer()) {
        _sock.close();
        errCount++;
        iret = -1;
      }

    }
      
    _sleepBeforeReconnect(errCount);

  } // while(true)

  return iret;
  
}

/////////////////////////////
// sleep before reconnecting

void ChillSdb2Dsr::_sleepBeforeReconnect(int errCount)
  
{

  time_t now = time(NULL);
  int secsSinceReconnect = now - _reconnectTime;

  if (secsSinceReconnect > 10) {
    errCount = 1;
  } else if (errCount > 10) {
    errCount = 10;
  }

  int sleepMsecs = errCount * 1000;
  umsleep(sleepMsecs);

  _reconnectTime = time(NULL);

}

/////////////////////////////
// read data from the server

int ChillSdb2Dsr::_readFromServer()
  
{

  if (_params.debug) {
    cerr << "Reading from SDB server" << endl;
    cerr << "  Host: " << _params.sdb_server_host << endl;
    cerr << "  Port: " << _params.sdb_server_port << endl;
  }
      
  while (true) {
    
    // read id and length of payload
    
    int idLen[2];
    if (_peekAtBuffer(idLen, sizeof(idLen))) {
      // no data on server, reconnect
      cerr << "ERROR - ChillSdb2Dsr::_readFromServer" << endl;
      cerr << "  No data from server, reconnecting ... " << endl;
      return -1;
    }

    int id = idLen[0];
    int nbytes = idLen[1];
    
    if (_params.debug) {
      cerr << "id, nbytes: " << hex << id << dec
           << ":" << _hskId2String(id) << ", " << nbytes << endl;
    }
    
    // handle housekeeping packet
    
    switch (id) {
      
      case HSK_ID_RADAR_INFO:
        _readRadarInfo(nbytes);
        break;
      case HSK_ID_SCAN_SEG:
        _readScanSeg(nbytes);
        break;
      case HSK_ID_PROCESSOR_INFO:
        _readProcInfo(nbytes);
        break;
      case HSK_ID_PWR_UPDATE:
        _readPowerUpdate(nbytes);
        break;
      case HSK_ID_EVENT_NOTICE:
        _readEventNotice(nbytes);
        break;
      case HSK_ID_CAL_TERMS:
        _readCalTerms(nbytes);
        break;
      case HSK_ID_XMIT_INFO:
        _readXmitInfo(nbytes);
        break;
      case HSK_ID_ANT_OFFSET:
        _readAntCorr(nbytes);
        break;
      case HSK_ID_XMIT_SAMPLE:
        _readXmitSample(nbytes);
        break;
      case HSK_ID_PHASE_CODE:
        _readPhaseCode(nbytes);
        break;
      case MOM_ID_SDB:
        _readRay(nbytes);
        break;
      case HSK_ID_VERSION:
        _readVersion(nbytes);
        break;
      case HSK_ID_TRACK_INFO:
        _readTrack(nbytes);
        break;
      default:
        if (nbytes < 0 || nbytes > 20000) {
          cerr << "ERROR - bad len: " << nbytes << endl;
          // resynchronize
          if (_reSync()) {
            return -1;
          }
        } else if (nbytes != 0) {
          if (_params.debug) {
            cerr << "===== PACKET TYPE NOT USED ============" << endl;
            cerr << "id, len: " << hex << id << dec
                 << ":" << _hskId2String(id) << ", " << nbytes << endl;
            cerr << "  skipping ahead" << endl;
            cerr << "=======================================" << endl;
          }
          _seekAhead(nbytes);
        }
        
    } // switch
      
  } // while (true)
    
  return 0;
  
}

//////////////////////////////////////////////////
// seek ahead

int ChillSdb2Dsr::_seekAhead(int nBytes)
  
{

  TaArray<char> buf_;
  char *buf = buf_.alloc(nBytes);

  if (_sock.readBufferHb(buf, nBytes, 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillSdb2Dsr::_seekAhead" << endl;
    cerr << "  Seeking ahead by nbytes: " << nBytes << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////
// re-sync the data stream
// returns 0 on success, -1 on error

int ChillSdb2Dsr::_reSync()
  
{

  if (_params.debug) {
    cerr << "Trying to resync ....." << endl;
  }

  int syncCount = 0;
  while (true) {
    
    // peek at the id

    int32_t id;
    if (_peekAtBuffer(&id, sizeof(id))) {
      cerr << "ERROR - ChillSdb2Dsr::_reSync" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }

    // check if we have a housekeeping header which matches

    if (id == HSK_ID_RADAR_INFO ||
        id == HSK_ID_SCAN_SEG ||
        id == HSK_ID_PROCESSOR_INFO ||
        id == HSK_ID_PWR_UPDATE ||
        id == HSK_ID_EVENT_NOTICE ||
        id == HSK_ID_CAL_TERMS ||
        id == HSK_ID_VERSION ||
        id == HSK_ID_XMIT_INFO ||
        id == HSK_ID_TRACK_INFO ||
        id == HSK_ID_ANT_OFFSET ||
        id == HSK_ID_XMIT_SAMPLE ||
        id == HSK_ID_PHASE_CODE ||
        id == HSK_ID_FILTER_INFO) {
      // back in sync
      if (_params.debug) {
        cerr << "Back in sync, syncCount, ID: "
             << syncCount << ", " << hex << id << dec << endl;
      }
      return 0;
    }

    // no sync yet, read 1 byte and start again
    
    char byteVal;
    if (_sock.readBufferHb(&byteVal, 1, 1, PMU_auto_register, 1000)) {
      cerr << "ERROR - ChillTsTcp2Fmq::_reSync" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
    syncCount++;

  } // while

  return 0;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure

int ChillSdb2Dsr::_peekAtBuffer(void *buf, int nbytes)

{
  
  while (true) {
    PMU_auto_register("peekAtBuffer");
    if (_sock.peek(buf, nbytes, 1000) == 0) {
      return 0;
    } else {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
        PMU_auto_register("Timed out ...");
        continue;
      }
      cerr << "ERROR - ChillTsTcp2Fmq::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  return -1;

}

//////////////////////////////////////////////////
// read in struct

int ChillSdb2Dsr::_readStruct(int nbytes,
                              int structSize,
                              void *buf)
  
{

  int bytesToRead = structSize;
  if (nbytes < bytesToRead) {
    bytesToRead = nbytes;
  }
  if (_sock.readBufferHb(buf,
                         bytesToRead,
                         1024,
                         PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillSdb2Dsr::_readStruct" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// read radarInfo

int ChillSdb2Dsr::_readRadarInfo(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_radarInfo), &_radarInfo)) {
    cerr << "ERROR - ChillSdb2Dsr::_readRadarInfo" << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_radar_info_print(cerr, _radarInfo);
  }
  _radarInfoAvail = true;
  _paramsPending = true;

  return 0;

}

//////////////////////////////////////////////////
// read scan segment

int ChillSdb2Dsr::_readScanSeg(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_scanSeg), &_scanSeg)) {
    cerr << "ERROR - ChillSdb2Dsr::_readScanSeg" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_scan_seg_print(cerr, _scanSeg);
  }
  _scanSegAvail = true;

  bool endOfVol = false;
  bool endOfTilt = false;

  if ((int) _scanSeg.volume_num != _volNum) {
    endOfVol = true;
    endOfTilt = true;
  } else if ((int) (_scanSeg.sweep_num - 1) != _tiltNum) {
    endOfTilt = true;
  }

  if (endOfTilt) {
    _rQueue->putEndOfTilt(_tiltNum, _rayHdr.time);
  }
  if (endOfVol) {
    _rQueue->putEndOfVolume(_volNum, _rayHdr.time);
  }

  _volNum = _scanSeg.volume_num;
  _tiltNum = _scanSeg.sweep_num - 1;
  
  if (endOfVol) {
    _rQueue->putStartOfVolume(_volNum, _rayHdr.time);
  }
  if (endOfTilt) {
    _rQueue->putStartOfTilt(_tiltNum, _rayHdr.time);
  }

  return 0;

}

//////////////////////////////////////////////////
// read proc info

int ChillSdb2Dsr::_readProcInfo(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_procInfo), &_procInfo)) {
    cerr << "ERROR - ChillSdb2Dsr::_readProcInfo" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_proc_info_print(cerr, _procInfo);
  }
  
  _procInfoAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read power update

int ChillSdb2Dsr::_readPowerUpdate(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_powerUpdate), &_powerUpdate)) {
    cerr << "ERROR - ChillSdb2Dsr::_readPowerUpdate" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_power_print(stderr, _powerUpdate);
  }
  
  _powerUpdateAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read event notice

int ChillSdb2Dsr::_readEventNotice(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_eventNotice), &_eventNotice)) {
    cerr << "ERROR - ChillSdb2Dsr::_readEventNotice" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_event_notice_print(stderr, _eventNotice);
  }
  
  _eventNoticeAvail = true;

  if (_scanSegAvail) {

  }
  
  return 0;

}

//////////////////////////////////////////////////
// read cal terms

int ChillSdb2Dsr::_readCalTerms(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_calTerms), &_calTerms)) {
    cerr << "ERROR - ChillSdb2Dsr::_readCalTerms" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_cal_terms_print(cerr, _calTerms);
  }

  _calTermsAvail = true;
  _calibPending = true;
  _calibTime = _rayHdr.time;

  return 0;

}

//////////////////////////////////////////////////
// read xmit info

int ChillSdb2Dsr::_readXmitInfo(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_xmitInfo), &_xmitInfo)) {
    cerr << "ERROR - ChillSdb2Dsr::_readXmitInfo" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_xmit_info_print(cerr, _xmitInfo);
  }

  _xmitInfoAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read antenna correction

int ChillSdb2Dsr::_readAntCorr(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_antCorr), &_antCorr)) {
    cerr << "ERROR - ChillSdb2Dsr::_readAntCorr" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_ant_corr_print(cerr, _antCorr);
  }

  _antCorrAvail = true;
 
  return 0;

}

//////////////////////////////////////////////////
// read xmit sample

int ChillSdb2Dsr::_readXmitSample(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_xmitSample), &_xmitSample)) {
    cerr << "ERROR - ChillSdb2Dsr::_readXmitSample" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_xmit_sample_print(cerr, _xmitSample);
  }

  _xmitSampleAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read phase code

int ChillSdb2Dsr::_readPhaseCode(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_phaseCode), &_phaseCode)) {
    cerr << "ERROR - ChillSdb2Dsr::_readPhaseCode" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_phasecode_print(cerr, _phaseCode);
  }

  _phaseCodeAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read version

int ChillSdb2Dsr::_readVersion(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_version), &_version)) {
    cerr << "ERROR - ChillSdb2Dsr::_readVersion" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_sdb_version_print(cerr, _version);
  }

  _versionAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read track

int ChillSdb2Dsr::_readTrack(int nbytes)
  
{

  if (_readStruct(nbytes, sizeof(_track), &_track)) {
    cerr << "ERROR - ChillSdb2Dsr::_readTrack" << endl;
    return -1;
  }

  _trackAvail = true;

  return 0;

}

//////////////////////////////////////////////////
// read ray

int ChillSdb2Dsr::_readRay(int nbytes)
  
{

  // read in ray header

  if (_readStruct(nbytes, sizeof(_rayHdr), &_rayHdr)) {
    cerr << "ERROR - ChillSdb2Dsr::_readRay" << endl;
    return -1;
  }

  // compute az and el

  _computeAngles();

  // read in data

  int nFields = _rayHdr.bytes_per_gate / sizeof(float);
  int nBytes = _rayHdr.gates * _rayHdr.bytes_per_gate;
  TaArray<char> gateData_;
  char *gateData = gateData_.alloc(nBytes);
  if (_sock.readBufferHb(gateData, nBytes, 1024,
                         PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillSdb2Dsr::_readRay" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }

  // write params if needed

  if (_paramsPending) {
    if (_writeParams(nFields)) {
      return -1;
    }
    _paramsPending = false;
  }

  // write calibration if needed

  if (_calibPending) {
    if (_writeCalib()) {
      return -1;
    }
    if (_params.write_cal_xml_files) {
      // write out the XML calibration file
      _writeCalibXml();
    }
    // reset flag
    _calibPending = false;
  }

  // write the beam

  if (_writeBeam(gateData, nFields, nBytes)) {
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// compuet the azimuth and elevation angles

void ChillSdb2Dsr::_computeAngles()

{

  // first time?

  if (_prevAz < -9990 && _prevEl < -9990) {
    _el = _rayHdr.elevation;
    _az = _rayHdr.azimuth;
    _prevEl = _rayHdr.elevation;
    _prevAz = _rayHdr.azimuth;
    return;
  }

  bool up = true;
  bool clockwise = true;

  if (_scanSeg.scan_type == SCAN_TYPE_RHI) {

    // RHI

    _az = _rayHdr.azimuth;

    // elevation direction?
    
    double del = _rayHdr.elevation - _prevEl;
    if (del > 180) {
      del -= 360.0;
    } else if (del < -180) {
      del += 360.0;
    }
    
    if (del > 0) {
      // up
      _el = _rayHdr.elevation - _rayHdr.elevation_width / 2.0;
      if (_el < 0) {
        _el += 360.0;
      }
    } else {
      // down
      _el = _rayHdr.elevation + _rayHdr.elevation_width / 2.0;
      if (_el > 360.0) {
        _el -= 360.0;
      }
      up = false;
    }

  } else {

    // PPI

    _el = _rayHdr.elevation;

    // azimuth direction?
    
    double daz = _rayHdr.azimuth - _prevAz;
    if (daz > 180) {
      daz -= 360.0;
    } else if (daz < -180) {
      daz += 360.0;
    }
    
    if (daz > 0) {
      // clockwise
      _az = _rayHdr.azimuth - _rayHdr.azimuth_width / 2.0;
      if (_az < 0) {
        _az += 360.0;
      }
    } else {
      // anticlockwise
      _az = _rayHdr.azimuth + _rayHdr.azimuth_width / 2.0;
      if (_az > 360.0) {
        _az -= 360.0;
      }
      clockwise = false;
    }

  } // PPI or RHI

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  rayHdr az: " << _rayHdr.azimuth << endl;
    cerr << "  rayHdr el: " << _rayHdr.elevation << endl;
    cerr << "  rayHdr az_width: " << _rayHdr.azimuth_width << endl;
    cerr << "  rayHdr el_width: " << _rayHdr.elevation_width << endl;
    cerr << "  up: " << (up?"Y":"N") << endl;
    cerr << "  clockwise: " << (clockwise?"Y":"N") << endl;
    cerr << "  corrected az: " << _az << endl;
    cerr << "  corrected el: " << _el << endl;
  }

  // save for next time

  _prevEl = _rayHdr.elevation;
  _prevAz = _rayHdr.azimuth;

}

////////////////////////////////////////
// open the output FMQ

int ChillSdb2Dsr::_openFmq()
  
{

  if (_rQueue != NULL) {
    delete _rQueue;
  }

  _rQueue = new DsRadarQueue();

  // initialize the output queue
  
  if (_rQueue->init(_params.output_fmq_url,
		    _progName.c_str(),
		    _params.debug >= Params::DEBUG_VERBOSE,
		    DsFmq::READ_WRITE, DsFmq::END,
		    _params.output_fmq_compress,
		    _params.output_fmq_nslots,
		    _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::_openFmq" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
    cerr << _rQueue->getErrStr() << endl;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _rQueue->setCompressionMethod(TA_COMPRESSION_ZLIB);
  }
  if (_params.data_mapper_report_interval > 0) {
    _rQueue->setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }

  return 0;

}

////////////////////////////////////////
// Write radar and field params to queue
// Returns 0 on success, -1 on failure

int ChillSdb2Dsr::_writeParams(int nFields)

{

  if (!_radarInfoAvail) {
    return 0;
  }
  
  // initialize 
  
  int nGates = _rayHdr.gates;
  int nSamples = _rayHdr.num_pulses;
  double pulseWidthUs = 1.0;
  if (_procInfo.pulse_type == PULSE_TYPE_RECT_200NS) {
    pulseWidthUs = 0.2;
  }
  double prt = _procInfo.prt_usec / 1.0e6;
  double wavelengthCm = _radarInfo.wavelength_cm;
  double wavelengthM = wavelengthCm / 100.0;
  
  double nyquistVel = ((wavelengthM / prt) / 4.0); // m/s
  double unambigRange = ((3.0e8 * prt) / 2.0) / 1000.0; // km

  // create message
  
  DsRadarMsg msg;
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = nFields;
  rp.numGates = nGates;
  rp.samplesPerBeam = nSamples;
  rp.scanType = 0;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  if (_scanSeg.scan_type == SCAN_TYPE_RHI) {
    rp.scanMode = DS_RADAR_RHI_MODE;
  } else if (_scanSeg.scan_flags & SF_SECTOR) {
    rp.scanMode = DS_RADAR_SECTOR_MODE;
  }
  rp.polarization = DS_POLARIZATION_DUAL_TYPE;
  
  rp.radarConstant = _radarInfo.base_radar_constant_db;
  rp.altitude = _radarInfo.altitude_m / 1000.0;
  rp.latitude = _radarInfo.latitude_d;
  rp.longitude = _radarInfo.longitude_d;
  
  rp.gateSpacing = _procInfo.gate_spacing_m / 1000.0; // km
  rp.startRange = (-1.0 * _procInfo.range_offset_m) / 1000.0; // km

  rp.horizBeamWidth = _radarInfo.beamwidth_d;
  rp.vertBeamWidth = _radarInfo.beamwidth_d;
  
  rp.pulseWidth = pulseWidthUs;
  rp.pulseRepFreq = 1.0 / prt;
  rp.wavelength = wavelengthCm;
  
  rp.xmitPeakPower = pow(10.0, _xmitSample.h_power_dbm / 10.0);
  rp.receiverGain = _calTerms.gain_h_rx_1_db;
  rp.receiverMds = _calTerms.noise_h_rx_1;
  rp.antennaGain = _radarInfo.gain_ant_h_db;
  rp.systemGain = rp.receiverGain + rp.antennaGain;
  
  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.measXmitPowerDbmH = _xmitSample.h_power_dbm;
  rp.measXmitPowerDbmV = _xmitSample.v_power_dbm;

  rp.radarName = _radarInfo.radar_name;
  rp.scanTypeName = "SUR";
  if (_scanSeg.scan_type == SCAN_TYPE_RHI) {
    rp.scanTypeName = "RHI";
  } else if (_scanSeg.scan_flags & SF_SECTOR) {
    rp.scanTypeName = "SEC";
  }
  
  // Add field parameters

  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _addField("DBZ", "dBZ", fp);
  _addField("VEL", "m/s", fp);
  _addField("WIDTH", "m/s", fp);
  _addField("NCP", "", fp);
  _addField("ZDR", "dB", fp);
  _addField("PHIDP", "deg", fp);
  _addField("RHOHV", "", fp);
  _addField("LDR_H", "dB", fp);
  _addField("LDR_V", "dB", fp);
  _addField("KDP", "deg/km", fp);
  _addField("DBZ_C", "dBZ", fp);
  _addField("ZDR_C", "dB", fp);
  _addField("PHIDP_F", "deg", fp);
  _addField("AVG_V_RE", "", fp);
  _addField("AVG_V_IM", "", fp);
  _addField("AVG_H_RE", "", fp);
  _addField("AVG_H_RE", "", fp);
  _addField("LAG0_H", "", fp);
  _addField("LAG0_V", "", fp);
  _addField("LAG0_HX", "", fp);
  _addField("LAG0_VX", "", fp);
  _addField("LAG1_H_RE", "", fp);
  _addField("LAG1_H_IM", "", fp);
  _addField("LAG1_V_RE", "", fp);
  _addField("LAG1_V_IM", "", fp);
  _addField("LAG2_H_RE", "E", fp);
  _addField("LAG2_H_IM", "", fp);
  _addField("LAG2_V_RE", "E", fp);
  _addField("LAG2_V_IM", "", fp);
  _addField("LAG0_HV_RE", "", fp);
  _addField("LAG0_HV_IM", "", fp);
  _addField("LAG0_HV_IM", "", fp);
  _addField("LAG0_HV_IM", "", fp);

  if (nFields > 24) {
    _addField("RHOHV_HCX", "", fp);
  }
  if (nFields > 25) {
    _addField("RHOHV_VCX", "", fp);
  }

  for (int ii = 0; ii < nFields - 26; ii++) {
    char name[128];
    sprintf(name, "UNKNOWN_%d", ii);
    _addField(name, "", fp);
  }
  
  // write the message
  
  if (_rQueue->putDsMsg(msg,
                       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - ChillSdb2Dsr::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// Add a field to the field params message.

void ChillSdb2Dsr::_addField(const string &name,
                             const string &units,
                             vector<DsFieldParams*> &fp)
{
  DsFieldParams* fparams =
    new DsFieldParams(name.c_str(), units.c_str(),
                      1.0, 0.0, sizeof(fl32));
  fp.push_back(fparams);
}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int ChillSdb2Dsr::_writeCalib()

{

  if (!_calTermsAvail) {
    return 0;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> ChillSdb2Dsr::writeCalib" << endl;
  }

  // create DsRadar message
  
  DsRadarMsg msg;

  // set calib in message to the beam calibration
  
  DsRadarCalib &calib = msg.getRadarCalib();
  
  calib.setRadarName(_radarInfo.radar_name);
  calib.setCalibTime(_calibTime);
  
  // field-specific set methods
  
  calib.setWavelengthCm(_radarInfo.wavelength_cm);
  calib.setBeamWidthDegH(_radarInfo.beamwidth_d);
  calib.setBeamWidthDegV(_radarInfo.beamwidth_d);
  calib.setAntGainDbH(_radarInfo.gain_ant_h_db);
  calib.setAntGainDbV(_radarInfo.gain_ant_v_db);

  double pulseWidthUs = 1.0;
  if (_procInfo.pulse_type == PULSE_TYPE_RECT_200NS) {
    pulseWidthUs = 0.2;
  }
  calib.setPulseWidthUs(pulseWidthUs);
  calib.setXmitPowerDbmH(_xmitSample.h_power_dbm);
  calib.setXmitPowerDbmV(_xmitSample.v_power_dbm);

  calib.setTwoWayWaveguideLossDbH(-9999);
  calib.setTwoWayWaveguideLossDbV(-9999);
  calib.setTwoWayRadomeLossDbH(-9999);
  calib.setTwoWayRadomeLossDbV(-9999);

  calib.setReceiverMismatchLossDb(-9999);

  calib.setRadarConstH(_radarInfo.base_radar_constant_db);
  calib.setRadarConstV(_radarInfo.base_radar_constant_db);

  calib.setNoiseDbmHc(_calTerms.noise_h_rx_2);
  calib.setNoiseDbmHx(_calTerms.noise_h_rx_1);
  calib.setNoiseDbmVc(_calTerms.noise_v_rx_2);
  calib.setNoiseDbmVx(_calTerms.noise_v_rx_1);

  calib.setReceiverGainDbHc(_calTerms.gain_h_rx_2_db);
  calib.setReceiverGainDbHx(_calTerms.gain_h_rx_1_db);
  calib.setReceiverGainDbVc(_calTerms.gain_v_rx_2_db);
  calib.setReceiverGainDbVx(_calTerms.gain_v_rx_1_db);

  calib.setReceiverSlopeDbHc(1.0);
  calib.setReceiverSlopeDbHx(1.0);
  calib.setReceiverSlopeDbVc(1.0);
  calib.setReceiverSlopeDbVx(1.0);

  calib.setBaseDbz1kmHc(-9999);
  calib.setBaseDbz1kmHx(-9999);
  calib.setBaseDbz1kmVc(-9999);
  calib.setBaseDbz1kmVx(-9999);

  calib.setSunPowerDbmHc(-9999);
  calib.setSunPowerDbmHx(-9999);
  calib.setSunPowerDbmVc(-9999);
  calib.setSunPowerDbmVx(-9999);

  calib.setNoiseSourcePowerDbmH(_calTerms.noise_source_h_db);
  calib.setNoiseSourcePowerDbmV(_calTerms.noise_source_v_db);

  calib.setPowerMeasLossDbH(_radarInfo.power_measurement_loss_h_db);
  calib.setPowerMeasLossDbV(_radarInfo.power_measurement_loss_v_db);

  calib.setCouplerForwardLossDbH(_radarInfo.dc_loss_h_db);
  calib.setCouplerForwardLossDbV(_radarInfo.dc_loss_v_db);

  calib.setZdrCorrectionDb(_calTerms.zdr_bias_db);
  calib.setLdrCorrectionDbH(_calTerms.ldr_bias_h_db);
  calib.setLdrCorrectionDbV(_calTerms.ldr_bias_v_db);
  calib.setSystemPhidpDeg(_radarInfo.phidp_rot_d);

  calib.setTestPowerDbmH(_radarInfo.test_power_h_db);
  calib.setTestPowerDbmV(_radarInfo.test_power_v_db);
  
  // write the message
  
  if (_rQueue->putDsMsg(msg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - ChillSdb2Dsr::writeCalib" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int ChillSdb2Dsr::_writeBeam(const char *gateData, int nFields, int nBytes)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> ChillSdb2Dsr::writeBeam, az: " << _az 
	 << " el: " << _el 
	 << " nSamples: " << _rayHdr.num_pulses << endl;
  } else if (_params.debug) {
    if ((_nBeamsWritten % 90) == 0)
      cerr << "-->> ChillSdb2Dsr::writeBeam, az: " << _az 
           << " el: " << _el 
           << " nSamples: " << _rayHdr.num_pulses << endl;
  }

  if (_nBeamsSinceParams > _params.nbeams_for_params_and_calib) {
    _writeParams(nFields);
    _writeCalib();
    _nBeamsSinceParams = 0;
  }

  // beam header
  
  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  dsBeam.dataTime = _rayHdr.time;
  dsBeam.nanoSecs = _rayHdr.ns_time;
  dsBeam.volumeNum = _scanSeg.volume_num;
  dsBeam.tiltNum = _scanSeg.sweep_num - 1;
  dsBeam.elevation = _el;
  dsBeam.azimuth = _az;

  dsBeam.targetAz = -9999;
  dsBeam.targetElev = -9999;

  if (_scanSeg.scan_type == SCAN_TYPE_RHI) {
    dsBeam.targetAz = _scanSeg.current_fixed_angle;
    dsBeam.scanMode = DS_RADAR_RHI_MODE;
  } else {
    dsBeam.targetElev = _scanSeg.current_fixed_angle;
    if (_scanSeg.scan_flags & SF_SECTOR) {
      dsBeam.scanMode = DS_RADAR_SECTOR_MODE;
    } else {
      dsBeam.scanMode = DS_RADAR_SURVEILLANCE_MODE;
    }
  }

  dsBeam.antennaTransition = 0;
  dsBeam.beamIsIndexed = 1;
  dsBeam.angularResolution = 1;
  dsBeam.nSamples = _rayHdr.num_pulses;
  dsBeam.measXmitPowerDbmH = _xmitSample.h_power_dbm;
  dsBeam.measXmitPowerDbmV = _xmitSample.v_power_dbm;

  // load the data into the message
  
  dsBeam.loadData(gateData, nBytes, sizeof(fl32));
  
  // write the message
  
  if (_rQueue->putDsMsg(_msg, DsRadarMsg::RADAR_BEAM)) {
    cerr << "ERROR - ChillSdb2Dsr::writeBeams" << endl;
    cerr << "  Cannot put radar beam to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  _nBeamsWritten++;
  _nBeamsSinceParams++;

  return 0;

}

////////////////////////////////////////
// Get string for housekeeping id

string ChillSdb2Dsr::_hskId2String(int id)

{

  switch (id) {

    case HSK_ID_RADAR_INFO:
      return "HSK_ID_RADAR_INFO";
    case HSK_ID_SCAN_SEG:
      return "HSK_ID_SCAN_SEG";
    case HSK_ID_PROCESSOR_INFO:
      return "HSK_ID_PROCESSOR_INFO";
    case HSK_ID_PWR_UPDATE:
      return "HSK_ID_PWR_UPDATE";
    case HSK_ID_EVENT_NOTICE:
      return "HSK_ID_EVENT_NOTICE";
    case HSK_ID_CAL_TERMS:
      return "HSK_ID_CAL_TERMS";
    case HSK_ID_VERSION:
      return "HSK_ID_VERSION";
    case HSK_ID_XMIT_INFO:
      return "HSK_ID_XMIT_INFO";
    case HSK_ID_TRACK_INFO:
      return "HSK_ID_TRACK_INFO";
    case HSK_ID_ANT_OFFSET:
      return "HSK_ID_ANT_OFFSET";
    case HSK_ID_XMIT_SAMPLE:
      return "HSK_ID_XMIT_SAMPLE";
    case HSK_ID_PHASE_CODE:
      return "HSK_ID_PHASE_CODE";
    case HSK_ID_FILTER_INFO:
      return "HSK_ID_FILTER_INFO";
    case MOM_ID_ALL_DATA:
        return "MOM_ID_ALL_DATA";
    default:
      return "unknown";
  }

}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int ChillSdb2Dsr::_writeCalibXml()

{

  if (!_radarInfoAvail ||
      !_procInfoAvail ||
      !_calTermsAvail ||
      !_xmitSampleAvail) {
    return 0;
  }

  time_t now = time(NULL);
  int interval = now - _prevCalibXmlWriteTime;
  if (interval < _params.min_write_interval_secs) {
    return 0;
  }
  _prevCalibXmlWriteTime = now;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> ChillSdb2Dsr::writeCalibXml" << endl;
  }
  
  // compute radar constants
  
  double radarConstH = _computeRadarConstant(_xmitSample.h_power_dbm,
                                             _radarInfo.gain_ant_h_db,
                                             _params.twoWayWaveguideLossDbH,
                                             _params.twoWayRadomeLossDbH);
  double radarConstV = _computeRadarConstant(_xmitSample.v_power_dbm,
                                             _radarInfo.gain_ant_v_db,
                                             _params.twoWayWaveguideLossDbV,
                                             _params.twoWayRadomeLossDbV);
                                     
  // set calib object
  
  IwrfCalib calib;
  calib.setRadarName(_radarInfo.radar_name);
  calib.setCalibTime(_calibTime);
  
  // field-specific set methods
  
  calib.setWavelengthCm(_radarInfo.wavelength_cm);
  calib.setBeamWidthDegH(_radarInfo.beamwidth_d);
  calib.setBeamWidthDegV(_radarInfo.beamwidth_d);
  calib.setAntGainDbH(_radarInfo.gain_ant_h_db);
  calib.setAntGainDbV(_radarInfo.gain_ant_v_db);

  double pulseWidthUs = _params.pulseWidthUs;
  if (_procInfo.pulse_type == PULSE_TYPE_RECT_200NS) {
    pulseWidthUs = 0.2;
  }
  calib.setPulseWidthUs(pulseWidthUs);
  calib.setXmitPowerDbmH(_xmitSample.h_power_dbm);
  calib.setXmitPowerDbmV(_xmitSample.v_power_dbm);
  
  calib.setTwoWayWaveguideLossDbH(_params.twoWayWaveguideLossDbH);
  calib.setTwoWayWaveguideLossDbV(_params.twoWayWaveguideLossDbV);
  calib.setTwoWayRadomeLossDbH(_params.twoWayRadomeLossDbH);
  calib.setTwoWayRadomeLossDbV(_params.twoWayRadomeLossDbV);
  
  calib.setReceiverMismatchLossDb(_params.receiverMismatchLossDb);

  calib.setRadarConstH(radarConstH);
  calib.setRadarConstV(radarConstV);
  calib.setKSquaredWater(kSquared);

  if (_procInfo.polarization_mode == POL_MODE_VH) {

    // switching receiver

    calib.setNoiseDbmHc(10.0 * log10(_calTerms.noise_h_rx_2));
    calib.setNoiseDbmVc(10.0 * log10(_calTerms.noise_v_rx_2));
    calib.setNoiseDbmHx(10.0 * log10(_calTerms.noise_h_rx_1));
    calib.setNoiseDbmVx(10.0 * log10(_calTerms.noise_v_rx_1));
    
    calib.setReceiverGainDbHc(_calTerms.gain_h_rx_2_db);
    calib.setReceiverGainDbVc(_calTerms.gain_v_rx_2_db);
    calib.setReceiverGainDbHx(_calTerms.gain_h_rx_1_db);
    calib.setReceiverGainDbVx(_calTerms.gain_v_rx_1_db);
    
    calib.setBaseDbz1kmHc(_calTerms.noise_h_rx_2
                          - _calTerms.gain_h_rx_2_db
                          + radarConstH);
    calib.setBaseDbz1kmVc(_calTerms.noise_v_rx_2
                          - _calTerms.gain_v_rx_2_db
                          + radarConstV);
    calib.setBaseDbz1kmHx(_calTerms.noise_h_rx_1
                          - _calTerms.gain_h_rx_1_db
                          + radarConstH);
    calib.setBaseDbz1kmVx(_calTerms.noise_v_rx_1
                          - _calTerms.gain_v_rx_1_db
                          + radarConstV);
    
  } else {

      // fixed receiver

    calib.setNoiseDbmHc(10.0 * log10(_calTerms.noise_h_rx_2));
    calib.setNoiseDbmVc(10.0 * log10(_calTerms.noise_v_rx_1));
    calib.setNoiseDbmHx(10.0 * log10(_calTerms.noise_h_rx_1));
    calib.setNoiseDbmVx(10.0 * log10(_calTerms.noise_v_rx_2));
    
    calib.setReceiverGainDbHc(_calTerms.gain_h_rx_2_db);
    calib.setReceiverGainDbVc(_calTerms.gain_v_rx_1_db);
    calib.setReceiverGainDbHx(_calTerms.gain_h_rx_1_db);
    calib.setReceiverGainDbVx(_calTerms.gain_v_rx_2_db);
    
    calib.setBaseDbz1kmHc(_calTerms.noise_h_rx_2 - 
                          _calTerms.gain_h_rx_2_db + 
                          radarConstH);
    calib.setBaseDbz1kmVc(_calTerms.noise_v_rx_1 - 
                          _calTerms.gain_v_rx_1_db + 
                          radarConstV);
    calib.setBaseDbz1kmHx(_calTerms.noise_h_rx_1 - 
                          _calTerms.gain_h_rx_1_db + 
                          radarConstH);
    calib.setBaseDbz1kmVx(_calTerms.noise_v_rx_2 - 
                          _calTerms.gain_v_rx_2_db +
                          radarConstV);
    
  }

  calib.setI0DbmHc(calib.getNoiseDbmHc() - calib.getReceiverGainDbHc());
  calib.setI0DbmVc(calib.getNoiseDbmVc() - calib.getReceiverGainDbVc());
  calib.setI0DbmHx(calib.getNoiseDbmHx() - calib.getReceiverGainDbHx());
  calib.setI0DbmVx(calib.getNoiseDbmVx() - calib.getReceiverGainDbVx());
    
  calib.setReceiverSlopeDbHc(1.0);
  calib.setReceiverSlopeDbVc(1.0);
  calib.setReceiverSlopeDbHx(1.0);
  calib.setReceiverSlopeDbVx(1.0);

  calib.setSunPowerDbmHc(_calTerms.sun_pwr_h_rx_2_db);
  calib.setSunPowerDbmHx(_calTerms.sun_pwr_h_rx_1_db);
  calib.setSunPowerDbmVc(_calTerms.sun_pwr_v_rx_2_db);
  calib.setSunPowerDbmVx(_calTerms.sun_pwr_v_rx_1_db);

  calib.setNoiseSourcePowerDbmH(_calTerms.noise_source_h_db);
  calib.setNoiseSourcePowerDbmV(_calTerms.noise_source_v_db);

  calib.setPowerMeasLossDbH(_radarInfo.power_measurement_loss_h_db);
  calib.setPowerMeasLossDbV(_radarInfo.power_measurement_loss_v_db);

  calib.setCouplerForwardLossDbH(_radarInfo.dc_loss_h_db);
  calib.setCouplerForwardLossDbV(_radarInfo.dc_loss_v_db);

  calib.setZdrCorrectionDb(_calTerms.zdr_bias_db);
  calib.setLdrCorrectionDbH(_calTerms.ldr_bias_h_db);
  calib.setLdrCorrectionDbV(_calTerms.ldr_bias_v_db);
  calib.setSystemPhidpDeg(_radarInfo.phidp_rot_d);

  calib.setTestPowerDbmH(_radarInfo.test_power_h_db);
  calib.setTestPowerDbmV(_radarInfo.test_power_v_db);

  // convert to xml

  string xmlStr;
  calib.convert2Xml(xmlStr);
  
  // make output day dir

  DateTime ctime(_calibTime);
  char dayDir[MAX_PATH_LEN];
  sprintf(dayDir, "%s%s%.4d%.2d%.2d", _params.calibration_xml_dir, PATH_DELIM,
          ctime.getYear(), ctime.getMonth(), ctime.getDay());

  if (ta_makedir_recurse(dayDir)) {
    int errNum = errno;
    cerr << "ERROR - ChillSdb2Dsr::_writeCalibXml()" << endl;
    cerr << "  Cannot create output day dir: " << dayDir << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output path

  char calPath[MAX_PATH_LEN];
  sprintf(calPath, "%s%scal.%.4d%.2d%.2d_%.2d%.2d%.2d.xml",
          dayDir, PATH_DELIM,
          ctime.getYear(), ctime.getMonth(), ctime.getDay(),
          ctime.getHour(), ctime.getMin(), ctime.getSec());
  
  // write the XML cal to file

  FILE *out;
  if ((out = fopen(calPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ChillSdb2Dsr::_writeCalibXml()" << endl;
    cerr << "  Cannot open cal file: " << calPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  if (fwrite(xmlStr.c_str(), xmlStr.size(), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - ChillSdb2Dsr::_writeCalibXml()" << endl;
    cerr << "  Cannot write cal to file: " << calPath << endl;
    cerr << strerror(errNum) << endl;
    fclose(out);
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote output file: " << calPath << endl;
  }
  
  // close file
  
  fclose(out);
  
  // write latest data info fil
  
  string relPath;
  Path::stripDir(_params.calibration_xml_dir, calPath, relPath);
  DsLdataInfo ldata(_params.calibration_xml_dir);
  ldata.setDataFileExt("xml");
  ldata.setDataType("xml");
  ldata.setWriter(_progName.c_str());
  ldata.setRelDataPath(relPath);

  if (ldata.write(_calibTime)) {
    cerr << "ERROR - ChillSdb2Dsr::_writeCalibXml()" << endl;
    cerr << "  Cannot write latest data file to copy dir: "
	 << _params.calibration_xml_dir << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// compute radar constant, in meter units

double ChillSdb2Dsr::_computeRadarConstant(double xmitPowerDbm,
                                           double antennaGainDb,
                                           double twoWayWaveguideLossDb,
                                           double twoWayRadomeLossDb)
  
                                     
{

  double antGainPower = pow(10.0, antennaGainDb / 10.0);
  double gainSquared = antGainPower * antGainPower;
  double _wavelengthM = _radarInfo.wavelength_cm / 100.0;
  double lambdaSquared = _wavelengthM * _wavelengthM;
  double pulseMeters = _params.pulseWidthUs * 1.0e-6 * lightSpeed;
  
  double hBeamWidthRad = _radarInfo.beamwidth_d * DEG_TO_RAD;
  double vBeamWidthRad = _radarInfo.beamwidth_d * DEG_TO_RAD;

  double peakPowerMilliW = pow(10.0, xmitPowerDbm / 10.0);

  double theoreticalG = (M_PI * M_PI) / (hBeamWidthRad * vBeamWidthRad);
  double theoreticalGdB = 10.0 * log10(theoreticalG);
  
  double receiverMismatchLossDb = _params.receiverMismatchLossDb;
  
  double denom = (peakPowerMilliW * piCubed * pulseMeters * gainSquared *
                hBeamWidthRad * vBeamWidthRad * kSquared * 1.0e-24);

  double num = (1024.0 * log(2.0) * lambdaSquared);
  
  double factor = num / denom;
  
  double radarConst = (10.0 * log10(factor)
                       + twoWayWaveguideLossDb
                       + twoWayRadomeLossDb
                       + receiverMismatchLossDb);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==== Computing radar constant ====" << endl;
    cerr << "  wavelengthCm: " << _radarInfo.wavelength_cm << endl;
    cerr << "  beamWidthDeg: " << _radarInfo.beamwidth_d << endl;
    cerr << "  antGainDb: " << antennaGainDb << endl;
    cerr << "  theoretical antGainDb: " << theoreticalGdB << endl;
    cerr << "  xmitPowerDbm: " << xmitPowerDbm << endl;
    cerr << "  pulseWidthUs: " << _params.pulseWidthUs << endl;
    cerr << "  waveguideLoss: " << twoWayWaveguideLossDb << endl;
    cerr << "  radomeLoss: " << twoWayRadomeLossDb << endl;
    cerr << "  receiverLoss: " << receiverMismatchLossDb << endl;
    cerr << "  antGainPower: " << antGainPower << endl;
    cerr << "  gainSquared: " << gainSquared << endl;
    cerr << "  lambdaSquared: " << lambdaSquared << endl;
    cerr << "  pulseMeters: " << pulseMeters << endl;
    cerr << "  hBeamWidthRad: " << hBeamWidthRad << endl;
    cerr << "  vBeamWidthRad: " << vBeamWidthRad << endl;
    cerr << "  peakPowerMilliW: " << peakPowerMilliW << endl;
    cerr << "  piCubed: " << piCubed << endl;
    cerr << "  kSquared: " << kSquared << endl;
    cerr << "  num: " << num << endl;
    cerr << "  denom: " << denom << endl;
    cerr << "  factor: " << factor << endl;
    cerr << "  RadarConst: " << radarConst << endl;
  }

  return radarConst;

}

