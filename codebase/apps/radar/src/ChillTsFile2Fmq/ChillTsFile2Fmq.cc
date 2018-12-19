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
// ChillTsFile2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
// ChillTsFile2Fmq reads legacy CHILL time-series data from a file
// and reformats it into IWRF time series forma.
// It saves the time series data out to a file message queue (FMQ),
// which can be read by multiple clients. Its purpose is mainly
// for simulation and debugging of time series operations.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <ctime>
#include <dataport/swap.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DmapAccess.hh>
#include <radar/chill_to_iwrf.hh>
#include "ChillTsFile2Fmq.hh"

using namespace std;

// Constructor

ChillTsFile2Fmq::ChillTsFile2Fmq(int argc, char **argv) :
        _iwrfPulse(_iwrfInfo)
  
{

  isOK = true;
  _nPulses = 0;

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

  iwrf_radar_info_init(_iwrfRadarInfo);
  iwrf_scan_segment_init(_iwrfScanSeg);
  iwrf_antenna_correction_init(_iwrfAntCorr);
  iwrf_ts_processing_init(_iwrfTsProc);
  iwrf_xmit_power_init(_iwrfXmitPower);
  iwrf_xmit_sample_init(_iwrfXmitSample);
  iwrf_calibration_init(_iwrfCalib);
  iwrf_event_notice_init(_iwrfEventNotice);
  iwrf_phasecode_init(_iwrfPhasecode);
  iwrf_xmit_info_init(_iwrfXmitInfo);

  _packetSeqNum = 0;
  _pulseSeqNum = 0;
  _usecsSleepSim = 0;

  _nGatesRemove = 0;

  _reader = NULL;

  // set programe name
 
  _progName = "ChillTsFile2Fmq";

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

  // initialize message
  
  _msg.clearAll();
  _msg.setType(0);

  if (_params.mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    _reader = new DsInputPath(_progName,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.input_dir,
                              _params.max_realtime_valid_age,
                              PMU_auto_register,
                              _params.use_ldata_info_file,
                              true);
    
  } else {
    
    // ARCHIVE or SIMULATE mode

    if (_args.inputFileList.size() < 1) {
      cerr << "ERROR - " << _progName << endl;
      cerr << "  ARCHIVE or SIMULATE mode" << endl;
      cerr << "  No files specified" << endl;
      cerr << "  You must specify a file list on the command line" << endl;
      isOK = false;
      return;
    }
    
    _reader = new DsInputPath(_progName,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _args.inputFileList);
    
  }
  
  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

ChillTsFile2Fmq::~ChillTsFile2Fmq()

{

  if (_reader) {
    delete _reader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ChillTsFile2Fmq::Run ()
{
  
  int iret = 0;
  
  if (_params.mode == Params::SIMULATE) {
    
    // simulate mode - go through the file list repeatedly

    while (true) {
      
      PMU_auto_register("Run");
      _reader->reset();

      char *path;
      while ((path = _reader->next()) != NULL) {
        _filePath = path;
        if (_processFile()) {
          cerr << "WARNING - ChillTsFile2Fmq" << endl;
          cerr << "  SIMULATE mode" << endl;
          cerr << "  Errors in processing file: " << path << endl;
          iret = -1;
        }
      } // while ((filePath ...
      
    } // while (true)

  } else {
    
    // loop until end of data
    
    char *path;
    while ((path = _reader->next()) != NULL) {
      _filePath = path;
      if (_processFile()) {
        cerr << "WARNING - ChillTsFile2Fmq::Ru" << endl;
        cerr << "  Errors in processing file: " << path << endl;
        iret = -1;
      }
    } // while ((filePath ...
    
  }

  return iret;

}

//////////////////////////////////////////////////
// process a file

int ChillTsFile2Fmq::_processFile()
{
  
  PMU_auto_register(_filePath.c_str());

  FILE *in;
  if ((in = fopen(_filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ChillTsFile2Fmq::_processFile" << endl;
    cerr << "  Cannot open file: " << _filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Processing file: " << _filePath << endl;
  }

  while (!feof(in)) {

    // read in generic packet header

    generic_packet_header_t genHdr;
    if (fread(&genHdr, sizeof(genHdr), 1, in) != 1) {
      if (feof(in)) {
        break;
      } else {
        int errNum = errno;
        cerr << "ERROR - ChillTsFile2Fmq::_processFile" << endl;
        cerr << "  Reading generic packet header" << endl;
        cerr << "  File: " << _filePath << endl;
        cerr << "  " << strerror(errNum) << endl;
      }
    }

    if (genHdr.magic_word == HS2_MAGIC) {

      // read in id and length of payload
      
      int idLen[2];
      if (fread(idLen, sizeof(idLen), 1, in) != 1) {
        if (feof(in)) {
          break;
        } else {
          int errNum = errno;
          cerr << "ERROR - ChillTsFile2Fmq::_processFile" << endl;
          cerr << "  Reading idLen[2]" << endl;
          cerr << "  File: " << _filePath << endl;
          cerr << "  " << strerror(errNum) << endl;
        }
      }
      
      int id = idLen[0];
      int len = idLen[1];
      int idLenSize = sizeof(idLen);

      // seek back to start of housekeeping struct
      
      _seekAhead(in, -1 * idLenSize);
      
      switch (id) {

        case HSK_ID_RADAR_INFO:
          _readRadarInfo(in);
          break;
        case HSK_ID_SCAN_SEG:
          _readScanSeg(in);
          break;
        case HSK_ID_PROCESSOR_INFO:
          _readProcInfo(in);
          break;
        case HSK_ID_PWR_UPDATE:
          _readPowerUpdate(in);
          break;
        case HSK_ID_EVENT_NOTICE:
          _readEventNotice(in);
          break;
        case HSK_ID_CAL_TERMS:
          _readCalTerms(in);
          break;
        case HSK_ID_XMIT_INFO:
          _readXmitInfo(in);
          break;
        case HSK_ID_ANT_OFFSET:
          _readAntCorr(in);
          break;
        case HSK_ID_XMIT_SAMPLE:
          _readXmitSample(in);
          break;
        case HSK_ID_PHASE_CODE:
          _readPhaseCode(in);
          break;
        default:
          if (_params.debug) {
            cerr << "===== NOT HANDLED YET ==================" << endl;
            fprintf(stderr, "  id: %x, %d\n", id, id);
            cerr << "  len: " <<  len << endl;
            cerr << "========================================" << endl;
          }
          _seekAhead(in, genHdr.payload_length);

      } // switch (id)
      
    } else if (genHdr.magic_word == TS_MAGIC) {

      _readPulse(in, genHdr);

    } // if (genHdr.magic_word == HS2_MAGIC) {
      
  } // while (!feof(in))

  return 0;

}

//////////////////////////////////////////////////
// seek ahead

int ChillTsFile2Fmq::_seekAhead(FILE *in, int offset)
  
{
  if (fseek(in, offset, SEEK_CUR)) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << endl;
    cerr << "  Seeking in file: " << _filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    cerr << "  Current pos: " << ftell(in) << endl;
    cerr << "  Seek offset: " << offset << endl;
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////
// read radarInfo

int ChillTsFile2Fmq::_readRadarInfo(FILE *in)
  
{

  // read in

  if (fread(&_radarInfo, sizeof(_radarInfo), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readRadarInfo" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_radar_info_print(cerr, _radarInfo);
  }
  
  _radarInfoAvail = true;

  // convert to IWRF

  chill_iwrf_radar_info_load(_radarInfo, _packetSeqNum, _iwrfRadarInfo);
  _packetSeqNum++;

  // in sim mode, sleep a while if needed

  if (_params.mode == Params::SIMULATE) {
    STRncopy(_iwrfRadarInfo.radar_name, _params.sim_mode_radar_name,
             IWRF_MAX_RADAR_NAME);
    STRncopy(_iwrfRadarInfo.site_name, _params.sim_mode_site_name,
             IWRF_MAX_SITE_NAME);
    _iwrfRadarInfo.latitude_deg = _params.sim_mode_latitude_deg;
    _iwrfRadarInfo.longitude_deg = _params.sim_mode_longitude_deg;
    _iwrfRadarInfo.altitude_m = _params.sim_mode_altitude_meters;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, _iwrfRadarInfo);
  }

  // add to message
  
  _msg.addPart(IWRF_RADAR_INFO_ID, sizeof(_iwrfRadarInfo), &_iwrfRadarInfo);

  return 0;

}

//////////////////////////////////////////////////
// read scan segment

int ChillTsFile2Fmq::_readScanSeg(FILE *in)
  
{

  // read in

  if (fread(&_scanSeg, sizeof(_scanSeg), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readScanSeg" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_scan_seg_print(cerr, _scanSeg);
  }
  
  _scanSegAvail = true;

  // convert to IWRF

  chill_iwrf_scan_seg_load(_scanSeg, _packetSeqNum, _iwrfScanSeg);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_scan_segment_print(stderr, _iwrfScanSeg);
  }
  
  // add to message
  
  _msg.addPart(IWRF_SCAN_SEGMENT_ID, sizeof(_iwrfScanSeg), &_iwrfScanSeg);

  return 0;

}

//////////////////////////////////////////////////
// read proc info

int ChillTsFile2Fmq::_readProcInfo(FILE *in)
  
{

  // read in

  if (fread(&_procInfo, sizeof(_procInfo), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readProcInfo" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_proc_info_print(cerr, _procInfo);
  }
  
  _procInfoAvail = true;

  // convert to IWRF

  chill_iwrf_ts_proc_load(_procInfo, _packetSeqNum, _iwrfTsProc);
  _packetSeqNum++;

  if (_params.remove_negative_range_gates) {
    _nGatesRemove =
      (int) (_iwrfTsProc.burst_range_offset_m / _iwrfTsProc.gate_spacing_m + 0.5);
    _iwrfTsProc.start_range_m += _nGatesRemove * _iwrfTsProc.gate_spacing_m;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===>>> Removing negative range gates, n = " << _nGatesRemove << endl;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_ts_processing_print(stderr, _iwrfTsProc);
  }
  
  // add to message
  
  _msg.addPart(IWRF_TS_PROCESSING_ID, sizeof(_iwrfTsProc), &_iwrfTsProc);

  return 0;

}

//////////////////////////////////////////////////
// read power update

int ChillTsFile2Fmq::_readPowerUpdate(FILE *in)
  
{

  // read in

  if (fread(&_powerUpdate, sizeof(_powerUpdate), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readPowerUpdate" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_power_update_print(cerr, _powerUpdate);
  }
  
  _powerUpdateAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_power_load(_powerUpdate, _packetSeqNum, _iwrfXmitPower);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_power_print(stderr, _iwrfXmitPower);
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_POWER_ID, sizeof(_iwrfXmitPower), &_iwrfXmitPower);

  return 0;

}

//////////////////////////////////////////////////
// read event notice

int ChillTsFile2Fmq::_readEventNotice(FILE *in)
  
{

  // read in

  if (fread(&_eventNotice, sizeof(_eventNotice), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readEventNotice" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_event_notice_print(cerr, _eventNotice);
  }
  
  _eventNoticeAvail = true;

  if (_scanSegAvail) {

    // convert to IWRF

    chill_iwrf_event_notice_load(_eventNotice, _scanSeg,
                                 _packetSeqNum, _iwrfEventNotice);
    _packetSeqNum++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      chill_event_notice_print(cerr, _eventNotice);
      iwrf_event_notice_print(stderr, _iwrfEventNotice);
    }

    // add to message
    
    _msg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(_iwrfEventNotice),
                 &_iwrfEventNotice);

  }
  
  return 0;

}

//////////////////////////////////////////////////
// read cal terms

int ChillTsFile2Fmq::_readCalTerms(FILE *in)
  
{

  // read in
  
  if (fread(&_calTerms, sizeof(_calTerms), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readCalTerms" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_cal_terms_print(cerr, _calTerms);
  }

  _calTermsAvail = true;

  if (_radarInfoAvail && _powerUpdateAvail) {

    // convert to IWRF

    chill_iwrf_calibration_load(_radarInfo, _calTerms, _powerUpdate,
                                _packetSeqNum, _iwrfCalib);
    _packetSeqNum++;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      iwrf_calibration_print(stderr, _iwrfCalib);
    }

    // add to message
    
    _msg.addPart(IWRF_CALIBRATION_ID, sizeof(_iwrfCalib), &_iwrfCalib);

  }
  
  return 0;

}

//////////////////////////////////////////////////
// read xmit info

int ChillTsFile2Fmq::_readXmitInfo(FILE *in)
  
{

  // read in

  if (fread(&_xmitInfo, sizeof(_xmitInfo), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readXmitInfo" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_xmit_info_print(cerr, _xmitInfo);
  }

  _xmitInfoAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_info_load(_xmitInfo, _packetSeqNum, _iwrfXmitInfo);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_xmit_info_print(cerr, _xmitInfo);
    iwrf_xmit_info_print(stderr, _iwrfXmitInfo);
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_INFO_ID, sizeof(_iwrfXmitInfo), &_iwrfXmitInfo);

  return 0;

}

//////////////////////////////////////////////////
// read antenna correction

int ChillTsFile2Fmq::_readAntCorr(FILE *in)
  
{

  // read in

  if (fread(&_antCorr, sizeof(_antCorr), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readAntCorr" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_ant_corr_print(cerr, _antCorr);
  }

  _antCorrAvail = true;
 
  // convert to IWRF

  chill_iwrf_ant_corr_load(_antCorr, _packetSeqNum, _iwrfAntCorr);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_antenna_correction_print(stderr, _iwrfAntCorr);
  }
  
  // add to message
  
  _msg.addPart(IWRF_ANTENNA_CORRECTION_ID, sizeof(_iwrfAntCorr), &_iwrfAntCorr);

  return 0;

}

//////////////////////////////////////////////////
// read xmit sample

int ChillTsFile2Fmq::_readXmitSample(FILE *in)
  
{

  // read in

  if (fread(&_xmitSample, sizeof(_xmitSample), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readXmitSample" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_xmit_sample_print(cerr, _xmitSample);
  }

  _xmitSampleAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_sample_load(_xmitSample, _packetSeqNum, _iwrfXmitSample);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_sample_print(stderr, _iwrfXmitSample);
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_SAMPLE_ID, sizeof(_iwrfXmitSample), &_iwrfXmitSample);
        
  return 0;

}

//////////////////////////////////////////////////
// read phase code

int ChillTsFile2Fmq::_readPhaseCode(FILE *in)
  
{

  // read in

  if (fread(&_phaseCode, sizeof(_phaseCode), 1, in) != 1) {
    if (!feof(in)) {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readPhaseCode" << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_phasecode_print(cerr, _phaseCode);
  }

  _phaseCodeAvail = true;

  // convert to IWRF

  chill_iwrf_phasecode_load(_phaseCode, _packetSeqNum, _iwrfPhasecode);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_phasecode_print(stderr, _iwrfPhasecode);
  }
  
  // add to message
  
  _msg.addPart(IWRF_PHASECODE_ID, sizeof(_iwrfPhasecode), &_iwrfPhasecode);

  return 0;

}

//////////////////////////////////////////////////
// read pulse

int ChillTsFile2Fmq::_readPulse(FILE *in,
                                const generic_packet_header &genHdr)
  
{

  timeseries_packet_header pulseHdr;
  memcpy(&pulseHdr, &genHdr, sizeof(pulseHdr));
  
  double elev = pulseHdr.antenna_elevation * 360.0 / 65536.0;
  if (elev > 180) {
    elev -= 360.0;
  }
  double az = pulseHdr.antenna_azimuth * 360.0 / 65536.0;
  int nanoSecs = pulseHdr.timestamp * 25;
  
  unsigned int timeOfDay = (pulseHdr.hdr_word3 << 15 ) >> 15;
  unsigned int hvFlag = (pulseHdr.hdr_word3 << 13) >> 30;
  unsigned int endOfInt = (pulseHdr.hdr_word3 << 12) >> 31;
  unsigned int pulseCount = (pulseHdr.hdr_word3 >> 23);
  
  int nGatesChill = (int) pulseHdr.gatecount;
  int nGatesOut = nGatesChill - _nGatesRemove;
  int nBytesData = nGatesChill * sizeof(timeseries_sample_t);
  time_t midnight = pulseHdr.hdr_word7;
  time_t pulseTimeSecs = midnight + timeOfDay;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "===============================================" << endl;
    fprintf(stderr, "CHILL PULSE HEADER:\n");
    fprintf(stderr, "  Time: %s.%.6d\n",
            DateTime::strm(pulseTimeSecs).c_str(),
            nanoSecs / 1000);
    fprintf(stderr, "  elev: %g\n", elev);
    fprintf(stderr, "  az: %g\n", az);
    fprintf(stderr, "  hvFlag: %d\n", hvFlag);
    fprintf(stderr, "  endOfInt: %d\n", endOfInt);
    fprintf(stderr, "  pulseCount: %d\n", pulseCount);
    fprintf(stderr, "  nGatesChill: %d\n", nGatesChill);
    fprintf(stderr, "  nGatesOut: %d\n", nGatesOut);
    fprintf(stderr, "  nBytesData: %d\n", nBytesData);
  }

  // read in IQ data

  TaArray<timeseries_sample_t> gateData_;
  timeseries_sample_t *gateData = gateData_.alloc(nGatesChill);

  if ((int) fread(gateData, sizeof(timeseries_sample_t),
                  nGatesChill, in) != nGatesChill) {
    if (feof(in)) {
      return 0;
    } else {
      int errNum = errno;
      cerr << "ERROR - ChillTsFile2Fmq::_readPulse" << endl;
      cerr << "  Reading gate data, nGatesChill: " << nGatesChill << endl;
      cerr << "  File: " << _filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  // set up IWRF pulse object
  
  _iwrfPulse.clear();
  _iwrfPulse.setPktSeqNum(_packetSeqNum);
  _packetSeqNum++;
  _iwrfPulse.setTime(pulseTimeSecs, nanoSecs);

  _iwrfPulse.set_pulse_seq_num(_pulseSeqNum);
  _pulseSeqNum++;
  _iwrfPulse.set_scan_mode(_iwrfScanSeg.scan_mode);
  _iwrfPulse.set_follow_mode(_iwrfScanSeg.follow_mode);
  _iwrfPulse.set_sweep_num(_iwrfScanSeg.sweep_num);
  _iwrfPulse.set_volume_num(_iwrfScanSeg.volume_num);
  if (_iwrfScanSeg.scan_mode == IWRF_SCAN_MODE_RHI) {
    _iwrfPulse.set_fixed_az(_iwrfScanSeg.current_fixed_angle);
  } else {
    _iwrfPulse.set_fixed_el(_iwrfScanSeg.current_fixed_angle);
  }
  _iwrfPulse.set_elevation(elev);
  _iwrfPulse.set_azimuth(az);
  _iwrfPulse.set_prt(_iwrfTsProc.prt_usec / 1.0e6);
  _iwrfPulse.set_prt_next(_iwrfTsProc.prt_usec / 1.0e6);
  _iwrfPulse.set_pulse_width_us(_iwrfTsProc.pulse_width_us);
  _iwrfPulse.set_n_gates(nGatesOut);
  _iwrfPulse.set_n_channels(2);
  _iwrfPulse.set_iq_encoding(IWRF_IQ_ENCODING_SCALED_SI16);
  if (hvFlag == 1) {
    _iwrfPulse.set_hv_flag(true);
  } else {
    _iwrfPulse.set_hv_flag(false);
  }
  _iwrfPulse.set_antenna_transition(false);
  _iwrfPulse.set_phase_cohered(true);
  _iwrfPulse.set_iq_offset(0, 0);
  _iwrfPulse.set_iq_offset(1, nGatesOut * 2);
  _iwrfPulse.set_scale(1.0);
  _iwrfPulse.set_offset(0.0);
  _iwrfPulse.set_n_gates_burst(0);
  _iwrfPulse.set_start_range_m(_iwrfTsProc.start_range_m);
  _iwrfPulse.set_gate_spacing_m(_iwrfTsProc.gate_spacing_m);

  // load IQ data into array for IWRF

  TaArray<si16> packed_;
  si16 *packed = packed_.alloc(nGatesOut * 4);
  int index = 0;
  for (int ii = _nGatesRemove; ii < nGatesChill; ii++) {
    packed[index++] = gateData[ii].i2;
    packed[index++] = gateData[ii].q2;
  }
  for (int ii = _nGatesRemove; ii < nGatesChill; ii++) {
    packed[index++] = gateData[ii].i1;
    packed[index++] = gateData[ii].q1;
  }

  _iwrfPulse.setIqPacked(nGatesOut, 2,
                         IWRF_IQ_ENCODING_SCALED_SI16, packed,
                         _params.iq_data_scale, _params.iq_data_offset);

  // assemble into a membuf

  MemBuf pulseBuf;
  _iwrfPulse.assemble(pulseBuf);
  _msg.addPart(IWRF_PULSE_HEADER_ID, pulseBuf.getLen(), pulseBuf.getPtr());

  // in sim mode, sleep a while if needed

  if (_params.mode == Params::SIMULATE && 
      _params.sim_mode_prt_usecs > 0) {
    _usecsSleepSim += _params.sim_mode_prt_usecs;
    if (_usecsSleepSim > 50000) {
      uusleep((int) _usecsSleepSim);
      _usecsSleepSim = 0;
    }
  }

  // if the message is large enough, write to the FMQ
  
  if (_writeToFmq()) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////
// write to output FMQ if msg large enough
// returns 0 on success, -1 on failure

int ChillTsFile2Fmq::_writeToFmq()

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
    
    
