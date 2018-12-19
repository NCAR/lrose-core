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
////////////////////////////////////////////////////////////////////////
// Dsr2CP2Mom.cc
//
// Dsr2CP2Mom object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////////////
//
// Dsr2CP2Mom reads an input radar FMQ, and
// writes CP2Moments UDP data
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/uusleep.h>
#include <toolsa/ucopyright.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <cmath>

#include "Dsr2CP2Mom.hh"

using namespace std;

// Constructor

Dsr2CP2Mom::Dsr2CP2Mom(int argc, char **argv)

{

  isOK = true;
  _pProductSocket = NULL;
  _xContents = 0;
  _sContents = 0;
  _prevSbandAz = 0.0;
  _sbandAz = 0.0;
  _xbandAz = 0.0;
  _clockWise = false;
  _xbandAvail = true;

  // set programe name
  
  _progName = "Dsr2CP2Mom";
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

  // Instantiate and initialize the DsRadar queues
  // Block on SBAND, not on XBAND

  DsFmq::openPosition startPos = DsFmq::START;
  if (_params.seek_to_end_of_input) {
    startPos = DsFmq::END;
  }
  
  if (_sbandQueue.init(_params.sband_fmq_url, _progName.c_str(),
		       _params.debug,
		       DsFmq::BLOCKING_READ_ONLY, startPos)) {
    cerr << "ERROR - Dsr2CP2Mom::_run" << endl;
    cerr << "  Cannot init radar queue: " << _params.sband_fmq_url << endl;
    isOK = false;
  }
  if (_xbandQueue.init(_params.xband_fmq_url, _progName.c_str(),
		       _params.debug,
		       DsFmq::BLOCKING_READ_ONLY, startPos)) {
    cerr << "ERROR - Dsr2CP2Mom::_run" << endl;
    cerr << "  Cannot init radar queue: " << _params.xband_fmq_url << endl;
    isOK = false;
  }
  
  // initialize the output UDP

  if (_initUdpOutput()) {
    isOK = false;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

}

/////////////////////////////////////////////////////////
// destructor

Dsr2CP2Mom::~Dsr2CP2Mom()

{

  if (_pProductSocket) {
    delete _pProductSocket;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Dsr2CP2Mom::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    if (_params.debug) {
      cerr << "Dsr2CP2Mom::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.sband_fmq_url << endl;
    }
    umsleep(1000);
  }

  return 0;

}

//////////////////////////////////////////////////
// _run

int Dsr2CP2Mom::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // read xband message up front,
  // so that there is definitely one available
  
  if (_readMsg(_xbandQueue, _xbandMsg, _xContents) == 0) {
    _xbandAvail = true;
  }

  if (_params.debug) {
    cerr << "-->> Xband data is available" << endl;
  }

  // read beams from the queue
  
  _nBeamsRead = 0;
  
  while (true) {

    // read S band

    if (_readSband() == 0) {

      // read an X band
      
      _readXband();
      
      // synchronize the queues
      
      _syncQueues(false);
      
      // If we have radar and field params, and beam data,
      // process this beam
      
      if ((_sContents & DsRadarMsg::RADAR_BEAM) && _sbandMsg.allParamsSet()) {
	
	if (_params.debug) {
	  _printBeamInfo(_sbandMsg, "SBAND");
	  _printBeamInfo(_xbandMsg, "XBAND");
	}
	
	// compute az error

	double deltaSbandAz = _sbandAz - _prevSbandAz;
	if (deltaSbandAz > 180) {
	  deltaSbandAz -= 360;
	} else if (deltaSbandAz < -180) {
	  deltaSbandAz += 360;
	}
	double sign = -1;
	_clockWise = false;
	if (deltaSbandAz  >= 0) {
	  _clockWise = true;
	  sign = 1;
	}
	_prevSbandAz = _sbandAz;
	double azError = (_xbandAz - _sbandAz) * sign;
	
	if (_params.debug) {
	  cerr << "----> clockwise, azError: "
	       << _clockWise << ", " << azError << endl;
	}
	
	_processSbandBeam();
	_processXbandBeam();
	_nBeamsRead++;
	
      } // if (_sContents ...
      
    } //  if (_readMsg()
    
  } // while (true)
  
  return 0;

}

////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue, setting the flags about beam_data
// and _endOfVolume appropriately.
//

int Dsr2CP2Mom::_readMsg(DsRadarQueue &radarQueue,
			 DsRadarMsg &radarMsg,
			 int &contents) 
  
{
  
   
  PMU_auto_register("Reading radar queue");
  
  if (radarQueue.getDsMsg(radarMsg, &contents)) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// process S-band beam

int Dsr2CP2Mom::_processSbandBeam()

{
  
  const DsRadarParams &radarParams = _sbandMsg.getRadarParams();
  const vector<DsFieldParams*> &fieldParamsArray = _sbandMsg.getFieldParams();
  const DsRadarBeam &radarBeam = _sbandMsg.getRadarBeam();

  if (_params.debug) {
    DateTime btime(radarBeam.dataTime);
    cerr << "Beam time, volnum, az, el: "
	 << DateTime::strm(radarBeam.dataTime) << ", "
	 << radarBeam.volumeNum << ", "
	 << radarBeam.azimuth << ", "
	 << radarBeam.elevation << endl;
  }

  // loop through requested output fields

  int iret = 0;
  for (int ii = 0; ii < _params.sband_output_fields_n; ii++) {
    
    int fieldId = _params._sband_output_fields[ii].field_id;
    string dsrname = _params._sband_output_fields[ii].dsr_name;
    double missingVal = _params._sband_output_fields[ii].missing_val;
    
    for (int jj = 0; jj < (int) fieldParamsArray.size(); jj++) {
      const DsFieldParams &fieldParams = *(fieldParamsArray[jj]);
      if (dsrname == fieldParams.name) {
	if (_processField(dsrname, jj, fieldId,
			  radarParams, fieldParams,
			  radarBeam, missingVal)) {
	  iret = -1;
	}
      }
    } // jj

  } // ii
  
  return iret;

}

////////////////////////////////////////////////////////////////
// process S-band beam

int Dsr2CP2Mom::_processXbandBeam()

{
  
  const DsRadarParams &radarParams = _xbandMsg.getRadarParams();
  const vector<DsFieldParams*> &fieldParamsArray = _xbandMsg.getFieldParams();
  const DsRadarBeam &radarBeam = _xbandMsg.getRadarBeam();

  if (_params.debug) {
    DateTime btime(radarBeam.dataTime);
    cerr << "Beam time, volnum, az, el: "
	 << DateTime::strm(radarBeam.dataTime) << ", "
	 << radarBeam.volumeNum << ", "
	 << radarBeam.azimuth << ", "
	 << radarBeam.elevation << endl;
  }

  // loop through requested output fields

  int iret = 0;
  for (int ii = 0; ii < _params.xband_output_fields_n; ii++) {
    
    int fieldId = _params._xband_output_fields[ii].field_id;
    string dsrname = _params._xband_output_fields[ii].dsr_name;
    double missingVal = _params._xband_output_fields[ii].missing_val;
    
    for (int jj = 0; jj < (int) fieldParamsArray.size(); jj++) {
      const DsFieldParams &fieldParams = *(fieldParamsArray[jj]);
      if (dsrname == fieldParams.name) {
	if (_processField(dsrname, jj, fieldId,
			  radarParams, fieldParams,
			  radarBeam, missingVal)) {
	  iret = -1;
	}
      }
    } // jj

  } // ii
  
  return iret;

}

///////////////////////
// process this field

int Dsr2CP2Mom::_processField(const string &fieldName,
			      int dsrFieldNum,
			      int cp2FieldId,
			      const DsRadarParams &radarParams,
			      const DsFieldParams &fieldParams,
			      const DsRadarBeam &radarBeam,
			      double missingVal)

{

  // set header

  CP2Net::CP2ProductHeader header;
  header.prodType = (CP2Net::PRODUCT_TYPES) cp2FieldId;
  header.beamNum = _nBeamsRead;
  header.gates = radarParams.numGates;
  header.az = radarBeam.azimuth;
  header.el = radarBeam.elevation;
  header.gateWidthKm = radarParams.gateSpacing;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Sending field name, dsrNum, cp2Id, beamNum, el, az, gateSpacing: "
	 << fieldName << ", "
	 << dsrFieldNum << ", "
	 << cp2FieldId << ", "
	 << header.beamNum << ", "
	 << header.el << ", "
	 << header.az << ", "
	 << header.gateWidthKm << endl;
  }

  // unpack data

  TaArray<double> _data;
  double *data = (double *) _data.alloc(radarParams.numGates);
  
  if(fieldParams.byteWidth == 1) {
    
    ui08 *dd = (ui08 *) radarBeam.getData() + dsrFieldNum;
    ui08 missing = (ui08) fieldParams.missingDataValue;
    double scale = fieldParams.scale;
    double bias = fieldParams.bias;
    for (int ii = 0; ii < radarParams.numGates;
	 ii++, dd += radarParams.numFields) {
      if (*dd == missing) {
	data[ii] = missingVal;
      } else {
	data[ii] = *dd * scale + bias;
      }
    }

  } else if (fieldParams.byteWidth == 2) {
    
    ui16 *dd = (ui16 *) radarBeam.getData() + dsrFieldNum;
    ui16 missing = (ui16) fieldParams.missingDataValue;
    double scale = fieldParams.scale;
    double bias = fieldParams.bias;
    for (int ii = 0; ii < radarParams.numGates;
	 ii++, dd += radarParams.numFields) {
      if (*dd == missing) {
	data[ii] = missingVal;
      } else {
	data[ii] = *dd * scale + bias;
      }
    }

  } else if (fieldParams.byteWidth == 4) {

    fl32 *dd = (fl32 *) radarBeam.getData() + dsrFieldNum;
    fl32 missing = (fl32) fieldParams.missingDataValue;
    for (int ii = 0; ii < radarParams.numGates;
	 ii++, dd += radarParams.numFields) {
      if (*dd == missing) {
	data[ii] = missingVal;
      } else {
	data[ii] = *dd;
      }
    }

  } else {

    cerr << "ERROR - invalid byte width: " << fieldParams.byteWidth << endl;
    return -1;

  }

  // copy header and data to membuf

  MemBuf outBuf;
  outBuf.add(&header, sizeof(header));
  outBuf.add(data, radarParams.numGates * sizeof(double));

  // send the buffer

  int bytesSent = 0;
  int ntries = 0;
  while (ntries < 10) {
    bytesSent =
      _pProductSocket->writeDatagram((const char*)outBuf.getPtr(),
				     outBuf.getLen());
    if(bytesSent == (int) outBuf.getLen()) {
      return 0;
    }
    umsleep(100);
    ntries++;
  }

  cerr << "ERROR - packet sending failed" << endl;

  return -1;

}

////////////////////////////////////////////////////////////////////
// initialize the UDP output

int Dsr2CP2Mom::_initUdpOutput()

{

  // create the UDP socket
  
  _pProductSocket = new CP2UdpSocket(_params.udp_network,
				     _params.udp_port,
				     true,
				     _udpBufferSize, 0,
				     _params.debug >= Params::DEBUG_NORM);
  if (!_pProductSocket->ok()) {
    cerr << "ERROR - Dsr2CP2Mom::_initUdp" << endl;
    cerr << "Cannot open socket: network, port: "
	 << _params.udp_network << ", "
	 << _params.udp_port << endl;
    delete _pProductSocket;
    _pProductSocket = NULL;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// send a field

void Dsr2CP2Mom::_sendField(CP2Net::CP2ProductHeader& header, 
			    std::vector<double>& data,
			    CP2Net::CP2Packet& packet,
			    bool forceSend)
{
  
  int incr = data.size()*sizeof(data[0]) + sizeof(header);

  // if this packet will get too large by adding new data, 
  // go ahead and send it
  
  if (packet.packetSize()+incr > _soMaxMsgSize) {
    int bytesSent = 0;
    while (bytesSent != packet.packetSize()) {
      bytesSent =
	_pProductSocket->writeDatagram((const char*)packet.packetData(),
				       packet.packetSize());
      if(bytesSent != packet.packetSize())
	umsleep(1000);
    }
    packet.clear();
  }
  
  packet.addProduct(header, data.size(), &data[0]);

  if (forceSend) {
    int bytesSent = 0;
    while (bytesSent != packet.packetSize()) {
      bytesSent =
	_pProductSocket->writeDatagram((const char*)packet.packetData(),
				       packet.packetSize());
      if(bytesSent != packet.packetSize())
	umsleep(1000);
    }
    packet.clear();
  }

}

////////////////////////////////////////////////////////////////
// print beam info

void Dsr2CP2Mom::_printBeamInfo(const DsRadarMsg &radarMsg,
				const string &label)
  
{
  
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  
  DateTime btime(radarBeam.dataTime);
  cerr << "Beam label, time, volnum, az, el: "
       << label << ", "
       << DateTime::strm(radarBeam.dataTime) << ", "
       << radarBeam.volumeNum << ", "
       << radarBeam.azimuth << ", "
       << radarBeam.elevation << endl;

}

////////////////////////////////////////////////////////////////
// read S band
// Sband is primary - always read
// returns 0 on success, -1 on failure

int Dsr2CP2Mom::_readSband()

{

  if (_readMsg(_sbandQueue, _sbandMsg, _sContents)) {
    return -1;
  }

  _sbandAz = _getAz(_sbandMsg);

  return 0;

}

////////////////////////////////////////////////////////////////
// read X band
// Xband is secondary - do not read if ahead
// returns 0 on success, -1 on failure

int Dsr2CP2Mom::_readXband()

{

  double sTime = _getTimeDbl(_sbandMsg);
  double xTime = _getTimeDbl(_xbandMsg);
  if (xTime > sTime) {
    return 0;
  }

  if (_readMsg(_xbandQueue, _xbandMsg, _xContents)) {
    return -1;
  }

  _xbandAz = _getAz(_xbandMsg);

  return 0;

}

////////////////////////////////////////////////////////////////
// get azimuth, elevation, time

double Dsr2CP2Mom::_getAz(const DsRadarMsg &radarMsg)
  
{
  return radarMsg.getRadarBeam().azimuth;
}

double Dsr2CP2Mom::_getEl(const DsRadarMsg &radarMsg)
  
{
  return radarMsg.getRadarBeam().elevation;
}

time_t Dsr2CP2Mom::_getTime(const DsRadarMsg &radarMsg)
  
{
  return radarMsg.getRadarBeam().dataTime;
}

// full time including partial secs

double Dsr2CP2Mom::_getTimeDbl(const DsRadarMsg &radarMsg)
  
{
  double dtime = radarMsg.getRadarBeam().dataTime;
  double nanoSecs = radarMsg.getRadarBeam().nanoSecs;
  dtime += nanoSecs / 1.0e9;
  return dtime;
}

//////////////////////
// sync the two queues

void Dsr2CP2Mom::_syncQueues(bool allowSbandRead)

{
  
  // first get times the same
  
  while (true) {

    double sTime = _getTimeDbl(_sbandMsg);
    double xTime = _getTimeDbl(_xbandMsg);

    double diff = sTime - xTime;
    if (fabs(diff) < 0.1) {
      break;
    }
    
    if (sTime < xTime) {
      if (allowSbandRead) {
	// s lags, read an S beam
	_readSband();
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "_syncQueues: S band catching up" << endl;
	}
      } else {
	return;
      }
    } else if (xTime < sTime) {
      // x lags, read an X beam
      _readXband();
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "_syncQueues: X band catching up" << endl;
      }
    }

  }

}

