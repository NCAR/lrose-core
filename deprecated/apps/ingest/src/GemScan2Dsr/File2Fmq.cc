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
// File2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
///////////////////////////////////////////////////////////////
//
// File2Fmq takes data from a Gematronik scan radar file and
// writes it to a radar FMQ
//
////////////////////////////////////////////////////////////////

#include "File2Fmq.hh"
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/umisc.h>
#include <toolsa/uusleep.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cmath>
using namespace std;

// Constructor

File2Fmq::File2Fmq(const Params &params,
		   DsRadarQueue &r_queue,
		   const char *input_path,
		   int vol_num) :
  _params(params),
  _rQueue(r_queue),
  _inputPath(input_path),
  _volNum(vol_num)
  
{

  _nFields = 1;
  _nTilts = 1;

  _nAz = 360;
  _minAz = 0.0;
  _deltaAz = 1.0;

  _dbzScale = 0.5;
  _dbzBias = -32.0;
  _velScale = 0.1;
  _velBias = -12.8;
  _widthScale = 0.1;
  _widthBias = 0.0;
  _zdrScale = 0.125;
  _zdrBias = -8.0;

  _fieldName = "DBZ";
  _fieldUnits = "dBZ";
  _scale = 0.5;
  _bias = -32.0;

  // initialize from params

  _radarId = 0;
  _radarName = "unknown";
  _samplesPerBeam = _params.radar.samples_per_beam;
  _polarization = _params.radar.polarization;
  _altitude = _params.radar_altitude;
  _latitude = _params.radar_latitude;
  _longitude = _params.radar_longitude;
  _gateSpacing = 0.25;
  _startRange = 0.125;
  _beamWidth = _params.radar.beam_width;
  _pulseWidth = _params.radar.pulse_width;
  _prf = _params.radar.prf;
  _wavelength = _params.radar.wavelength;
  _radarConstant = _params.radar.radar_constant;
  _xmitPeakPwr = _params.radar.xmit_peak_pwr;
  _receiverMds = _params.radar.receiver_mds;
  _receiverGain = _params.radar.receiver_gain;
  _antennaGain = _params.radar.antenna_gain;
  _systemGain = _params.radar.system_gain;
  _useNow = (_params.mode == Params::SIMULATE);

  _fieldData = NULL;
  
}

// destructor

File2Fmq::~File2Fmq()

{

  if (_fieldData) {
    delete[] _fieldData;
  }

}

//////////////////////////////////////////////
// read data in from file
  
int File2Fmq::read()

{
  
  if (_params.debug) {
    cerr << "Processing file: " << _inputPath << endl;
  }
  
  // open file

  FILE *in;
  if ((in = fopen(_inputPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - File2Fmq::read" << endl;
    cerr << "  Cannot open file: " << _inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read headers into a vector of string

  _headers.clear();
  string hdr;
  int cc = fgetc(in);
  if (cc != 0x01) {
    cerr << "ERROR - File2Fmq::read" << endl;
    cerr << "  File should start with ^A, file: " << _inputPath << endl;
    fclose(in);
    return -1;
  }

  while (!feof(in)) {
    cc = fgetc(in);
    if (cc == EOF) {
      cerr << "ERROR - File2Fmq::read" << endl;
      cerr << "  Premature EOF, file: " << _inputPath << endl;
      fclose(in);
      return -1;
    }
    if (cc == 0x03) {
      break;
    }
    if (cc == 0x0D) {
      if (hdr.size() > 0) {
	_headers.push_back(hdr);
	hdr.clear();
      }
    } else if (cc >= 0x20) {
      // printable characters
      hdr += (char) cc;
    }
  }
  
  if (_params.debug) {
    _printHeaders(cerr);
  }

  // interpret the headers
  
  if (_interpretHeaders()) {
    cerr << "ERROR - File2Fmq::read" << endl;
    cerr << "  Bad headers - cannot interpret" << endl;
    fclose(in);
    return -1;
  }
  
  if (_params.debug) {
    _printMetaData(cerr);
  }

  // read in field data

  _fieldData = new ui08[_nBytesData];
  if (ufread(_fieldData, 1, _nBytesData, in) != _nBytesData) {
    cerr << "ERROR - File2Fmq::read" << endl;
    cerr << "  Cannot read in field data, nBytesData: "
	 << _nBytesData << endl;
    fclose(in);
    return -1;
  }

  // success

  fclose(in);
  return 0;

}

//////////////////////////////////////////////
// write data to queue
  
int File2Fmq::write()

{

  // start of volume flag

  _rQueue.putStartOfVolume(_volNum, _time);

  for (int itilt = 0; itilt < _nTilts; itilt++) {


    if (_params.debug) {
      cerr << "Processing tilt number: " << itilt << ", Time: " << _time << endl;
    }

    // start of tilt flag
    
    _rQueue.putStartOfTilt(itilt, _time);

    // radar and field params
    
    if (_writeParams()) {
      cerr << "ERROR - File2Fmq::write" << endl;
      cerr << "  Cannot write the params to the queue" << endl;
      cerr << "  File: " << _inputPath << endl;
      return -1;
    }

    // beams
    
    if (_writeBeams(itilt)) {
      cerr << "ERROR - File2Fmq::write" << endl;
      cerr << "  Cannot write the beams to queue" << endl;
      cerr << "  Tilt num: " << itilt << endl;
      cerr << "  File: " << _inputPath << endl;
      return -1;
    }

    // end of tilt flag
    
    _rQueue.putEndOfTilt(itilt, _time);

    // send heartbeat

    PMU_auto_register("Writing to FMQ");

  } // itilt
  
  // end of volume flag

  _rQueue.putEndOfVolume(_volNum, _time);

  return 0;

}

//////////////////////////////////////////////
// print the headers
  
void File2Fmq::_printHeaders(ostream &out)

{

  out << "Headers: " << endl;
  for (int ii = 0; ii < (int) _headers.size(); ii++) {
    out << _headers[ii] << endl;
  }

}

//////////////////////////////////////////////
// interpret the header data
  
int File2Fmq::_interpretHeaders()

{

  int iret = 0;
  string hdr;

  // version

  if (_findHeader("H1", hdr) == 0) {
    if (sscanf(hdr.c_str(), "%d", &_version) != 1) {
      cerr << "WARNING - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read file version number" << endl;
    }
  }
  
  // file type

  if (_findHeader("H3", hdr)) {
    iret = -1;
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not read file type" << endl;
  } else {
    if (sscanf(hdr.c_str(), "%d", &_fileType) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read file type" << endl;
    }
    if (_fileType != 2) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  File type incorrect: " << _version << endl;
      cerr << "  Should be: 2 = SCAN data " << endl;
    }
  }

  // date and time

  _time = 0;

  if (_findHeader("F5", hdr)) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not read date" << endl;
    iret = -1;
  } else {
    if (sscanf(hdr.c_str(), "%d %d %d", &_day, &_month, &_year) != 3) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read date" << endl;
      iret = -1;
    }
  }
  
  if (_findHeader("F6", hdr)) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not read time" << endl;
    iret = -1;
  } else {
    if (sscanf(hdr.c_str(), "%d %d %d", &_sec, &_min, &_hour) != 3) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read time" << endl;
      iret = -1;
    }
  }

  if (_useNow) {
    time_t now = time(NULL);
    DateTime dtime(now);
    _year  = dtime.getYear();
    _month = dtime.getMonth();
    _day   = dtime.getDay();
    _hour  = dtime.getHour();
    _min   = dtime.getMin();
    _sec   = dtime.getSec();
  }

  if (iret == 0 || _useNow) {
    DateTime dtime(_year, _month, _day, _hour, _min, _sec);
    _time = dtime.utime();
  }
  
  // radar name

  if (_findHeader("H8", hdr) == 0) {
    char id[1024];
    if (sscanf(hdr.c_str(), "%s", id) != 1) {
      cerr << "WARNING - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read radar name" << endl;
    } else {
      _radarName = id;
    }
  }
  
  // number of gates

  _nGates = 0;
  if (_findHeader("R1", hdr) == 0) {
    if (sscanf(hdr.c_str(), "%d", &_nGates) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read nGates" << endl;
      iret = -1;
    }
  }
  
  // number of data bytes

  _nBytesData = 0;
  if (_findHeader("R8", hdr) == 0) {
    if (sscanf(hdr.c_str(), "%d", &_nBytesData) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read nBytesData" << endl;
      iret = -1;
    }
  }

  // lat and lon
  
  if (!_params.override_radar_location) {
    if (_findHeader("F4", hdr) == 0) {
      double lat, lon;
      if (sscanf(hdr.c_str(), "%lg %lg", &lon, &lat) != 2) {
	cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
	cerr << "  Could not read lon/lat postion" << endl;
	iret = -1;
      } else {
	_longitude = lon;
	_latitude = lat;
      }
    }
  }
  
  // number of tilts

  if (_findHeader("A9", hdr)) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not find nTilts" << endl;
    iret = -1;
  } else {
    if (sscanf(hdr.c_str(), "%d", &_nTilts) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read nTilts" << endl;
      iret = -1;
    }
  }

  // delta azimuth

  //   if (_findHeader("A3", hdr) == 0) {
  //     if (sscanf(hdr.c_str(), "%lg", &_deltaAz) != 1) {
  //       cerr << "WARNING - File2Fmq::_interpretHeaders" << endl;
  //       cerr << "  Could not delta azimuth" << endl;
  //       iret = -1;
  //     }
  //   }

  // data scale and bias

  double dbzMin = -31.5, velMin = -6.5, widthMin = 0, zdrMin = -8;
  if (_findHeader("P1", hdr) == 0) {
    if (sscanf(hdr.c_str(), "%lg %lg %lg %lg",
	       &dbzMin, &velMin, &widthMin, &zdrMin) != 4) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not find min data values" << endl;
      iret = -1;
    } 
  }
  double dbzMax = 95.5, velMax = 6.5, widthMax = 6.5, zdrMax = 24;
  if (_findHeader("P2", hdr) == 0) {
    if (sscanf(hdr.c_str(), "%lg %lg %lg %lg",
	       &dbzMax, &velMax, &widthMax, &zdrMax) != 4) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not find max data values" << endl;
      iret = -1;
    } 
  }
  if (iret == 0) {
    _dbzScale = (dbzMax - dbzMin) / 254.0;
    _dbzBias = dbzMin - _dbzScale;
    _velScale = (velMax - velMin) / 254.0;
    _velBias = velMin - _velScale;
    _widthScale = widthMax / 256.0;
    _widthBias = 0.0;
    _zdrScale = (zdrMax - zdrMin) / 254.0;
    _zdrBias = zdrMin - _zdrScale;
  }

  // data field identification

  int fieldId = 1;
  if (_findHeader("F9", hdr)) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not read field ID" << endl;
    iret = -1;
  } else {
    if (sscanf(hdr.c_str(), "%d", &fieldId) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read field ID" << endl;
      iret = -1;
    }
  }

  if (iret == 0) {
    switch (fieldId) {
    case 0:
      _fieldName = "VEL";
      _fieldUnits = "m/s";
      _scale = _velScale;
      _bias = _velBias;
      break;
    case 1:
      _fieldName = "DBZ";
      _fieldUnits = "dBZ";
      _scale = _dbzScale;
      _bias = _dbzBias;
      break;
    case 2:
      _fieldName = "SPW";
      _fieldUnits = "m/s";
      _scale = _widthScale;
      _bias = _widthBias;
      break;
    case 3:
      _fieldName = "UncorrDBZ";
      _fieldUnits = "dBZ";
      _scale = _dbzScale;
      _bias = _dbzBias;
      break;
    case 4:
      _fieldName = "ClutFlag";
      _fieldUnits = "";
      _scale = 1.0;
      _bias = 0.0;
      break;
    case 5:
      _fieldName = "ZDR";
      _fieldUnits = "dB";
      _scale = _zdrScale;
      _bias = _zdrBias;
      break;
    }
  }

  // data compression

  _compressed = 0;
  if (_findHeader("F3", hdr)) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not read compression flag" << endl;
    iret = -1;
  } else {
    if (sscanf(hdr.c_str(), "%d", &_compressed) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read compression flag" << endl;
      iret = -1;
    }
  }

  if (iret == 0 && _compressed != 0) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Cannot deal with compressed data" << endl;
    iret = -1;
  }

  // gate spacing

  if (_findHeader("P5", hdr)) {
    cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
    cerr << "  Could not read gate spacing" << endl;
    iret = -1;
  } else {
    if (sscanf(hdr.c_str(), "%lg", &_gateSpacing) != 1) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read gate spacing" << endl;
      iret = -1;
    }
  }
  _startRange = _gateSpacing / 2.0;

  // max range
  
  _maxRange = _nGates * _gateSpacing;
  if (_findHeader("P4", hdr) == 0) {
    double range;
    if (sscanf(hdr.c_str(), "%lg", &range) != 1) {
      cerr << "WARNING - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not read max range" << endl;
    } else {
      _maxRange = range;
    }
  }

  // elevation angles

  _elevations.clear();
  for (int ii = 0; ii < _nTilts; ii++) {
    char label[32];
    sprintf(label, "W%.2d", ii + 1);
    if (_findHeader(label, hdr)) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Could not find W label for elevation # " << ii << endl;
      iret = -1;
    } else {
      double el;
      if (sscanf(hdr.c_str(), "%lg", &el) != 1) {
	cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
	cerr << "  Could not decode elev # " << ii << endl;
	iret = -1;
      } else {
	_elevations.push_back(el);
      }
    }
  }

  // check data size

  if (iret == 0) {
    int nBytesExpected = _nTilts * _nAz * _nGates;
    if (_params.debug) {
      cerr << "nBytesExpected: " << nBytesExpected << endl;
    }
    if (nBytesExpected != _nBytesData) {
      cerr << "ERROR - File2Fmq::_interpretHeaders" << endl;
      cerr << "  Incorrect nbytes for data" << endl;
      cerr << "  nBytesData: " << _nBytesData << endl;
      cerr << "  nBytesExpected: " << nBytesExpected << endl;
      iret = -1;
    }
  }
  
  return iret;
  
}

//////////////////////////////////////////////
// print the parameters
  
void File2Fmq::_printMetaData(ostream &out)

{
  
  out << "File meta-data " << endl;
  out << "  _version: " << _version << endl;
  out << "  _fileType: " << _fileType << endl;
  out << "  _time: " << DateTime::strn(_time) << endl;
  out << "  _nGates: " << _nGates << endl;
  out << "  _nTilts: " << _nTilts << endl;
  out << "  _nBytesData: " << _nBytesData << endl;
  out << "  _radarId: " << _radarId << endl;
  out << "  _radarName: " << _radarName << endl;
  out << "  _samplesPerBeam: " << _samplesPerBeam << endl;
  out << "  _polarization: " << _polarization << endl;
  out << "  _altitude: " << _altitude << endl;
  out << "  _latitude: " << _latitude << endl;
  out << "  _longitude: " << _longitude << endl;
  out << "  _gateSpacing: " << _gateSpacing << endl;
  out << "  _startRange: " << _startRange << endl;
  out << "  _beamWidth: " << _beamWidth << endl;
  out << "  _pulseWidth: " << _pulseWidth << endl;
  out << "  _prf: " << _prf << endl;
  out << "  _wavelength: " << _wavelength << endl;
  out << "  _radarConstant: " << _radarConstant << endl;
  out << "  _xmitPeakPwr: " << _xmitPeakPwr << endl;
  out << "  _receiverMds: " << _receiverMds << endl;
  out << "  _receiverGain: " << _receiverGain << endl;
  out << "  _antennaGain: " << _antennaGain << endl;
  out << "  _systemGain: " << _systemGain << endl;
  out << "  _deltaAz: " << _deltaAz << endl;
  out << "  _dbzScale: " << _dbzScale << endl;
  out << "  _dbzBias: " << _dbzBias << endl;
  out << "  _velScale: " << _velScale << endl;
  out << "  _velBias: " << _velBias << endl;
  out << "  _widthScale: " << _widthScale << endl;
  out << "  _widthBias: " << _widthBias << endl;
  out << "  _zdrScale: " << _zdrScale << endl;
  out << "  _zdrBias: " << _zdrBias << endl;
  out << "  _fieldName: " << _fieldName << endl;
  out << "  _fieldUnits: " << _fieldUnits << endl;
  out << "  _scale: " << _scale << endl;
  out << "  _bias: " << _bias << endl;
  out << "  _compressed: " << _compressed << endl;
  out << "  _maxRange: " << _maxRange << endl;

  out << "  _elevations:";
  for (int ii = 0; ii < (int) _elevations.size(); ii++) {
    out << " " << _elevations[ii];
  }
  out << endl;

}

//////////////////////////////////////////////
// find header with specified tag
  
int File2Fmq::_findHeader(const string &tag,
			  string &hdr)

{
  
  for (int ii = 0; ii < (int) _headers.size(); ii++) {
    const string &candidate = _headers[ii];
    if (candidate.find(tag, 0) != string::npos) {
      // strip off label and colon
      string colon = ":";
      size_t colonPos = candidate.find(colon, 0);
      if (colonPos != string::npos) {
	int nLeft = candidate.size() - (colonPos + 1);
	hdr.assign(candidate, colonPos + 1, nLeft);
	return 0;
      } else {
	cerr << "ERROR - File2Fmq::_findHeader" << endl;
	cerr << "  Bad format, no colon in: " << candidate << endl;
	return -1;
      }
    }
  }
  cerr << "ERROR - File2Fmq::_findHeader" << endl;
  cerr << "  Could not find hdr with tag: " << tag << endl;
  return -1;

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int File2Fmq::_writeParams()

{

  // Set radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rp = msg.getRadarParams();

  // first get params from the parameter file
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = _nGates;
  rp.samplesPerBeam = _params.radar.samples_per_beam;
  rp.scanType = _params.scan_type_id;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  
  rp.radarConstant = _params.radar.radar_constant;
  rp.altitude = _altitude;
  rp.latitude = _latitude;
  rp.longitude = _longitude;
  
  rp.gateSpacing = _gateSpacing;
  rp.startRange = _startRange;
  rp.horizBeamWidth = _params.radar.beam_width;
  rp.vertBeamWidth = _params.radar.beam_width;
  
  rp.pulseWidth = _params.radar.pulse_width;
  rp.pulseRepFreq = _params.radar.prf;
  rp.wavelength = _params.radar.wavelength;
  
  rp.xmitPeakPower = _params.radar.xmit_peak_pwr;
  rp.receiverMds = _params.radar.receiver_mds;
  rp.receiverGain = _params.radar.receiver_gain;
  rp.antennaGain = _params.radar.antenna_gain;
  rp.systemGain = _params.radar.system_gain;
  
  rp.unambigVelocity = ((rp.wavelength / 100.0) * rp.pulseRepFreq) / 4.0;
  rp.unambigRange = 150000.0 / rp.pulseRepFreq;
  
  rp.radarName = _radarName;
  rp.scanTypeName = _params.scan_type_name;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();
  DsFieldParams* thisParams =
    new DsFieldParams(_fieldName.c_str(), _fieldUnits.c_str(),
		      _scale, _bias);
  fp.push_back(thisParams);
  
  // write the message
  
  if (_rQueue.putDsMsg
      (msg,
       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - File2Fmq::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int File2Fmq::_writeBeams(int tilt_num)
  
{

  int iret = 0;

  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  int nBytesBeam = _nGates;
  int nBytesPpi = nBytesBeam * _nAz;

  // loop through the beams

  for (int iaz = 0; iaz < _nAz; iaz++) {

    // params
    
    beam.dataTime = _time;
    beam.volumeNum = _volNum;
    beam.tiltNum = tilt_num;
    beam.azimuth = iaz;
    beam.elevation = _elevations[tilt_num];
    beam.targetElev = beam.elevation;
    
    // data
    
    const ui08 *data = _fieldData + tilt_num * nBytesPpi + iaz * nBytesBeam;
    beam.loadData(data, nBytesBeam, sizeof(ui08));

    // write the message
    
    if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
      iret = -1;
    }
    
    if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
      umsleep(_params.beam_wait_msecs);
    }
    
  } // iaz

  if (iret) {
    cerr << "ERROR - File2Fmq::writeBeams" << endl;
    cerr << "  Cannot put radar beams to queue" << endl;
  }
  
  return iret;

}

/////////////////////////////////////////////
// Search for a field name in a name list.
// The list is a comma-delimted string.
//
// Returns -1 on failure, position on success

int File2Fmq::_findNameInList(const string &name,
			      const string &list)
  
{

  string comma = ",";
  
  // if no commas, so check entire string
  
  if (list.find(comma, 0) == string::npos) {
    if (name == list) {
      return 0;
    } else {
      return -1;
    }
  }

  // search through subparts
  
  int nn = 0;
  size_t currPos = 0;
  size_t nextPos = 0;
  
  while (nextPos != string::npos) {

    nextPos = list.find(comma, currPos);
    string candidate;
    candidate.assign(list, currPos, nextPos - currPos);

    if (candidate == name) {
      return nn;
    }

    currPos = nextPos + 1;
    nn++;

  } // while

  return -1;

}

