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
// TsScanInfoMerge.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////
//
// TsScanInfoMerge reads time-series data from 2 file message
// queues, a master and a slave. The master queue has some extra
// metadata missing in the slave. The slave queue is missing
// angle and scan meta data, and possibly time.
//
// The program synchronizes the two queues based on either (a) the
// pulse sequence number in the pulse headers, or (b) the time.
// It then copies the missing meta data information from the
// master to the slave. It writes out the updated slave queue.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include "TsScanInfoMerge.hh"

using namespace std;

// Constructor

TsScanInfoMerge::TsScanInfoMerge(int argc, char **argv)
  
{

  isOK = true;

  _masterReader = NULL;
  _slaveReader = NULL;
  
  // set programe name
  
  _progName = "TsScanInfoMerge";
  
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
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

TsScanInfoMerge::~TsScanInfoMerge()

{

  if (_masterReader) {
    delete _masterReader;
  }

  if (_slaveReader) {
    delete _slaveReader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Outer run method

int TsScanInfoMerge::Run()
{
  
  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running TsScanInfoMerge - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running TsScanInfoMerge - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running TsScanInfoMerge - debug mode" << endl;
  }

  int iret = 0;

  while (true) {

    PMU_auto_register("Run");

    if (_init()) {
      iret = -1;
      break;
    }

    if (_params.sync_mode == Params::SYNC_USING_NUMBERS) {
      if (_syncUsingNumbers()) {
        umsleep(1000);
        iret = -1;
      }
    } else {
      if (_syncUsingTime()) {
        umsleep(1000);
        iret = -1;
      }
    }
    
  } // while

  return iret;

}

//////////////////////////////////////////////////
// synchronize using pulse sequence numbers

int TsScanInfoMerge::_syncUsingNumbers()
{
  
  while (true) {
    
    PMU_auto_register("_run");

    if (_slaveSeqNum == 0 && _masterSeqNum != 0) {
      _readSlave();
      cerr << "ERROR - TsScanInfoMerge::_run" << endl;
      cerr << "  Forcing read on slave because seq num is 0" << endl;
      continue;
    }

    if (_masterSeqNum == 0) {
      _readMaster();
      cerr << "ERROR - TsScanInfoMerge::_run" << endl;
      cerr << "  Forcing read on master because seq num is 0" << endl;
      continue;
    }

    double diff = fabs((double) _masterSeqNum - (double) _slaveSeqNum);
    if (diff > 50000) {
      cerr << "ERROR - TsScanInfoMerge::_run" << endl;
      cerr << "  Resyncing because sequence numbers are very different" << endl;
      cerr << "  masterSeqNum: " << _masterSeqNum << endl;
      cerr << "  slaveSeqNum: " << _slaveSeqNum << endl;
      return -1;
    }

    if (_masterSeqNum == _slaveSeqNum) {

      // sequence numbers match, so copy the master data to
      // the slave and write out slave
      
      if (_mergeAndWriteUsingNumbers()) {
        return -1;
      }

      // read master

      if (_readMaster()) {
        return -1;
      }

    } else if (_masterSeqNum < _slaveSeqNum) {

      // master is behind, so must catch up
      
      if (_readMaster()) {
        return -1;
      }

    } else {

      // slave is behind, so must catch up
      
      if (_readSlave()) {
        return -1;
      }
      
    }

  } // while

  return 0;

}

//////////////////////////////////////////////////
// synchronize using time

int TsScanInfoMerge::_syncUsingTime()
{
  
  // read master until we have some data
  
  while (_readMaster() != 0) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Waiting for master" << endl;
    }
    umsleep(100);
  }
  
  // read slave until we have some data
  
  while (_readSlave() != 0) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Waiting for slave" << endl;
    }
    umsleep(100);
  }

  _printExtraVerbose("Start");
    
  while (true) {
    
    PMU_auto_register("_run");

    // read next pulse from the slave

    if (_readSlave()) {
      umsleep(100);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Waiting for slave" << endl;
      }
      continue;
    }

    _printExtraVerbose("SS");
    
    // is slave time between the prev master and latest master?

    while (_slaveTime > _masterTime) {

      // slave is behind master, read a master pulse
      if (_readMaster()) {
        umsleep(100);
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Waiting for master" << endl;
        }
        continue;
      }
      
      _printExtraVerbose("MM");

      if (_slaveTime >= _prevMasterTime  &&
          _slaveTime <= _masterTime) {
        // slave is between prev master and latest master
        // so ready to interpolate
        break;
      }
      
    } // while (_slaveTime ...
    
    _printExtraVerbose("II");
    
    // slave time is between prevMasterTime and masterTime
    // so interpolate angles based on times
    
    if (_mergeAndWriteUsingTime()) {
      return -1;
    }
    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// merge information and write out to fmq

int TsScanInfoMerge::_mergeAndWriteUsingNumbers()
{
  
  PMU_auto_register("_mergeAndWriteUsingNumbers");

  // if the sequence numbers are 0, then skip this
  // since no read has been done yet
  
  if (_masterSeqNum == 0 || _slaveSeqNum == 0) {
    return 0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Merging and writing" << endl;
  }

  // get the pulse message from the slave reader, and make a copy
  
  int msgLen = _slaveReader->getLatestPulsePacketLen();
  const void *msgBuf = _slaveReader->getLatestPulsePacket();
  
  char *copy = new char[msgLen];
  memcpy(copy, msgBuf, msgLen);

  // overwrite the pulse header with that from the master queue

  memcpy(copy, _masterReader->getLatestPulseHdr(),
         sizeof(iwrf_pulse_header_t));
  
  // add to outgoing message
  
  _writeMsg.addPart(IWRF_PULSE_HEADER_ID, msgLen, copy);
  
  // if the message is large enough, write to the FMQ
  
  _writeToFmq();
  
  // free up

  delete[] copy;
  
  return 0;

}

//////////////////////////////////////////////////
// merge information and write out to fmq

int TsScanInfoMerge::_mergeAndWriteUsingTime()
{
  
  PMU_auto_register("_mergeAndWriteUsingTime");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Merging and writing" << endl;
  }

  // interp pulse header info

  iwrf_pulse_header_t slaveHdr = _slaveReader->getLatestPulseHdr();
  const iwrf_pulse_header_t &latestHdr = *_masterReader->getLatestPulseHdr();
  const iwrf_pulse_header_t &prevHdr = *_masterReader->getPrevPulseHdr();

  double interpEl, interpAz;
  _interpAngles(prevHdr.elevation, latestHdr.elevation,
                prevHdr.azimuth, latestHdr.azimuth,
                _slaveTime,
                _prevMasterTime, _masterTime,
                interpEl, interpAz);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "el0, el1, az0, az1, stime, pmtime, mtime, iel, iaz: "
         << prevHdr.elevation << ", "
         << latestHdr.elevation << ", "
         << prevHdr.azimuth << ", "
         << latestHdr.azimuth << ", "
         << _slaveTime << ", "
         << _prevMasterTime << ", "
         << _masterTime << ", "
         << interpEl << ", "
         << interpAz << endl;
  }
    
  if (_params.merge_scan_info) {
    slaveHdr.scan_mode = latestHdr.scan_mode;
    slaveHdr.follow_mode = latestHdr.follow_mode;
    slaveHdr.volume_num = latestHdr.volume_num;
    slaveHdr.sweep_num = latestHdr.sweep_num;
    slaveHdr.antenna_transition = latestHdr.antenna_transition;
    slaveHdr.fixed_el = latestHdr.fixed_el;
    slaveHdr.fixed_az = latestHdr.fixed_az;
  }
  if (_params.merge_angles) {
    slaveHdr.elevation = interpEl;
    slaveHdr.azimuth = interpAz;
  }

  // get the pulse message from the slave reader, and make a copy
  
  int msgLen = _slaveReader->getLatestPulsePacketLen();
  const void *msgBuf = _slaveReader->getLatestPulsePacket();
  
  char *copy = new char[msgLen];
  memcpy(copy, msgBuf, msgLen);

  // overwrite the pulse header with that from the master queue

  memcpy(copy, &slaveHdr, sizeof(iwrf_pulse_header_t));
  
  // add to outgoing message
  
  _writeMsg.addPart(IWRF_PULSE_HEADER_ID, msgLen, copy);
  
  // if the message is large enough, write to the FMQ
  
  _writeToFmq();
  
  // free up

  delete[] copy;
  
  return 0;

}

////////////////////////////////////////////////////////////
// interpolate the angle between the given times

void TsScanInfoMerge::_interpAngles(double prevEl, double latestEl,
                                    double prevAz, double latestAz,
                                    double pulseTime,
                                    double prevTime, double latestTime,
                                    double &interpEl, double &interpAz)
  
{

  // compute the interpolation fraction
  
  double timeFraction = (pulseTime - prevTime) / (latestTime - prevTime);
  
  // compute delta angles

  double deltaEl = latestEl - prevEl;
  double deltaAz = latestAz - prevAz;

  // constrain the differences from -180 to 180

  if (deltaEl < -180) {
    deltaEl += 360.0;
  } else if (deltaEl > 180.0) {
    deltaEl -= 360.0;
  }

  if (deltaAz < -180) {
    deltaAz += 360.0;
  } else if (deltaAz > 180.0) {
    deltaAz -= 360.0;
  }

  // compute the interpolated angles

  double el = prevEl + deltaEl * timeFraction;
  double az = prevAz + deltaAz * timeFraction;
  
  // constrain the angles from 0 to 360 in az 
  // and -180 to 180 in el

  if (el < -180) {
    el += 360.0;
  } else if (el > 180.0) {
    el -= 360.0;
  }

  if (az < 0) {
    az += 360.0;
  } else if (az > 360.0) {
    az -= 360.0;
  }

  interpEl = el;
  interpAz = az;

}
  
//////////////////////////////////////////////////
// read a pulse from the master queue

int TsScanInfoMerge::_readMaster()
{

  while (true) {

    PMU_auto_register("_readMaster");
    
    // read next pulse
    
    IwrfTsPulse *pulse = _masterReader->getNextPulse();
    if (pulse == NULL) {
      return -1;
    }

    // set sequence number
    
    _masterSeqNum = pulse->get_pulse_seq_num();

    // set times and keep track of pulse headers

    if (_masterTime < 0) {
      _latestPulseHdr = pulse->getHdr();
    }

    _prevMasterTime =  _masterTime;
    _prevPulseHdr = _latestPulseHdr;

    _latestPulseHdr = pulse->getHdr();
    _masterTime = _masterReader->getTimeSinceStart(_latestPulseHdr);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> got master, seq num: " << _masterSeqNum << endl;
    }
    delete pulse;
    
    return 0;

  }

  return -1;
  
}

//////////////////////////////////////////////////
// read a pulse from the slave queue

int TsScanInfoMerge::_readSlave()
{
  
  while (true) {

    PMU_auto_register("_readSlave");
    
    // read next pulse
    
    const DsMsgPart *part = _slaveReader->getNextPart();
    if (part == NULL) {
      return -1;
    }
    int msgType = part->getType();
    int msgLen = part->getLength();
    const void *msgBuf = part->getBuf();

    if (msgType == IWRF_PULSE_HEADER_ID) {
      
      // pulse type
      // determine the pulse sequence number
      
      IwrfTsInfo &info = _slaveReader->getOpsInfo();
      IwrfTsPulse *pulse = new IwrfTsPulse(info);
      if (pulse->setFromBuffer((void *) msgBuf, msgLen, true) == 0) {
        _slaveSeqNum = pulse->get_pulse_seq_num();
        _slaveTime = _masterReader->getTimeSinceStart(pulse->getHdr());
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "-->> got slave, seq num: " << _slaveSeqNum << endl;
	}
        delete pulse;
        return 0;
      }
      
    } // if (msgType == IWRF_PULSE_HEADER_ID)
    
    // for non-pulse packets, we pass the data through to the
    // fmq, using data from the master queue as applicable
    
    char *copy = new char[msgLen];
    memcpy(copy, msgBuf, msgLen);

    switch (msgType) {
      // case IWRF_RADAR_INFO_ID: {
      //   iwrf_radar_info_t radarInfo =
      //     _slaveReader->getOpsInfo().getRadarInfo();
      //   memcpy(copy, &radarInfo, sizeof(iwrf_radar_info_t));
      //   break;
      // }
      case IWRF_SCAN_SEGMENT_ID: {
        memcpy(copy, _masterReader->getLatestScanSeg(),
               sizeof(iwrf_scan_segment_t));
        break;
      }
      // case IWRF_TS_PROCESSING_ID: {
      //   if (_params.use_ts_processing_from_master) {
      //     memcpy(copy, _masterReader->getLatestTsProc(),
      //            sizeof(iwrf_ts_processing_t));
      //   }
      //   break;
      // }
      default: {}
    }

    // add to outgoing message
    
    _writeMsg.addPart(msgType, msgLen, copy);
    
    // if the message is large enough, write to the FMQ
    
    _writeToFmq();
    
    // free up

    delete[] copy;
    
  }

  return -1;
  
}

//////////////////////////////////////////////////
// initialize
// returns 0 on success, -1 on failure

int TsScanInfoMerge::_init()
{

  _masterSeqNum = 0;
  _slaveSeqNum = 0;
  
  _masterTime = -1;
  _prevMasterTime = -1;
  _slaveTime = -1;

  // close if needed

  if (_masterReader) {
    delete _masterReader;
    _masterReader = NULL;
  }

  if (_slaveReader) {
    delete _slaveReader;
    _slaveReader = NULL;
  }
  
  _writer.closeMsgQueue();

  // create the input FMQs
  
  _masterReader = new MasterReader(_params);
  _slaveReader = new SlaveReader(_params);
  
  return 0;

}
  
///////////////////////////////////////
// initialize the output queue

int TsScanInfoMerge::_openOutputFmq()
  
{

  if (_writer.isOpen()) {
    // already open
    return 0;
  }

  if (_masterReader == NULL) {
    cerr << "ERROR - TsScanInfoMerge::_openOutputFmq()" << endl;
    cerr << "  Master reader FMQ not yet open" << endl;
    return -1;
  }

  int numSlots = _masterReader->getFmqNumSlots();
  int bufSize = _masterReader->getFmqBufSize();

  if (numSlots == 0 || bufSize == 0) {
    cerr << "ERROR - TsScanInfoMerge::_openOutputFmq()" << endl;
    cerr << "  Master reader FMQ not yet open" << endl;
    return -1;
  }

  if (_writer.initReadWrite(_params.output_fmq_name,
                            _progName.c_str(),
                            _params.debug >= Params::DEBUG_EXTRA,
                            Fmq::END, // start position
                            false,    // compression
                            numSlots, bufSize)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: "
         << _params.output_fmq_name << endl;
    cerr << "  nSlots: " << numSlots << endl;
    cerr << "  nBytes: " << bufSize << endl;
    cerr << _writer.getErrStr() << endl;
    return -1;
  }

  _writer.setSingleWriter();
  if (_params.data_mapper_report_interval > 0) {
    _writer.setRegisterWithDmap
      (true, _params.data_mapper_report_interval);
  }
  _writeMsg.clearAll();
  _writeMsg.setType(0);

  return 0;

}

///////////////////////////////////////
// write to output FMQ if ready
// returns 0 on success, -1 on failure

int TsScanInfoMerge::_writeToFmq()
  
{

  // make sure it's open

  if (_openOutputFmq()) {
    return -1;
  }

  // if the message is large enough, write to the FMQ
  
  int nParts = _writeMsg.getNParts();
  if (nParts < _params.n_pulses_per_message) {
    return 0;
  }
  
  void *buf = _writeMsg.assemble();
  int len = _writeMsg.lengthAssembled();
  if (_writer.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - TsScanInfoMerge" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_name << endl;
    _writeMsg.clearParts();
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "  Wrote msg, nparts, len: "
         << nParts << ", " << len << endl;
  }
  _writeMsg.clearParts();

  return 0;

}
    
///////////////////////////////////////
// debug print for pulse info
// returns 0 on success, -1 on failure

void TsScanInfoMerge::_printExtraVerbose(const string &label)
  
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    
    cerr << label << " -->> slaveTime, seqNum: "
         << _slaveTime << ", "
         << _slaveSeqNum << endl;
    
    cerr << label << " -->> prevMasterTime, masterTime, seqNum: "
         << _prevMasterTime << ", "
         << _masterTime << ", " 
         << _masterSeqNum << endl;
    
  }

}

    
