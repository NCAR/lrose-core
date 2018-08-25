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
// GamicVol2Dsr.cc
//
// GamicVol2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2009
//
///////////////////////////////////////////////////////////////
//
// GamicVol2Dsr reads GAMIC volume files and reformats
// the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <dataport/bigend.h>
#include "GamicVol2Dsr.hh"
using namespace std;

const double GamicVol2Dsr::_angleConversion = 360.0 / 65536.0;

// Constructor

GamicVol2Dsr::GamicVol2Dsr(int argc, char **argv)

{

  _input = NULL;
  isOK = true;
  _firstBeamInFile = true;
  _nGatesOut = 0;
  _nGatesOutPrev = 0;
  _volNum = -1;
  
  // set programe name

  _progName = "GamicVol2Dsr";
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
  }

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: GamicVol2Dsr::GamicVol2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: GamicVol2Dsr::GamicVol2Dsr." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - GamicVol2Dsr" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    isOK = false;
    return;
  }

  if (_params.output_fmq_compress) {
    _rQueue.setCompressionMethod(TA_COMPRESSION_ZLIB);
  }

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  return;

}

// destructor

GamicVol2Dsr::~GamicVol2Dsr()

{

  if (_input) {
    delete _input;
  }

  _freeBeams();

  // unregister process

  PMU_auto_unregister();

}

///////////////////////////////
// free up beams memory

int GamicVol2Dsr::_freeBeams()

{
  for (int ii = 0; ii < (int) _beams.size(); ii++) {
    delete _beams[ii];
  }
  _beams.clear();
}

//////////////////////////////////////////////////
// Run

int GamicVol2Dsr::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.mode == Params::SIMULATE) {
    
    // simulate mode - go through the file list repeatedly
    
    while (true) {
      
      char *inputPath;
      _input->reset();
      while ((inputPath = _input->next()) != NULL) {
	PMU_auto_register("Simulate mode");
	if (_processFile(inputPath)) {
	  cerr << "ERROR = GamicVol2Dsr::Run" << endl;
	  cerr << "  Processing file: " << inputPath << endl;
	  iret = -1;
	}
      } // while

    }

  } else {
    
    // loop until end of data
    
    char *inputPath;
    while ((inputPath = _input->next()) != NULL) {
      
      PMU_auto_register("Non-simulate mode");
      
      if (_processFile(inputPath)) {
	cerr << "ERROR = GamicVol2Dsr::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
      
    }

  } // if (_params.mode == Params::SIMULATE)
    
  return iret;

}

///////////////////////////////
// process file

int GamicVol2Dsr::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }
  
  // open file - file closes automatically when inFile goes
  // out of scope
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopenUncompress(input_path, "r")) == NULL) {
    cerr << "ERROR - GamicVol2Dsr::_processFile" << endl;
    cerr << "  Cannot open input file: " << input_path << endl;
    perror("  ");
    return -1;
  }

  // set volume number - increments per file

  _volNum++;

  // read in header
  
  if (_readHeader(in)) {
    cerr << "ERROR - GamicVol2Dsr::_processFile" << endl;
    cerr << "  Reading header, file: " << input_path << endl;
    return -1;
  }

  // read in the tilts
  
  _firstBeamInFile = true;
  _freeBeams();

  for (int itilt = 0; itilt < _nTilts; itilt++) {
    
    if (_readTilt(in, itilt)) {
      cerr << "ERROR - GamicVol2Dsr::_processFile" << endl;
      cerr << "  Reading tilt number: " << itilt << endl;
      cerr << "  file: " << input_path << endl;
      return -1;
    }

    // write tilt

    if (_writeTilt(itilt)) {
      cerr << "ERROR - GamicVol2Dsr::_processFile" << endl;
      cerr << "  Writing FMQ data for tilt number: " << itilt << endl;
      cerr << "  url: " << _params.output_fmq_url << endl;
      return -1;
    }
	
    _freeBeams();

  } // itilt

  // write end of vol flag

  _rQueue.putEndOfVolume(_volNum, _latestBeamTime);

  return 0;
  
}

///////////////////////////////
// read in header

int GamicVol2Dsr::_readHeader(FILE *in)

{

  char label[8];
  MEM_zero(label);
  if (_readBuf(in, "label", label, 4)) {
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "label: " << label << endl;
  }
  if (strncmp(label, "raVM", 4)) {
    cerr << "Unexpected file label: " << label << endl;
    cerr << "  Expecting raVM" << endl;
    return -1;
  }

  // file time

  si32 jday;
  if (_readSi32(in, "jday", jday)) {
    return -1;
  }
  si32 msecsOfDay;
  if (_readSi32(in, "msecsOfDay", msecsOfDay)) {
    return -1;
  }
  _computeTime(jday, msecsOfDay, _fileTime, _fileNanosecs);

  // position

  fl64 dval;
  if (_readFl64(in, "latitude", dval)) {
    return -1;
  }
  _latitude = dval;

  if (_readFl64(in, "longitude", dval)) {
    return -1;
  }
  _longitude = dval;

  if (_readFl64(in, "altitude", dval)) {
    return -1;
  }
  _altitude = dval;

  // volume type - 1 == PPI, 2 == RHI

  si32 ival;
  if (_readSi32(in, "vol_type", ival)) {
    return -1;
  }
  _volType = ival;
  if (_volType == 2) {
    _isRhi = true;
  } else {
    _isRhi = false;
  }

  // scan name

  si32 len1;
  if (_readSi32(in, "len_scan_name", len1)) {
    return -1;
  }
  TaArray<char> buf1_;
  char *buf1 = buf1_.alloc(len1);
  if (_readBuf(in, "buf1", buf1, len1)) {
    return -1;
  }
  _scanName = _getStrFromBuf(buf1, len1);

  // radar name

  si32 len2;
  if (_readSi32(in, "len_radar_name", len2)) {
    return -1;
  }
  TaArray<char> buf2_;
  char *buf2 = buf2_.alloc(len2);
  if (_readBuf(in, "buf2", buf2, len2)) {
    return -1;
  }
  _radarName = _getStrFromBuf(buf2, len2);
  
  // host name

  si32 len3;
  if (_readSi32(in, "host_name_len", len3)) {
    return -1;
  }
  TaArray<char> buf3_;
  char *buf3 = buf3_.alloc(len3);
  if (_readBuf(in, "buf3", buf3, len3)) {
    return -1;
  }
  _hostName = _getStrFromBuf(buf3, len3);
  
  // beam width

  if (_readFl64(in, "beam_width_vert", dval)) {
    return -1;
  }
  _beamWidthVert = dval;

  if (_readFl64(in, "beam_width_horiz", dval)) {
    return -1;
  }
  _beamWidthHoriz = dval;

  // ntilts

  ui16 sval;
  if (_readUi16(in, "ntilts", sval)) {
    return -1;
  }
  _nTilts = sval;

  if (_params.debug) {
    cerr << "  fileTime: " << DateTime::strm(_fileTime) << endl;
    cerr << "  latitude: " << _latitude << endl;
    cerr << "  longitude: " << _longitude << endl;
    cerr << "  altitude: " << _altitude << endl;
    cerr << "  volType: " << _volType << endl;
    cerr << "  isRhi: " << (const char *) (_isRhi? "Y" : "N") << endl;
    cerr << "  scanName: " << _scanName << endl;
    cerr << "  radarName: " << _radarName << endl;
    cerr << "  hostName: " << _hostName << endl;
    cerr << "  beamWidthVert: " << _beamWidthVert << endl;
    cerr << "  beamWidthHoriz: " << _beamWidthHoriz << endl;
    cerr << "  nTilts: " << _nTilts << endl;
  }

  return 0;

}

///////////////////////////////
// read in tilt

int GamicVol2Dsr::_readTilt(FILE *in, int tiltNum)

{

  if (_params.debug) {
    cerr << "Reading tilt number: " << tiltNum << endl;
  }

  // tilt type
  
  ui16 sval;
  if (_readUi16(in, "ttype", sval)) {
    return -1;
  }
  _tiltType = sval;
  if (_tiltType == 2) {
    _isRhi = true;
  } else {
    _isRhi = false;
  }

  // read in magic
  
  char magik[8];
  MEM_zero(magik);
  if (_readBuf(in, "magik", magik, 4)) {
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "magik: " << magik << endl;
  }
  if (strncmp(magik, "raSC", 4)) {
    cerr << "Unexpected tilt magik: " << magik << endl;
    cerr << "  Expecting raSC" << endl;
    return -1;
  }

  // max range, gate spacing

  si32 ival;
  if (_readSi32(in, "irange", ival)) {
    return -1;
  }
  _maxRange = ival / 1000.0;
  
  if (_readSi32(in, "gate_spacing", ival)) {
    return -1;
  }
  _gateSpacingMeters = ival;
  _gateSpacingKm = ival / 1000.0;
  _gateSpacingKmOut = _gateSpacingKm;
  if (_params.remap_to_constant_gate_spacing) {
    _gateSpacingKmOut = _params.constant_gate_spacing_km;
  }
  
  // number of samples in range and in the dwell, and PRF
  
  if (_readSi32(in, "irangesamples", ival)) {
    return -1;
  }
  _nAveragingInRange = ival;
  
  if (_readSi32(in, "iprf", ival)) {
    return -1;
  }
  _prf = ival;
  
  if (_readSi32(in, "itimesample", ival)) {
    return -1;
  }
  _nSamples = ival;

  // delta angle

  fl64 dval;
  if (_readFl64(in, "anglestep", dval)) {
    return -1;
  }
  _deltaAngle = dval;

  // flags

  if (_readSi32(in, "iunfolding", ival)) {
    return -1;
  }
  _unfoldingFlag = ival;
  
  char bval = 0;
  if (_readChar(in, "longpulse", bval)) {
    return -1;
  }
  _longPulseFlag = bval;
  if (_longPulseFlag) {
    _pulseWidthUs = _params.long_pulse_width_us;
  } else {
    _pulseWidthUs = _params.short_pulse_width_us;
  }
  
  if (_readSi32(in, "clutter", ival)) {
    return -1;
  }
  _clutterFlag = ival;
  
  if (_readSi32(in, "ifilter", ival)) {
    return -1;
  }
  _filterNum = ival;
  
  if (_readChar(in, "anglesync", bval)) {
    return -1;
  }
  _angleSyncFlag = bval;
  
  if (_readFl64(in, "maxDBZ", dval)) {
    return -1;
  }
  _maxDbz = dval;
  
  if (_readFl64(in, "minDBZ", dval)) {
    return -1;
  }
  _minDbz = dval;

  if (_minDbz > _maxDbz) {
    double tmp = _minDbz;
    _minDbz = _maxDbz;
    _maxDbz = tmp;
  }
  
  if (_readFl64(in, "nyquist", dval)) {
    return -1;
  }
  _nyquist = dval;
  
  if (_readFl64(in, "wavelength", dval)) {
    return -1;
  }
  _wavelengthMeters = dval;
  
  if (_readUi16(in, "beam_count", sval)) {
    return -1;
  }
  _nBeams = sval;

  if (_params.debug) {
    cerr << "==== tilt number: " << tiltNum << " ====" << endl;
    cerr << "  maxRange: " << _maxRange << endl;
    cerr << "  gateSpacingKm: " << _gateSpacingKm << endl;
    cerr << "  gateSpacingKmOut: " << _gateSpacingKmOut << endl;
    cerr << "  nAveragingInRange: " << _nAveragingInRange << endl;
    cerr << "  prf: " << _prf << endl;
    cerr << "  nSamples: " << _nSamples << endl;
    cerr << "  deltaAngle: " << _deltaAngle << endl;
    cerr << "  unfoldingFlag: " << _unfoldingFlag << endl;
    cerr << "  longPulseFlag: " << _longPulseFlag << endl;
    cerr << "  pulseWidthUs: " << _pulseWidthUs << endl;
    cerr << "  clutterFlag: " << _clutterFlag << endl;
    cerr << "  filterNum: " << _filterNum << endl;
    cerr << "  angleSync: " << _angleSyncFlag << endl;
    cerr << "  maxDbz: " << _maxDbz << endl;
    cerr << "  minDbz: " << _minDbz << endl;
    cerr << "  nyquist: " << _nyquist << endl;
    cerr << "  wavelengthMeters: " << _wavelengthMeters << endl;
    cerr << "  nBeams: " << _nBeams << endl;
  } 

  // loop through beams

  for (int ibeam = 0; ibeam < _nBeams; ibeam++) {

    Beam *beam = new Beam(_progName, _params);

    beam->nFieldsIn = 4;
    beam->nFieldsOut = _params.output_fields_n;

    if (_readUi16(in, "bin_type", sval)) {
      return -1;
    }
    beam->binType = sval;
    
    if (_readUi16(in, "az", sval)) {
      return -1;
    }
    beam->azimuth = sval * _angleConversion;

    if (_readUi16(in, "el", sval)) {
      return -1;
    }
    beam->elevation = sval * _angleConversion;

    si32 jday;
    if (_readSi32(in, "jday", jday)) {
      return -1;
    }
    si32 msecsOfDay;
    if (_readSi32(in, "msecsOfDay", msecsOfDay)) {
      return -1;
    }
    time_t utime;
    int nanosecs;
    _computeTime(jday, msecsOfDay, utime, nanosecs);
    beam->time = utime;
    beam->nanosecs = nanosecs;

    // write start of volume flag if first beam in file
    
    if (_firstBeamInFile) {
      _rQueue.putStartOfVolume(_volNum, utime);
    }

    // save beam time for later

    _latestBeamTime;
    _latestBeamNanosecs;

    if (_readSi32(in, "nGates", ival)) {
      return -1;
    }
    beam->nGatesIn = ival;
  
    if (_readSi32(in, "nBytesBeam", ival)) {
      return -1;
    }
    beam->nBytesIn = ival;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "------------------------------------" << endl;
      cerr << "  beamTime: " << DateTime::strm(beam->time)
	   << "." << beam->nanosecs / 1000 << endl;
      cerr << "  az: " << beam->azimuth << endl;
      cerr << "  el: " << beam->elevation << endl;
      cerr << "  binType: " << beam->binType << endl;
      cerr << "  nFieldsIn: " << beam->nFieldsIn << endl;
      cerr << "  nGatesIn: " << beam->nGatesIn << endl;
      cerr << "  nBytesIn: " << beam->nBytesIn << endl;
    }

    // read in data

    TaArray<char> inBuf_;
    char *inBuf = inBuf_.alloc(beam->nBytesIn);
    if (_readBuf(in, "beam_data", inBuf, beam->nBytesIn)) {
      return -1;
    }

    if (_params.remap_to_constant_gate_spacing) {
      _remapOutputData(inBuf, beam);
    } else {
      _loadOutputData(inBuf, beam);
    }

    _firstBeamInFile = false;
    _beams.push_back(beam);

  } // ibeam
  
  // tilt time

  si32 jday;
  if (_readSi32(in, "jday", jday)) {
    return -1;
  }
  si32 msecsOfDay;
  if (_readSi32(in, "msecsOfDay", msecsOfDay)) {
    return -1;
  }
  _computeTime(jday, msecsOfDay, _tiltTime, _tiltNanosecs);

  if (_readUi16(in, "startAngle", sval)) {
    return -1;
  }
  _startAngle = sval * _angleConversion;
  
  if (_readUi16(in, "stopAngle", sval)) {
    return -1;
  }
  _stopAngle = sval * _angleConversion;
  
  if (_readUi16(in, "elev", sval)) {
    return -1;
  }
  double telev = sval * _angleConversion;
  _targetAngle = ((int) (telev * 100.0 + 0.5)) / 100.0;
  
  if (_readSi32(in, "startrange", ival)) {
    return -1;
  }
  _startRange = ival;
  
  if (_params.debug) {
    cerr << "---- tilt number: " << tiltNum << " (cont) ----" << endl;
    cerr << "  tiltTime: " << DateTime::strm(_tiltTime)
	 << "." << _tiltNanosecs / 1000 << endl;
    cerr << "  startAngle: " << _startAngle << endl;
    cerr << "  stopAngle: " << _stopAngle << endl;
    cerr << "  targetAngle: " << _targetAngle << endl;
    cerr << "  startRange: " << _startRange << endl;
    cerr << "=======================================" << endl;
  } 

  return 0;

}     

///////////////////////////////
// read a byte

int GamicVol2Dsr::_readChar(FILE *in, const string &label, char &val)

{
  if (fread(&val, sizeof(char), 1, in) != 1) {
    cerr << "ERROR reading item: " << label << endl;
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << label << ": " << val << endl;
  }
  return 0;
}
    
///////////////////////////////
// read an unsigned short

int GamicVol2Dsr::_readUi16(FILE *in, const string &label, ui16 &val)

{
  if (fread(&val, sizeof(ui16), 1, in) != 1) {
    cerr << "ERROR reading item: " << label << endl;
    return -1;
  }
  BE_to_array_16(&val, sizeof(ui16));
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << label << ": " << val << endl;
  }
  return 0;
}
    
///////////////////////////////
// read an integer

int GamicVol2Dsr::_readSi32(FILE *in, const string &label, si32 &val)

{
  if (fread(&val, sizeof(si32), 1, in) != 1) {
    cerr << "ERROR reading item: " << label << endl;
    return -1;
  }
  BE_to_array_32(&val, sizeof(si32));
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << label << ": " << val << endl;
  }
  return 0;
}
    
///////////////////////////////
// read a float

int GamicVol2Dsr::_readFl32(FILE *in, const string &label, fl32 &val)

{
  if (fread(&val, sizeof(fl32), 1, in) != 1) {
    cerr << "ERROR reading item: " << label << endl;
    return -1;
  }
  BE_to_array_32(&val, sizeof(fl32));
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << label << ": " << val << endl;
  }
  return 0;
}
    
///////////////////////////////
// read a double

int GamicVol2Dsr::_readFl64(FILE *in, const string &label, fl64 &val)

{
  if (fread(&val, sizeof(fl64), 1, in) != 1) {
    cerr << "ERROR reading item: " << label << endl;
    return -1;
  }
  BE_to_array_64(&val, sizeof(fl64));
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << label << ": " << val << endl;
  }
  return 0;
}
    
///////////////////////////////
// read a char buffer

int GamicVol2Dsr::_readBuf(FILE *in, const string &label,
			   char *buf, int len)

{
  if (fread(buf, 1, len, in) != len) {
    cerr << "ERROR reading item: " << label << endl;
    return -1;
  }
#ifdef NOTNOW
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr <<  "array " << label << ": ";
    for (int ii = 0; ii < len; ii++) {
      cerr << buf[ii];
    }
    cerr << endl;
  }
#endif
  return 0;
}
    
///////////////////////////////
// skip bytes

int GamicVol2Dsr::_skipBytes(FILE *in, int len)

{
  TaArray<char> buf_;
  char *buf = buf_.alloc(len);
  if (_readBuf(in, "skipping", buf, len)) {
    cerr << "ERROR skipping nbytes: " << len << endl;
    return -1;
  }
  return 0;
}
    
///////////////////////////////
// get string from buffer
// take every second character, the others are nulls

string GamicVol2Dsr::_getStrFromBuf(char *buf, int len)
  
{
  string str;
  for (int ii = 0; ii < len / 2; ii++) {
    str += buf[ii * 2 + 1];
  }
  return str;
}
    
///////////////////////////////////////////////////
// compute date and time from jday and msecsOfDay

int GamicVol2Dsr::_computeTime(int jday,
			       int msecsOfDay,
			       time_t &utime,
			       int &nanosecs)

{

  int uday = jday - 2440588;
  utime = uday * 86400 + msecsOfDay / 1000;
  nanosecs = (msecsOfDay % 1000) * 1000000;

}

///////////////////////////////
// write out data for tilt

int GamicVol2Dsr::_writeTilt(int tiltNum)

{

  // write radar and field params
  
  _writeParams();

  // start of tilt flag

  _rQueue.putStartOfTilt(tiltNum, _beams[0]->time);

  // beam data
  
  for (int ii = 0; ii < (int) _beams.size(); ii++) {
    // write beam
    if (_writeBeam(tiltNum, *_beams[ii])) {
      return -1;
    }
  }

  // end of tilt flag

  _rQueue.putEndOfTilt(tiltNum, _beams[_beams.size()-1]->time);

  return 0;

}

///////////////////////////////
// write radar and field params

int GamicVol2Dsr::_writeParams()

{

  DsRadarMsg msg;

  // load up radar params

  DsRadarParams &rParams = msg.getRadarParams();
  _loadRadarParams(rParams);

   if (_params.debug >= Params::DEBUG_VERBOSE) {
     rParams.print(cerr);
   }
  
  // load up field params

  vector< DsFieldParams* > &fieldParams = msg.getFieldParams();
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    double scale = 1.0, bias = 0.0;
    if (_params._output_fields[ii].ftype == Params::DBZ_TYPE) {
      scale = (_maxDbz - _minDbz) / 255.0;
      bias = _minDbz;
    } else if (_params._output_fields[ii].ftype == Params::VEL_TYPE) {
      scale = _nyquist / 128.0;
      bias = -_nyquist;
    } else if (_params._output_fields[ii].ftype == Params::WIDTH_TYPE) {
      bias = 0.0;
      scale = _nyquist / 255.0;
    }
    // create new field - this will be deleted by the destructor of msg
    DsFieldParams *fparams =
      new DsFieldParams(_params._output_fields[ii].name,
			_params._output_fields[ii].units,
			scale, bias);
    fieldParams.push_back(fparams);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fparams->print(cerr);
    }
  } // ii
  
  // send the params

  if (_rQueue.putDsMsg
      (msg,
       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - Bprp2Dsr::_writeRadarAndFieldParams" << endl;
    cerr << "  Cannot put radar and field params message to FMQ" << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////
// write beam

int GamicVol2Dsr::_writeBeam(int tiltNum, Beam &beam)

{

  DsRadarMsg msg;
  DsRadarBeam &dsBeam = msg.getRadarBeam();

  // params

  time_t beamTime = beam.time;
  int nanosecs = beam.nanosecs;
  if (_params.mode == Params::SIMULATE) {
    // set time to current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    beamTime = tv.tv_sec;
    nanosecs = tv.tv_usec * 1000;
  } else if (_params.time_correction != 0) {
    beamTime += _params.time_correction;
  }

  dsBeam.dataTime = beamTime;
  dsBeam.nanoSecs = nanosecs;
  dsBeam.volumeNum = _volNum;
  dsBeam.tiltNum = tiltNum;
  dsBeam.azimuth = beam.azimuth;
  dsBeam.elevation = beam.elevation;
  dsBeam.nSamples = _nSamples;
  if (_angleSyncFlag) {
    dsBeam.beamIsIndexed = true;
    dsBeam.angularResolution = _deltaAngle;
  }

  dsBeam.measXmitPowerDbmH = -9999;
  dsBeam.measXmitPowerDbmV = -9999;

  if (_isRhi) {
    dsBeam.targetAz = _targetAngle;
    dsBeam.targetElev = NAN;
    dsBeam.scanMode = DS_RADAR_RHI_MODE;
  } else {
    dsBeam.targetElev = _targetAngle;
    dsBeam.targetAz = NAN;
    dsBeam.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  }

  // data

  dsBeam.loadData(beam.fieldData.buf(), beam.nBytesOut, 1);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    dsBeam.print(cerr);
  }
  
  // write the message
  
  int iret = 0;
  if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
    iret = -1;
  }

  if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }

  return iret;

}

////////////////////
// load radar params

void GamicVol2Dsr::_loadRadarParams(DsRadarParams &rParams)

{

  double missing = -9999.0;
  
  rParams.radarId = _params.radar.radar_id;
  rParams.radarType = DS_RADAR_GROUND_TYPE;
  rParams.numFields = _params.output_fields_n;
  rParams.numGates = _nGatesOut;
  rParams.samplesPerBeam = _nSamples;
  rParams.scanType = 0;
  if (_isRhi) {
    rParams.scanMode = DS_RADAR_RHI_MODE;
  } else {
    rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  }
  
  rParams.polarization = _params.radar.polarization;
  
  if (_params.radar.altitude == missing) {
    rParams.altitude = _altitude / 1000.0;
  } else {
    rParams.altitude = _params.radar.altitude;
  }

  if (_params.radar.latitude == missing) {
    rParams.latitude = _latitude;
  } else {
    rParams.latitude = _params.radar.latitude;
  }

  if (_params.radar.longitude == missing) {
    rParams.longitude = _longitude;
  } else {
    rParams.longitude = _params.radar.longitude;
  }

  if (strlen(_params.radar.radar_name) == 0) {
    rParams.radarName = _radarName;
  } else {
    rParams.radarName = _params.radar.radar_name;
  }

  rParams.startRange = _gateSpacingKmOut / 2.0;
  rParams.gateSpacing = _gateSpacingKmOut;
  
  rParams.horizBeamWidth = _beamWidthHoriz;
  rParams.vertBeamWidth = _beamWidthVert;

  rParams.radarConstant = _params.radar.radar_constant;
  rParams.xmitPeakPower = _params.radar.xmit_peak_pwr;
  rParams.receiverMds = _params.radar.receiver_mds;
  rParams.receiverGain = _params.radar.receiver_gain;
  rParams.antennaGain = _params.radar.antenna_gain;
  rParams.systemGain = _params.radar.system_gain;

  rParams.pulseWidth = _pulseWidthUs;
  rParams.wavelength = _wavelengthMeters * 100.0;
  rParams.pulseRepFreq = _prf;
  rParams.unambigVelocity = _nyquist;
  rParams.unambigRange = _maxRange;
  
  rParams.scanTypeName = _scanName;

}


///////////////////////////////
// load output data buffer
// using gate spacing in input data

void GamicVol2Dsr::_loadOutputData(const char *inBuf, Beam *beam)

{

  // initialize output buffer
  
  beam->nGatesOut = beam->nGatesIn;
  if (_params.set_ngates_out) {
    beam->nGatesOut = _params.ngates_out;
  }
  beam->nBytesOut = _params.output_fields_n * beam->nGatesOut;
  ui08 *outBuf = beam->fieldData.alloc(beam->nBytesOut);
  memset(outBuf, 0, beam->nBytesOut);
  
  // compute number of gates to actually copy
  
  int nGatesCopy = beam->nGatesOut;
  if (nGatesCopy > beam->nGatesIn) {
    nGatesCopy = beam->nGatesIn;
  }
  
  _nGatesOut = beam->nGatesOut;
  if (_nGatesOutPrev != 0 && _nGatesOutPrev != _nGatesOut) {
    // write params if ngatesOut has changed
    _writeParams();
  }
  _nGatesOutPrev = _nGatesOut;
  
  // load up output data
  
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    
    int inIndex = _params._output_fields[ii].pos_in_file;
    if (inIndex >= beam->nFieldsIn) {
      if (_firstBeamInFile) {
	cerr << "WARNING - output field name: "
	     << _params._output_fields[ii].name << endl;
	cerr << " Position in file out of range: " << inIndex << endl;
	cerr << "  N fields in input file: " << beam->nFieldsIn << endl;
	  cerr << "  Field will be set to missing" << endl;
      }
      continue;
    }
    int outIndex = ii;
    
    for (int jj = 0; jj < nGatesCopy; jj++) {
      outBuf[outIndex] = inBuf[inIndex];
      inIndex += beam->nFieldsIn;
      outIndex += beam->nFieldsOut;
    }
    
  } // ii

}
    
///////////////////////////////////////////////////
// load output data buffer, remapping onto a 
// constant gate spacing

void GamicVol2Dsr::_remapOutputData(const char *inBuf, Beam *beam)

{

  // compute number of output gates

  double maxRangeKm = beam->nGatesIn * _gateSpacingKm;
  _nGatesOut =
    (int) (maxRangeKm / _gateSpacingKmOut + 0.5);
  if (_params.set_ngates_out) {
    _nGatesOut = _params.ngates_out;
  }
  beam->nGatesOut = _nGatesOut;

  // write params if number of output gates has changed
  
  if (_nGatesOutPrev != 0 && _nGatesOutPrev != _nGatesOut) {
    // write params if ngatesOut has changed
    _writeParams();
  }
  _nGatesOutPrev = _nGatesOut;
  
  // allocate output buffer
  
  beam->nBytesOut = _params.output_fields_n * _nGatesOut;
  ui08 *outBuf = beam->fieldData.alloc(beam->nBytesOut);
  memset(outBuf, 0, beam->nBytesOut);
  
  // load up output data
  
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    
    int inIndex = _params._output_fields[ii].pos_in_file;
    if (inIndex >= beam->nFieldsIn) {
      if (_firstBeamInFile) {
	cerr << "WARNING - output field name: "
	     << _params._output_fields[ii].name << endl;
	cerr << " Position in file out of range: " << inIndex << endl;
	cerr << "  N fields in input file: " << beam->nFieldsIn << endl;
	  cerr << "  Field will be set to missing" << endl;
      }
      continue;
    }
    
    double range = _gateSpacingKmOut / 2.0;
    int outIndex = ii;
    for (int jj = 0; jj < _nGatesOut; jj++, range += _gateSpacingKmOut) {
      int inGate = (int) (range / _gateSpacingKm + 0.5);
      int kk = inGate * beam->nFieldsIn + inIndex;
      outBuf[outIndex] = inBuf[kk];
      outIndex += beam->nFieldsOut;
    }

  } // ii

}
    
