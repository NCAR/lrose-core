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
// SpolSysconRelay.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// SpolSysconRelay reads data from a server in TCP, and writes
// it out to an FMQ
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include "SpolSysconRelay.hh"
#include "OpsInfo.hh"
#include "RsmInfo.hh"

using namespace std;

// Constructor

SpolSysconRelay::SpolSysconRelay(int argc, char **argv)
  
{

  isOK = true;

  _opsInfo = NULL;
  _rsmInfo = NULL;

  // set programe name
 
  _progName = "SpolSysconRelay";

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

  // set up hearbeat func for reading from socket

  _heartBeatFunc = PMU_auto_register;

  // open the output fmq

  if (_params.monitor_mode) {
    if (_openMonitorFmq()) {
      isOK = false;
      return;
    }
  } else {
    if (_openOutputFmq()) {
      isOK = false;
      return;
    }
  }
  
  // create info objects

  if (!_params.monitor_mode) {
    _opsInfo = new OpsInfo(_progName, _params, _outputFmq);
    if (_params.print_rsm_info) {
      _rsmInfo = new RsmInfo(_progName, _params);
    }
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

SpolSysconRelay::~SpolSysconRelay()

{

  if (_opsInfo) {
    delete _opsInfo;
  }

  if (_rsmInfo) {
    delete _rsmInfo;
  }

  if (_params.monitor_mode) {
    _monitorFmq.closeMsgQueue();
  } else {
    _outputFmq.closeMsgQueue();
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpolSysconRelay::Run ()
{

  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running SpolSysconRelay - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running SpolSysconRelay - debug mode" << endl;
  }
  
  if (_params.monitor_mode) {
    return _runMonitor();
  } else {
    return _run();
  }

}
//////////////////////////////////////////////////
// Run in normal mode

int SpolSysconRelay::_run()
{

  int iret = 0;

  while (true) {
    
    PMU_auto_register("Reading ...");

    // every 100 msecs, check ops info is available

    while (_opsInfo->checkForData(100) == 0) {
      int iwrfId;
      if (_opsInfo->read(iwrfId)) {
        cerr << "ERROR - SpolSysconRelay::_run()" << endl;
        cerr << "  Reading ops info" << endl;
        umsleep(1000);
        iret = -1;
        break;
      }
      if (iwrfId == 0) {
        umsleep(100);
        break;
      }
    }
    
    // read any rsm packets available

    if (_rsmInfo) {
      while (_rsmInfo->checkForData(100) == 0) {
	if (_rsmInfo->read()) {
	  cerr << "ERROR - SpolSysconRelay::_run()" << endl;
	  cerr << "  Reading rsm info" << endl;
	  iret = -1;
	  break;
	}
      }
    }

    umsleep(100); // perhaps a good idea?

  } // while(true)

  return iret;
  
}


//////////////////////////////////////////////////
// Run in monitor mode

int SpolSysconRelay::_runMonitor()
{

  while (true) {

    PMU_auto_register("Monitoring FMQ");

    // read in a message
    
    if (_monitorFmq.readMsgBlocking()) {
      cerr << "ERROR -  SpolSysconRelay::_runMonitor" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.monitor_fmq_path << endl;
      cerr << _monitorFmq.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "--->> Got  FMQ message" << endl;
    }
    
    const void *msg = _monitorFmq.getMsg();
    int len = _monitorFmq.getMsgLen();
    int msgType = _monitorFmq.getMsgType();
    
    switch (msgType) {

      case IWRF_RADAR_INFO_ID:
        if (len == sizeof(iwrf_radar_info_t)) {
          iwrf_radar_info_t iwrfRadarInfo;
          memcpy(&iwrfRadarInfo, msg, sizeof(iwrfRadarInfo));
          iwrf_radar_info_print(stdout, iwrfRadarInfo);
        }
        break;

      case IWRF_SCAN_SEGMENT_ID:
        if (len == sizeof(iwrf_scan_segment_t)) {
          iwrf_scan_segment_t iwrfScanSeg;
          memcpy(&iwrfScanSeg, msg, sizeof(iwrfScanSeg));
          iwrf_scan_segment_print(stdout, iwrfScanSeg);
        }
        break;
          
      case IWRF_TS_PROCESSING_ID:
        if (len == sizeof(iwrf_ts_processing_t)) {
          iwrf_ts_processing_t iwrfTsProc;
          memcpy(&iwrfTsProc, msg, sizeof(iwrfTsProc));
          iwrf_ts_processing_print(stdout, iwrfTsProc);
        }
        break;

      case IWRF_XMIT_POWER_ID:
        if (len == sizeof(iwrf_xmit_power_t)) {
          iwrf_xmit_power_t iwrfXmitPower;
          memcpy(&iwrfXmitPower, msg, sizeof(iwrfXmitPower));
          iwrf_xmit_power_print(stdout, iwrfXmitPower);
        }
        break;

      case IWRF_EVENT_NOTICE_ID:
        if (len == sizeof(iwrf_event_notice_t)) {
          iwrf_event_notice_t iwrfEventNotice;
          memcpy(&iwrfEventNotice, msg, sizeof(iwrfEventNotice));
          iwrf_event_notice_print(stdout, iwrfEventNotice);
        }
        break;

      default: {}
        
    }
    
    fflush(stdout);

  } // while
  
  return 0;
  
}

////////////////////////////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int SpolSysconRelay::_openOutputFmq()
  
{
  
  // initialize the output FMQ
  
  if (_outputFmq.initReadWrite(_params.output_fmq_path,
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
    cerr << _outputFmq.getErrStr() << endl;
    return -1;
  }

  _outputFmq.setSingleWriter();

  if (_params.data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap(true,
                                   _params.data_mapper_report_interval);
  }

  return 0;

}

////////////////////////////////////////////////////////////
// open the output FMQ for monitoring
// returns 0 on success, -1 on failure

int SpolSysconRelay::_openMonitorFmq()
  
{
  
  if (_monitorFmq.initReadBlocking
      (_params.monitor_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA,
       Fmq::END)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot open FMQ for monitoring: "
         << _params.monitor_fmq_path << endl;
    cerr << _monitorFmq.getErrStr() << endl;
    return -1;
  }

  return 0;

}

