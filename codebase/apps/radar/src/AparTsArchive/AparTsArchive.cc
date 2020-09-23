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
// AparTsArchive.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2020
//
///////////////////////////////////////////////////////////////
//
// AparTsArchive reads data from TsTcpServer in TS TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <dsserver/DsLdataInfo.hh>
#include "AparTsArchive.hh"

using namespace std;

// Constructor

AparTsArchive::AparTsArchive(int argc, char **argv)
  
{

  isOK = true;
  MEM_zero(_scanPrev);
  _nPulses = 0;
  _prevSeqNum = 0;
  
  // set programe name
  
  _progName = "AparTsArchive";

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
  
  // create the reader from FMQ
  
  AparTsDebug_t aparDebug = AparTsDebug_t::OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    aparDebug = AparTsDebug_t::NORM;
  } 
  _pulseReader = new AparTsReaderFmq(_params.fmq_name, aparDebug);
  
  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  // compute sector size etc for files

  _out = NULL;
  _needNewFile = true;
  _nPulsesFile = 0;

  // run period and exit time

  _startTime = time(NULL);
  _exitTime = -1;
  if (_params.exit_after_specified_period) {
    _exitTime = _startTime + _params.run_period_secs;
    if (_params.debug) {
      cerr << "NOTE: run period (secs): " << _params.run_period_secs << endl;
      cerr << "      will exit at time: " << DateTime::strm(_exitTime) << endl;
    }
  }

  return;
  
}

// destructor

AparTsArchive::~AparTsArchive()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AparTsArchive::Run ()
{
  
  PMU_auto_register("Run");
  
  while (true) {

    time_t now = time(NULL);
    if (_exitTime > 0 && now > _exitTime) {
      if (_params.debug) {
        cerr << "Exiting at time: " << DateTime::strm(now) << endl;
      }
      return 0;
    }
    
    // read next pulse
    
    AparTsPulse *pulse = _pulseReader->getNextPulse();
    if (pulse == NULL) {
      return 0;
    }
    _currentScanMode = (apar_ts_scan_mode_t) pulse->getScanMode();

    // check if we need a new file? If so open file
    
    if (_checkNeedNewFile(*pulse)) {
      return -1;
    }
    
    // write pulse info to file, if info has changed since last write

    if (_out != NULL) {
      if (_handlePulse(*pulse)) {
	return -1;
      }
    }

    // delete pulse to clean up

    delete pulse;
  
  } // while
  
  return 0;
  
}

/////////////////////////////////////////
// Check if we need a new file
// Opens new file as needed.
//
// Returns 0 to continue, -1 to exit

int AparTsArchive::_checkNeedNewFile(const AparTsPulse &pulse)

{

  bool needNewFile = false;

  const AparTsInfo &info = pulse.getTsInfo();
  const apar_ts_scan_segment_t &scan = info.getScanSegment();

  if (scan.scan_mode != _scanPrev.scan_mode ||
      scan.volume_num != _scanPrev.volume_num ||
      scan.sweep_num != _scanPrev.sweep_num) {
    if (_params.debug) {
      cerr << "==>> New scan info" << endl;
      if (_params.debug >= Params::DEBUG_EXTRA) {
	apar_ts_scan_segment_print(stderr, scan);
      }
    }
    needNewFile = true;
    _scanPrev = scan;
  }
  
  // do we have too many pulses in the file?
  
  _nPulsesFile++;
  if (_nPulsesFile > _params.max_pulses_per_file) {
    needNewFile = true;
  }

  // open a new file if needed
    
  if (needNewFile) {
    
    // check for special case of writing one file only
    
    if (_out != NULL && _params.one_file_only) {
      _closeFile();
      return -1;
    }

    _nPulsesFile = 0;
    
    if (_openNewFile(pulse) == 0) {

      if (_params.debug >= Params::DEBUG_EXTRA) {
	info.print(stderr);
      }

      // write ops info to the start of the file
      
      info.writeMetaToFile(_out, 0);
  
    }

  }

  return 0;

}

/////////////////////////////
// handle a pulse

int AparTsArchive::_handlePulse(AparTsPulse &pulse)

{

  // write ops info to file, if info has changed since last write
  
  const AparTsInfo &info = pulse.getTsInfo();

  info.writeMetaQueueToFile(_out, true);

  // reformat pulse as needed

  if (_params.output_packing == Params::PACKING_FL32) {
    pulse.convertToPacked(apar_ts_iq_encoding_t::FL32);
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    pulse.convertToPacked(apar_ts_iq_encoding_t::SCALED_SI16);
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    pulse.convertToPacked(apar_ts_iq_encoding_t::DBM_PHASE_SI16);
  }

  // write pulse to file
  
  pulse.writeToFile(_out);

  _nPulses++;
  si64 seqNum = pulse.getSeqNum();
  double thisEl = pulse.getElevation();
  double thisAz = pulse.getAzimuth();
  
  if (_prevSeqNum > 0 && seqNum > (_prevSeqNum + 1)) {
    cerr << "WARNING - missing sequence numbers, prev: " << _prevSeqNum
         << ", latest: " << seqNum << endl;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    if ((_nPulses % 1000) == 0) {
      cerr << "El, az, npulses received: "
           << thisEl << " " << thisAz << " " << _nPulses << endl; 
    }
  } // debug

  _prevSeqNum = seqNum;

  return 0;

}

/////////////////////////////////
// open a new file

int AparTsArchive::_openNewFile(const AparTsPulse &pulse)

{

  // close out old file

  _closeFile();

  // get time

  time_t ptime = pulse.getTime();
  int nanoSecs = pulse.getNanoSecs();
  int milliSecs = nanoSecs / 1000000;
  if (milliSecs > 999) {
    milliSecs = 999;
  }

  date_time_t ttime;
  ttime.unix_time = ptime;
  _outputTime = ptime;
  uconvert_from_utime(&ttime);

  // compute antenna pos strings
  
  apar_ts_scan_mode_t scanMode = pulse.getScanMode();

  char fixedAngleStr[64];
  char movingAngleStr[64];
  
  if (scanMode == apar_ts_scan_mode_t::RHI) {
    double el = pulse.getElevation();
    if (el < 0) {
      el += 360;
    }
    double az = pulse.getAzimuth();
    if (_params.use_fixed_angle_for_file_name) {
      az = pulse.getFixedAngle();
      if (az < -9990) {
        az = 999; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (az * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (el + 0.5));
  } else {
    double az = pulse.getAzimuth();
    if (az < 0) {
      az += 360;
    }
    double el = pulse.getElevation();
    if (_params.use_fixed_angle_for_file_name) {
      el = pulse.getFixedAngle();
      if (el < -9990) {
        el = 99.9; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (el * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (az + 0.5));
  }

  // compute scan mode string
  
  string scanModeStr;
  if (_params.add_scan_mode_to_file_name) {
    switch (scanMode) {
      case apar_ts_scan_mode_t::RHI:
        scanModeStr = ".rhi";
        break;
      case apar_ts_scan_mode_t::PPI:
      default:
        scanModeStr = ".ppi";
    }
  }

  // make the output dir

  char subdir[1024];
  _outputDir = _params.output_dir;
  if (_params.debug) {
    cerr << "Using outputDir: " << _outputDir << endl;
  }
  
  sprintf(subdir, "%s/%.4d%.2d%.2d", _outputDir.c_str(),
          ttime.year, ttime.month, ttime.day);

  if (ta_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - AparTsArchive" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output path

  string format;
  format = ".apar_ts";

  string packing;
  if (_params.output_packing == Params::PACKING_FL32) {
    packing = ".fl32";
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    packing = ".scaled";
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    packing = ".dbm_phase";
  }

  char name[1024];
  sprintf(name, "%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d%s%s%s%s%s",
          ttime.year, ttime.month, ttime.day,
          ttime.hour, ttime.min, ttime.sec, milliSecs,
	  fixedAngleStr, movingAngleStr, scanModeStr.c_str(),
          packing.c_str(), format.c_str());
  _outputName = name;

  char relPath[1024];
  sprintf(relPath, "%.4d%.2d%.2d/%s",
          ttime.year, ttime.month, ttime.day, name);
  _relPath = relPath;

  char path[1024];
  sprintf(path, "%s/%s", _outputDir.c_str(), relPath);
  
  // open file

  if ((_out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - AparTsArchive" << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened new file: " << _relPath << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close file
//
// Returns 0 if file already open, -1 if not

int AparTsArchive::_closeFile()

{

  // close out old file
  
  if (_out != NULL) {

    fclose(_out);
    _out = NULL;

    if (_params.debug) {
      cerr << "Done with file: " << _relPath << endl;
    }

    // write latest data info file
    
    DsLdataInfo ldata(_outputDir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(_outputTime);
    ldata.setRelDataPath(_relPath);
    ldata.setWriter("AparTsArchive");
    ldata.setDataType("apar_ts");
    if (ldata.write(_outputTime)) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _outputDir << endl;
    }

    return 0;

  }

  return -1;

}
  
