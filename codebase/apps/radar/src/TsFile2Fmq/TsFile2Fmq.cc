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
// TsFile2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2009
//
///////////////////////////////////////////////////////////////
//
// TsFile2Fmq reads raw time-series data from a file.
// It saves the time series data out to a file message queue (FMQ),
// which can be read by multiple clients. Its purpose is mainly
// for simulation and debugging of time series operations.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <algorithm>
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
#include <dirent.h>
#include <dataport/swap.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DmapAccess.hh>
#include "TsFile2Fmq.hh"

using namespace std;

// Constructor

TsFile2Fmq::TsFile2Fmq(int argc, char **argv)
  
{

  isOK = true;
  _inputPulseCount = 0;
  _outputPulseCount = 0;
  _timeOffset = 0;
  _reader = NULL;

  // set programe name
 
  _progName = "TsFile2Fmq";

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
  
  if (_fmq.initReadWrite(_params.output_fmq_path,
			 _progName.c_str(),
			 _params.debug >= Params::DEBUG_EXTRA, // set debug?
			 Fmq::END, // start position
			 false,    // compression
			 _params.output_fmq_nslots,
			 _params.output_fmq_size)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
    cerr << _fmq.getErrStr() << endl;
    isOK = false;
    return;
  }
  _fmq.setSingleWriter();
  if (_params.output_fmq_blocking) {
    _fmq.setBlockingWrite();
  }
  if (_params.write_latest_data_info) {
    _fmq.setRegisterWithDmap(true, _params.latest_data_info_interval);
  }

  // initialize message
  
  _msg.clearAll();
  _msg.setType(0);

  // initialize the time series data reader object

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_NORM) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }

  if (_params.mode == Params::REALTIME) {
    _reader = new IwrfTsReaderFile(_params.input_dir,
				   _params.max_realtime_valid_age,
				   PMU_auto_register,
				   _params.use_ldata_info_file,
				   iwrfDebug);
  } else if (_params.mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() < 1) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  No files specified in ARCHIVE or SIMULATE mode" << endl;
      cerr << "  You must use the -f option to specify the file list." << endl;
      isOK = false;
      return;
    }
    _reader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  } else if (_params.mode == Params::SIMULATE) {
    vector<string> inputFileList = _args.inputFileList;
    if (inputFileList.size() < 1) {
      // get file list from input dir
      DIR *dirp;
      if ((dirp = opendir(_params.input_dir)) != NULL) {
        struct dirent *dp;
        for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
          if (dp->d_name[0] == '.') {
            continue;
          }
          // compute path
          string path = _params.input_dir;
          path += PATH_DELIM;
          path += dp->d_name;
          inputFileList.push_back(path);
        } // for (dp ...
        closedir(dirp);
      } // if ((dirp ...
      if (inputFileList.size() < 1) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  SIMULATE mode, no files found." << endl;
        cerr << "  You must either use the -f command line option" << endl;
        cerr << "  to specify the file list, or make sure there are" << endl;
        cerr << "  files in input_dir: " << _params.input_dir << endl;
        isOK = false;
        return;
      }
    }
    sort(inputFileList.begin(), inputFileList.end());
    _reader = new IwrfTsReaderFile(inputFileList, iwrfDebug);
    if (_params.debug) {
      cerr << "SIMULATE mode, using file list:" << endl;
      for (size_t ii = 0; ii < inputFileList.size(); ii++) {
        cerr << "  : " << inputFileList[ii] << endl;
      }
    }
  } // if (_params.mode == ...

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

TsFile2Fmq::~TsFile2Fmq()

{

  if (_reader) {
    delete _reader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsFile2Fmq::Run ()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.mode == Params::SIMULATE) {

    return _runSim();

  } else {
    
    // loop until end of data
    
    IwrfTsPulse *pulse;
    while ((pulse = _reader->getNextPulse()) != NULL) {
      PMU_auto_register("Procssing pulse");
      if (_processPulse(pulse)) {
	iret = -1;
      }
      delete pulse;
    } // while
      
  } // if (_params.mode == Params::SIMULATE)

  return iret;

}

//////////////////////////////////////////////////
// Run in simulate mdoe

int TsFile2Fmq::_runSim()
{
  
  // simulate mode - go through the file list repeatedly

  int iret = 0;
  long nPulses = 0;
  struct timeval startVal;
  gettimeofday(&startVal, NULL);
  double prevSecs = startVal.tv_sec + startVal.tv_usec * 1.0e-6;
  double effectivePrf = 0;
  double prevNPulses = 0;

  while (true) {
    
    _reader->reset();

    IwrfTsPulse *pulse;
    while ((pulse = _reader->getNextPulse()) != NULL) {
      
      PMU_auto_register("Simulate mode");
      nPulses++;

      if (_processPulse(pulse)) {
        iret = -1;
      }
      delete pulse;
      
      if (nPulses % 1000 == 0) {
        
        double desiredSecs = (1.0 / _params.sim_mode_prf) * 1000.0;
        struct timeval nowVal;
        gettimeofday(&nowVal, NULL);
        double nowSecs = nowVal.tv_sec + nowVal.tv_usec * 1.0e-6;
        double elapsedSecs = nowSecs - prevSecs;

        if (elapsedSecs < desiredSecs) {
          int usecsSleep = (int) ((desiredSecs - elapsedSecs) * 1.0e6 + 0.5);
          if (usecsSleep > 100) {
            uusleep(usecsSleep);
            if (_params.debug) {
              cerr << "---->>> sleeping secs: " << (usecsSleep * 1.0e-6) << endl;
            }
          }
          gettimeofday(&nowVal, NULL);
          nowSecs = nowVal.tv_sec + nowVal.tv_usec * 1.0e-6;
          elapsedSecs = nowSecs - prevSecs;
        } else {
          if (_params.debug) {
            cerr << "---->>> time elapsed, NOT sleeping" << endl;
          }
        }

        effectivePrf = (nPulses - prevNPulses) / elapsedSecs;
        prevSecs = nowSecs;
        prevNPulses = nPulses;
        
        if (_params.debug) {
          cerr << "-->>> time, nPulses, effective prf: "
               << DateTime::strm(time(NULL)) << ", "
               << nPulses << ", "
               << effectivePrf << endl;
        }
        
      } // if (nPulses % 1000 == 0)
      
    } // while (pulse ...

    umsleep(1000);
      
  } // while (true)

  return iret;

}

//////////////////////////////////////////////////
// process pulse

int TsFile2Fmq::_processPulse(IwrfTsPulse *pulse)
  
{

  int iret;
  _inputPulseCount++;

  if (_params.mode == Params::SIMULATE &&
      _params.sim_override_radar_params) {
    IwrfTsInfo &info = pulse->getTsInfo();
    info.set_radar_name(_params.sim_radar_name);
    info.set_radar_site_name(_params.sim_site_name);
    info.set_radar_latitude_deg(_params.sim_latitude_deg);
    info.set_radar_longitude_deg(_params.sim_longitude_deg);
    info.set_radar_altitude_m(_params.sim_altitude_meters);
    info.set_radar_beamwidth_deg_h(_params.sim_beam_width_h);
    info.set_radar_beamwidth_deg_v(_params.sim_beam_width_v);
  }
  
  if (_params.override_xmit_rcv_mode) {
    IwrfTsInfo &info = pulse->getTsInfo();
    info.set_proc_xmit_rcv_mode(_params.xmit_rcv_mode);
  }

  if (_params.sim_staggered_prt) {

    iret = _processPulseSimStaggeredPrt(pulse);

  } else if (_params.sim_select_pulse_interval) {
    
    iret = _processPulseSimSelectInterval(pulse);

  } else {

    iret = _processPulseNormal(pulse);

  }

  return iret;

}

//////////////////////////////////////////////////
// process pulse in normal mode

int TsFile2Fmq::_processPulseNormal(IwrfTsPulse *pulse)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Processing pulse, time: %s.%.3d\n",
	    DateTime::strm(pulse->getHdr().packet.time_secs_utc).c_str(),
	    (pulse->getHdr().packet.time_nano_secs / 1000000));
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    pulse->printHeader(stderr);
  }

  _setPulseTime(pulse);

  // add a second channel as needed
  
  if (_params.mode == Params::SIMULATE && _params.sim_add_dual_pol_channel) {
    _addSecondChannel(pulse);
  }

  // convert encoding as applicable

  _convertIqEncoding(pulse);

  // add ops info to message
  
  pulse->getTsInfo().addMetaQueueToMsg(_msg, true);

  // add pulse to message

  MemBuf buf;
  pulse->assemble(buf);
  _msg.addPart(IWRF_PULSE_HEADER_ID, buf.getLen(), buf.getPtr());

  // if the message is large enough, write to the FMQ
  
  if (_writeToFmq()) {
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process pulse in staggered PRT simulation

int TsFile2Fmq::_processPulseSimStaggeredPrt(IwrfTsPulse *pulse)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== INPUT PULSE ===================" << endl;
    fprintf(stderr, "time: %s.%.3d\n",
	    DateTime::strm(pulse->getHdr().packet.time_secs_utc).c_str(),
	    (pulse->getHdr().packet.time_nano_secs / 1000000));
    if (_params.debug >= Params::DEBUG_EXTRA) {
      pulse->printHeader(stderr);
    }
    cerr << "=========================================" << endl;
  }

  _setPulseTime(pulse);

  // add a second channel as needed

  if (_params.mode == Params::SIMULATE && _params.sim_add_dual_pol_channel) {
    _addSecondChannel(pulse);
  }

  int mm = 2, nn = 3;
  if (_params.stagger_mode == Params::STAGGER_2_3) {
    mm = 2;
    nn = 3;
  } else if (_params.stagger_mode == Params::STAGGER_3_4) {
    mm = 3;
    nn = 4;
  } else if (_params.stagger_mode == Params::STAGGER_4_5) {
    mm = 4;
    nn = 5;
  }

  int stride = mm + nn;
  int pos = _inputPulseCount % stride;
  bool outputPulse = false;

  if (pos == 0) {

    // the mm pulse has the shorter prt since this applies to
    // the prt since the previous pulse

    double fixedPrt = pulse->getPrt();
    pulse->set_prt(fixedPrt * nn);
    pulse->set_prt_next(fixedPrt * mm);
    pulse->set_pulse_seq_num(_outputPulseCount);
    _outputPulseCount++;
    outputPulse = true;

  } else if (pos == mm) {

    // the mm pulse has the longer prt since this applies to
    // the prt since the previous pulse

    double fixedPrt = pulse->getPrt();
    pulse->set_prt(fixedPrt * mm);
    pulse->set_prt_next(fixedPrt * nn);
    pulse->set_pulse_seq_num(_outputPulseCount);
    _outputPulseCount++;
    outputPulse = true;
    
  }

  // convert encoding as applicable

  _convertIqEncoding(pulse);

  if (outputPulse) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "======== OUTPUT STAGGERED PULSE ==========" << endl;
      fprintf(stderr, "time: %s.%.3d, prt: %g, prtNext %g\n",
              DateTime::strm(pulse->getHdr().packet.time_secs_utc).c_str(),
              (pulse->getHdr().packet.time_nano_secs / 1000000),
              pulse->get_prt(), pulse->get_prt_next());
      if (_params.debug >= Params::DEBUG_EXTRA) {
        pulse->printHeader(stderr);
      }
      cerr << "=========================================" << endl;
    }

    // add ops info to message
    
    pulse->getTsInfo().addMetaQueueToMsg(_msg, true);
    
    // add pulse to message
    
    MemBuf buf;
    pulse->assemble(buf);
    _msg.addPart(IWRF_PULSE_HEADER_ID, buf.getLen(), buf.getPtr());
    
    // if the message is large enough, write to the FMQ
    
    if (_writeToFmq()) {
      return -1;
    }

  } // if (outputPulse) ...
  
  return 0;

}

//////////////////////////////////////////////////
// process pulse in selected interval mode

int TsFile2Fmq::_processPulseSimSelectInterval(IwrfTsPulse *pulse)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== INPUT PULSE ===================" << endl;
    fprintf(stderr, "time: %s.%.3d\n",
	    DateTime::strm(pulse->getHdr().packet.time_secs_utc).c_str(),
	    (pulse->getHdr().packet.time_nano_secs / 1000000));
    if (_params.debug >= Params::DEBUG_EXTRA) {
      pulse->printHeader(stderr);
    }
    cerr << "=========================================" << endl;
  }

  _setPulseTime(pulse);

  // add a second channel as needed

  if (_params.mode == Params::SIMULATE && _params.sim_add_dual_pol_channel) {
    _addSecondChannel(pulse);
  }

  // convert encoding as applicable

  _convertIqEncoding(pulse);

  int interval = _params.sim_pulse_interval;
  int pos = _inputPulseCount % interval;
  if (pos == 0) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>>> pulse at selected interval" << endl;
    }
    
    double fixedPrt = pulse->getPrt();
    pulse->set_prt(fixedPrt * interval);
    pulse->set_prt_next(fixedPrt * interval);
    pulse->set_pulse_seq_num(_outputPulseCount);
    _outputPulseCount++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "======== OUTPUT PULSE ===================" << endl;
      fprintf(stderr, "time: %s.%.3d\n",
              DateTime::strm(pulse->getHdr().packet.time_secs_utc).c_str(),
              (pulse->getHdr().packet.time_nano_secs / 1000000));
      if (_params.debug >= Params::DEBUG_EXTRA) {
        pulse->printHeader(stderr);
      }
      cerr << "=========================================" << endl;
    }

    // add ops info to message
    
    pulse->getTsInfo().addMetaQueueToMsg(_msg, true);
    
    // add pulse to message
    
    MemBuf buf;
    pulse->assemble(buf);
    _msg.addPart(IWRF_PULSE_HEADER_ID, buf.getLen(), buf.getPtr());
    
    // if the message is large enough, write to the FMQ
    
    if (_writeToFmq()) {
      return -1;
    }

  } // if (outputPulse) ...
  
  return 0;

}

//////////////////////////////////////////////////
// add a second dual-pol channel to the pulse

int TsFile2Fmq::_addSecondChannel(IwrfTsPulse *pulse)
  
{

  if (pulse->getNChannels() > 1) {
    cerr << "ERROR - TsFile2Fmq::_addSecondChannel" << endl;
    cerr << "  Pulse already has multiple channels" << endl;
    cerr << "  nchannels: " << pulse->getNChannels() << endl;
    return -1;
  }

  // make sure we have floats

  pulse->convertToFL32();

  // create array for dual channels

  int nGates = pulse->getNGates();
  int nIq = nGates * 4;
  TaArray<fl32> iq_;
  fl32 *iq = iq_.alloc(nIq);

  // copy in channel 0 data

  memcpy(iq, pulse->getIq0(), nGates * 2 * sizeof(fl32));
 
  // set channel pointers

  fl32 *chan0 = iq;
  fl32 *chan1 = iq + (nGates * 2);

  for (int ii = 0; ii < nGates; ii++) {

    int ipos = ii * 2;
    int qpos = ipos + 1;

    double i0 = chan0[ipos];
    double q0 = chan0[qpos];

    double power0 = i0 * i0 + q0 * q0;
    double dbm0 = 10.0 * log10(power0);
    double phaseDeg0 = atan2(q0, i0) * RAD_TO_DEG;

    double dbm1 = dbm0 - _params.sim_fixed_zdr;
    double power1 = pow(10.0, dbm1 / 10.0);
    double mag1 = sqrt(power1);
    double phaseDeg1 = phaseDeg0 + _params.sim_fixed_phidp;
    double phaseRad1 = phaseDeg1 * DEG_TO_RAD;
    double i1 = mag1 * cos(phaseRad1);
    double q1 = mag1 * sin(phaseRad1);

    chan1[ipos] = i1;
    chan1[qpos] = q1;

  }

  // update iq data in pulse object

  pulse->setIqFloats(nGates, 2, iq);

  return 0;

}

//////////////////////////////////////////////////
// set the pulse time

void TsFile2Fmq::_setPulseTime(IwrfTsPulse *pulse)
  
{

  if (_params.set_time_to_now) {
    // optionally set the time to the current time
    time_t now = time(NULL);
    time_t modPulseTime = pulse->getTime() + _timeOffset;
    double deltaTime = fabs((double) now - (double) modPulseTime);
    if (deltaTime > 1 || _timeOffset == 0) {
      time_t pulseTime = pulse->getTime();
      _timeOffset = now - pulseTime;
    }
    pulse->setTime(pulse->getTime() + _timeOffset,
                   pulse->getNanoSecs());
  } else if (_params.set_time_offset) {
    // optionally offset the time
    pulse->setTime(pulse->getTime() + _params.time_offset_secs,
                   pulse->getNanoSecs());
  }

}

//////////////////////////////////////////////////
// set the output encoding

void TsFile2Fmq::_convertIqEncoding(IwrfTsPulse *pulse)
  
{

  if (!_params.convert_iq_encoding) {
    return;
  }

  switch (_params.iq_encoding) {
    case Params::IQ_ENCODING_FL32:
      pulse->convertToFL32();
      break;
    case Params::IQ_ENCODING_SCALED_SI16:
      pulse->convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
      break;
    case Params::IWRF_IQ_ENCODING_DBM_PHASE_SI16:
      pulse->convertToPacked(IWRF_IQ_ENCODING_DBM_PHASE_SI16);
      break;
    case Params::IWRF_IQ_ENCODING_SIGMET_FL16:
      pulse->convertToPacked(IWRF_IQ_ENCODING_SIGMET_FL16);
      break;
    default: {}
  } // switch

}

////////////////////////////////////////////
// write to output FMQ if msg large enough
// returns 0 on success, -1 on failure

int TsFile2Fmq::_writeToFmq()

{

  // if the message is large enough, write to the FMQ
  
  int nParts = _msg.getNParts();
  if (nParts < _params.n_pulses_per_message) {
    return 0;
  }

  void *buf = _msg.assemble();
  int len = _msg.lengthAssembled();
  if (_fmq.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - TsTcp2Fmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    _msg.clearParts();
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Wrote msg, nparts, len: "
         << nParts << ", " << len << endl;
  }
  _msg.clearParts();

  return 0;

}
    
    
