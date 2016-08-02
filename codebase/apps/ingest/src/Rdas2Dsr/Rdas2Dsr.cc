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
// Rdas2Dsr.cc
//
// Rdas2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////
//
// Rdas2Dsr reads radar data from the RDAS relay server and writes
// it to an FMQ in DsRadar format
//
////////////////////////////////////////////////////////////////

#include <toolsa/ucopyright.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <dataport/bigend.h>
#include <rapformats/DsRadarMsg.hh>
#include "Rdas2Dsr.hh"
using namespace std;

// Constructor

Rdas2Dsr::Rdas2Dsr(int argc, char **argv)

{
  
  isOK = true;
  _beamCount = 0;
  _volNum = 0;
  _tiltNum = 0;
  _scanType = 0;
  _socketIsOpen = false;

  _siteNum = -9999;
  _siteName = "unknown";
  _polarizationStr = "unknown";
  _samplesPerAz = -9999;
  _samplesPerGate = -9999;
  _samplesPerBeam = -9999;
  _altitude = -9999;
  _latitude = -9999;
  _longitude = -9999;
  _startRange = -9999;
  _gateSpacing = -9999;
  _prf = -9999;
  _frequency = -9999;
  _wavelength = -9999;
  _pulseWidth = -9999;
  _horizBeamWidth = -9999;
  _vertBeamWidth = -9999;
  _peakPower = -9999;
  _receiverMds = -9999;
  _receiverGain = -9999;
  _antGain = -9999;
  _systemGain = -9999;
  _radarConst = -9999;
  _calSlope = -9999;
  _calOffset1km = -9999;
  _mdsCount = -9999;
  _atmosAtten = -9999;

  _rCorrNGates = 0;
  _rCorrStartRange = 0;
  _rCorrGateSpacing = 0;
  _rangeCorrection = NULL;
  _range = NULL;

  _calibLut = new double[_maxCount];
  _calibLutReady = false;

  // set programe name
  
  _progName = "Rdas2Dsr";
  ucopyright((char *) _progName.c_str());
  
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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the output queue
  
  if (_rQueue.init(_params.output_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_compression,
		   _params.output_n_slots,
		   _params.output_buf_size)) {
    if (_rQueue.init(_params.output_url,
		     _progName.c_str(),
		     _params.debug >= Params::DEBUG_VERBOSE,
		     DsFmq::CREATE, DsFmq::START,
		     _params.output_compression,
		     _params.output_n_slots,
		     _params.output_buf_size)) {
      cerr << "ERROR - Rdas2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_url << endl;
      isOK = false;
      return;
    }
  }
  
  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  return;

}

// destructor

Rdas2Dsr::~Rdas2Dsr()

{

  if (_rangeCorrection != NULL) {
    delete[] _rangeCorrection;
  }
  if (_range != NULL) {
    delete[] _range;
  }
  if (_calibLut != NULL) {
    delete[] _calibLut;
  }

  // close socket

  _closeSocket();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Rdas2Dsr::Run()
{
  
  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    
    PMU_auto_register("Reading rdas data");
    
    if (!_socketIsOpen) {
      if (_openSocket()) {
	umsleep(1000);
	continue;
      }
      _socketIsOpen = true;
    }
    
    // wait on packet, timing out after 10 secs each time
    
    int iret = _socket.readSelect(10000);
  
    if (iret == 0) {
      
      // data is available, read incoming data
      
      _readIncoming();
      
    } else if (iret == SockUtil::TIMED_OUT) {
	
      // timeout
      
      PMU_auto_register("Timeout waiting for data");
      
    } else {
      
      // select error - disconnect and try again later
	
      PMU_auto_register("Closing connection");
      if (_params.debug) {
	cerr << "Select error - closing connection to rdas ..." << endl;
      }
      _closeSocket();
      
    } // if (iret == 0)
    
  } // while
  
  return 0;
  
}

//////////////////////////////////////////////////
// check that the connection is open

int Rdas2Dsr::_openSocket()
  
{

  PMU_auto_register("Opening socket");
  
  // open socket connection to rdas
  
  int iret = _socket.open(_params.relay_host,
			  _params.relay_port,
			  1000);
  
  if (iret == 0) {
    if (_params.debug) {
      cerr << "  Connected to RdasRelay ..." << endl;
    }
    return 0;
  }
  
  // time-out?
  
  if (iret == SockUtil::TIMED_OUT) {
    PMU_auto_register("Waiting for connection");
    return -1;
  }
    
  // failure

  PMU_auto_register("Cannot open socket");
  cerr << "ERROR - Rdas2Dsr::_openSocket" << endl;
  cerr << "  " << _socket.getErrStr() << endl;

  return -1;

}
      
//////////////////////////////////////////////////
// close connection to rdas

void Rdas2Dsr::_closeSocket()

{

  if (_socketIsOpen) {
    PMU_auto_register("Closing connection");
    _socket.close();
    _socketIsOpen = false;
  }

}
      
//////////////////////////////////////////////////
// read incoming data

int Rdas2Dsr::_readIncoming()
  
{
  
  PMU_auto_register("Reading incoming data");
  
  // read magic cookie
  
  ui08 c1 = 0, c2 = 0, c3 = 0, c4 = 0;
  
  while (true) {
    if (_socket.readBuffer(&c4, 1, 1000)) {
      if (_socket.getErrNum() != SockUtil::TIMED_OUT) {
	_closeSocket();
	return -1;
      }
      PMU_auto_register("Waiting for cookie");
      continue;
    }
    if (_socket.getNumBytes() != 1) {
      continue;
    }
    if (c1 == 0x5A && c2 == 0x5A && c3 == 0x5A && c4 == 0x5A) {
      return _readBeam();
    }
    if (c1 == 0x5C && c2 == 0x5C && c3 == 0x5C && c4 == 0x5C) {
      return _readParams();
    }
    if (c1 == 0x5D && c2 == 0x5D && c3 == 0x5D && c4 == 0x5D) {
      int iret = _readCalib();
      if (iret == 0) {
 	if (_params.debug >= Params::DEBUG_VERBOSE) {
 	  _printRadarCharacteristics(cerr);
 	}
      }
      return iret;
    }
    c1 = c2;
    c2 = c3;
    c3 = c4;

  } // while

  return 0;
}

//////////////////////////////////////////////////
// read beam

int Rdas2Dsr::_readBeam()
  
{
  
  PMU_auto_register("Reading beam");
  
  // read in header, minus the cookie
  
  rdas_beam_hdr_t hdr;
  int nLeftInHdr = sizeof(hdr) - sizeof(si32);
  if (_socket.readBuffer(&hdr.version, nLeftInHdr, 10000) ||
      (int) _socket.getNumBytes() != nLeftInHdr) {
    cerr << "ERROR - Rdas2Dsr::_readBeam" << endl;
    cerr << "  " << _socket.getErrStr() << endl;
    _closeSocket();
    return -1;
  }
  
  // swap header if appropriate
  
  bool doSwap = false;
  if (hdr.year < 1970 || hdr.year > 10000) {
    doSwap = true;
  }
  if (doSwap) {
    BE_to_array_32(&hdr, sizeof(hdr));
  }

  // check nfields

  if (hdr.nfields != 1) {
    if (_params.debug) {
      cerr << "ERROR -  Rdas2Dsr::_readBeam" << endl;
      cerr << "  Cannot handle more than 1 field, only DBZ supported" << endl;
      cerr << "  Nfields: " << hdr.nfields << endl;
    }
  }
  
  // read in data

  int nGates = hdr.ngates;
  int nBytesData = nGates * sizeof(ui16);
  if (nBytesData > 100000) {
    if (_params.debug) {
      cerr << "ERROR -  Rdas2Dsr::_readBeam" << endl;
      cerr << "  Bad header, nBytesData: " << nBytesData << endl;
      return -1;
    }
  }

  ui16 *counts = new ui16[nGates];
  if (_socket.readBuffer(counts, nBytesData, 10000) ||
      (int) _socket.getNumBytes() != nBytesData) {
    cerr << "ERROR - Rdas2Dsr::_readBeam" << endl;
    cerr << "  " << _socket.getErrStr() << endl;
    delete[] counts;
    return -1;
  }
  
  if (doSwap) {
    BE_to_array_16(counts, nBytesData);
  }
  _handleBeam(hdr, nGates, counts);
  
  // free up

  delete[] counts;

  return 0;

}

//////////////////////////////////////////////////
// read params

int Rdas2Dsr::_readParams()
  
{

  PMU_auto_register("Reading params");
  
  // read in length
  
  si32 paramBufLen;
  if (_socket.readBuffer(&paramBufLen, sizeof(si32), 10000) ||
      _socket.getNumBytes() != sizeof(si32)) {
    cerr << "ERROR - Rdas2Dsr::_readParams" << endl;
    cerr << "  " << _socket.getErrStr() << endl;
    _closeSocket();
    return -1;
  }
  
  // swap length if appropriate
  
  bool doSwap = false;
  if (paramBufLen < 0 || paramBufLen > 10000000) {
    doSwap = true;
  }
  if (doSwap) {
    BE_to_array_32(&paramBufLen, sizeof(si32));
  }
  
  // read in buf

  char *paramBuf = new char[paramBufLen + 1];
  memset(paramBuf, 0, paramBufLen + 1);
  if (_socket.readBuffer(paramBuf, paramBufLen, 10000) ||
      (int) _socket.getNumBytes() != paramBufLen) {
    cerr << "ERROR - Rdas2Dsr::_readParams" << endl;
    cerr << "  " << _socket.getErrStr() << endl;
    delete[] paramBuf;
    return -1;
  }

//   if (_params.debug >= Params::DEBUG_VERBOSE) {
//     cerr << "================ params =================" << endl;
//     cerr << paramBuf << endl;
//   }

  // get relevant parameters from the buffer

  int iret = 0;

  if (_findParamValStr(paramBuf, "siteName", _siteName)) {
    iret = -1;
  }
  if (_findParamValStr(paramBuf, "polarization", _polarizationStr)) {
    iret = -1;
  }

  if (_findParamInt(paramBuf, "siteNum", _siteNum)) {
    iret = -1;
  }
  if (_findParamInt(paramBuf, "samplesPerAz", _samplesPerAz)) {
    iret = -1;
  }
  if (_findParamInt(paramBuf, "samplesPerGate", _samplesPerGate)) {
    iret = -1;
  }
  if (_samplesPerAz > 0 && _samplesPerGate > 0) {
    _samplesPerBeam = _samplesPerAz * _samplesPerGate;
  }

  if (_findParamDouble(paramBuf, "altitude", _altitude)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "latitude", _latitude)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "longitude", _longitude)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "startRange", _startRange)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "gateSpacing", _gateSpacing)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "prf", _prf)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "frequency", _frequency)) {
    iret = -1;
  }
  if (_frequency > 0) {
    _wavelength = (3.0e8 / (_frequency * 1.0e9)) * 100.0;
  }
  if (_findParamDouble(paramBuf, "pulseWidth", _pulseWidth)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "horizBeamWidth", _horizBeamWidth)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "vertBeamWidth", _vertBeamWidth)) {
    iret = -1;
  }
  if (_findParamDouble(paramBuf, "peakPower", _peakPower)) {
    iret = -1;
  }
  //   if (_findParamDouble(paramBuf, "receiverMds", _receiverMds)) {
  //     iret = -1;
  //   }
  //   if (_findParamDouble(paramBuf, "receiverGain", _receiverGain)) {
  //     iret = -1;
  //   }
  if (_findParamDouble(paramBuf, "antGain", _antGain)) {
    iret = -1;
  }

  // free up

  delete[] paramBuf;

  if (_params.debug) {
    cerr << "---->> Got params <<----" << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// read calib

int Rdas2Dsr::_readCalib()
  
{
  
  PMU_auto_register("Reading calib");
  
  // read in length
  
  si32 calibBufLen;
  if (_socket.readBuffer(&calibBufLen, sizeof(si32), 10000) ||
      _socket.getNumBytes() != sizeof(si32)) {
    cerr << "ERROR - Rdas2Dsr::_readCalib" << endl;
    cerr << "  " << _socket.getErrStr() << endl;
    _closeSocket();
    return -1;
  }
  
  // swap length if appropriate
  
  bool doSwap = false;
  if (calibBufLen < 0 || calibBufLen > 10000000) {
    doSwap = true;
  }
  if (doSwap) {
    BE_to_array_32(&calibBufLen, sizeof(si32));
  }
  
  // read in buf

  char *calibBuf = new char[calibBufLen + 1];
  memset(calibBuf, 0, calibBufLen + 1);
  if (_socket.readBuffer(calibBuf, calibBufLen, 10000) ||
      (int) _socket.getNumBytes() != calibBufLen) {
    cerr << "ERROR - Rdas2Dsr::_readCalib" << endl;
    cerr << "  " << _socket.getErrStr() << endl;
    delete[] calibBuf;
    return -1;
  }

//   if (_params.debug >= Params::DEBUG_VERBOSE) {
//     cerr << "================ calib =================" << endl;
//     cerr << calibBuf << endl;
//   }
  
  // get relevant parameters from the buffer

  int iret = 0;

  if (_findFieldDouble(calibBuf, "radarConst", _radarConst)) {
    iret = -1;
  }
  if (_findFieldDouble(calibBuf, "calSlope", _calSlope)) {
    iret = -1;
  }
  if (_findFieldDouble(calibBuf, "calOffset1km", _calOffset1km)) {
    iret = -1;
  }
  if (_findFieldDouble(calibBuf, "mdsCount", _mdsCount)) {
    iret = -1;
  }
  if (_findFieldDouble(calibBuf, "atmosAtten", _atmosAtten)) {
    iret = -1;
  }

  // load in the calib table

  if (_loadCalibTable(calibBuf)) {
    delete[] calibBuf;
    return -1;
  }

  // free up

  delete[] calibBuf;

  return 0;

}

//////////////////////////////////////////////////
// load up calibration table

int Rdas2Dsr::_loadCalibTable(const char *calibBuf)
  
{

  int nPointsTable = 0;

  if (_findFieldInt(calibBuf, "nPointsTable", nPointsTable)) {
    cerr << "ERROR - loadCalibTable()" << endl;
    cerr << "  Cannot find nPointsTable field" << endl;
    return -1;
  }

  const char *searchPtr = calibBuf;
  const char *nextSearch;
  string tableEntry;
  vector<double> dbz1kmArray;
  vector<double> countArray;
  double prevCount = -1;

  for (int ii = 0; ii < nPointsTable; ii++) {

    if (_findXmlField(searchPtr, "point", tableEntry, nextSearch)) {
      cerr << "ERROR - loadCalibTable()" << endl;
      cerr << "  Cannot load calib table entry: " << ii << endl;
      return -1;
    }
    searchPtr = nextSearch;

    int posn;
    if (_findFieldInt(tableEntry.c_str(), "posn", posn)) {
      return -1;
    }

    if (posn != ii) {
      cerr << "ERROR - loadCalibTable()" << endl;
      cerr << "  Incorrect posn for entry:" << ii << endl;
      cerr << "  Posn: " << posn << endl;
      return -1;
    }

    double dbz1km;
    if (_findFieldDouble(tableEntry.c_str(), "dbz1km", dbz1km)) {
      return -1;
    }

    double count;
    if (_findFieldDouble(tableEntry.c_str(), "count", count)) {
      return -1;
    }
    if (count < prevCount) {
      cerr << "ERROR - loadCalibTable()" << endl;
      cerr << "  Counts do not increase monotonically:" << ii << endl;
      cerr << "  This count: " << count << endl;
      cerr << "  Prev count: " << prevCount << endl;
      return -1;
    }

    dbz1kmArray.push_back(dbz1km);
    countArray.push_back(count);
    
  }

  // load up lookup table

  for (int icount = 0; icount < _maxCount; icount++) {
    _calibLut[icount] = -9999;
  }
  
  // values below cal range, set to lowest count

  int lowestCount = (int) countArray[0] + 1;
  if (lowestCount > _maxCount - 1) {
    lowestCount = _maxCount - 1;
  }
  for (int ii = 0; ii <= lowestCount; ii++) {
    _calibLut[ii] = dbz1kmArray[0];
  }

  // values above cal range, set to highest count
  
  int highestCount = (int) countArray[countArray.size()-1];
  if (highestCount < 0) {
    highestCount = 0;
  }
  for (int ii = highestCount; ii < _maxCount; ii++) {
    _calibLut[ii] = dbz1kmArray[countArray.size()-1];
  }
  
  // calibrated range

  for (int ii = 0; ii < (int) countArray.size() - 1; ii++) {
  
    double count1 = countArray[ii];
    double count2 = countArray[ii + 1];
    double dcount = count2 - count1;
    double dbz1 = dbz1kmArray[ii];
    double dbz2 = dbz1kmArray[ii + 1];
    double ddbz = dbz2 - dbz1;
    double slope = ddbz / dcount;

    int countMin = (int) countArray[ii] + 1;
    int countMax = (int) countArray[ii + 1];
    
    double dbz = dbz1 + (countMin - count1) * slope;

    for (int icount = countMin; icount <= countMax; icount++, dbz += slope) {
      _calibLut[icount] = dbz;
    }

  } // ii

//   if (_params.debug >= Params::DEBUG_VERBOSE) {
//     cerr << "Count->dBZ lookup table:" << endl;
//     for (int icount = lowestCount; icount <= highestCount; icount++) {
//       double dbz1km = (icount - _calOffset1km) / _calSlope;
//       double error = _calibLut[icount] - dbz1km;
//       cerr << "icount, dbz, from_fit, error: " << icount << ", "
// 	   << _calibLut[icount] << ", "
// 	   << dbz1km << ", "
// 	   << error << endl;
//     }
//   }

  _calibLutReady = true;

  if (_params.debug) {
    cerr << "---->> Got calibration <<----" << endl;
  }

  return 0;

}
  
//////////////////////////////////////////////////
// handle the beam from the radar

int Rdas2Dsr::_handleBeam(const rdas_beam_hdr_t &hdr,
			  int nGates,
			  const ui16 *counts)
  
{
  
  PMU_auto_register("Handling beam data");
  
  // beam time
  
  DateTime beamTime;
  
  if (_params.override_time) {
    beamTime.set(time(NULL));
  } else {
    beamTime.set(hdr.year, hdr.month, hdr.day, hdr.hour, hdr.min, hdr.sec);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Got beam, time: " << beamTime.str()
	 << ", el: " << hdr.el << ", az: " << hdr.az << endl;
  }
  
  if(hdr.end_of_vol_flag) {
    _rQueue.putEndOfVolume(_volNum, beamTime.utime());
    _volNum++;
    _tiltNum = 0;
    _rQueue.putStartOfVolume(_volNum, beamTime.utime());
  }
  
  if(hdr.end_of_tilt_flag) {
    _rQueue.putEndOfTilt(_tiltNum, beamTime.utime());
    _tiltNum++;
    _rQueue.putStartOfTilt(_tiltNum, beamTime.utime());
  }

  // compute range correction as required

  _computeRangeCorrTable(nGates);

  // compute dBZ and SNR from counts

  TaArray<fl32> dbz_, snr_;
  fl32 *dbz = dbz_.alloc(nGates);
  fl32 *snr = snr_.alloc(nGates);
  for (int ii = 0; ii < nGates; ii++) {
    int count = counts[ii];
    if (count > _maxCount - 1) {
      count = _maxCount - 1;
    }
    double dbz1km = _calibLut[count];
    if (!_calibLutReady) {
      dbz1km = (count - _calOffset1km) / _calSlope;
    }
    dbz[ii] = (fl32) (dbz1km + _rangeCorrection[ii]);
    double dsnr = (count - _mdsCount) / _calSlope;
    snr[ii] = (fl32) dsnr;
  }
  
  // send params every 90 beams
  
  if ((_beamCount % 90) == 0) {
    _writeRadarAndFieldParams(0, nGates, 0.015, 0.030);
  }
  _beamCount++;
  
  // send beam
  
  _writeBeam(beamTime.utime(),  nGates, hdr, dbz, snr);

  return 0;
  
}

////////////////////////////////////////
// write out the radar and field params


int Rdas2Dsr::_writeRadarAndFieldParams(int radarId,
					int nGates,
					double start_range,
					double gate_spacing)
  
{

  // load up radar params and field params message
  
  // radar params

  DsRadarMsg msg;
  DsRadarParams &rParams = msg.getRadarParams();
  
  rParams.radarId = _siteNum;
  rParams.radarType = DS_RADAR_GROUND_TYPE;
  rParams.numFields = 2;
  rParams.numGates = nGates;
  rParams.samplesPerBeam = _samplesPerBeam;
  rParams.scanType = _scanType;
  rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  if (_polarizationStr == "Circular") {
    rParams.polarization = (si32) DS_POLARIZATION_RIGHT_CIRC_TYPE;
  } else if (_polarizationStr == "Vertical") {
    rParams.polarization = (si32) DS_POLARIZATION_VERT_TYPE;
  } else {
    rParams.polarization = (si32) DS_POLARIZATION_HORIZ_TYPE;
  }
  rParams.radarConstant = _radarConst;
  rParams.altitude = _altitude;
  rParams.latitude = _latitude;
  rParams.longitude = _longitude;
  rParams.gateSpacing = _gateSpacing;
  rParams.startRange = _startRange;
  rParams.horizBeamWidth = _horizBeamWidth;
  rParams.vertBeamWidth = _vertBeamWidth;
  rParams.pulseWidth = _pulseWidth;
  rParams.pulseRepFreq = _prf;
  rParams.wavelength = _wavelength;
  rParams.xmitPeakPower = _peakPower;
  rParams.receiverMds = _receiverMds;
  rParams.receiverGain = _receiverGain;
  rParams.antennaGain = _antGain;
  rParams.systemGain = _systemGain;
  rParams.unambigVelocity =
    ((rParams.wavelength / 100.0) * rParams.pulseRepFreq) / 4.0;
  rParams.unambigRange = (3.0e8 / (2.0 *  rParams.pulseRepFreq)) / 1000.0;
  
  rParams.radarName = _siteName;
  rParams.scanTypeName = "surveillance";
  
  // field params

  vector< DsFieldParams* > &fieldParams = msg.getFieldParams();
  DsFieldParams *dbzFld =
    new DsFieldParams( "DBZ", "dBZ", 1.0, 0.0, 4, -9999);
  fieldParams.push_back(dbzFld);
  DsFieldParams *snrFld =
    new DsFieldParams( "SNR", "dB", 1.0, 0.0, 4, -9999);
  fieldParams.push_back(snrFld);
  
  // send the params

  if (_rQueue.putDsMsg
      (msg,
       DS_DATA_TYPE_RADAR_PARAMS | DS_DATA_TYPE_RADAR_FIELD_PARAMS)) {
    cerr << "ERROR - Rdas2Dsr::_writeRadarAndFieldParams" << endl;
    cerr << "  Cannot put radar and field params message to FMQ" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// write out a beam

int Rdas2Dsr::_writeBeam(time_t beamTime,
			 int nGates,
			 const rdas_beam_hdr_t &hdr,
			 const fl32 *dbz,
			 const fl32 *snr)
  
{

  // prepare data array, gate-by-gate

  TaArray<fl32> outData_;
  fl32 *outData = outData_.alloc(nGates * 2);
  for (int ii = 0, jj = 0; ii < nGates; ii++) {
    outData[jj++] = dbz[ii];
    outData[jj++] = snr[ii];
  }
  int nBytesOut = nGates * 2 * sizeof(fl32);
  
  DsRadarMsg msg;
  DsRadarBeam &radarBeam = msg.getRadarBeam();
  radarBeam.loadData(outData, nBytesOut, sizeof(fl32));
  radarBeam.dataTime = beamTime;
  radarBeam.azimuth = hdr.az;
  radarBeam.elevation = hdr.el;
  radarBeam.targetElev = hdr.el_target;
  radarBeam.tiltNum = _tiltNum;
  radarBeam.volumeNum = _volNum;
  
  // send the params
  
  if (_rQueue.putDsBeam
      (msg, DS_DATA_TYPE_RADAR_BEAM_DATA)) {
    cerr << "ERROR - Rdas2Dsr::_writeBeam" << endl;
    cerr << "  Cannot put beam message to FMQ" << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////
// find simple field string from XML buffer
//
// returns 0 on success, -1 on failure

int Rdas2Dsr::_findXmlField(const char *xml_buf,
			    const char *field_name,
			    string &valStr,
			    const char* &startNext)
  
{

  // compute start and end tokens

  char startTok[BUFSIZ];
  char endTok[BUFSIZ];

  sprintf(startTok, "<%s>", field_name);
  sprintf(endTok, "</%s>", field_name);

  // find start and end tokens
  
  const char *startPtr = strstr(xml_buf, startTok);
  if (startPtr == NULL) {
    cerr << "Cannot find field start tok in XML buffer: " << startTok << endl;
    return -1;
  }

  const char *endPtr = strstr(startPtr, endTok);
  if (endPtr == NULL) {
    cerr << "Cannot find field end tok in XML buffer: " << endTok << endl;
    return -1;
  }

  startPtr += strlen(startTok);
  
  if (endPtr < startPtr) {
    cerr << "Bad format in XML buffer, field: " << startTok << endl;
    return -1;
  }

  // copy the text between the tokens to the value field

  int valLen = endPtr - startPtr;
  char tmpStr[BUFSIZ];
  memcpy(tmpStr, startPtr, valLen);
  tmpStr[valLen] = '\0';
  valStr = tmpStr;

  // where to start for next search

  if (startNext != NULL) {
    startNext = endPtr + strlen(endTok);
  }
  
  return 0;

}

///////////////////////////////////////
// find param value str from XML buffer
//
// returns 0 on success, -1 on failure

int Rdas2Dsr::_findParamValStr(const char *xml_buf,
			       const char *param_name,
			       string &valStr)
  
{
  
  // compute start and end tokens

  char startTok[256];
  char endTok[256];

  sprintf(startTok, "<param name=\"%s\"", param_name);
  sprintf(endTok, "</param>");

  // find start and end tokens
  
  const char *startPtr = strstr(xml_buf, startTok);
  if (startPtr == NULL) {
    cerr << "Cannot find param start tok in XML buffer: " << startTok << endl;
    return -1;
  }

  const char *endPtr = strstr(startPtr, endTok);
  if (endPtr == NULL) {
    cerr << "Cannot find param end tok in XML buffer: " << endTok << endl;
    return -1;
  }

  const char *next;
  if (_findXmlField(startPtr, "value", valStr, next)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////
// find param value int from XML buffer
//
// returns 0 on success, -1 on failure

int Rdas2Dsr::_findParamInt(const char *xml_buf,
			    const char *param_name,
			    int &val)
  
{
  
  string valStr;
  int ival;
  if (_findParamValStr(xml_buf, param_name, valStr)) {
    cerr << "ERROR - cannot find param: " << param_name << endl;
    return -1;
  } else {
    if (sscanf(valStr.c_str(), "%d", &ival) != 1) {
      cerr << "ERROR - cannot decode param: " << param_name
	   << ", valStr: " << valStr << endl;
      return -1;
    }
  }
  
  val = ival;
  return 0;

}
  
//////////////////////////////////////////
// find param value double from XML buffer
//
// returns 0 on success, -1 on failure

int Rdas2Dsr::_findParamDouble(const char *xml_buf,
			       const char *param_name,
			       double &val)
  
{
  
  string valStr;
  double dval;
  if (_findParamValStr(xml_buf, param_name, valStr)) {
    cerr << "ERROR - cannot find param: " << param_name << endl;
    return -1;
  } else {
    if (sscanf(valStr.c_str(), "%lg", &dval) != 1) {
      cerr << "ERROR - cannot decode param: " << param_name
	   << ", valStr: " << valStr << endl;
      return -1;
    }
  }
  
  val = dval;
  return 0;

}
  
//////////////////////////////////////////
// find value integer from XML buffer
//
// returns 0 on success, -1 on failure

int Rdas2Dsr::_findFieldInt(const char *xml_buf,
			    const char *param_name,
			    int &val)
  
{
  
  string valStr;
  int ival;
  const char *next;
  if (_findXmlField(xml_buf, param_name, valStr, next)) {
    cerr << "ERROR - cannot find param: " << param_name << endl;
    return -1;
  } else {
    if (sscanf(valStr.c_str(), "%d", &ival) != 1) {
      cerr << "ERROR - cannot decode param: " << param_name
	   << ", valStr: " << valStr << endl;
      return -1;
    }
  }
  
  val = ival;
  return 0;

}
  
//////////////////////////////////////////
// find value double from XML buffer
//
// returns 0 on success, -1 on failure

int Rdas2Dsr::_findFieldDouble(const char *xml_buf,
			       const char *param_name,
			       double &val)
  
{
  
  string valStr;
  double dval;
  const char *next;
  if (_findXmlField(xml_buf, param_name, valStr, next)) {
    cerr << "ERROR - cannot find param: " << param_name << endl;
    return -1;
  } else {
    if (sscanf(valStr.c_str(), "%lg", &dval) != 1) {
      cerr << "ERROR - cannot decode param: " << param_name
	   << ", valStr: " << valStr << endl;
      return -1;
    }
  }
  
  val = dval;
  return 0;

}
  
//////////////////////////////////////////
// print radar characteristics

void Rdas2Dsr::_printRadarCharacteristics(ostream &out)

{
  
  out << "=====================" << endl;
  out << "RADAR CHARACTERISTICS" << endl;
  out << "=====================" << endl;

  out << "  siteNum: " << _siteNum << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  polarizationStr: " << _polarizationStr << endl;
  
  out << "  samplesPerAz: " << _samplesPerAz << endl;
  out << "  samplesPerGate: " << _samplesPerGate << endl;
  out << "  samplesPerBeam: " << _samplesPerBeam << endl;
  
  out << "  altitude: " << _altitude << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  
  out << "  startRange: " << _startRange << endl;
  out << "  gateSpacing: " << _gateSpacing << endl;
  
  out << "  prf: " << _prf << endl;
  out << "  frequency: " << _frequency << endl;
  out << "  wavelength: " << _wavelength << endl;
  out << "  pulseWidth: " << _pulseWidth << endl;
  
  out << "  horizBeamWidth: " << _horizBeamWidth << endl;
  out << "  vertBeamWidth: " << _vertBeamWidth << endl;
  
  out << "  peakPower: " << _peakPower << endl;
  out << "  receiverMds: " << _receiverMds << endl;
  out << "  receiverGain: " << _receiverGain << endl;
  out << "  antGain: " << _antGain << endl;
  out << "  systemGain: " << _systemGain << endl;
  
  out << "  radarConst: " << _radarConst << endl;
  out << "  calSlope: " << _calSlope << endl;
  out << "  calOffset1km: " << _calOffset1km << endl;
  out << "  mdsCount: " << _mdsCount << endl;
  out << "  atmosAtten: " << _atmosAtten << endl;

  out << "=====================" << endl;

}
  
//////////////////////////////////////////
// compute range correction table

void Rdas2Dsr::_computeRangeCorrTable(int nGates)

{

  // check if we need a new table

  if (_rangeCorrection != NULL &&
      nGates == _rCorrNGates &&
      _startRange == _rCorrStartRange &&
      _gateSpacing == _rCorrGateSpacing) {
    return;
  }

  // allocate table as required

  if (_rangeCorrection == NULL) {
    _rangeCorrection = new double[nGates];
    _range = new double[nGates];
  } else {
    if (nGates != _rCorrNGates) {
      delete[] _rangeCorrection;
      _rangeCorrection = new double[nGates];
      delete[] _range;
      _range = new double[nGates];
    }
  }

  // compute table

  for (int i = 0; i < nGates; i++) {
    _range[i] = _startRange + i * _gateSpacing;
    double rangeMeters = _range[i] * 1000.0;
    double log10Range = log10(rangeMeters) - 3.0;
    _rangeCorrection[i] =
      20.0 * log10Range + rangeMeters * _atmosAtten;
  }
  
  // save state

  _rCorrNGates = nGates;
  _rCorrStartRange = _startRange;
  _rCorrGateSpacing = _gateSpacing;
  
}
				      
