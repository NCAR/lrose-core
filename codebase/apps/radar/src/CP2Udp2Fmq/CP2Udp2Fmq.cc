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
// CP2Udp2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// CP2Udp2Fmq reads data from CP2 Udp port in CP2 TimeSeries format
// and writes it to files in TsArchive format
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
#include <sys/time.h>
#include <ctime>
#include "CP2Udp2Fmq.hh"
#include <toolsa/udatetime.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <rapformats/DsRadarBeam.hh>

using namespace std;

const double CP2Udp2Fmq::PIRAQ3D_SCALE = 1.0/(unsigned int)pow(2.0,31);

// Constructor

CP2Udp2Fmq::CP2Udp2Fmq(int argc, char **argv) : 
	_tsPulse_s(_tsInfo_s),
	_tsPulse_x(_tsInfo_x),
	x_pulseCollator(256)
  
{

  isOK = true;
  _nPulses_s = _nPulses_x = 0;
  _prevAz = -999;
  _thisVolNum = 0;
  _baseSweepNum = 0;
  _udpFd = -1;
  _outPktSeqNum_s = 0;
  _outPktSeqNum_x = 0;

  _pulseTimeErrorSum = 0.0;
  _pulseTimeErrorCount = 0.0;
  _pulseTimeError = 0.0;
  _timeOfPrevErrorPrint = 0;

  for (int ii = 0; ii < NCHANNELS; ii++) {
    _prevSeqChan[ii] = 0;
  }

  // set programe name
  
  _progName = "CP2Udp2Fmq";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "CP2Udp2Fmq.ops";
  if (_params.loadFromArgs(argc, argv,  _args.override.list,
			   &_paramsPath)) {
    cerr << "CP2Udp2Fmq::CP2Udp2Fmq ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    return;
  }
  
  _params.checkAllSet(stdout);

  if (_verbose())
    _params.print(stdout, PRINT_NORM);

  // initialize the output FMQs

  if (_fmq_s.initReadWrite(_params.outFmqName_s,
			   _progName.c_str(),
			   _verbose(),    // set debug?
			   Fmq::END,    // start position
			   false,         // compression
			   _params.fmqNSlots,
			   _params.fmqNBytes)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize S-Band FMQ: " << _params.outFmqName_s << endl;
    cerr << "  nSlots: " << _params.fmqNSlots << endl;
    cerr << "  nBytes: " << _params.fmqNBytes << endl;
    cerr << _fmq_s.getErrStr() << endl;
    isOK = false;
    return;
  }
  if (_params.data_mapper_report_interval > 0) {
    _fmq_s.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  _fmq_s.setSingleWriter();

  if (_fmq_x.initReadWrite(_params.outFmqName_x,
			   _progName.c_str(),
			   _verbose(),    // set debug?
			   Fmq::END,    // start position
			   false,         // compression
			   _params.fmqNSlots,
			   _params.fmqNBytes)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize X-Band FMQ: " << _params.outFmqName_x << endl;
    cerr << "  nSlots: " << _params.fmqNSlots << endl;
    cerr << "  nBytes: " << _params.fmqNBytes << endl;
    cerr << _fmq_x.getErrStr() << endl;
    isOK = false;
    return;
  }
  if (_params.data_mapper_report_interval > 0) {
    _fmq_x.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  _fmq_x.setSingleWriter();

  if (_params.swapIQ_s) {
    cerr << "WARNING - S-Band IQ swapping enabled" << _progName << endl;
  }

  if (_params.swapIQ_x) {
    cerr << "WARNING - X-Band IQ swapping enabled" << _progName << endl;
  }

  // initialize messages
  
  _msg_s.clearAll();
  _msg_s.setType(0);
  _msg_x.clearAll();
  _msg_x.setType(0);
  
  _tsInfoValid_s = _tsInfoValid_x = false;
  
  // intialize mutex for read threading
  
  pthread_mutex_init(&_readMutex, NULL);
  
  // init process mapper registration

  if (_params.regWithProcmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // read in calibration data

  string errStr;
  if (_calib_s.readFromXmlFile(_params.cal_file_s, errStr)) {
    cerr << "ERROR - CP2Udp2Fmq constructor" << endl;
    cerr << "Reading S-band calibration file: " << _params.cal_file_s << endl;
    cerr << errStr << endl;
    isOK = false;
  }
  if (_calib_x.readFromXmlFile(_params.cal_file_x, errStr)) {
    cerr << "ERROR - CP2Udp2Fmq constructor" << endl;
    cerr << "Reading X-band calibration file: " << _params.cal_file_x << endl;
    cerr << errStr << endl;
    isOK = false;
  }
  
  return;
  
}

// destructor

CP2Udp2Fmq::~CP2Udp2Fmq()

{

  _closeUdp();

  pthread_mutex_destroy(&_readMutex);

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int CP2Udp2Fmq::Run ()
{
  
  PMU_auto_register("Run");

  if (_verbose()) {
    cerr << "Running CP2Udp2Fmq - verbose debug mode" << endl;
  } else if (_debug()) {
    cerr << "Running CP2Udp2Fmq - debug mode" << endl;
  }
  
  // open udp

  while (true) {
    
    PMU_auto_register("Opening UDP Socket");
    
    // open socket to server
    
    if (_openUdp() == 0) {
      break;
    }

    umsleep(1000);
    
  } // while(true)

  // start read thread
  
  pthread_t readThread = 0;
  if (pthread_create(&readThread, NULL, _readUdpInThread, this)) {
    cerr << "ERROR - CP2Udp2Fmq" << endl;
    cerr << "  Cannot create read thread" << endl;
    return -1;
  }

  // read messages put on the queue by the read thread

  _readFromQueue();

  // never gets here

  return 0;
  
}

///////////////////
// open port

int CP2Udp2Fmq::_openUdp()

{

  _closeUdp();

  // get socket

  if  ((_udpFd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::open" << endl;
    cerr << "  Could not create socket." << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // set the socket for reuse

  int val = 1;
  int valen = sizeof(val);
  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);

  // bind local address to the socket

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof (addr));
  addr.sin_port = htons(_params.udp_port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind (_udpFd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::open" << endl;
    cerr << "  Bind error, udp port: " << _params.udp_port << endl;
    cerr << "  " << strerror(errNum) << endl;
    _closeUdp();
    return -1;
  }

  if (_debug()) {
    cerr << "Opened UDP port: " << _params.udp_port << endl;
  }
  
  return 0;

}

/////////////
// close port

void CP2Udp2Fmq::_closeUdp()

{

  if (_udpFd >= 0) {
    close(_udpFd);
  }
  _udpFd = -1;
  

}

//////////////////////////////////////////////////////
// read messages put on the queue by the read thread

void CP2Udp2Fmq::_readFromQueue()

{

  // set number of pulses to 0

  _nPulses_s = _nPulses_x = 0;
  
  // read data
  
  while (true) {

    PMU_auto_register("Waiting for data");
    
    // check if we have data on the queue

    if (_readQueue.empty()) {
      // cerr << "queue is empty .........." << endl;
      umsleep(10);
      continue;
    }

    // pop off the back of the queue

    pthread_mutex_lock(&_readMutex);
    MemBuf *mbuf = _readQueue.back();
    _readQueue.pop_back();
    pthread_mutex_unlock(&_readMutex);

    // handle this packet

    _handlePacket(mbuf);
    
    // delete the message buffer
    // which was created by the read thread

    delete mbuf;
    
    // if the S-Band message is large enough, write to the FMQ

    int nParts = _msg_s.getNParts();
    if (nParts >= _params.nPartsPerMsg) {
      void *buf = _msg_s.assemble();
      int len = _msg_s.lengthAssembled();
      if (_fmq_s.writeMsg(0, 0, buf, len)) {
 	cerr << "ERROR - CP2Udp2Fmq::_readFromServer" << endl;
 	cerr << "  Cannot write S-Band FMQ: " << _params.outFmqName_s << endl;
      }
      umsleep(10);
      if (_verbose()) {
	cerr << endl << "  Writing S-Band msg, nparts, len: "
	     << nParts << ", " << len << endl;
      }
      _msg_s.clearParts();
    }
    
    // if the X-Band message is large enough, write to the FMQ
    
    nParts = _msg_x.getNParts();
    if (nParts >= _params.nPartsPerMsg) {
      void *buf = _msg_x.assemble();
      int len = _msg_x.lengthAssembled();
      if (_fmq_x.writeMsg(0, 0, buf, len)) {
 	cerr << "ERROR - CP2Udp2Fmq::_readFromServer" << endl;
 	cerr << "  Cannot write X-Band FMQ: " << _params.outFmqName_x << endl;
      }
      umsleep(10);
      if (_verbose()) {
	cerr <<  endl << "  Writing X-Band msg, nparts, len: "
	     << nParts << ", " << len << endl;
      }
      _msg_x.clearParts();
    }

  } // while
    
}

///////////////////////////////////////////
// handle packet read from the queue
//
// Returns 0 on success, -1 on failure

int CP2Udp2Fmq::_handlePacket(MemBuf *mbuf)

{

  int len = mbuf->getLen();

  if (len < (int) sizeof(_pulseHeader)) {
    cerr << "CP2Udp2Fmq::handlePacket" << endl;
    cout << "  ERROR reading packet, too short, len: " << len << endl;
    return -1;
  }
      
  // put this datagram into a packet
  
  bool packetBad = _pulsePacket.setPulseData(len, mbuf->getPtr());
  if (packetBad) {
    cerr << "CP2Udp2Fmq::handlePacket" << endl;
    cout << "  bad packet" << endl;
    return -1;
  }

  // Extract the pulses and process them.
  // Observe paranoia for validating packets and pulses.
  // From here on out, we are divorced from the
  // data transport.

  for (int ii = 0; ii < _pulsePacket.numPulses(); ii++) {
    
    CP2Pulse* pPulse = _pulsePacket.getPulse(ii);
    
    // check for missing sequence number
    
    int channel = pPulse->header.channel;
    int seqNum = pPulse->header.pulse_num;
    
    if (channel < NCHANNELS) {
      if (_prevSeqChan[channel] != 0 &&
	  _prevSeqChan[channel] != seqNum - 1) {
	cerr << "Got pulse, seq num, chan: " << seqNum << ", " << channel << endl;
	cerr << "=======================>> ERROR, missing pulses, chan, nmissing: "
	     << channel << ", " << seqNum - _prevSeqChan[channel] - 1 << endl;
      }
      _prevSeqChan[channel] = seqNum;
    }
    
    if (pPulse) {

      if  (_params.applyPulseScaling) {
	// scale the I/Q counts to something. we will probably do 
	// this eventually in cp2exec.
	float* pData = pPulse->data;
	int gates = pPulse->header.gates;
	for (int g = 0; g < 2*gates; g++) {
	  pData[g] *= PIRAQ3D_SCALE;
	}
      }

      int chan = pPulse->header.channel;
      if (chan >= 0 && chan < 3) {
	// check error flags, count pulses, etc.
	_pulseBookKeeping(pPulse);
	// reformat pulse as ds_radar_ts and send to fmq
	_reformatPulse(pPulse);
      }

    } else {

      cout << "CP2Udp2Fmq::handlePacket - getPulse(" << ii 
	   << ") returned NULL" << endl;
      
    }
    
  } // ii

  return 0;

}

/////////////////////////////////////////////////////////////////////

void CP2Udp2Fmq::_pulseBookKeeping(CP2Pulse* pPulse) 

{
  // look for pulse number errors
  int chan = pPulse->header.channel;

  if (_debug()) {
    if ((round(pPulse->header.az) != round(_lastPulseAz[chan])) &&
	(chan == 0))
      cout << "channel=" << chan << " az/el=" << pPulse->header.az
	   << "/" << pPulse->header.el 
	   << " nPulses s/x=" << _nPulses_s << "/"
	   << _nPulses_x 
	   << " x_coll_high=" << x_pulseCollator.queue_highWater() << endl;
  }

  // check for consecutive pulse numbers
  if (_lastPulseNum[chan]) {
    if (_lastPulseNum[chan]+1 != pPulse->header.pulse_num) {
      _errorCount[chan]++;
    }
  }
  _lastPulseNum[chan] = pPulse->header.pulse_num;
  _lastPulseAz[chan] = pPulse->header.az;

  // count the pulses
  _pulseCount[chan]++;

  char timestr[256];

  // Choose to use S-Band antenna control messages only
  if (chan == 0)
    {
      // check for new volume
      if (pPulse->header.volNum != _thisVolNum)
	{
	  _thisVolNum = pPulse->header.volNum;
	  _baseSweepNum= pPulse->header.sweepNum;
	  if (_debug()) {
	    time_t timenow = time(0);
	    std::cout << "New vol detected =" << pPulse->header.volNum 
		      <<  " Base sweep number =" << _baseSweepNum
		      << " at " << ctime_r(&timenow, timestr) 
		      << endl;
	  }
	}

      // check for new sweep
      if (_thisSweepNum != pPulse->header.sweepNum - _baseSweepNum)
	{
	  if (_debug()) {
	    time_t timenow = time(0);
	    std::cout << "New sweep detected from " << _thisSweepNum
		      << " to " << pPulse->header.sweepNum - _baseSweepNum
		      << " at " << ctime_r(&timenow, timestr) 
		      << endl;
	  }
	  _thisSweepNum= pPulse->header.sweepNum - _baseSweepNum;
	}
      
      {
	if (_antTransFlag != pPulse->header.antTrans)
	  {
	    if (_debug()) {
	      if (!_antTransFlag)
		cout << "Starting antenna transition at az/el=";
	      else
		cout << "Ending antenna transition at az/el=";
	      cout << pPulse->header.az << "/"
		   << pPulse->header.el << endl;
	    }
	    _antTransFlag = pPulse->header.antTrans;
	  }
      }
    }

  // look for eofs.
  //?? pjp - what are eofs used for???
  // the code here only ever sets the flags, they aren't used or reset anywhere
  if (pPulse->header.status & PIRAQ_FIFO_EOF) {
    switch (chan) 
      {
      case 0:
	if (!_eof[0]) {
	  _eof[0] = true;
	}
	break;
      case 1:
	if (!_eof[1]) {
	  _eof[1] = true;
	}
	break;
      case 2:
	if (!_eof[2]) {
	  _eof[2] = true;
	}
	break;
      }
  }
}

/////////////////////////////
// reformat pulse packet

int CP2Udp2Fmq::_reformatPulse(CP2Pulse* pPulse)
{
  if (pPulse->header.channel == 0)
    return _reformatPulse_s(pPulse);
  else
    return _reformatPulse_x(pPulse);
}

///////////////////////////////////
// reformat pulse packet - S-band

int CP2Udp2Fmq::_reformatPulse_s(CP2Pulse* pPulse)

{

  if (!pPulse) {
    return -1;
  }
  
  struct CP2PulseHeader *cp2Hdr = &(pPulse->header);
  int channel = cp2Hdr->channel;
  if (!channel == 0) {
    cerr << "ERROR - CP2Udp2Fmq::_reformatPulse_s" << endl;
    cerr << "  Bad channel = " << channel << endl;
    return -1;
  }
  
  _nPulses_s++;

  // Check whether ts_info should be sent due to scan change or
  // periodic send
  bool opsInfoChanged = _opsInfoChanged(*cp2Hdr, _prevPulseHeader_s);
  if (((_nPulses_s % _params.nPulsesPerInfo) == 0) || opsInfoChanged) {
    if (_debug() && opsInfoChanged) {
      cerr << "  --->> Detected S-Band pulse scan mode change" << endl;
    }
    
    if (_debug()) {
      cerr << "  --->> Adding S-Band TS info message" << endl;
    }
    _reformatInfo(*cp2Hdr, 
		  _params.ts_pulse_info_s,
		  _calib_s,
		  _params.ts_ops_info_sband,
		  _tsInfo_s);
    _tsInfoValid_s = true;
    _addInfo2Msg(_tsInfo_s, _msg_s, _outPktSeqNum_s);
  }
  _prevPulseHeader_s = *cp2Hdr;
  
  // set pulse header
  
  _setPulseHeader(*cp2Hdr,
		  _params.ts_pulse_info_s,
		  _params.ts_ops_info_sband,
		  _tsPulse_s);

  // set Iq data
  
  _tsPulse_s.setIqFloats(cp2Hdr->gates, 1, pPulse->data);

  // condition the data

  if (_params.swapIQ_s) {
    _tsPulse_s.swapIQ();
  }
  if (_params.invertQ_s) {
    _tsPulse_s.invertQ();
  }

  // assemble pulse in membuf

  _convertBuf_s.setAllowShrink(false);
  _convertBuf_s.reset();
  _tsPulse_s.assemble(_convertBuf_s);
  
  // add to the message
  
  _addPulse2Msg(_tsPulse_s, _msg_s, _outPktSeqNum_s);

  // check for missing pulses

  _checkForMissing(*cp2Hdr);
  
  return 0;

}

//////////////////////////////////
// reformat pulse packet - X-band

int CP2Udp2Fmq::_reformatPulse_x(CP2Pulse* pPulse)

{

  if (!pPulse) {
    return -1;
  }

  struct CP2PulseHeader *cp2Hdr = &(pPulse->header);
  int channel = cp2Hdr->channel;

  if ((channel < 1) || (channel > 2)) {
    cerr << "ERROR - CP2Udp2Fmq::_reformatPulse_x" << endl;
    cerr << "  Bad channel = " << channel << endl;
    return -1;
  }
  
  // create a CP2FullPuse, which is a class that will
  // hold the IQ data.
  CP2FullPulse* pFullPulse = new CP2FullPulse(pPulse);

  // send the pulse to the collator. The collator finds matching 
  // pulses. If orphan pulses are detected, they are deleted
  // by the collator. Otherwise, matching pulses returned from
  // the collator can be deleted here.
  x_pulseCollator.addPulse(pFullPulse, channel-1);

  // now see if we have some matching pulses

  CP2FullPulse* pHPulse;
  CP2FullPulse* pVPulse;

  while (x_pulseCollator.gotMatch(&pHPulse, &pVPulse)) {

    struct CP2PulseHeader *cp2Hdr_h = pHPulse->header();
    struct CP2PulseHeader *cp2Hdr_v = pVPulse->header();
    
    if ((cp2Hdr_h->pulse_num != cp2Hdr_v->pulse_num) ||
	(cp2Hdr_h->gates != cp2Hdr_v->gates)) {
      
      cerr << "X-Band collator returned mismatched pulses: "
	   << "PulseNums h/v=" << cp2Hdr_h->pulse_num << "/"
	   << cp2Hdr_v->pulse_num
	   << "Gates     h/v=" <<cp2Hdr_h->gates << "/"
	   << cp2Hdr_v->gates << endl;

      continue;

    }

    _nPulses_x++;
    
    // Check whether ts_info should be sent due to scan change or
    // periodic send
    
    bool opsInfoChanged = _opsInfoChanged(*cp2Hdr, _prevPulseHeader_x);
    if (((_nPulses_x % _params.nPulsesPerInfo) == 0) || opsInfoChanged)  {
      if (_debug() && opsInfoChanged) {
	cerr << "  --->> Detected X-Band pulse scan mode change" << endl;
      }
      if (_debug()) {
	cerr << "  --->> Adding X-Band TS info message" << endl;
      }
      _reformatInfo(*cp2Hdr, 
		    _params.ts_pulse_info_x,
		    _calib_x,
		    _params.ts_ops_info_xband,
		    _tsInfo_x);
      _tsInfoValid_x = true;
      _addInfo2Msg(_tsInfo_x, _msg_x, _outPktSeqNum_x);
    }
    _prevPulseHeader_x = *cp2Hdr;
    
    // set the pulse header
    
    _setPulseHeader(*cp2Hdr_h,
		    _params.ts_pulse_info_x,
		    _params.ts_ops_info_xband,
		    _tsPulse_x);
    
    // concatenate the H and V channel data into a single buffer
    
    MemBuf iqBuf;
    iqBuf.add(pHPulse->data(), cp2Hdr_h->gates * 2 * sizeof(fl32));
    iqBuf.add(pVPulse->data(), cp2Hdr_v->gates * 2 * sizeof(fl32));
    
    // set Iq data
    
    _tsPulse_x.setIqFloats(cp2Hdr_h->gates, 2, (const fl32*) iqBuf.getPtr());
    
    // condition the data
    
    if (_params.swapIQ_x) {
      _tsPulse_x.swapIQ();
    }
    if (_params.invertQ_x) {
      _tsPulse_x.invertQ();
    }
    
    // assemble pulse in membuf
    
    _convertBuf_x.setAllowShrink(false);
    _convertBuf_x.reset();
    _tsPulse_x.assemble(_convertBuf_x);
    
    // add to the message

    _addPulse2Msg(_tsPulse_x, _msg_x, _outPktSeqNum_x);
    
    // de-allocate pulse instances
    
    delete pHPulse;
    delete pVPulse;

  } // while
    
  // check for missing pulses
  
  _checkForMissing(*cp2Hdr);
  
  return 0;

}

/////////////////////////////
// reformat info packet

int CP2Udp2Fmq::_reformatInfo(const CP2PulseHeader &cp2Hdr,
			      const Params::ts_pulse_info_params &pulseinfo,
			      const DsRadarCalib &calib,
			      const Params::ts_ops_info_params &opsinfo,
			      IwrfTsInfo &tsInfo)

{

  tsInfo.clear();
  
  double pulseTime = cp2Hdr.pulse_num * cp2Hdr.prt;
  time_t pulseTimeSecs = (time_t) pulseTime;
  int pulseTimeNanoSecs = (int) ((pulseTime - pulseTimeSecs) * 1.0e9);
  
  // signal processing
  
  tsInfo.set_proc_start_range_m(opsinfo.startRangeM);/* range to center of first gate - meters */
  tsInfo.set_proc_gate_spacing_m(opsinfo.gateSpacingM); /* spacing between gates - meters */

  tsInfo.set_proc_xmit_rcv_mode(opsinfo.xmitRcvMode);  /* ts_xmit_rcv_mode_t */
  tsInfo.set_proc_prf_mode(opsinfo.prfMode);           /* ts_prf_mode_t */
  tsInfo.set_proc_xmit_phase_mode(opsinfo.xmitPhaseMode); /* ts_xmit_phase_mode_t */
  tsInfo.set_proc_pulse_type(IWRF_PULSE_TYPE_RECT);
  tsInfo.set_proc_cal_type(IWRF_CAL_TYPE_NOISE_SOURCE_HV);
  tsInfo.set_proc_max_gate(cp2Hdr.gates);
  tsInfo.setTsProcessingTime(pulseTimeSecs, pulseTimeNanoSecs);

  // scan
      
  tsInfo.set_scan_mode(cp2Hdr.scanType);            /* ts_scan_mode_t */
  tsInfo.set_scan_volume_num(cp2Hdr.volNum);            /* ts_scan_mode_t */
  tsInfo.set_scan_sweep_num(cp2Hdr.sweepNum);            /* ts_scan_mode_t */
  tsInfo.set_scan_init_direction_cw(1);
  tsInfo.set_scan_init_direction_up(1);
  
  tsInfo.setScanSegmentTime(pulseTimeSecs, pulseTimeNanoSecs);

  tsInfo.setRadarId(opsinfo.radarId); /* to match ids between ops info and pulse headers */
  
  /* location */
  
  tsInfo.set_radar_altitude_m(opsinfo.altitudeM);    /* altitude in meters */
  tsInfo.set_radar_latitude_deg(opsinfo.latitudeDeg);  /* degrees */
  tsInfo.set_radar_longitude_deg(opsinfo.longitudeDeg); /* degrees */
    
  /* radar parameters */
    
  tsInfo.set_radar_wavelength_cm(calib.getWavelengthCm()); /* radar wavelength in cm */
  tsInfo.set_radar_beamwidth_deg_h(calib.getBeamWidthDegH()); /* beam width H in degrees */
  tsInfo.set_radar_beamwidth_deg_v(calib.getBeamWidthDegV()); /* beam width V in degrees */
  tsInfo.set_radar_nominal_gain_ant_db_h(calib.getAntGainDbH());
  tsInfo.set_radar_nominal_gain_ant_db_v(calib.getAntGainDbV());
  tsInfo.set_radar_platform_type(IWRF_RADAR_PLATFORM_FIXED);

  /* radar and site name */
    
  tsInfo.set_radar_name(opsinfo.radarName); /**< UTF-8 encoded radar name */
  tsInfo.set_radar_site_name(opsinfo.siteName); /**< UTF-8 encoded radar name */
  tsInfo.setRadarInfoTime(pulseTimeSecs, pulseTimeNanoSecs);
    
  /* time of calibration */
    
  tsInfo.setCalibrationTime(calib.getCalibTime(), 0);
  
  /* calibration parameters */

  tsInfo.setFromDsRadarCalib(calib);
  
  // set structs active

  tsInfo.setRadarInfoActive(true);
  tsInfo.setScanSegmentActive(true);
  tsInfo.setTsProcessingActive(true);
  tsInfo.setCalibrationActive(true);
 
  return 0;

}

//////////////////////////////////////////////////
// check for scan change in operations information
// checks scanType, sweepNum, gates and prt

bool CP2Udp2Fmq::_opsInfoChanged(const CP2PulseHeader &cp2Hdr,
				 const CP2PulseHeader &prevCp2Hdr) {
  bool changed = false;
  if (cp2Hdr.scanType != prevCp2Hdr.scanType) {
    if (_debug()) {
      cerr << "CP2Udp2Fmq::opsInfoChanged detected scanType change from " 
	   << prevCp2Hdr.scanType << " to "
	   << cp2Hdr.scanType << endl;
    }
    changed = true;
  }
  
  if (cp2Hdr.sweepNum != prevCp2Hdr.sweepNum) {
    if (_debug()) {
      cerr << "CP2Udp2Fmq::opsInfoChanged detected sweepNum change from "  
	   << prevCp2Hdr.sweepNum << " to "
	   << cp2Hdr.sweepNum << endl;
    }
    changed = true;
  }
  
  if (cp2Hdr.gates != prevCp2Hdr.gates) {
    if (_debug()) {
      cerr << "CP2Udp2Fmq::opsInfoChanged detected gates change from " 
	   << prevCp2Hdr.gates << " to "
	   << cp2Hdr.gates << endl;
    }
    changed = true;
  }
  
  if (abs(cp2Hdr.prt - prevCp2Hdr.prt) > (prevCp2Hdr.prt * 0.02)) {
    if (_debug()) {
      cerr << "CP2Udp2Fmq::opsInfoChanged detected prt change from " 
	   << prevCp2Hdr.prt << " to "
	   << cp2Hdr.prt << endl;
    }
    changed = true;
  }
  return changed;
}

/////////////////////////////
// check for missing pulses

void CP2Udp2Fmq::_checkForMissing(const struct CP2PulseHeader &cp2Hdr)

{
  
  double az = cp2Hdr.az;
  double el = cp2Hdr.el;
  
  if (_verbose()) {
    if ((_nPulses_s % 1000) == 0) {
      cerr << "S-Band El, az, npulses received: "
           << el << " " << az << " " << _nPulses_s << endl; 
    }
    if ((_nPulses_x % 1000) == 0) {
      cerr << "X-Band El, az, npulses received: "
           << el << " " << az << " " << _nPulses_x << endl; 
    }
    if (_prevAz > -900) {
      double deltaAz = fabs(az - _prevAz);
      if (deltaAz >= 180) {
        deltaAz -= 360.0;
      }
      if (fabs(deltaAz) > 1.0) {
        cerr << "Missing azimuth, prev: " << _prevAz
             << ", this: " << az << endl;
      }
    }
  } // debug

  _prevAz = az;
  
}

////////////////////////
/// write the pulse info

void CP2Udp2Fmq::_setPulseHeader(const CP2PulseHeader &cp2Hdr,
				 const Params::ts_pulse_info_params &pulseinfo,
				 const Params::ts_ops_info_params &opsinfo,
				 IwrfTsPulse &pulse)

{

  pulse.clear();

  pulse.setRadarId(opsinfo.radarId); /* to match ids between ops info and pulse headers */
  
  // get time from pulse header
  
  double pulseTime = cp2Hdr.pulse_num * cp2Hdr.prt;
  
  if (_params.apply_computed_time_correction) {

    // compute time error relative to computer time, using S-band pulses
    
    if (&pulse == &_tsPulse_s) {
      struct timeval tval;
      gettimeofday(&tval, NULL);
      double nowTime = tval.tv_sec + tval.tv_usec / 1000000.0;
      double pulseTimeError = pulseTime - nowTime;
      _pulseTimeErrorSum += pulseTimeError;
      _pulseTimeErrorCount++;
      if (_pulseTimeErrorCount > 10000) {
	_pulseTimeError = _pulseTimeErrorSum / _pulseTimeErrorCount;
	if (_params.debug) {
	  cerr << "======>> correcting time, _pulseTimeError: " << _pulseTimeError << endl;
	}
	_pulseTimeErrorSum = 0.0;
	_pulseTimeErrorCount = 0.0;
      }
    }

    pulseTime -= (_pulseTimeError + _params.pulse_time_latency);

  } else {

    // check pulse utime
    // adjust utime in case it is off by 2**32
    
    double two_to_32_by_1000 = pow(2.0, 32) / 1000.0;
    time_t now = time(NULL);
    double timeError = (double) now - pulseTime;
    int nWraps = (int) floor(timeError / two_to_32_by_1000 + 0.5);
    
    if (nWraps != 0) {
      double correctedTime = pulseTime + two_to_32_by_1000 * nWraps;
      if (now - _timeOfPrevErrorPrint >= 5) {
	cerr << "====>> WARNING - pulse time incorrect, correcting" << endl;
	cerr << "  pulse_num: " << cp2Hdr.pulse_num << endl;
	cerr << "  nWraps: " << nWraps << endl;
	cerr << "  pulse data utime: " << DateTime::strm((time_t) pulseTime) << endl;
	cerr << "  corrected utime: " << DateTime::strm((time_t) correctedTime) << endl;
	cerr << "  now: " << DateTime::strm(now) << endl;
	_timeOfPrevErrorPrint = now;
      }
      pulseTime = correctedTime;
    }

  } // if (_params.apply_computed_time_correction) {
  
  time_t utime = (time_t) pulseTime;
  double fracSecs = pulseTime - utime;
  int nanosecs = (int) (fracSecs * 1.0e9 + 0.5);

  pulse.set_scan_mode(cp2Hdr.scanType);
  pulse.set_follow_mode(IWRF_FOLLOW_MODE_NONE);
  
  pulse.setTime(utime, nanosecs);
  pulse.set_pulse_seq_num(cp2Hdr.pulse_num);

  pulse.set_elevation(cp2Hdr.el);
  pulse.set_azimuth(cp2Hdr.az);

  double targetAngle = (cp2Hdr.antSize / (65536.0 / 360.0));
  if (cp2Hdr.scanType == DS_RADAR_RHI_MODE) {
    pulse.set_fixed_az(targetAngle);
  } else {
    pulse.set_fixed_el(targetAngle);
  }
  

  pulse.set_prt(cp2Hdr.prt);
  pulse.set_prt_next(cp2Hdr.prt);
  pulse.set_pulse_width_us(cp2Hdr.xmit_pw * 1.0e6);

  pulse.set_n_gates(cp2Hdr.gates);
  pulse.set_sweep_num(_thisSweepNum);
  pulse.set_volume_num(cp2Hdr.volNum); 

  pulse.set_n_channels(pulseinfo.nChannels);   
  pulse.set_hv_flag(cp2Hdr.horiz);      

  pulse.set_antenna_transition(cp2Hdr.antTrans);
  pulse.set_phase_cohered(pulseinfo.phaseCohered);    
  
  pulse.set_status(cp2Hdr.status);

  // no burst pulse info, start at index 0
  pulse.set_burst_arg_diff(0, 0);
  pulse.set_burst_mag(0, 1.0);
  pulse.set_burst_arg(0, 90.0);

  if (pulseinfo.nChannels > 1) {
    pulse.set_burst_arg_diff(1, 0);
    pulse.set_burst_mag(1, 1.0);
    pulse.set_burst_arg(1, 90.0);
  }



}

///////////////////////////////////////////////////////////
// Thread function to read UDP data

void *CP2Udp2Fmq::_readUdpInThread(void *thread_data)
  
{
  
  // args points to the parent class
  
  CP2Udp2Fmq *parent = (CP2Udp2Fmq *) thread_data;
  bool debug = parent->_debug();
  bool verbose = parent->_verbose();

  // initialize sequence numbers for checking for missing pulses

  si64 prevSeqChan[NCHANNELS];
  for (int ii = 0; ii < NCHANNELS; ii++) {
    prevSeqChan[ii] = 0;
  }

  // set up receive buffer

  int bufSize = parent->_params.udpRxBuffsz;
  ui08 *rxBuf = new ui08[bufSize];
  
  while (true) {

    int iret = readSelect(parent->_udpFd, 1000);
    
    if (iret == -2) {
      cerr << "ERROR - CP2Udp2Fmq::_readUdpInThread()" << endl;
      cerr << "  Cannot perform select on UDP socket" << endl;
      umsleep(10);
      continue;
    }

    if (iret == -1) {
      // timed out
      continue;
    }

    struct sockaddr_in from_name;
    socklen_t fromlen = sizeof(from_name);
    int len = recvfrom(parent->_udpFd, rxBuf, bufSize, 0,
		       (struct sockaddr *) &from_name, &fromlen);

    if (verbose) {
      cerr << "CP2Udp2Fmq::_readUdpInThread - Packet read bytes = " 
	   << len << endl;
    }
    
    if (len < 0) {
      int errNum = errno;
      cerr << "ERROR - CP2Udp2Fmq::_readUdpInThread()" << endl;
      cerr << "  " << strerror(errNum) << endl;
      continue;
    }

    // check we have room on the queue.
    // only allow the queue to reach a size of 10000

    if ((int) parent->_readQueue.size() > maxQueueSize) {
      continue;
    }

    // create MemBuf containing UDP message

    MemBuf *mbuf = new MemBuf();
    mbuf->add(rxBuf, len);
    
    // check this packet
    
    if (verbose) {
      _checkPacket(mbuf, prevSeqChan, parent->_readQueue);
    }
    
    // add to Membuf to the deque, locking mutex while busy
    // the buffer will be freed by the parent

    pthread_mutex_lock(&parent->_readMutex);
    parent->_readQueue.push_front(mbuf);
    if (verbose ||
	(debug && parent->_readQueue.size() % 50 == 0)) {
      cerr << "======> read queue size: " << parent->_readQueue.size() << endl;
    }
    pthread_mutex_unlock(&parent->_readMutex);

  } // while

  return NULL;

}

/****************************************************
 * returns 0 on success, -1 on timeout, -2 on select failure
 *
 * Blocks if wait_msecs == -1
 */

int CP2Udp2Fmq::readSelect(int sd, long wait_msecs)

{

  int ret = 0;

  struct timeval wait;
  struct timeval *waitp;
  waitp = &wait;

  /*
   * listen only on sd socket
   */

  fd_set read_fd;
  FD_ZERO(&read_fd);
  FD_SET(sd, &read_fd);
  int maxfdp1 = sd + 1;

 again:
  
  /*
   * set timeval structure
   */

  if (-1 == wait_msecs) {
    waitp = NULL;
  } else {
    wait.tv_sec = wait_msecs / 1000;
    wait_msecs -= wait.tv_sec * 1000;
    wait.tv_usec = wait_msecs * 1000;
  }

  if (0 > (ret = select(maxfdp1, &read_fd, NULL, NULL, waitp))) {

      if (errno == EINTR) /* system call was interrupted */
        goto again;

      int errNum = errno;
      cerr << "ERROR - CP2Udp2Fmq::readSelect failed on id: " << sd << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -2; /* select failed */

    }

  if (ret == 0) {
    // timeout
#ifdef DEBUG
    cerr << "SKU_read_select: Select timed out after msecs: " << wait_msecs << endl;
#endif
    return -1;
  }

  return 0;

}


///////////////////////////////////////////
// handle packet read from the queue
//
// Returns 0 on success, -1 on failure

int CP2Udp2Fmq::_checkPacket(MemBuf *mbuf,
			     si64 *prevSeqChan,
			     deque<MemBuf *> &readQueue)


{

  int len = mbuf->getLen();
  
  if (len < (int) sizeof(CP2PulseHeader)) {
    cerr << "CP2Udp2Fmq::checkPacket" << endl;
    cout << "  ERROR reading packet, too short, len: " << len << endl;
    return -1;
  }
  
  // put this datagram into a packet
  
  CP2Packet pulsePacket;
  bool packetBad = pulsePacket.setPulseData(len, mbuf->getPtr());
  if (packetBad) {
    cerr << "CP2Udp2Fmq::handlePacket" << endl;
    cout << "  bad packet" << endl;
    return -1;
  }

  // Check for missing pulses

  for (int ii = 0; ii < pulsePacket.numPulses(); ii++) {
    
    CP2Pulse* pPulse = pulsePacket.getPulse(ii);
    
    // check for missing sequence number
    
    int channel = pPulse->header.channel;
    int seqNum = pPulse->header.pulse_num;
    
    // cerr << "Got pulse, seq num, chan: " << seqNum << ", " << channel << endl;
    if (channel < NCHANNELS) {
      if (prevSeqChan[channel] != 0 &&
	  prevSeqChan[channel] != seqNum - 1) {
	cerr << "Got pulse, seq num, chan: " << seqNum << ", " << channel << endl;
	cerr << "*******************>> ERROR, missing pulses, chan, nmissing: "
	     << channel << ", " << seqNum - prevSeqChan[channel] - 1 << endl;
	cerr << "Queue size: " << readQueue.size() << endl;
      }
      prevSeqChan[channel] = seqNum;
    }
    
  } // ii

  return 0;

}

///////////////////////////////////////////////////////////////
// add info to DsMessage

void CP2Udp2Fmq::_addInfo2Msg(IwrfTsInfo &info,
			      DsMessage &msg,
			      si64 &outPktSeqNum)
  
{

  iwrf_sync_t sync;
  iwrf_sync_init(sync);
  sync.packet.seq_num = outPktSeqNum++;
  msg.addPart(IWRF_SYNC_ID, sizeof(sync), &sync);

  if (info.isRadarInfoActive()) {
    info.setRadarInfoPktSeqNum(outPktSeqNum++);
    const iwrf_radar_info_t &radar_info = info.getRadarInfo();
    msg.addPart(IWRF_RADAR_INFO_ID, sizeof(radar_info), &radar_info);
  }

  if (info.isScanSegmentActive()) {
    info.setScanSegmentPktSeqNum(outPktSeqNum++);
    const iwrf_scan_segment_t &scan_seg = info.getScanSegment();
    msg.addPart(IWRF_SCAN_SEGMENT_ID, sizeof(scan_seg), &scan_seg);
  }

  if (info.isTsProcessingActive()) {
    info.setTsProcessingPktSeqNum(outPktSeqNum++);
    const iwrf_ts_processing_t &proc = info.getTsProcessing();
    msg.addPart(IWRF_TS_PROCESSING_ID, sizeof(proc), &proc);
  }

  if (info.isCalibrationActive()) {
    info.setCalibrationPktSeqNum(outPktSeqNum++);
    const iwrf_calibration_t &calib = info.getCalibration();
    msg.addPart(IWRF_CALIBRATION_ID, sizeof(calib), &calib);
  }

#ifdef NOTYET

  if (info.isEventNoticeActive()) {
    info.setEventNoticePktSeqNum(outPktSeqNum++);
    const iwrf_event_notice_t &enotice = info.getEventNotice();
    msg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(enotice), &enotice);
  }

  if (info.isAntennaCorrectionActive()) {
    info.setAntennaCorrectionPktSeqNum(outPktSeqNum++);
    const iwrf_antenna_correction_t &ant_corr = info.getAntennaCorrection();
    msg.addPart(IWRF_ANTENNA_CORRECTION_ID, sizeof(ant_corr), &ant_corr);
  }

  if (info.isXmitPowerActive()) {
    info.setXmitPowerPktSeqNum(outPktSeqNum++);
    const iwrf_xmit_power_t &xmit_power = info.getXmitPower();
    msg.addPart(IWRF_XMIT_POWER_ID, sizeof(xmit_power), &xmit_power);
  }

  if (info.isXmitSampleActive()) {
    info.setXmitSamplePktSeqNum(outPktSeqNum++);
    const iwrf_xmit_sample_t &xmit_sample = info.getXmitSample();
    msg.addPart(IWRF_XMIT_SAMPLE_ID, sizeof(xmit_sample), &xmit_sample);
  }

  if (info.isPhasecodeActive()) {
    info.setPhasecodePktSeqNum(outPktSeqNum++);
    const iwrf_phasecode_t &phasecode = info.getPhasecode();
    msg.addPart(IWRF_PHASECODE_ID, sizeof(phasecode), &phasecode);
  }

  if (info.isXmitInfoActive()) {
    info.setXmitInfoPktSeqNum(outPktSeqNum++);
    const iwrf_xmit_info_t &xmit_info = info.getXmitInfo();
    msg.addPart(IWRF_XMIT_INFO_ID, sizeof(xmit_info), &xmit_info);
  }

  if (info.isRvp8InfoActive()) {
    info.setRvp8InfoPktSeqNum(outPktSeqNum++);
    const iwrf_rvp8_ops_info_t &rvp8 = info.getRvp8Info();
    msg.addPart(IWRF_RVP8_OPS_INFO_ID, sizeof(rvp8), &rvp8);
  }

#endif

}

///////////////////////////////////////////////////////////////
// add pulse to DsMessage

void CP2Udp2Fmq::_addPulse2Msg(IwrfTsPulse &pulse, DsMessage &msg,
			       si64 &outPktSeqNum)
  
{


  if (pulse.isRvp8HeaderActive()) {
    pulse.setRvp8PktSeqNum(outPktSeqNum++);
    const iwrf_rvp8_pulse_header_t &rvp8 = pulse.getRvp8Hdr();
    msg.addPart(IWRF_RVP8_PULSE_HEADER_ID, sizeof(rvp8), &rvp8);
  }

  pulse.setPktSeqNum(outPktSeqNum++);
  MemBuf buf;
  pulse.assemble(buf);
  msg.addPart(IWRF_PULSE_HEADER_ID, buf.getLen(), buf.getPtr());

}

