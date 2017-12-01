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
// EdgeVol2Dsr.cc
//
// EdgeVol2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////
//
// EdgeVol2Dsr reads raw EEC EDGE volume data and reformats
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
#include <rapmath/RapComplex.hh>
#include "EdgeVol2Dsr.hh"
using namespace std;

const double EdgeVol2Dsr::_angleConversion = 360.0 / 65536.0;
const double EdgeVol2Dsr::_lightSpeed = 3.00e8;
 
// Constructor

EdgeVol2Dsr::EdgeVol2Dsr(int argc, char **argv)

{

  _input = NULL;
  isOK = true;
  _nGates = 0;
  _volNum = -1;
  _needToSwap = false;
  _nyquist = 0.0;
  
  // set programe name

  _progName = "EdgeVol2Dsr";
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
    cerr << "ERROR: EdgeVol2Dsr::EdgeVol2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: EdgeVol2Dsr::EdgeVol2Dsr." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(),
		  _params.instance,
		  PROCMAP_REGISTER_INTERVAL);
  }
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_EXTRA,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_EXTRA,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_EXTRA,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - EdgeVol2Dsr" << endl;
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

EdgeVol2Dsr::~EdgeVol2Dsr()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  if (_params.mode == Params::REALTIME) {
    PMU_auto_unregister();
  }

}

//////////////////////////////////////////////////
// Run

int EdgeVol2Dsr::Run ()
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
	  cerr << "ERROR = EdgeVol2Dsr::Run" << endl;
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
	cerr << "ERROR = EdgeVol2Dsr::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }

    }

  } // if (_params.mode == Params::SIMULATE)
    
  return iret;

}

///////////////////////////////
// process file

int EdgeVol2Dsr::_processFile(const char *input_path)

{

  PMU_auto_register("Processing file");
      
  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // set volume number - increments per file

  _volNum++;

  // clear times

  _sweepStartTime = 0;
  _volStartTime = 0;
  _latestRayTime = 0;

  // Reformat the file

  if (_reformatFile(input_path)) {
    return -1;
  }

  // put end of volume flag

  _rQueue.putEndOfVolume(_volNum, _latestRayTime);

  return 0;

}

///////////////////////////////
// reformat the file into Dsr

int EdgeVol2Dsr::_reformatFile(const char *input_path)

{

  PMU_auto_register("Reformatting file");
      
  if (_params.debug) {
    cerr << "Reformatting file" << endl;
  }
  
  // open file - file closes automatically when inFile goes
  // out of scope
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopenUncompress(input_path, "r")) == NULL) {
    cerr << "ERROR - EdgeVol2Dsr::_reformatFile" << endl;
    cerr << "  Cannot open input file: " << input_path << endl;
    perror("  ");
    return -1;
  }

  // read in product and ingest headers
  
  if (_readHeaders(in)) {
    cerr << "ERROR - EdgeVol2Dsr::_reformatFile" << endl;
    cerr << "  Reading header, file: " << input_path << endl;
    return -1;
  }
  
  // read the data, a sweep at a time
  
  int iret = 0;
  while (!feof(in)) {

    if (_readSweepData(in) == 0) {

      if (_processSweep()) {
	cerr << "ERROR - EdgeVol2Dsr::_processFile" << endl;
	cerr << "  Processing sweep, file: " << input_path << endl;
	iret = -1;
      }
      
    } else {
      
      iret = -1;
      
    }

  } // while
  
  return iret;

}

/////////////////////////////////////
// read in product and ingest headers

int EdgeVol2Dsr::_readHeaders(FILE *in)

{

  // product configuration

  if (_readRecord(in)) {
    cerr << "ERROR - EdgeVol2Dsr::_readHeader" << endl;
    cerr << "  Cannot read in product header" << endl;
    return -1;
  }

  memcpy(&_prodHdr, _record, sizeof(_prodHdr));
  
  // check is swapping is needed

  if (_prodHdr.id_hdr.id == 27) {
    // no swapping needed
    _needToSwap = false;
    if (_params.debug) {
      cerr << "Note: byte swapping is not needed for this file" << endl;
    }
  } else {
    _needToSwap = true;
    _swap(_prodHdr);
    if (_prodHdr.id_hdr.id == 27) {
      if (_params.debug) {
        cerr << "Note: byte swapping is on for this file" << endl;
      }
    } else {
      // bad data
      cerr << "ERROR - EdgeVol2Dsr::_readHeader" << endl;
      cerr << "  Cannot recognize product header, even after swapping" << endl;
      _needToSwap = false;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _print(_prodHdr, cerr);
  }

  // ingest headers

  if (_readRecord(in)) {
    cerr << "ERROR - EdgeVol2Dsr::_readHeader" << endl;
    cerr << "  Cannot read in product header" << endl;
    return -1;
  }

  memcpy(&_inHdr, _record, sizeof(_inHdr));
  _swap(_inHdr);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _print(_inHdr, cerr);
  }
  _nbytesExtendedHdr =
    _inHdr.ingest_conf.nbytes_in_ext_hdr - sizeof(ray_header_t);

  // compute derived quantities

  _pulseWidthUs = _prodHdr.end.pulse_width_us_100 / 100.0;
  _wavelengthCm = _prodHdr.end.wavelength_cm_100 / 100.0;
  _wavelengthM = _wavelengthCm / 100.0;
  _prf = _prodHdr.end.prf_hz;
  _prtSec = 1.0 / _prf;
  _unambigRangeKm = (_prtSec * _lightSpeed) / 2000.0;

  // nyquist depends on pulsing scheme

  _nyquist = _wavelengthM / (4.0 * _prtSec);
  switch (_prodHdr.end.trig_rate_scheme) {
    case PRF_DUAL_4_5:
      _nyquist *= 4.0;
      break;
    case PRF_DUAL_3_4:
      _nyquist *= 3.0;
      break;
    case PRF_DUAL_2_3:
      _nyquist *= 2.0;
      break;
    default: {}
  }
  if (_params.override_nyquist_velocity) {
    _nyquist = _params.nyquist_velocity;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  pulseWidthUs: " << _pulseWidthUs << endl;
    cerr << "  wavelengthCm: " << _wavelengthCm << endl;
    cerr << "  wavelengthM: " << _wavelengthM << endl;
    cerr << "  prf: " << _prf << endl;
    cerr << "  prtSec: " << _prtSec << endl;
    cerr << "  nyquist: " << _nyquist << endl;
    cerr << "  unambigRangeKm: " << _unambigRangeKm << endl;
  }

  return 0;

}

/////////////////////////////////////
// read in data from a sweep

int EdgeVol2Dsr::_readSweepData(FILE *in)

{

  // read in first record in sweep

  if (_readRecord(in)) {
    if (feof(in)) {
      return 0;
    }
    cerr << "ERROR - EdgeVol2Dsr::_readSweepData" << endl;
    return -1;
  }
  ui08 *ptr = _record;

  // check raw header

  memcpy(&_rawHdr, ptr, sizeof(_rawHdr));
  _swap(_rawHdr);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _print(_rawHdr, cerr);
  }
  ptr += sizeof(_rawHdr);
  _sweepNum = _rawHdr.sweep_num;
  _tiltNum = _sweepNum - 1;
  
  // set ingest data headers for each field
  
  _nRaysSweep = 0;
  _inDatHdrs.clear();
  _nBytesData = 0;
  for (int ifield = 0; ifield < 16; ifield++) {
    ingest_data_header_t inDatHdr;
    memcpy(&inDatHdr, ptr, sizeof(inDatHdr));
    _swap(inDatHdr);
    if (inDatHdr.id_hdr.id != 24 ||
	inDatHdr.sweep_num != _sweepNum) {
      break;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====>>> field number: " << _inDatHdrs.size() << endl;
      _print(inDatHdr, cerr);
    }
    sigmet_time_t time = inDatHdr.time;
    DateTime stime(time.year, time.month, time.day, 0, 0, time.sec);
    _sweepStartTime = stime.utime() + time.msecs / 1000.0;
    _fixedAngle = _binAngleToDouble(inDatHdr.fixed_angle);
    _nRaysSweep = inDatHdr.n_rays_written;
    int nBytesThisField = inDatHdr.id_hdr.nbytes - sizeof(ingest_data_header_t);
    _nBytesData += nBytesThisField;
    ptr += sizeof(inDatHdr);
    _inDatHdrs.push_back(inDatHdr);
  }

  // compute the number of fields,
  // i.e. those headers with non-zero data_codes

  _nFields = 0;
  for (size_t ii = 0; ii < _inDatHdrs.size(); ii++) {
    if (_inDatHdrs[ii].data_code != 0) {
      _nFields++;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> nFields: " << _nFields << endl;
    cerr << "==>> nBytesData: " << _nBytesData << endl;
  }

  if (_nFields < 1) {
    cerr << "ERROR - EdgeVol2Dsr::_readSweepData()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }

  if ((_params.debug && (_tiltNum == 0)) ||
      (_params.debug >= Params::DEBUG_VERBOSE)) {
    cerr << "Found the following fields: " << endl;
    for (size_t ii = 0; ii < _inDatHdrs.size(); ii++) {
      const ingest_data_header_t &inDatHdr = _inDatHdrs[ii];
      int fieldId = inDatHdr.data_code;
      if (fieldId != 0) {
        string name = _fieldId2Name(fieldId);
        string units = _fieldId2Units(fieldId);
        if (name.find("WIDTH") != string::npos && units.size() == 0) {
          units = "m/s";
        }
        double scale = 1.0, bias = 0.0;
        _fieldId2ScaleBias(fieldId, scale, bias);
        cerr << "  fieldId, name, units, byteWidth, scale, bias: "
             << fieldId << ", " << name << ", " << units << ", "
             << inDatHdr.bits_per_bin / 8 << ", "
             << scale << ", " << bias << endl;
      }
    }
  }

  // add remaining data to buffer
  
  _inBuf.reset();
  int nBytesLeft = RAW_RECORD_LEN - (ptr - _record);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "first record, nBytesLeft: " << nBytesLeft << endl;
  }
  _inBuf.add(ptr, nBytesLeft);

  // read remaining records for this sweep
  
  while (!feof(in)) {

    // save current position, for stepping back

    long pos = ftell(in);

    // read in next record

    if (_readRecord(in)) {
      if (feof(in)) {
        break;
      }
      cerr << "ERROR - EdgeVol2Dsr::_readData" << endl;
      return -1;
    }
    ptr = _record;

    // check header for sweep number change

    memcpy(&_rawHdr, ptr, sizeof(_rawHdr));
    if (_params.debug >= Params::DEBUG_EXTRA) {
      _print(_rawHdr, cerr);
    }
    ptr += sizeof(_rawHdr);

    if (_rawHdr.sweep_num != _sweepNum) {
      // gone one record too far, step back, return
      fseek(in, pos, SEEK_SET);
      break;
    }
    
    // add to in buffer

    int nBytesLeft = RAW_RECORD_LEN - (ptr - _record);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "later record, nBytesLeft: " << nBytesLeft << endl;
    }
    _inBuf.add(ptr, nBytesLeft);

  } // while

  // reset the data buffer

  _dataBuf.reset();

  // create an array filled with zeros, for copying as required

  si16 zeros[65536];
  memset(zeros, 0, 65536 * sizeof(si16));

  // read input buffer, uncompressing

  int nComp16 = _inBuf.getLen() / sizeof(si16);
  si16 *comp = (si16 *) _inBuf.getPtr();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "nComp16: " << nComp16 << endl;
  }

  int count = 0;
  int prevCount = 0;
  int nRayFieldsFound = 0;
  _nBytesRayField.clear();

  for (int ii = 0; ii < nComp16; ii++) {
    si16 code = comp[ii];
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "ii, code, nRayFields: "
	   << ii << ", " << code <<  ", "
	   << nRayFieldsFound << " - ";
    }
    if (code == 1) {
      int nBytesField = (count - prevCount) * 2;
      prevCount = count;
      if (_params.debug >= Params::DEBUG_EXTRA) {
	cerr << "  ==>> end of field, nbytes: " << nBytesField << " <<==";
      }
      nRayFieldsFound++;
      _nBytesRayField.push_back(nBytesField);
      if (nRayFieldsFound == _nRaysSweep * (int) _inDatHdrs.size()) {
	break;
      }
    } else if (code >= 0x0003) {
      si16 nZeros = code;
      if (_params.debug >= Params::DEBUG_EXTRA) {
	cerr << " " << nZeros << " zeros";
      }
      count += nZeros;
      // add zeros to data buffer
      _dataBuf.add(zeros, nZeros * sizeof(si16));
    } else {
      si16 nDataWords = (code & 0x7FFF);
      if(nDataWords >= 1) {
	if (_params.debug >= Params::DEBUG_EXTRA) {
	  cerr << " " << nDataWords << "  data words follow";
	}

        // add data zeros to data buffer

        _dataBuf.add(comp + ii + 1, nDataWords * sizeof(si16));
	ii += nDataWords;
	count += nDataWords;
      }
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << ", count: " << count << endl;
    }
  } // ii

  // get the number of gates from the ray header at
  // the start of the data
  
  ray_header_t rayHdr;
  memcpy(&rayHdr, _dataBuf.getPtr(), sizeof(rayHdr));
  _swap(rayHdr);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _print(rayHdr, cerr);
  }

  // Check if there are any 2-byte fields.
  // If so, we need to output 2 byte data for all fields,

  _outputByteWidth = 1;
  for (size_t ifield = 0; ifield < _inDatHdrs.size(); ifield++) {
    const ingest_data_header_t &inDatHdr = _inDatHdrs[ifield];
    if (inDatHdr.data_code != 0 && inDatHdr.bits_per_bin > 8) {
      _outputByteWidth = 2;
      break;
    }
  }
  
  return 0;

}

/////////////////////////////////////
// process a sweep of data
//
// Returns 0 on success, -1 on failure

int EdgeVol2Dsr::_processSweep()

{

  // set the ray info, so that we know where the rays start etc
  // also determing _nGates

  if (_setRayInfo()) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Processing sweep: " << endl;
    cerr << "  outputByteWidth: " << _outputByteWidth << endl;
    cerr << "  nGates, nFields: "
         << _nGates << ", " << _nFields << endl;
  }
  
  // write params

  if (_writeParams()) {
    return -1;
  }

  // find start index with earliest time
  // the rays are stored in azimuth order, but the sweep may have started
  // somewhere other than north, so we need to find the starting point

  int startIndex = 0;
  for (size_t iray = 1; iray < _rayInfo.size(); iray++) {
    if (_rayInfo[iray].hdr.seconds < _rayInfo[iray-1].hdr.seconds) {
      startIndex = iray;
      break;
    }
  }
  
  // write beams

  DsRadarMsg msg;
  DsRadarBeam &dsBeam = msg.getRadarBeam();
  
  // initialize pointer to start of data buf

  int iret = 0;
  int rayFieldNum = 0;
  for (size_t iray = 0; iray < _rayInfo.size(); iray++) {
    
    // get time, azimuth etc from ray header for first field
    
    int rayIndex = (iray + startIndex) % _rayInfo.size();
    ui08 *rayPtr = _rayInfo[rayIndex].offset;
    ray_header_t rayHdr;
    memcpy(&rayHdr, rayPtr, sizeof(rayHdr));
    _swap(rayHdr);

    // load up data

    int nGateFields = _nGates * _nFields;
    MemBuf outBuf;
    outBuf.reserve(nGateFields * _outputByteWidth);
    memset(outBuf.getPtr(), 0, outBuf.getLen());

    int fieldNum = 0;
    time_t rayTime = 0;

    for (size_t ifield = 0; ifield < _inDatHdrs.size(); ifield++) {

      const ingest_data_header_t &inDatHdr = _inDatHdrs[ifield];
      if (inDatHdr.data_code == 0) {
        rayPtr += _nBytesRayField[rayFieldNum];
        rayFieldNum++;
        continue;
      }

      // get ray header for this field

      memcpy(&rayHdr, rayPtr, sizeof(rayHdr));
      _swap(rayHdr);
      int nGates = rayHdr.n_gates;
      int inByteWidth = inDatHdr.bits_per_bin / 8;
      ui08 *dataPtr = rayPtr + sizeof(ray_header_t);

      // if first field, set beam meta data

      if (fieldNum == 0) {
        rayTime = _setBeamMetadata(dsBeam, rayHdr);
      }

      if (_outputByteWidth == 1) {
        
        // all fields must therefore be 1 byte in width
        ui08 *ibuf = dataPtr;
        ui08 *obuf = (ui08 *) outBuf.getPtr() + fieldNum;
        for (int igate = 0; igate < nGates;
             igate++, obuf += _nFields, ibuf++) {
          *obuf = *ibuf;
        }

      } else {
        
        ui16 *obuf = (ui16 *) outBuf.getPtr() + fieldNum;
        if (inByteWidth == 1) {
          ui08 *ibuf = dataPtr;
          for (int igate = 0; igate < nGates;
               igate++, obuf += _nFields, ibuf++) {
            *obuf = *ibuf;
          }
        } else {
          if (_needToSwap) {
            // copy and swap
            TaArray<ui16> tmp_;
            ui16 *ibuf = tmp_.alloc(nGates);
            memcpy(ibuf, dataPtr, nGates * sizeof(ui16));
            _swap(ibuf, nGates);
            for (int igate = 0; igate < nGates;
                 igate++, obuf += _nFields, ibuf++) {
              *obuf = *ibuf;
            }
          } else {
            ui16 *ibuf = (ui16 *) dataPtr;
            for (int igate = 0; igate < nGates;
                 igate++, obuf += _nFields, ibuf++) {
              *obuf = *ibuf;
            }
          }
        }

      } // if (_outputByteWidth == 1)
      
      rayPtr += _nBytesRayField[rayFieldNum];
      rayFieldNum++;
      fieldNum++;

    } // ifield

    dsBeam.loadData(outBuf.getPtr(), outBuf.getLen(), _outputByteWidth);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      dsBeam.print(cerr);
    }

    // set start time as appropriate

    if (_volStartTime == 0) {
      _volStartTime = rayTime;
      // put start of volume flag
      _rQueue.putStartOfVolume(_volNum, _volStartTime);
    }

    // save latest ray time

    _latestRayTime = rayTime;
    
    if (iray == 0) {
      _rQueue.putStartOfTilt(_tiltNum, _latestRayTime);
    }
    
    // write the message
    
    if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
      iret = -1;
    }
    
    if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
      umsleep(_params.beam_wait_msecs);
    }

  } // iray
  
  // write end of tilt flag
  
  _rQueue.putEndOfTilt(_tiltNum, (time_t) _latestRayTime);

  return iret;

}

/////////////////////////////////////
// Set the ray info

int EdgeVol2Dsr::_setRayInfo()

{

  _rayInfo.clear();
  int maxGates = 0;

  // get the time for each ray, so we can output them in the correct order

  ui08 *rptr = (ui08 *) _dataBuf.getPtr();
  ui08 *end = rptr + _dataBuf.getLen();

  int rayFieldNum = 0;
  for (int iray = 0; iray < _nRaysSweep; iray++) {

    if (rptr + sizeof(ray_header_t) > end) {
      cerr << "ERROR - EdgeVol2Dsr::_setRayInfo" << endl;
      cerr << "  Data buffer too small - overflow occurred" << endl;
      cerr << "  ray num: " << iray << endl;
      return -1;
    }

    // save ray info
      
    RayInfo_t info;

    for (size_t ifield = 0; ifield < _inDatHdrs.size(); ifield++) {

      const ingest_data_header_t &inDatHdr = _inDatHdrs[ifield];
      int fieldId = inDatHdr.data_code;

      ray_header_t rayHdr;
      memcpy(&rayHdr, rptr, sizeof(rayHdr));
      _swap(rayHdr);
      
      if (ifield == 0) {
        info.offset = rptr; // offset saved in first field
      }

      if (fieldId != 0) {
        info.hdr = rayHdr; // header saved using real field
        if (rayHdr.n_gates > maxGates) {
          maxGates = rayHdr.n_gates;
        }
      }

      rptr += _nBytesRayField[rayFieldNum];
      rayFieldNum++;

    } // ifield
    
    _rayInfo.push_back(info);

  } // iray

  _nGates = maxGates;

  return 0;
  
}

/////////////////////////
// set the beam metadata
// returns the ray time

time_t EdgeVol2Dsr::_setBeamMetadata(DsRadarBeam &dsBeam,
                                       const ray_header_t &rayHdr)

{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _print(rayHdr, cerr);
  }
    
  double rayTime = (double) _sweepStartTime + rayHdr.seconds;
  if (_params.correct_radar_time) {
    rayTime += _params.time_correction_secs;
  }

  time_t raySecs = (time_t) rayTime;
  int rayNanoSecs = (int) ((rayTime - raySecs) * 1.0e9);
    
  dsBeam.dataTime = rayTime;
  dsBeam.nanoSecs = rayNanoSecs;

  dsBeam.volumeNum = _volNum;
  dsBeam.tiltNum = _tiltNum;
    
  double startEl = _binAngleToDouble(rayHdr.start_el);
  double endEl = _binAngleToDouble(rayHdr.end_el);
  double startAz = _binAngleToDouble(rayHdr.start_az);
  double endAz = _binAngleToDouble(rayHdr.end_az);

  double meanEl = RapComplex::computeMeanDeg(startEl, endEl);
  double meanAz = RapComplex::computeMeanDeg(startAz, endAz);
  if (meanAz < 0) {
    meanAz += 360.0;
  }
  
  dsBeam.azimuth = meanAz;
  dsBeam.elevation = meanEl;
  dsBeam.nSamples = _prodHdr.end.nsamples;
  dsBeam.beamIsIndexed = false;
    
  bool isRhi = false;
  if (_inHdr.task_conf.scan_info.scan_mode == SCAN_MODE_RHI) {
    isRhi = true;
  }

  if (isRhi) {
    dsBeam.targetAz = _fixedAngle;
    dsBeam.targetElev = NAN;
    dsBeam.scanMode = DS_RADAR_RHI_MODE;
  } else {
    dsBeam.targetElev = _fixedAngle;
    dsBeam.targetAz = NAN;
    dsBeam.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  }

  return raySecs;

}
    
//////////////////////////////
// read in a record

int EdgeVol2Dsr::_readRecord(FILE *in)

{

  if (fread(_record, 1, RAW_RECORD_LEN, in) != RAW_RECORD_LEN) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR reading raw record" << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
    return -1;
  }
  
  return 0;
  
}

///////////////////////////////
// print a char array

void EdgeVol2Dsr::_printCharArray(ostream &out, const char *buf, int len)

{
  for (int ii = 0; ii < len; ii++) {
    if (buf[ii] == '\0') {
      return;
    }
    if (isprint(buf[ii])) {
      out << buf[ii];
    } else {
      out << " ";
    }
  }
}

///////////////////////////////
// write radar and field params

int EdgeVol2Dsr::_writeParams()

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
  for (size_t ii = 0; ii < _inDatHdrs.size(); ii++) {
    const ingest_data_header_t &inDatHdr = _inDatHdrs[ii];
    int fieldId = inDatHdr.data_code;
    if (fieldId != 0) {
      string name = _fieldId2Name(fieldId);
      string units = _fieldId2Units(fieldId);
      if (name.find("WIDTH") != string::npos && units.size() == 0) {
        units = "m/s";
      }
      double scale = 1.0, bias = 0.0;
      _fieldId2ScaleBias(fieldId, scale, bias);
      // create new field - this will be deleted by the destructor of msg
      DsFieldParams *fparams =
        new DsFieldParams(name.c_str(), units.c_str(),
                          scale, bias, _outputByteWidth);
      fieldParams.push_back(fparams);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        fparams->print(cerr);
      }
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

////////////////////
// load radar params

void EdgeVol2Dsr::_loadRadarParams(DsRadarParams &rParams)

{

  double missing = -9999.0;
  
  rParams.radarId = _prodHdr.end.site_mask;
  rParams.radarType = DS_RADAR_GROUND_TYPE;
  rParams.numFields = _nFields;
  rParams.numGates = _nGates;
  rParams.samplesPerBeam = _prodHdr.end.nsamples;
  rParams.scanType = 0;
  if (_prodHdr.conf.ptype == 2) {
    rParams.scanMode = DS_RADAR_RHI_MODE;
  } else {
    rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  }
  
  if (_prodHdr.end.polarization == 1) {
    rParams.polarization = DS_POLARIZATION_HORIZ_TYPE;
  } else {
    rParams.polarization = DS_POLARIZATION_VERT_TYPE;
  }
  
  rParams.altitude = (_inHdr.ingest_conf.ground_ht_msl_meters +
		      _inHdr.ingest_conf.radar_ht_agl_meters) / 1000.0;

  rParams.latitude = _binAngleToDouble(_inHdr.ingest_conf.latitude);
  rParams.longitude = _binAngleToDouble(_inHdr.ingest_conf.longitude);

  rParams.radarName = _label2Str(_prodHdr.end.hardware_name, 16);
  rParams.scanTypeName = _label2Str(_prodHdr.conf.task_name, 12);
  
  rParams.startRange =
    _inHdr.task_conf.range_info.range_first_bin_cm / 100000.0;
  rParams.gateSpacing =
    _inHdr.task_conf.range_info.output_gate_spacing_cm / 100000.0;
  
  rParams.horizBeamWidth =
    _binAngleToDouble(_inHdr.task_conf.misc_info.beam_width_h);
  if (rParams.horizBeamWidth < 0.1) {
    rParams.horizBeamWidth = 1.0;
  }
  rParams.vertBeamWidth =
    _binAngleToDouble(_inHdr.task_conf.misc_info.beam_width_v);
  if (rParams.vertBeamWidth < 0.1) {
    rParams.vertBeamWidth = 1.0;
  }

  rParams.radarConstant = _inHdr.task_conf.calib_info.radar_cont_h_100 / 100.0;
  rParams.xmitPeakPower = _inHdr.task_conf.misc_info.xmit_power_watts;
  rParams.receiverMds = _prodHdr.end.cal_noise_db_100 / 100.0;
  rParams.receiverGain = missing;
  rParams.antennaGain = missing;
  rParams.systemGain = missing;

  rParams.pulseWidth = _pulseWidthUs;
  rParams.wavelength = _wavelengthCm;
  rParams.pulseRepFreq = _prf;

  rParams.unambigVelocity = _nyquist;
  rParams.unambigRange = _unambigRangeKm;

}

////////////////////////////////////////////////////////////////    
// convert time to string

string EdgeVol2Dsr::_time2Str(const sigmet_time_t &time)

{
  
  DateTime stime(time.year, time.month, time.day, 0, 0, time.sec);
  char msecsStr[32];
  sprintf(msecsStr, "%.3d", time.msecs);
  string tstr = DateTime::strm(stime.utime());
  tstr += ".";
  tstr += msecsStr;
  return tstr;
  
}

////////////////////////////////////////////////////////////////    
// convert label to string, ensuring null termination

string EdgeVol2Dsr::_label2Str(const char *label, int maxLen)

{
  
  TaArray<char> _cstr;
  char *cstr = _cstr.alloc(maxLen + 1);
  memset(cstr, 0, maxLen + 1);
  memcpy(cstr, label, maxLen);
  return cstr;

}

////////////////////////////////////////////////////////////////    
// convert a field ID to name

string EdgeVol2Dsr::_fieldId2Name(int fieldId)

{

  switch (fieldId) {
  case FIELD_DBZ_TOT:
  case FIELD_DBZ_TOT_2:
    return "DBZ_TOT";
  case FIELD_DBZ_2:
  case FIELD_DBZ:
    return "DBZ";
  case FIELD_VEL:
  case FIELD_VEL_2:
    return "VEL";
  case FIELD_WIDTH:
  case FIELD_WIDTH_2:
    return "WIDTH";
  case FIELD_ZDR:
  case FIELD_ZDR_2:
    return "ZDR";
  case FIELD_DBZ_CORR:
  case FIELD_DBZ_CORR_2:
    return "DBZ_CORR";
  case FIELD_KDP:
  case FIELD_KDP_2:
    return "KDP";
  case FIELD_PHIDP:
  case FIELD_PHIDP_2:
    return "PHIDP";
  case FIELD_VEL_CORR:
  case FIELD_VEL_CORR_2:
    return "VEL_CORR";
  case FIELD_SQI:
  case FIELD_SQI_2:
    return "SQI";
  case FIELD_RHOHV:
  case FIELD_RHOHV_2:
    return "RHOHV";
  case FIELD_LDRH:
  case FIELD_LDRH_2:
    return "LDRH";
  case FIELD_LDRV:
  case FIELD_LDRV_2:
    return "LDRV";
  default:
    return "UNKNOWN";
  }
}

////////////////////////////////////////////////////////////////    
// convert a field ID to units

string EdgeVol2Dsr::_fieldId2Units(int fieldId)

{

  switch (fieldId) {
  case FIELD_DBZ_TOT:
  case FIELD_DBZ_TOT_2:
  case FIELD_DBZ_2:
  case FIELD_DBZ:
  case FIELD_DBZ_CORR:
  case FIELD_DBZ_CORR_2:
    return "dBZ";
  case FIELD_VEL:
  case FIELD_VEL_2:
  case FIELD_VEL_CORR:
  case FIELD_VEL_CORR_2:
    return "m/s";
    return "m/s";
  case FIELD_ZDR:
  case FIELD_ZDR_2:
  case FIELD_LDRH:
  case FIELD_LDRH_2:
  case FIELD_LDRV:
  case FIELD_LDRV_2:
    return "dB";
  case FIELD_KDP:
  case FIELD_KDP_2:
    return "deg/km";
  case FIELD_PHIDP:
  case FIELD_PHIDP_2:
    return "deg";
  case FIELD_SQI:
  case FIELD_SQI_2:
  case FIELD_RHOHV:
  case FIELD_RHOHV_2:
  default:
    return "";
  }
}

//////////////////////////////////////
// Get scale, bias from field ID

void EdgeVol2Dsr::_fieldId2ScaleBias(int fieldId,
				       double &scale, double &bias)
  
{
  
  scale = 1.0;
  bias = 0.0;
  
  switch (fieldId) {
    
  case FIELD_DBZ_TOT:
  case FIELD_DBZ:
  case FIELD_DBZ_CORR: {
    scale = 0.5;
    bias = -32.0;
    break;
  }
  case FIELD_VEL: {
    scale = _nyquist / 127.0;
    bias = (-1.0 * _nyquist) * (128.0 / 127.0);
    break;
  }
  case FIELD_VEL_CORR: {
    scale = 75.0 / 127.0;
    bias = (-1.0 * 75.0) * (128.0 / 127.0);
    break;
  }
  case FIELD_WIDTH: {
    scale = _nyquist / 256.0;
    bias = 0.0;
    break;
  }
  case FIELD_WIDTH_2: {
    scale = 0.01;
    bias = 0.0;
    break;
  }
  case FIELD_ZDR: {
    scale = 1.0 / 16.0;
    bias = -8.0;
    break;
  }
  case FIELD_DBZ_TOT_2:
  case FIELD_DBZ_2:
  case FIELD_DBZ_CORR_2:
  case FIELD_VEL_2:
  case FIELD_KDP_2:
  case FIELD_VEL_CORR_2:
  case FIELD_ZDR_2:
  case FIELD_LDRH_2: {
    scale = 0.01;
    bias = -327.67;
    break;
  }
  case FIELD_KDP: {
    // 1-byte KDP is a mess
    break;
  }
  case FIELD_PHIDP: {
    scale = 180.0 / 254.0;
    bias = scale * -1.0;
    break;
  }
  case FIELD_PHIDP_2: {
    scale = 360.0 / 65534.0;
    bias = scale * -1.0;
    break;
  }
  case FIELD_SQI:
  case FIELD_RHOHV: { // apply sqrt()
    scale = 1.0 / 253.0;
    bias = 0.0;
    break;
  }
  case FIELD_SQI_2:
  case FIELD_RHOHV_2: {
    scale = 1.0 / 65533.0;
    bias = scale * -1.0;
    break;
  }
  case FIELD_LDRH:
  case FIELD_LDRV: {
    scale = 0.2;
    bias = -45.2;
    break;
  }
  default: {}

  }

}

//////////////////////////////////
// convert binary angle to double

double EdgeVol2Dsr::_binAngleToDouble(ui16 binAngle)

{
  return (double) binAngle * 360.0 / ((double) 0xffff + 1.0);
}

double EdgeVol2Dsr::_binAngleToDouble(si32 binAngle)

{
  return (double) binAngle * 360.0 / ((double) 0xffffffff + 1.0);
}

///////////////////////////////////////////////////
// printing routines

void EdgeVol2Dsr::_print(const sigmet_id_hdr_t &hdr, ostream &out)

{
  out << "  id: " << hdr.id << endl;
  out << "  version: " << hdr.version << endl;
  out << "  nbytes: " << hdr.nbytes << endl;
  out << "  flags: " << hdr.flags << endl;
}

void EdgeVol2Dsr::_print(const prod_header_t &hdr, ostream &out)
  
{
  out << "===== PRODUCT HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  _print(hdr.id_hdr, out);
  _print(hdr.conf, out);
  _print(hdr.end, out);
  out << "=================================" << endl;

}

void EdgeVol2Dsr::_print(const prod_conf_t &prod, ostream &out)
  
{
  out << "----- PRODUCT CONFIGURATION -----" << endl;
  out << "  Size: " << sizeof(prod) << endl;
  _print(prod.id_hdr, out);
  out << "  prodType: " << prod.ptype << endl;
  out << "  scheduling " << prod.scheduling << endl;
  out << "  secs_between_runs: " << prod.secs_between_runs << endl;
  out << "  product_time: " << _time2Str(prod.product_time) << endl;
  out << "  ingest_time: " << _time2Str(prod.ingest_time) << endl;
  out << "  start_sched_time: " << _time2Str(prod.start_sched_time) << endl;
  out << "  product name: " << _label2Str(prod.prod_name, 12) << endl;
  out << "  task name: " << _label2Str(prod.task_name, 12) << endl;
  out << "  flags: " << prod.flags << endl;
  out << "  xsize: " << prod.xsize << endl;
  out << "  ysize: " << prod.ysize << endl;
  out << "  zsize: " << prod.zsize << endl;
  out << "  range_last_bin_cm: " << prod.range_last_bin_cm << endl;
  out << "  data_type_out: " << prod.data_type_out << endl;
  out << "  projection name: "
      << _label2Str(prod.projection_name, 12) << endl;
  out << "  data_type_in: " << prod.data_type_in << endl;
  out << "  projection type: " << (int) prod.projection_type << endl;
  out << "  radial_smoothing_range_km_100: "
      << prod.radial_smoothing_range_km_100 << endl;
  out << "  n_runs: " << prod.n_runs << endl;
  out << "  zr_coeff: " << prod.zr_coeff << endl;
  out << "  zr_expon: " << prod.zr_expon << endl;
  out << "  2d_smooth_x: " << prod.twod_smooth_x << endl;
  out << "  2d_smooth_y: " << prod.twod_smooth_y << endl;

  out << "---------------------------------" << endl;

}

void EdgeVol2Dsr::_print(const prod_end_t &end, ostream &out)
  
{
  out << "----- PRODUCT END -----" << endl;
  out << "  Size: " << sizeof(end) << endl;
  out << "  prod_sitename: "
      << _label2Str(end.prod_sitename, 16) << endl;
  out << "  prod_version: "
      << _label2Str(end.prod_version, 8) << endl;
  out << "  iris_version: "
      << _label2Str(end.iris_version, 8) << endl;
  out << "  oldest_data_time: "
      << _time2Str(end.oldest_data_time) << endl;
  out << "  minutes_lst_west_of_gmt: " << end.minutes_lst_west_of_gmt << endl;
  out << "  hardware_name: "
      << _label2Str(end.hardware_name, 16) << endl;
  out << "  ingest_site_name: "
      << _label2Str(end.ingest_site_name, 16) << endl;
  out << "  minutes_rec_west_of_gmt: " << end.minutes_rec_west_of_gmt << endl;
  out << "  latitude_center: " << end.latitude_center << endl;
  out << "  longitude_center: " << end.longitude_center << endl;
  out << "  ground_ht_msl_meters: " << end.ground_ht_msl_meters << endl;
  out << "  radar_ht_agl_meters: " << end.radar_ht_agl_meters << endl;
  out << "  prf_hz: " << end.prf_hz << endl;
  out << "  pulse_width_us_100: " << end.pulse_width_us_100 << endl;
  out << "  dsp_type: " << end.dsp_type << endl;
  out << "  trig_rate_scheme: " << end.trig_rate_scheme << endl;
  out << "  nsamples: " << end.nsamples << endl;
  out << "  clut_filter_file_name: "
      << _label2Str(end.clut_filter_file_name, 12) << endl;
  out << "  dop_filter_first_bin: " << (int) end.dop_filter_first_bin << endl;
  out << "  wavelength_cm_100: " << end.wavelength_cm_100 << endl;
  out << "  trunc_ht_above_radar_cm: " << end.trunc_ht_above_radar_cm << endl;
  out << "  range_first_bin_cm: " << end.range_first_bin_cm << endl;
  out << "  range_last_bin_cm: " << end.range_last_bin_cm << endl;
  out << "  n_gates: " << end.n_gates << endl;
  out << "  input_file_count: " << end.input_file_count << endl;
  out << "  polarization: " << end.polarization << endl;
  out << "  i0_cal_db_100: " << end.i0_cal_db_100 << endl;
  out << "  cal_noise_db_100: " << end.cal_noise_db_100 << endl;
  out << "  radar_cont_h_100: " << end.radar_cont_h_100 << endl;
  out << "  receiver_bandwidth: " << end.receiver_bandwidth << endl;
  out << "  noise_level_db_100: " << end.noise_level_db_100 << endl;
  out << "  lambert_lat1: " << end.lambert_lat1 << endl;
  out << "  lambert_lat2: " << end.lambert_lat2 << endl;
  out << "  earth_radius_cm: " << end.earth_radius_cm << endl;
  out << "  earth_flattening_1000000: "
      << end.earth_flattening_1000000 << endl;
  out << "  faults_bits: " << end.faults_bits << endl;
  out << "  site_mask: " << end.site_mask << endl;
  out << "  log_filter_first: " << end.log_filter_first << endl;
  out << "  dsp_clutmap: " << end.dsp_clutmap << endl;
  out << "  proj_ref_lat: " << end.proj_ref_lat << endl;
  out << "  proj_ref_lon: " << end.proj_ref_lon << endl;
  out << "  sequence_num: " << end.sequence_num << endl;
  out << "  melting_ht_m_msl: " << end.melting_ht_m_msl << endl;
  out << "  ht_radar_above_ref_m: " << end.ht_radar_above_ref_m << endl;
  out << "  n_results_elements: " << end.n_results_elements << endl;
  out << "  wind_speed: " << (int) end.wind_speed << endl;
  out << "  wind_dirn: " << (double) end.wind_dirn  * (360.0 / 256.0) << endl;
  out << "  local_tz: " << _label2Str(end.local_tz, 8) << endl;

  out << "---------------------------------" << endl;

}

void EdgeVol2Dsr::_print(const ingest_header_t &hdr, ostream &out)
  
{
  out << "===== INGEST HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  _print(hdr.id_hdr, out);
  _print(hdr.ingest_conf, out);
  _print(hdr.task_conf, out);
  out << "=========================" << endl;
}


void EdgeVol2Dsr::_print(const ingest_conf_t &conf, ostream &out)
  
{
  out << "----- INGEST CONFIGURATION -----" << endl;
  out << "  Size: " << sizeof(conf) << endl;
  out << "  file_name: " << _label2Str(conf.file_name, 80) << endl;
  out << "  num_data_files: " << conf.num_data_files << endl;
  out << "  nsweeps_completed: " << conf.nsweeps_completed << endl;
  out << "  total_size_files: " << conf.total_size_files << endl;
  out << "  volume_time: " << _time2Str(conf.volume_time) << endl;
  out << "  nbytes_in_ray_hdrs: " << conf.nbytes_in_ray_hdrs << endl;
  out << "  nbytes_in_ext_hdr: " << conf.nbytes_in_ext_hdr << endl;
  out << "  nbytes_in_task_config: " << conf.nbytes_in_task_config << endl;
  out << "  playback_version: " << conf.playback_version << endl;
  out << "  iris_version: " << _label2Str(conf.iris_version, 8) << endl;
  out << "  hardware_name: " << _label2Str(conf.hardware_name, 16) << endl;
  out << "  time_zone_local_mins_west_of_gmt: "
      << conf.time_zone_local_mins_west_of_gmt << endl;
  out << "  site_name: " << _label2Str(conf.site_name, 16) << endl;
  out << "  time_zone_rec_mins_west_of_gmt: "
      << conf.time_zone_rec_mins_west_of_gmt << endl;
  out << "  latitude: " << _binAngleToDouble(conf.latitude) << endl;
  out << "  longitude: " << _binAngleToDouble(conf.longitude) << endl;
  out << "  ground_height_m_msl: " << conf.ground_ht_msl_meters << endl;
  out << "  radar_height_m_agl: " << conf.radar_ht_agl_meters << endl;
  out << "  radar_height_m_msl: " << conf.radar_ht_msl_cm / 100.0 << endl;
  out << "  nrays_per_revolution: " << conf.nrays_per_revolution << endl;
  out << "  index_of_first_ray: " << conf.index_of_first_ray << endl;
  out << "  n_rays_sweep: " << conf.n_rays_sweep << endl;
  out << "  nbytes_gparm: " << conf.nbytes_gparm << endl;
  out << "  radar_ht_msl_cm: " << conf.radar_ht_msl_cm << endl;
  out << "  platform_vel[0]: " << conf.platform_vel[0] << endl;
  out << "  platform_vel[1]: " << conf.platform_vel[1] << endl;
  out << "  platform_vel[2]: " << conf.platform_vel[2] << endl;
  out << "  ant_offset[0]: " << conf.ant_offset[0] << endl;
  out << "  ant_offset[1]: " << conf.ant_offset[1] << endl;
  out << "  ant_offset[2]: " << conf.ant_offset[2] << endl;
  out << "  fault_bits: " << conf.fault_bits << endl;
  out << "  ht_melting_m_msl: " << conf.ht_melting_m_msl << endl;
  out << "  timezone_name: " << _label2Str(conf.timezone_name, 8) << endl;
  out << "  flags: " << conf.flags << endl;
  out << "  conf_name: " << _label2Str(conf.conf_name, 16) << endl;
  out << "-------------------------" << endl;
}

void EdgeVol2Dsr::_print(const task_conf_t &conf, ostream &out)
  
{
  out << "----- TASK CONFIGURATION -----" << endl;
  out << "  Size: " << sizeof(conf) << endl;
  _print(conf.id_hdr, out);
  _print(conf.sched_info, out);
  _print(conf.dsp_info, out);
  _print(conf.calib_info, out);
  _print(conf.range_info, out);
  _print(conf.scan_info, out);
  _print(conf.misc_info, out);
  _print(conf.end_info, out);
  out << "-------------------------" << endl;
}

void EdgeVol2Dsr::_print(const task_sched_info_t &info, ostream &out)
  
{
  out << "~~~~~ SCHED INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  start_secs: " << info.start_secs << endl;
  out << "  stop_secs: " << info.stop_secs << endl;
  out << "  skip_secs: " << info.skip_secs << endl;
  out << "  last_run_secs: " << info.last_run_secs << endl;
  out << "  time_used_secs: " << info.time_used_secs << endl;
  out << "  day_last_run: " << info.day_last_run << endl;
  out << "  flag: " << info.flag << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const string &label,
                           const dsp_data_mask_t &mask,
                           ostream &out)
  
{
  out << "  DSP DATA MASK: " << label << endl;
  out << "    word_0: " << mask.word_0 << endl;
  out << "    xhdr_type: " << mask.xhdr_type << endl;
  out << "    word_1: " << mask.word_1 << endl;
  out << "    word_2: " << mask.word_2 << endl;
  out << "    word_3: " << mask.word_3 << endl;
  out << "    word_4: " << mask.word_4 << endl;
}


void EdgeVol2Dsr::_print(const task_dsp_info_t &info, ostream &out)
  
{
  out << "~~~~~ DSP INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  major_mode: " << info.major_mode << endl;
  out << "  dsp_type: " << info.dsp_type << endl;
  _print("data_mask", info.data_mask, out);
  _print("data_mask_orig", info.data_mask_orig, out);
  out << "  low_prf_hz: " << info.low_prf_hz << endl;
  out << "  low_prf_frac_hz: " << info.low_prf_frac_hz << endl;
  out << "  low_prf_sample_size: " << info.low_prf_sample_size << endl;
  out << "  low_prf_averaging: " << info.low_prf_averaging << endl;
  out << "  refl_db_thresh_100: " << info.refl_db_thresh_100 << endl;
  out << "  velocity_db_thresh_100: " << info.velocity_db_thresh_100 << endl;
  out << "  width_db_thresh_100: " << info.width_db_thresh_100 << endl;
  out << "  prf_hz: " << info.prf_hz << endl;
  out << "  pulse_width_usec_100: " << info.pulse_width_usec_100 << endl;
  out << "  trig_rate_flag: " << info.trig_rate_flag << endl;
  out << "  n_pulses_stabilization: " << info.n_pulses_stabilization << endl;
  out << "  agc_coeff: " << info.agc_coeff << endl;
  out << "  nsamples: " << info.nsamples << endl;
  out << "  gain_code: " << info.gain_code << endl;
  out << "  filter_name: " << _label2Str(info.filter_name, 12) << endl;
  out << "  dop_filter_first_bin: " << (int) info.dop_filter_first_bin << endl;
  out << "  log_filter_unused: " << (int) info.log_filter_unused << endl;
  out << "  fixed_gain_1000: " << info.fixed_gain_1000 << endl;
  out << "  gas_atten_10000: " << info.gas_atten_10000 << endl;
  out << "  clutmap_num: " << info.clutmap_num << endl;
  out << "  xmit_phase_sequence: " << info.xmit_phase_sequence << endl;
  out << "  config_header_command_mask: "
      << info.config_header_command_mask << endl;
  out << "  ts_playback_flags: " << info.ts_playback_flags << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const task_calib_info_t &info, ostream &out)
  
{
  out << "~~~~~ CALIB INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  log_slope: " << info.log_slope << endl;
  out << "  log_noise_thresh: " << info.log_noise_thresh << endl;
  out << "  clut_corr_thresh: " << info.clut_corr_thresh << endl;
  out << "  sqi_thresh_256: " << info.sqi_thresh_256 << endl;
  out << "  sig_power_thresh_16: " << info.sig_power_thresh_16 << endl;
  out << "  z_calib_16: " << info.z_calib_16 << endl;
  out << "  flags_z_uncorrected: " << info.flags_z_uncorrected << endl;
  out << "  flags_z_corrected: " << info.flags_z_corrected << endl;
  out << "  flags_vel: " << info.flags_vel << endl;
  out << "  flags_width: " << info.flags_width << endl;
  out << "  flags_zdr: " << info.flags_zdr << endl;
  out << "  flags: " << info.flags << endl;
  out << "  ldr_bias_100: " << info.ldr_bias_100 << endl;
  out << "  zdr_bias_16: " << info.zdr_bias_16 << endl;
  out << "  clut_thresh_100: " << info.clut_thresh_100 << endl;
  out << "  clut_skip: " << info.clut_skip << endl;
  out << "  io_horiz_100: " << info.io_horiz_100 << endl;
  out << "  io_vert_100: " << info.io_vert_100 << endl;
  out << "  cal_time_noise_100: " << info.cal_time_noise_100 << endl;
  out << "  cal_noise_vert: " << info.cal_noise_vert << endl;
  out << "  radar_cont_h_100: " << info.radar_cont_h_100 << endl;
  out << "  radar_cont_v_100: " << info.radar_cont_v_100 << endl;
  out << "  receiver_bandwidth_khz: " << info.receiver_bandwidth_khz << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const task_range_info_t &info, ostream &out)
  
{
  out << "~~~~~ RANGE INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  range_first_bin_cm: " << info.range_first_bin_cm << endl;
  out << "  range_last_bin_cm: " << info.range_last_bin_cm << endl;
  out << "  n_input_gates: " << info.n_input_gates << endl;
  out << "  n_output_gates: " << info.n_output_gates << endl;
  out << "  input_gate_spacing_cm: " << info.input_gate_spacing_cm << endl;
  out << "  output_gate_spacing_cm: " << info.output_gate_spacing_cm << endl;
  out << "  bin_length_variable: " << info.bin_length_variable << endl;
  out << "  gate_averaging: " << info.gate_averaging << endl;
  out << "  gate_smoothing: " << info.gate_smoothing << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const task_scan_info_t &info, ostream &out)
  
{
  out << "~~~~~ SCAN INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  scan_mode: " << info.scan_mode << endl;
  out << "  angular_res_deg_1000: " << info.angular_res_deg_1000 << endl;
  out << "  scan_speed_bin_per_sec: " << info.scan_speed_bin_per_sec << endl;
  out << "  n_sweeps: " << info.n_sweeps << endl;
  int nSweeps = info.n_sweeps;
  if (nSweeps > MAX_SWEEPS) {
    nSweeps = MAX_SWEEPS;
  }
  if (info.scan_mode == SCAN_MODE_RHI) {
    out << "  RHI scan" << endl;
    out << "  start_el: "
	<< _binAngleToDouble(info.u.rhi.start_el) << endl;
    out << "  end_el: "
	<< _binAngleToDouble(info.u.rhi.end_el) << endl;
    out << "  start_end: " << (int) info.u.rhi.start_end << endl;
    for (int ii = 0; ii < nSweeps; ii++) {
      cerr << "    sweep num, az: " << ii << ", "
	   << _binAngleToDouble(info.u.rhi.az_list[ii]) << endl;
    }
  } else if (info.scan_mode == SCAN_MODE_PPI) {
    out << "  PPI scan" << endl;
    out << "  start_az: "
	<< _binAngleToDouble(info.u.ppi.start_az) << endl;
    out << "  end_az: "
	<<  _binAngleToDouble(info.u.ppi.end_az) << endl;
    out << "  start_end: " << (int) info.u.ppi.start_end << endl;
    for (int ii = 0; ii < nSweeps; ii++) {
      cerr << "    sweep num, el: "
	   << ii << ", "
	   <<  _binAngleToDouble(info.u.ppi.el_list[ii]) << endl;
    }
  } else {
    out << "  SURVEILLANCE scan" << endl;
    out << "  start_az: "
	<< _binAngleToDouble(info.u.ppi.start_az) << endl;
    out << "  end_az: "
	<<  _binAngleToDouble(info.u.ppi.end_az) << endl;
    for (int ii = 0; ii < nSweeps; ii++) {
      cerr << "    sweep num, el: "
	   << ii << ", "
	   <<  _binAngleToDouble(info.u.ppi.el_list[ii]) << endl;
    }
  }
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const task_misc_info_t &info, ostream &out)
  
{
  out << "~~~~~ MISC INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  wavelength_cm_100: " << info.wavelength_cm_100 << endl;
  out << "  user_id: " << _label2Str(info.user_id, 16) << endl;
  out << "  xmit_power_watts: " << info.xmit_power_watts << endl;
  out << "  flags: " << info.flags << endl;
  out << "  polarization: " << info.polarization << endl;
  out << "  trunc_ht_above_radar_cm: "
      << info.trunc_ht_above_radar_cm << endl;
  out << "  beam_width_h: " << _binAngleToDouble(info.beam_width_h) << endl;
  out << "  beam_width_v: " << _binAngleToDouble(info.beam_width_v) << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const task_end_info_t &info, ostream &out)
  
{
  out << "~~~~~ END INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  id_major: " << info.id_major << endl;
  out << "  id_minor: " << info.id_minor << endl;
  out << "  task_name: " << _label2Str(info.task_name, 12) << endl;
  out << "  script: " << _label2Str(info.script, 80) << endl;
  out << "  ntasks_hybrid: " << info.ntasks_hybrid << endl;
  out << "  task_state: " << info.task_state << endl;
  out << "  start_time: " << _time2Str(info.start_time) << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void EdgeVol2Dsr::_print(const raw_product_header_t &hdr, ostream &out)

{
  out << "===== RAW HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  record_num: " << hdr.record_num << endl;
  out << "  sweep_num: " << hdr.sweep_num << endl;
  out << "  byte_offset: " << hdr.byte_offset << endl;
  out << "  ray_num: " << hdr.ray_num << endl;
  out << "  flags: " << hdr.flags << endl;
}

void EdgeVol2Dsr::_print(const ingest_data_header_t &hdr, ostream &out)
  
{
  out << "===== INGEST DATA HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  _print(hdr.id_hdr, out);
  out << "  time: " << _time2Str(hdr.time) << endl;
  out << "  sweep_num: " << hdr.sweep_num << endl;
  out << "  nrays_per_revolution: " << hdr.nrays_per_revolution << endl;
  out << "  angle_of_first_pointer: " << hdr.angle_of_first_pointer << endl;
  out << "  n_rays_total: " << hdr.n_rays_total << endl;
  out << "  n_rays_written: " << hdr.n_rays_written << endl;
  out << "  fixed_angle: " << _binAngleToDouble(hdr.fixed_angle) << endl;
  out << "  bits_per_bin: " << hdr.bits_per_bin << endl;
  out << "  data_code: " << hdr.data_code << endl;
}

void EdgeVol2Dsr::_print(const ray_header_t &hdr, ostream &out)
  
{
  out << "~~~~~ RAY HEADER ~~~~~" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  start_az: " << _binAngleToDouble(hdr.start_az) << endl;
  out << "  start_el: " << _binAngleToDouble(hdr.start_el) << endl;
  out << "  end_az: " << _binAngleToDouble(hdr.end_az) << endl;
  out << "  end_el: " << _binAngleToDouble(hdr.end_el) << endl;
  out << "  n_gates: " << hdr.n_gates << endl;
  out << "  seconds: " << hdr.seconds << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

////////////////////////////////////////////////////////////////
// byte swap routines
//
// These only activate if _needToSwap has been set to true.
// Otherwise they simply return.

void EdgeVol2Dsr::_swap(si16 *vals, int n)
{
  if (!_needToSwap) return;
  BE_swap_array_16(vals, n * sizeof(si16));
}

void EdgeVol2Dsr::_swap(ui16 *vals, int n)

{
  if (!_needToSwap) return;
  BE_swap_array_16(vals, n * sizeof(ui16));
}

void EdgeVol2Dsr::_swap(si32 *vals, int n)
{
  if (!_needToSwap) return;
  BE_swap_array_32(vals, n * sizeof(si32));
}

void EdgeVol2Dsr::_swap(ui32 *vals, int n)

{
  if (!_needToSwap) return;
  BE_swap_array_32(vals, n * sizeof(ui32));
}

void EdgeVol2Dsr::_swap(fl32 *vals, int n)

{
  if (!_needToSwap) return;
  BE_swap_array_32(vals, n * sizeof(fl32));
}

void EdgeVol2Dsr::_swap(sigmet_id_hdr_t &val)

{
  if (!_needToSwap) return;
  _swap(&val.id, 2);
  _swap(&val.nbytes, 1);
  _swap(&val.flags, 1);
}

void EdgeVol2Dsr::_swap(sigmet_time_t &val)

{
  if (!_needToSwap) return;
  _swap(&val.sec, 1);
  _swap(&val.msecs, 4);
}

void EdgeVol2Dsr::_swap(prod_header_t &val)
{
  if (!_needToSwap) {
    return;
  }
  _swap(val.id_hdr);
  _swap(val.conf);
  _swap(val.end);
}

void EdgeVol2Dsr::_swap(prod_conf_t &val)
{
  if (!_needToSwap) {
    return;
  }
  _swap(val.id_hdr);
  _swap(&val.ptype, 2);
  _swap(&val.secs_between_runs, 1);
  _swap(val.product_time);
  _swap(val.ingest_time);
  _swap(val.start_sched_time);
  _swap(&val.flags, 1);
  _swap(&val.xscale, 10);
  _swap(&val.data_type_out, 1);
  _swap(&val.data_type_in, 1);
  _swap(&val.radial_smoothing_range_km_100, 1);
  _swap(&val.n_runs, 1);
  _swap(&val.zr_coeff, 2);
  _swap(&val.twod_smooth_x, 2);
}

void EdgeVol2Dsr::_swap(prod_end_t &val)
{
  if (!_needToSwap) return;
  _swap(val.oldest_data_time);
  _swap(&val.minutes_lst_west_of_gmt, 1);
  _swap(&val.minutes_rec_west_of_gmt, 1);
  _swap(&val.latitude_center, 2);
  _swap(&val.ground_ht_msl_meters, 2);
  _swap(&val.prf_hz, 2);
  _swap(&val.dsp_type, 3);
  _swap(&val.dop_filter_first_bin, 1);
  _swap(&val.wavelength_cm_100, 5);
  _swap(&val.input_file_count, 7);
  _swap(&val.lambert_lat1, 6);
  _swap(&val.log_filter_first, 2);
  _swap(&val.proj_ref_lat, 2);
  _swap(&val.sequence_num, 1);
  _swap(&val.melting_ht_m_msl, 3);
}

void EdgeVol2Dsr::_swap(ingest_header_t &val)
{
  if (!_needToSwap) return;
  _swap(val.id_hdr);
  _swap(val.ingest_conf);
  _swap(val.task_conf);
}


void EdgeVol2Dsr::_swap(ingest_conf_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.num_data_files, 2);
  _swap(&val.total_size_files, 1);
  _swap(val.volume_time);
  _swap(&val.nbytes_in_ray_hdrs, 4);
  _swap(&val.time_zone_local_mins_west_of_gmt, 1);
  _swap(&val.time_zone_rec_mins_west_of_gmt, 1);
  _swap(&val.latitude, 2);
  _swap(&val.ground_ht_msl_meters, 6);
  _swap(&val.radar_ht_msl_cm, 8);
  _swap(&val.ht_melting_m_msl, 1);
  _swap(&val.flags, 1);
}

void EdgeVol2Dsr::_swap(task_conf_t &val)
{
  if (!_needToSwap) return;
  _swap(val.id_hdr);
  _swap(val.sched_info);
  _swap(val.dsp_info);
  _swap(val.calib_info);
  _swap(val.range_info);
  _swap(val.scan_info);
  _swap(val.misc_info);
  _swap(val.end_info);
}

void EdgeVol2Dsr::_swap(task_sched_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.start_secs, 6);
  _swap(&val.flag, 1);
}

void EdgeVol2Dsr::_swap(dsp_data_mask_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.word_0, 6);
}

void EdgeVol2Dsr::_swap(task_dsp_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.major_mode, 2);
  _swap(val.data_mask);
  _swap(val.data_mask_orig);
  _swap(&val.low_prf_hz, 7);
  _swap(&val.prf_hz, 2);
  _swap(&val.trig_rate_flag, 5);
  _swap(&val.fixed_gain_1000, 4);
  _swap(&val.config_header_command_mask, 1);
  _swap(&val.ts_playback_flags, 1);
}

void EdgeVol2Dsr::_swap(task_calib_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.log_slope, 5);
  _swap(&val.z_calib_16, 6);
  _swap(&val.flags, 1);
  _swap(&val.ldr_bias_100, 11);
}

void EdgeVol2Dsr::_swap(task_range_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.range_first_bin_cm, 2);
  _swap(&val.n_input_gates, 2);
  _swap(&val.input_gate_spacing_cm, 2);
  _swap(&val.bin_length_variable, 3);
}

void EdgeVol2Dsr::_swap(scan_info_union_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.ppi.start_az, 2 + MAX_SWEEPS);
}

void EdgeVol2Dsr::_swap(task_scan_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.scan_mode, 4);
  _swap(val.u);
}

void EdgeVol2Dsr::_swap(task_misc_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.wavelength_cm_100, 1);
  _swap(&val.xmit_power_watts, 1);
  _swap(&val.flags, 2);
  _swap(&val.trunc_ht_above_radar_cm, 1);
  _swap(&val.nbytes_comment, 1);
  _swap(&val.beam_width_h, 12);
}

void EdgeVol2Dsr::_swap(task_end_info_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.id_major, 2);
  _swap(&val.ntasks_hybrid, 1);
  _swap(&val.task_state, 1);
  _swap(val.start_time);
}

void EdgeVol2Dsr::_swap(raw_product_header_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.record_num, 5);
}

void EdgeVol2Dsr::_swap(ingest_data_header_t &val)
{
  if (!_needToSwap) return;
  _swap(val.id_hdr);
  _swap(val.time);
  _swap(&val.sweep_num, 8);
}

void EdgeVol2Dsr::_swap(ray_header_t &val)
{
  if (!_needToSwap) return;
  _swap(&val.start_az, 6);
}

