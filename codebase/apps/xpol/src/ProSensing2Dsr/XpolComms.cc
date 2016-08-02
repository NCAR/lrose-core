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
// XpolComms.cc
//
// XpolComms object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////
//
// XpolComms for ProSensing server communication
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#include "XpolComms.hh"
#include <toolsa/DateTime.hh>
#include <dataport/swap.h>
#include <dataport/bigend.h>
using namespace std;

/////////////////////////////////////////////////////////
// constructor

XpolComms::XpolComms()

{

  _debug = false;
  _verbose = false;
  _overrideAzOffset = false;
  _azOffsetDeg = 0.0;

  // set swapping flag

  _swap = false;
  if (BE_is_big_endian()) {
    // Prosensing data is little-endian
    _swap = true;
  }

  // sockets

  _socketIsOpen = false;
  _serverHost = "localhost";
  _serverPort = 3000;

  // flag to indicate when props change

  _archiveIndex = -1;
  _metaDataHasChanged = false;
  _prevBlockIndex = 0;

}

/////////////////////////////////////////////////////////
// destructor

XpolComms::~XpolComms()

{

  // close socket

  _closeSocket();

}

/////////////////////////////////////////////////////////
// close any open connections

void XpolComms::close()

{

  _closeSocket();

}

//////////////////////////////////////////////////
// check that the connection is open

int XpolComms::_openSocket()
  
{

  // open socket connection to rdas
  
  int iret = _socket.open(_serverHost.c_str(), _serverPort, 1000);
  if (iret == 0) {
    if (_debug) {
      cerr << "  Connected to xpold ..." << endl;
      cerr << "    host: " << _serverHost << endl;
      cerr << "    port: " << _serverPort << endl;
    }
    return 0;
  }
  
  // time-out?
  
  if (_socket.getErrNum() == SockUtil::TIMED_OUT) {
    return -1;
  }
    
  // failure

  cerr << "ERROR - XpolComms::_openSocket" << endl;
  cerr << "  " << _socket.getErrStr() << endl;

  return -1;

}
      
//////////////////////////////////////////////////
// close connection to rdas

void XpolComms::_closeSocket()

{

  if (_socketIsOpen) {
    _socket.close();
    _socketIsOpen = false;
  }

}
      
//////////////////////////////////////////////////
// communicate with server
//
// send command buffer
// receive status or data

int XpolComms::_communicateWithServer()
  
{
  
  return _communicateWithServer(_writeMemBuf.getPtr(), 
                                _writeMemBuf.getLen(),
                                _readMemBuf.getPtr(), 
                                _readMemBuf.getLen());

}

int XpolComms::_communicateWithServer(void *sendBuf,
                                      int sendLen,
                                      void *rcvBuf,
                                      int rcvLen)
  
{
  
  if (!_socketIsOpen) {
    if (_openSocket()) {
      cerr << "ERROR - XpolComms::_communicateWithServer" << endl;
      cerr << "  Cannot open socket to server" << endl;
      cerr << "    host: " << _serverHost << endl;
      cerr << "    port: " << _serverPort << endl;
      cerr << _socket.getErrStr() << endl;
      return -1;
    }
    _socketIsOpen = true;
  }
    
  if (_socket.writeBuffer(sendBuf, sendLen)) {
    cerr << "ERROR - XpolComms::_communicateWithServer" << endl;
    cerr << "  Cannot write buffer to server" << endl;
    cerr << _socket.getErrStr() << endl;
    _closeSocket();
    return -1;
  }

  if (_socket.readBuffer(rcvBuf, rcvLen, 1000)) {
    cerr << "ERROR - XpolComms::_communicateWithServer" << endl;
    if (_socket.getErrNum() == SockUtil::TIMED_OUT) {
      cerr << "  Read timed out" << endl;
      return -1;
    } else {
      cerr << "  Cannot read response from server" << endl;
      cerr << _socket.getErrStr() << endl;
    }
    _closeSocket();
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// read bytes from server

int XpolComms::_readFromServer(void *rcvBuf, int rcvLen)
  
{
  
  if (!_socketIsOpen) {
    cerr << "ERROR - XpolComms::_readFromServer" << endl;
    cerr << "  Socket not open to server" << endl;
    cerr << "    host: " << _serverHost << endl;
    cerr << "    port: " << _serverPort << endl;
    return -1;
  }
  
  if (_socket.readBuffer(rcvBuf, rcvLen, 10000)) {
    cerr << "ERROR - XpolComms::_readFromServer" << endl;
    cerr << "  Cannot read data from server" << endl;
    _closeSocket();
    return -1;
  }

  return 0;

}

int XpolComms::_readFromServer()

{

  return _readFromServer(_readMemBuf.getPtr(), 
                         _readMemBuf.getLen());

}
  
//////////////////////////////////////////////////
// ping server to get status

int XpolComms::pingServer()
  
{

  _resetWriteBuf();
  _allocReadBuf(4);
  
  si32 command = PING_SERVER;
  _putNextSi32(command);

  if (_communicateWithServer()) {
    cerr << "ERROR - XpolComms::pingServer" << endl;
    return -1;
  }
  
  statusCode_t statusCode = (statusCode_t) _getNextSi32();

  if (statusCode != STATUS_OK) {
    cerr << "ERROR - XpolComms::pingServer" << endl;
    cerr << "  statusCode: " << _statusCode2Str(statusCode) << endl;
    return -1;
  }
  
  if (_debug) {
    cerr << "XpolComms::pingServer" << endl;
    cerr << "  statusCode: " << _statusCode2Str(statusCode) << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// read configuration

int XpolComms::readConf(bool withStatus)
  
{
  
  if (_debug) {
    cerr << "XpolComms::readConf - start" << endl;
  }

  _resetWriteBuf();
  _allocReadBuf(20);

  si32 command = GET_CONF;
  if (withStatus) {
    command = GET_CONF_AND_STATUS;
  }
  _putNextSi32(command);

  if (_communicateWithServer()) {
    cerr << "ERROR - XpolComms::readConf" << endl;
    return -1;
  }
  
  statusCode_t initStatus = (statusCode_t) _getNextSi32();
  if (initStatus != STATUS_OK && initStatus != STATUS_CFG_TRANSITION) {
    cerr << "ERROR - XpolComms::readConf" << endl;
    cerr << "  init status code: " << _statusCode2Str(initStatus) << endl;
    _closeSocket();
    return -1;
  }

  si32 totalSize = _getNextSi32();
  si32 archiveIndex = _getNextSi32();
  si32 confSize = _getNextSi32();
  si32 statusSize = _getNextSi32();

  if (_verbose) {
    cerr << "  initStatus: "
         << _statusCode2Str(initStatus) << endl;
    cerr << "  totalSize: " << totalSize << endl;
    cerr << "  archiveIndex: " << archiveIndex << endl;
    cerr << "  confSize: " << confSize << endl;
    cerr << "  statusSize: " << statusSize << endl;
  }

  // read configuration if available

  if (confSize > 0) {
    _allocReadBuf(confSize);
    if (_readFromServer()) {
      cerr << "ERROR - XpolComms::readConf" << endl;
      return -1;
    }
    _xpolStatus.unixTimeSecs = _getNextSi32();
    _xpolStatus.timeMicroSecs = _getNextSi32();
    _loadConf(_xpolConf);
    if (_verbose) {
      printConf(cerr);
    }
  } // if (confSize > 0)
  
  // read status if included

  if (statusSize > 0) {
    _allocReadBuf(statusSize);
    if (_readFromServer()) {
      cerr << "ERROR - XpolComms::readConf" << endl;
      return -1;
    }
    _loadStatus(_xpolStatus);
    if (_verbose) {
      printStatus(cerr);
    }
  } // if (statusSize > 0) 
    
  // get final status

  _allocReadBuf(sizeof(si32));
  if (_readFromServer()) {
    cerr << "ERROR - XpolComms::readConf" << endl;
    return -1;
  }
  statusCode_t finalStatus = (statusCode_t) _getNextSi32();
  
  if (_verbose) {
    cerr << "  finalStatus: "
         << _statusCode2Str(finalStatus) << endl;
  }

  if (finalStatus != STATUS_OK) {
    cerr << "ERROR - XpolComms::readConf" << endl;
    cerr << "  final status code: " << _statusCode2Str(finalStatus) << endl;
    _closeSocket();
    return -1;
  }

  if (_debug) {
    cerr << "XpolComms::readConf - successful" << endl;
  }

  return 0;

}


//////////////////////////////////////////////////
// read xpol status

int XpolComms::readStatus()
  
{
  
  if (_debug) {
    cerr << "XpolComms::readStatus - start" << endl;
  }

  _resetWriteBuf();
  _allocReadBuf(12);

  si32 command = GET_STATUS;
  _putNextSi32(command);

  if (_communicateWithServer()) {
    cerr << "ERROR - XpolComms::readStatus" << endl;
    return -1;
  }
  
  statusCode_t initStatus = (statusCode_t) _getNextSi32();
  if (initStatus != STATUS_OK && initStatus != STATUS_CFG_TRANSITION) {
    cerr << "ERROR - XpolComms::readStatus" << endl;
    cerr << "  init status code: " << _statusCode2Str(initStatus) << endl;
    _closeSocket();
    return -1;
  }

  si32 totalSize = _getNextSi32();
  si32 statusSize = _getNextSi32();

  if (_verbose) {
    cerr << "  initStatus: "
         << _statusCode2Str(initStatus) << endl;
    cerr << "  totalSize: " << totalSize << endl;
    cerr << "  statusSize: " << statusSize << endl;
  }

  // read status if available

  if (statusSize > 0) {
    _allocReadBuf(statusSize);
    if (_readFromServer()) {
      cerr << "ERROR - XpolComms::readStatus" << endl;
      return -1;
    }
    _loadStatus(_xpolStatus);
    if (_verbose) {
      printStatus(cerr);
    }
  } // if (statusSize > 0) 
  
  // get final status

  _allocReadBuf(sizeof(si32));
  if (_readFromServer()) {
    cerr << "ERROR - XpolComms::readStatus" << endl;
    return -1;
  }
  statusCode_t finalStatus = (statusCode_t) _getNextSi32();
  
  if (_verbose) {
    cerr << "  finalStatus: "
         << _statusCode2Str(finalStatus) << endl;
  }

  if (finalStatus != STATUS_OK) {
    cerr << "ERROR - XpolComms::readStatus" << endl;
    cerr << "  final status code: " << _statusCode2Str(finalStatus) << endl;
    _closeSocket();
    return -1;
  }

  if (_debug) {
    cerr << "XpolComms::readStatus - successful" << endl;
  }

  return 0;

}


//////////////////////////////////////////////////
// read server info

int XpolComms::readServerInfo()
  
{

  if (_debug) {
    cerr << "XpolComms::readServerInfo - start" << endl;
  }

  // set up command

  _resetWriteBuf();
  si32 command = GET_SERVER_INFO;
  _putNextSi32(command);
  _allocReadBuf(8);

  if (_communicateWithServer()) {
    cerr << "ERROR - XpolComms::readServerInfo" << endl;
    return -1;
  }
  
  statusCode_t initStatus = (statusCode_t) _getNextSi32();
  if (initStatus != STATUS_OK && initStatus != STATUS_CFG_TRANSITION) {
    cerr << "ERROR - XpolComms::readServerInfo" << endl;
    cerr << "  init status code: " << _statusCode2Str(initStatus) << endl;
    _closeSocket();
    return -1;
  }

  si32 serverInfoSize = _getNextSi32();

  if (_verbose) {
    cerr << "  initStatus: "
         << _statusCode2Str(initStatus) << endl;
    cerr << "  serverInfoSize: " << serverInfoSize << endl;
  }

  // read server info if available

  if (serverInfoSize > 0) {
    _allocReadBuf(serverInfoSize);
    if (_readFromServer()) {
      cerr << "ERROR - XpolComms::readServerInfo" << endl;
      return -1;
    }
    _loadServerInfo(_serverInfo);
    if (_verbose) {
      printServerInfo(cerr);
    }
  } // if (serverInfoSize > 0) 
  
  // get final status

  _allocReadBuf(sizeof(si32));
  if (_readFromServer()) {
    cerr << "ERROR - XpolComms::readServerInfo" << endl;
    return -1;
  }
  statusCode_t finalStatus = (statusCode_t) _getNextSi32();
  
  if (_verbose) {
    cerr << "  finalStatus: "
         << _statusCode2Str(finalStatus) << endl;
  }

  if (finalStatus != STATUS_OK) {
    cerr << "ERROR - XpolComms::readServerInfo" << endl;
    cerr << "  final status code: " << _statusCode2Str(finalStatus) << endl;
    _closeSocket();
    return -1;
  }

  if (_debug) {
    cerr << "XpolComms::readServerInfo - successful" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// read data

int XpolComms::readData(fieldId_t fieldId)
  
{

  if (_verbose) {
    cerr << "XpolComms::readData - start" << endl;
  }

  // set up command

  _resetWriteBuf();

  si32 command = GET_DATA;
  _putNextSi32(command);

  si32 reqLen = 60;
  _putNextSi32(reqLen);

  DataRequest req;
  req.productTypeCode = fieldId;
  req.blockOffset = -1;
  req.blockStepLength = 1;
  req.blockSeenStepLength = 1;
  req.maxNumBlocks = 1;
  req.matrixExtent1[0] = 0;
  req.matrixExtent1[1] = 0;
  req.matrixExtent1[2] = 0;
  req.matrixExtent1[3] = 0;
  req.matrixExtent2[0] = -1;
  req.matrixExtent2[1] = -1;
  req.matrixExtent2[2] = -1;
  req.matrixExtent2[3] = -1;
  req.expectedArchiveIndex = -1;
  req.formatFlags = 0x01 | 0x10000;

  if (_verbose) {
    printDataRequest(req, cerr);
  }

  if (_debug) {
    cerr << "PEDESTAL el, az: " 
         << _dataResponse.pedestal.azPosDeg << ", "
         << _dataResponse.pedestal.elPosDeg << endl;
  }

  // send command

  _putDataRequest(req);

  // read response

  _allocReadBuf(8);
  if (_communicateWithServer()) {
    cerr << "ERROR - XpolComms::readData" << endl;
    return -1;
  }
  statusCode_t initStatus = (statusCode_t) _getNextSi32();
  if (initStatus != STATUS_OK && initStatus != STATUS_CFG_TRANSITION) {
    cerr << "ERROR - XpolComms::readData" << endl;
    cerr << "  init status code: " << _statusCode2Str(initStatus) << endl;
    _closeSocket();
    return -1;
  }
  si32 responseSize = _getNextSi32();
  if (_verbose) {
    cerr << "  initStatus: "
         << _statusCode2Str(initStatus) << endl;
    cerr << "  responseSize: " << responseSize << endl;
  }
  
  // read data if available
  
  if (responseSize > 0) {
    _allocReadBuf(responseSize);
    if (_readFromServer()) {
      cerr << "ERROR - XpolComms::readData" << endl;
      return -1;
    }
    _loadDataResponse(_dataResponse);
    if (_verbose) {
      printDataResponse(cerr);
    }
    if (_loadDataBuffer()) {
      cerr << "ERROR - XpolComms::readData" << endl;
      return -1;
    }
  } // if (statusSize > 0) 
  
  // get final status

  _allocReadBuf(sizeof(si32));
  if (_readFromServer()) {
    cerr << "ERROR - XpolComms::readData" << endl;
    return -1;
  }
  statusCode_t finalStatus = (statusCode_t) _getNextSi32();
  
  if (_verbose) {
    cerr << "  finalStatus: "
         << _statusCode2Str(finalStatus) << endl;
  }
  
  if (finalStatus != STATUS_OK) {
    cerr << "ERROR - XpolComms::readData" << endl;
    cerr << "  final status code: " << _statusCode2Str(finalStatus) << endl;
    _closeSocket();
    return -1;
  }

  if (_verbose) {
    cerr << "XpolComms::readData - successful" << endl;
  }

  // if meta data has changed, reread the meta data
  // so that we know sizes, ngates etc

  _metaDataHasChanged = false;
  if (_dataResponse.archiveIndex != _archiveIndex) {
    if (readMetaData()) {
      return -1;
    }
    _archiveIndex = _dataResponse.archiveIndex;
    _metaDataHasChanged = true;
  }

  // alloc buffers for covariance fields
  
  _allocCovarBuffers();

  // load up the covariance fields
  
  _loadCovarBuffers();

  return 0;

}

//////////////////////////////////////////////////
// read the meta data associated with the data

int XpolComms::readMetaData()
{
  
  if (readStatus()) {
    cerr << "ERROR - XpolComms::_readMetaData" << endl;
    return -1;
  }
  
  if (_verbose) {
    printStatus(getStatus(), cerr);
  }
  
  if (readConf(true)) {
    cerr << "ERROR - XpolComms::_readMetaData" << endl;
    return -1;
  }

  if (_verbose) {
    cerr << "Server mode: " << getServerModeStr() << endl;
    printConf(getConf(), cerr);
  }
  
  if (readServerInfo()) {
    cerr << "ERROR - XpolComms::_readMetaData" << endl;
    return -1;
  }
  
  if (_verbose) {
    printServerInfo(_serverInfo, cerr);
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Load status object

void XpolComms::_loadStatus(XpolStatus &status)

{
  
  status.unixTimeSecs = _getNextSi32();
  status.timeMicroSecs = _getNextSi32();
  status.radarTemps[0] = _getNextSi32();
  status.radarTemps[1] = _getNextSi32();
  status.radarTemps[2] = _getNextSi32();
  status.radarTemps[3] = _getNextSi32();
  status.inclinometerRoll = _getNextSi32();
  status.inclinometerPitch = _getNextSi32();
  status.fuel = _getNextSi32();
  status.cpuTempC = _getNextFl32();
  status.scanMode = (scanMode_t) _getNextSi32();
  status.txPowerMw = _getNextFl32();

}
    
/////////////////////////////////////////////////////////
// Load conf object

void XpolComms::_loadConf(XpolConf &conf)

{
  
  conf.siteInfo = _getNextString(1024);
  conf.azOffset = _getNextFl64();
  if (_overrideAzOffset || fabs(conf.azOffset) > 360) {
    conf.azOffset = _azOffsetDeg;
  }
  conf.spare1 = _getNextSi32();
  conf.clutFilterWidthMPerSec = _getNextFl64();
  conf.clutAvInterval = _getNextSi32();
  conf.productsPerSec = _getNextFl64();
  conf.fftLength = _getNextSi32();
  conf.fftWindowType = _getNextSi32();
  conf.reserved1 = _getNextSi32();
  conf.autoFileRollNRecords = _getNextSi32();
  conf.autoFileRollNScans = _getNextSi32();
  conf.autoFileRollFileSizeMb = _getNextSi32();
  conf.autoFileRollElapsedTimeSec = _getNextSi32();
  conf.autoFileRollTypeBitFlags = _getNextSi32();
  conf.reserved2 = _getNextSi32();
  conf.filterBandwidthMhz = _getNextFl64();
  conf.freqTrackAdjThreshPerc = _getNextFl64();
  conf.freqTrackMode = _getNextSi32();
  conf.reserved3 = _getNextSi32();
  conf.groupIntervalUsec = _getNextSi32();
  conf.hDbzPerDbmOffset = _getNextFl64();
  conf.hNoisePowerDbm = _getNextFl64();
  conf.loFreqErrorMhz = _getNextFl64();
  conf.loFreqMhz = _getNextFl64();
  conf.maxSampledRangeM = _getNextFl64();
  conf.nGates = _getNextSi32();
  conf.nGroupPulses = _getNextSi32();
  conf.postDecimationLevel = _getNextSi32();
  conf.postAveragingInterval = _getNextSi32();
  conf.priUsecUnit1 = _getNextSi32();
  conf.priUsecUnit2 = _getNextSi32();
  conf.priUsecUnitTotal = _getNextSi32();
  conf.primOnBoardDecLevel = _getNextSi32();
  conf.pulseLenM = _getNextFl64();
  conf.reserved4 = _getNextSi32();
  conf.gateSpacingM = _getNextFl64();
  conf.reserved5 = _getNextSi32();
  conf.rangeResMPerGate = _getNextFl64();
  conf.recordMoments = _getNextSi32();
  conf.recordRaw = _getNextSi32();
  conf.recordingEnabled = _getNextSi32();
  conf.serverMode = _getNextSi32();
  conf.reserved6 = _getNextSi32();
  conf.serverState = _getNextSi32();
  conf.reserved7 = _getNextSi32();
  conf.softwareDecLevel = _getNextSi32();
  conf.sumPowers = _getNextSi32();
  conf.totAveragingInterval = _getNextSi32();
  conf.txDelayNanoSec = _getNextSi32();
  conf.txDelayPulseWidthMult = _getNextSi32();
  conf.txPulseCenterNanoSec = _getNextSi32();
  conf.txPulseCenterOffsetNanoSec = _getNextSi32();
  conf.txSampleSwitchDelayNanoSec = _getNextSi32();
  conf.txSampleSwitchHoldoffNanoSec = _getNextSi32();
  conf.useClutFilter = _getNextSi32();
  conf.vDbzPerDbmOffset = _getNextFl64();
  conf.vNoisePowerDbm = _getNextFl64();
  conf.zeroRangeGateIndex = _getNextFl64();

  _serverMode = (serverMode_t) conf.serverMode;

}
    
/////////////////////////////////////////////////////////
// Load server info object

void XpolComms::_loadServerInfo(ServerInfo &serverInfo)

{

  serverInfo.projectName = _getNextString(128);
  _loadDrxInfo(serverInfo.drxInfo);
  serverInfo.productCount = _getNextSi32();
  serverInfo.dataProdInfo.clear();
  for (int ii = 0; ii < serverInfo.productCount; ii++) {
    DataProdInfo prodInfo;
    _loadDataProdInfo(prodInfo);
    serverInfo.dataProdInfo.push_back(prodInfo);
  }

}
    
/////////////////////////////////////////////////////////
// Load drx spec object

void XpolComms::_loadDrxSpec(DrxSpec &drxSpec)

{

  drxSpec.numInputChannels = _getNextSi32();
  drxSpec.minSampleClockFreqHz = _getNextSi32();
  drxSpec.maxSampleClockFreqHz = _getNextSi32();
  drxSpec.minCenterFreqHz = _getNextSi32();
  drxSpec.maxCenterFreqHz = _getNextSi32();
  drxSpec.minNumDmaDescriptors = _getNextSi32();
  drxSpec.maxNumDmaDescriptors = _getNextSi32();
  drxSpec.minNumDmaDescripPackets = _getNextSi32();
  drxSpec.maxNumDmaDescripPackets = _getNextSi32();
  drxSpec.minDmaPacketSizeBytes = _getNextSi32();
  drxSpec.maxDmaPacketSizeBytes = _getNextSi32();
  drxSpec.dnaPacketSizeGranBytes = _getNextSi32();
  drxSpec.minBurstSizeBytes = _getNextSi32();
  drxSpec.maxBurstSizeBytes = _getNextSi32();
  drxSpec.burstSizeGranBytes = _getNextSi32();
  drxSpec.minNumBurstsPerPciIntr = _getNextFl64();
  drxSpec.maxNumBurstsPerPciIntr = _getNextFl64();
  drxSpec.minSkipCountSamples = _getNextSi32();
  drxSpec.maxSkipCountSamples = _getNextSi32();
  drxSpec.minCicDecLevel = _getNextSi32();
  drxSpec.maxCicDecLevel = _getNextSi32();
  drxSpec.minFirDecLevel = _getNextSi32();
  drxSpec.maxFirDecLevel = _getNextSi32();
  drxSpec.minPostDecLevel = _getNextSi32();
  drxSpec.maxPostDecLevel = _getNextSi32();
  drxSpec.maxFirFilterLen = _getNextSi32();
  drxSpec.minFirGain = _getNextFl64();
  drxSpec.maxFirGain = _getNextFl64();
  drxSpec.fullScaleFirGainLevel = _getNextFl64();
  drxSpec.minAnalogVoltageInputV = _getNextFl64();
  drxSpec.maxAnalogVoltageInputV = _getNextFl64();
  drxSpec.analogImpedanceOhms = _getNextFl64();
  drxSpec.minDigitizedCountValue = _getNextSi32();
  drxSpec.maxDigitizedCountValue = _getNextSi32();
  drxSpec.adResBitsPerSample = _getNextSi32();
  drxSpec.digitizedCountSizeBits = _getNextSi32();

}

/////////////////////////////////////////////////////////
// Load drx info object

void XpolComms::_loadDrxInfo(DrxInfo &drxInfo)

{

  drxInfo.manufacturerCode = _getNextSi32();
  drxInfo.manufacturerName = _getNextString(32);
  drxInfo.modelCode = _getNextSi32();
  drxInfo.modelName = _getNextString(32);
  _loadDrxSpec(drxInfo.drxSpec);

}

/////////////////////////////////////////////////////////
// Load drx conf object

void XpolComms::_loadDrxConf(DrxConf &drxConf)

{

  drxConf.mode = (drxMode_t) _getNextSi32();
  drxConf.trig = (drxTrig_t) _getNextSi32();
  drxConf.pciBusFreqMhz = _getNextSi32();
  drxConf.a2dSampleFreqHz = _getNextSi32();
  drxConf.numDmaDescriptors = _getNextSi32();
  drxConf.numDmaPacketsPerDesc = _getNextSi32();
  drxConf.dmaPacketSize = _getNextSi32();
  drxConf.blockSize = _getNextSi32();
  drxConf.reserved1 = _getNextSi32();
  drxConf.numBlocksPerPciIntr = _getNextFl64();
  drxConf.reserved2 = _getNextSi32();
  drxConf.reserved3 = _getNextSi32();
  drxConf.state[0] = (drxState_t) _getNextSi32();
  drxConf.state[1] = (drxState_t) _getNextSi32();
  drxConf.source[0] = (drxSource_t) _getNextSi32();
  drxConf.source[1] = (drxSource_t) _getNextSi32();
  drxConf.skipCount[0] = _getNextSi32();
  drxConf.skipCount[1] = _getNextSi32();
  drxConf.cicDecimation[0] = _getNextSi32();
  drxConf.cicDecimation[1] = _getNextSi32();
  drxConf.firDecimation[0] = _getNextSi32();
  drxConf.firDecimation[1] = _getNextSi32();
  drxConf.postDecimation[0] = _getNextSi32();
  drxConf.postDecimation[1] = _getNextSi32();
  drxConf.ncoFreqHz[0] = _getNextSi32();
  drxConf.ncoFreqHz[1] = _getNextSi32();

}

/////////////////////////////////////////////////////////
// Load dataProd info object

void XpolComms::_loadDataProdInfo(DataProdInfo &prodInfo)

{

  prodInfo.typeCode = _getNextSi32();
  prodInfo.shortName = _getNextString(32);
  prodInfo.longName = _getNextString(64);
  prodInfo.drxChannel = _getNextSi32();
  prodInfo.posDeviceIndex = _getNextSi32();
  prodInfo.gpsDeviceIndex = _getNextSi32();
  prodInfo.dataDomain = (dataDomain_t) _getNextSi32();
  prodInfo.dataUnits = (dataUnits_t) _getNextSi32();
  prodInfo.nInterleavedTracks = _getNextSi32();
  prodInfo.matrixDim = _getNextSi32();

}

/////////////////////////////////////////////////////////
// Load pedestal object

void XpolComms::_loadPedestal(Pedestal &ped)

{

  ped.azPosDeg = _getNextFl64();
  ped.elPosDeg = _getNextFl64();
  ped.azVelDegPerSec = _getNextFl64();
  ped.elVelDegPerSec = _getNextFl64();
  ped.azCurrentAmps = _getNextFl64();
  ped.elCurrentAmps = _getNextFl64();

}

/////////////////////////////////////////////////////////
// Load georef object

void XpolComms::_loadGeoref(Georef &geo)

{

  int latHemi = _getNextSi32();
  geo.latDeg = _getNextFl64();
  if (latHemi == 'S' && geo.latDeg > 0) {
    geo.latDeg *= -1.0;
  }

  int lonHemi = _getNextSi32();
  geo.lonDeg = _getNextFl64();
  if (lonHemi == 'W' && geo.lonDeg > 0) {
    geo.lonDeg *= -1.0;
  }

  geo.altMeters = _getNextFl64();

  int headRef = _getNextSi32();
  if (headRef == 'M') {
    geo.headingMagnetic = true;
  } else {
    geo.headingMagnetic = false;
  }
  
  geo.headingDeg = _getNextFl64();
  geo.speedKmh = _getNextFl64();

}

/////////////////////////////////////////////////////////
// Load response object

void XpolComms::_loadDataResponse(DataResponse &resp)

{
  
  resp.productTypeCode = _getNextSi32();
  resp.unixTimeSecs = _getNextSi32();
  resp.timeNanoSecs = _getNextSi32();

  _loadDrxConf(resp.drxConf);

  resp.drxFirFilterGain = _getNextFl64();
  resp.averagingIntervalLen = _getNextSi32();
  resp.archiveIndex = _getNextSi32();
  resp.blockIndex = _getNextSi64();
  resp.nBlocks = _getNextSi32();
  resp.blockSize = _getNextSi32();
  resp.numBlockDim = _getNextSi32();
  resp.blockDims[0] = _getNextSi32();
  resp.blockDims[1] = _getNextSi32();
  resp.blockDims[2] = _getNextSi32();
  resp.blockDims[3] = _getNextSi32();

  _loadPedestal(resp.pedestal);
  _loadGeoref(resp.georef);

  int nSkipped = resp.blockIndex - _prevBlockIndex - 1;
  if (nSkipped > 0 && _debug) {
    cerr << "===========>> Skipped n blocks: " << nSkipped << endl;
  }
  _prevBlockIndex = resp.blockIndex;

}
    
/////////////////////////////////////////////////////////
// Load data buffer

int XpolComms::_loadDataBuffer()
  
{

  if (_dataResponse.nBlocks < 0 || _dataResponse.blockSize < 0) {
    cerr << "ERROR - _loadDataBuffer, bad data sized" << endl;
    cerr << "  _dataResponse.nBlocks: " << _dataResponse.nBlocks << endl;
    cerr << "  _dataResponse.blockSize: " << _dataResponse.blockSize << endl;
    return -1;
  }

  _dataMemBuf.reset();
  int nBytesData = _dataResponse.nBlocks * _dataResponse.blockSize;
  _dataMemBuf.add(_readBuf + _readPos, nBytesData);
  _readPos += nBytesData;

  int nGates = _xpolConf.nGates;
  if (nGates < 1) {
    return 0;
  }

  int nBytesPerGate = nBytesData / _xpolConf.nGates;

  if (_verbose) {
    cerr << "  nGates: " << nGates << endl;
    cerr << "  nBytesData: " << nBytesData << endl;
    cerr << "  nbytesPerGate: " << nBytesPerGate << endl;
  }

  if (nBytesPerGate == 40) {
    _procMode = PROC_MODE_PP;
  } else if (nBytesPerGate == 32) {
    _procMode = PROC_MODE_PP_SUM;
  } else if (nBytesPerGate == 64) {
    _procMode = PROC_MODE_DUAL_PP_SUM;
  } else if (nBytesPerGate == 48) {
    _procMode = PROC_MODE_DUAL_PP_SUM;
  } else if (nBytesPerGate == 1024) {
    _procMode = PROC_MODE_FFT;
  } else {
    _procMode = PROC_MODE_UNKNOWN;
  }
  
  return 0;

}
    
/////////////////////////////////////////////////////////
// Allocate the covariance buffers

void XpolComms::_allocCovarBuffers()
  
{
  
  int nGates = _xpolConf.nGates;
  int nBytesDouble = nGates * sizeof(double);
  int nBytesComplex = nGates * sizeof(RadarComplex_t);
  
  _recvPwrV0 = (double *) _recvPwrV0Buf.reserve(nBytesDouble);
  _recvPwrV1 = (double *) _recvPwrV1Buf.reserve(nBytesDouble);
  _recvPwrV2 = (double *) _recvPwrV2Buf.reserve(nBytesDouble);

  _recvPwrH0 = (double *) _recvPwrH0Buf.reserve(nBytesDouble);
  _recvPwrH1 = (double *) _recvPwrH1Buf.reserve(nBytesDouble);
  _recvPwrH2 = (double *) _recvPwrH2Buf.reserve(nBytesDouble);
  
  _totPwrV = (double *) _totPwrVBuf.reserve(nBytesDouble);
  _totPwrH = (double *) _totPwrHBuf.reserve(nBytesDouble);
  
  _dopPpV0V1 = (RadarComplex_t *) _dopPpV0V1Buf.reserve(nBytesComplex);
  _dopPpV1V2 = (RadarComplex_t *) _dopPpV1V2Buf.reserve(nBytesComplex);
  _dopPpH0H1 = (RadarComplex_t *) _dopPpH0H1Buf.reserve(nBytesComplex);
  _dopPpH1H2 = (RadarComplex_t *) _dopPpH1H2Buf.reserve(nBytesComplex);
  
  _crossCorrVH = (RadarComplex_t *) _crossCorrVHBuf.reserve(nBytesComplex);

}

/////////////////////////////////////////////////////////
// Load the covariance buffers
//
// Marshall the data from the input buffer into arrays
// for each covariance
//
// This action depends on the mode.

void XpolComms::_loadCovarBuffers()
  
{
  
  int nGates = _xpolConf.nGates;
  int nGates2 = nGates * 2;
  float *data = (float *) _dataMemBuf.getPtr();
  
  if (_serverMode == SERVER_MODE_PP) {

    if (_xpolConf.sumPowers) {

      float *totPwrV = data;
      float *totPwrH = totPwrV + nGates;
      float *dopPpV0V1 = totPwrH + nGates;
      float *dopPpH0H1 = dopPpV0V1 + nGates2;
      float *crossCorrVH = dopPpH0H1 + nGates2;
      int jj = 0, kk = 1;
      for (int ii = 0; ii < nGates; ii++, jj += 2, kk += 2) {
        _totPwrV[ii] = totPwrV[ii];
        _totPwrH[ii] = totPwrH[ii];
        _dopPpV0V1[ii].re = dopPpV0V1[jj];
        _dopPpV0V1[ii].im = dopPpV0V1[kk];
        _dopPpH0H1[ii].re = dopPpH0H1[jj];
        _dopPpH0H1[ii].im = dopPpH0H1[kk];
        _crossCorrVH[ii].re = crossCorrVH[jj];
        _crossCorrVH[ii].im = crossCorrVH[kk];
      } // ii

    } else {

      float *recvPwrV0 = data;
      float *recvPwrV1 = recvPwrV0 + nGates;
      float *recvPwrH0 = recvPwrV1 + nGates;
      float *recvPwrH1 = recvPwrH0 + nGates;
      float *dopPpV0V1 = recvPwrH1 + nGates;
      float *dopPpH0H1 = dopPpV0V1 + nGates2;
      float *crossCorrVH = dopPpH0H1 + nGates2;
      int jj = 0, kk = 1;
      for (int ii = 0; ii < nGates; ii++, jj += 2, kk += 2) {
        _recvPwrV0[ii] = recvPwrV0[ii];
        _recvPwrV1[ii] = recvPwrV1[ii];
        _recvPwrH0[ii] = recvPwrH0[ii];
        _recvPwrH1[ii] = recvPwrH1[ii];
        _dopPpV0V1[ii].re = dopPpV0V1[jj];
        _dopPpV0V1[ii].im = dopPpV0V1[kk];
        _dopPpH0H1[ii].re = dopPpH0H1[jj];
        _dopPpH0H1[ii].im = dopPpH0H1[kk];
        _crossCorrVH[ii].re = crossCorrVH[jj];
        _crossCorrVH[ii].im = crossCorrVH[kk];
      } // ii

    } // if (_xpolConf.sumPowers)

  } else if (_serverMode == SERVER_MODE_DUAL_PP) {
    
    if (_xpolConf.sumPowers) {

      float *totPwrV = data;
      float *totPwrH = totPwrV + nGates;
      float *dopPpV0V1 = totPwrH + nGates;
      float *dopPpV1V2 = dopPpV0V1 + nGates2;
      float *dopPpH0H1 = dopPpV1V2 + nGates2;
      float *dopPpH1H2 = dopPpH0H1 + nGates2;
      float *crossCorrVH = dopPpH1H2 + nGates2;
      int jj = 0, kk = 1;
      for (int ii = 0; ii < nGates; ii++, jj += 2, kk += 2) {
        _totPwrV[ii] = totPwrV[ii];
        _totPwrH[ii] = totPwrH[ii];
        _dopPpV0V1[ii].re = dopPpV0V1[jj];
        _dopPpV0V1[ii].im = dopPpV0V1[kk];
        _dopPpV1V2[ii].re = dopPpV1V2[jj];
        _dopPpV1V2[ii].im = dopPpV1V2[kk];
        _dopPpH0H1[ii].re = dopPpH0H1[jj];
        _dopPpH0H1[ii].im = dopPpH0H1[kk];
        _dopPpH1H2[ii].re = dopPpH1H2[jj];
        _dopPpH1H2[ii].im = dopPpH1H2[kk];
        _crossCorrVH[ii].re = crossCorrVH[jj];
        _crossCorrVH[ii].im = crossCorrVH[kk];
      } // ii

    } else {

      float *recvPwrV0 = data;
      float *recvPwrV1 = recvPwrV0 + nGates;
      float *recvPwrV2 = recvPwrV1 + nGates;
      float *recvPwrH0 = recvPwrV2 + nGates;
      float *recvPwrH1 = recvPwrH0 + nGates;
      float *recvPwrH2 = recvPwrH1 + nGates;
      float *dopPpV0V1 = recvPwrH2 + nGates;
      float *dopPpV1V2 = dopPpV0V1 + nGates2;
      float *dopPpH0H1 = dopPpV1V2 + nGates2;
      float *dopPpH1H2 = dopPpH0H1 + nGates2;
      float *crossCorrVH = dopPpH1H2 + nGates2;
      int jj = 0, kk = 1;
      for (int ii = 0; ii < nGates; ii++, jj += 2, kk += 2) {
        _recvPwrV0[ii] = recvPwrV0[ii];
        _recvPwrV1[ii] = recvPwrV1[ii];
        _recvPwrV2[ii] = recvPwrV2[ii];
        _recvPwrH0[ii] = recvPwrH0[ii];
        _recvPwrH1[ii] = recvPwrH1[ii];
        _recvPwrH2[ii] = recvPwrH2[ii];
        _dopPpV0V1[ii].re = dopPpV0V1[jj];
        _dopPpV0V1[ii].im = dopPpV0V1[kk];
        _dopPpV1V2[ii].re = dopPpV1V2[jj];
        _dopPpV1V2[ii].im = dopPpV1V2[kk];
        _dopPpH0H1[ii].re = dopPpH0H1[jj];
        _dopPpH0H1[ii].im = dopPpH0H1[kk];
        _dopPpH1H2[ii].re = dopPpH1H2[jj];
        _dopPpH1H2[ii].im = dopPpH1H2[kk];
        _crossCorrVH[ii].re = crossCorrVH[jj];
        _crossCorrVH[ii].im = crossCorrVH[kk];
      } // ii
      
    } // if (_xpolConf.sumPowers)
    
  } else {
    
    cerr << "ERROR - mode not yet supported: "
         << serverMode2Str(_serverMode) << endl;

  } // if (_serverMode == SERVER_MODE_PP) {

}

/////////////////////////////////////////////////////////
// Load members from read buffer

si32 XpolComms::_getNextSi32()
  
{
  
  if (_readPos + sizeof(si32) > _readMemBuf.getLen()) {
    cerr << "ERROR - XpolComms::_getNextSi32()" << endl;
    cerr << "  read buffer too small: " << _readMemBuf.getLen() << endl;
    cerr << "  _readPos: " << _readPos << endl;
    return -9999;
  }

  si32 val = 0;
  memcpy(&val, _readBuf + _readPos, sizeof(val));
  _readPos += sizeof(si32);
  if (_swap) SWAP_array_32(&val, sizeof(val));
  return val;

}

si64 XpolComms::_getNextSi64()
  
{
  
  if (_readPos + sizeof(si64) > _readMemBuf.getLen()) {
    cerr << "ERROR - XpolComms::_getNextSi64()" << endl;
    cerr << "  read buffer too small: " << _readMemBuf.getLen() << endl;
    cerr << "  _readPos: " << _readPos << endl;
    return -9999;
  }

  si64 val = 0;
  memcpy(&val, _readBuf + _readPos, sizeof(val));
  _readPos += sizeof(si64);
  if (_swap) SWAP_array_64(&val, sizeof(val));
  return val;

}

fl32 XpolComms::_getNextFl32()
  
{
  
  if (_readPos + sizeof(fl32) > _readMemBuf.getLen()) {
    cerr << "ERROR - XpolComms::_getNextFl32()" << endl;
    cerr << "  read buffer too small: " << _readMemBuf.getLen() << endl;
    cerr << "  _readPos: " << _readPos << endl;
    return -9999;
  }

  fl32 val = 0;
  memcpy(&val, _readBuf + _readPos, sizeof(val));
  _readPos += sizeof(fl32);
  if (_swap) SWAP_array_32(&val, sizeof(val));
  return val;

}

fl64 XpolComms::_getNextFl64()
  
{
  
  if (_readPos + sizeof(fl64) > _readMemBuf.getLen()) {
    cerr << "ERROR - XpolComms::_getNextFl64()" << endl;
    cerr << "  read buffer too small: " << _readMemBuf.getLen() << endl;
    cerr << "  _readPos: " << _readPos << endl;
    return -9999;
  }

  fl64 val = 0;
  memcpy(&val, _readBuf + _readPos, sizeof(val));
  _readPos += sizeof(fl64);
  if (_swap) SWAP_array_64(&val, sizeof(val));
  return val;

}

string XpolComms::_getNextString(int len)
  
{

  char *txtbuf = new char[len + 1];
  txtbuf[len] = '\0';
  memcpy(txtbuf, _readBuf + _readPos, len);
  _readPos += len;
  string str(txtbuf);
  delete txtbuf;
  return str;

}

si32 XpolComms::_getSi32(const void *buf, int offsetBytes)
  
{
  
  si32 val = 0;
  memcpy(&val, ((char *) buf) + offsetBytes, sizeof(val));
  if (_swap) SWAP_array_32(&val, sizeof(val));
  return val;

}

fl32 XpolComms::_getFl32(const void *buf, int offsetBytes)
  
{
  
  fl32 val = 0;
  memcpy(&val, ((char *) buf) + offsetBytes, sizeof(val));
  if (_swap) SWAP_array_32(&val, sizeof(val));
  return val;

}

fl64 XpolComms::_getFl64(const void *buf, int offsetBytes)
  
{
  
  fl64 val = 0;
  memcpy(&val, ((char *) buf) + offsetBytes, sizeof(val));
  if (_swap) SWAP_array_32(&val, sizeof(val));
  return val;

}

/////////////////////////////////////////////////////////
// Put members into write buffer

void XpolComms::_putNextSi32(si32 val)
  
{
  if (_swap) SWAP_array_32(&val, sizeof(val));
  _writeMemBuf.add(&val, sizeof(val));
}

void XpolComms::_putNextFl32(fl32 val)
  
{
  if (_swap) SWAP_array_32(&val, sizeof(val));
  _writeMemBuf.add(&val, sizeof(val));
}

void XpolComms::_putNextFl64(fl64 val)
  
{
  if (_swap) SWAP_array_64(&val, sizeof(val));
  _writeMemBuf.add(&val, sizeof(val));
}

void XpolComms::_putNextString(const string &val, int len)
  
{
  char *str = new char[len + 1];
  memset(str, 0, len + 1);
  int ncopy = len;
  if ((int) (val.size() + 1) < len) {
    ncopy = val.size() + 1;
  }
  memcpy(str, val.c_str(), ncopy);
  _writeMemBuf.add(str, len);
}

/////////////////////////////////////////////////////////
// put client data request into write buffer

void XpolComms::_putDataRequest(DataRequest &req)

{

  _putNextSi32(req.productTypeCode);
  _putNextSi32(req.blockOffset);
  _putNextSi32(req.blockStepLength);
  _putNextSi32(req.blockSeenStepLength);
  _putNextSi32(req.maxNumBlocks);
  _putNextSi32(req.matrixExtent1[0]);
  _putNextSi32(req.matrixExtent1[1]);
  _putNextSi32(req.matrixExtent1[2]);
  _putNextSi32(req.matrixExtent1[3]);
  _putNextSi32(req.matrixExtent2[0]);
  _putNextSi32(req.matrixExtent2[1]);
  _putNextSi32(req.matrixExtent2[2]);
  _putNextSi32(req.matrixExtent2[3]);
  _putNextSi32(req.expectedArchiveIndex);
  _putNextSi32(req.formatFlags);

}

////////////////////////////
// allocate the read buffer

void XpolComms::_allocReadBuf(int len)

{

  _readBuf = (char *) _readMemBuf.prepare(len);
  _readPos = 0;

}

////////////////////////////
// allocate the write buffer

void XpolComms::_allocWriteBuf(int len)

{

  _writeBuf = (char *) _writeMemBuf.prepare(len);
  _writePos = 0;

}

////////////////////////////
// reset the write buffer

void XpolComms::_resetWriteBuf()

{

  _writeMemBuf.reset();

}

/////////////////////////////////////////////////////////
// Convert enums to strings

string XpolComms::_commandCode2Str(commandCode_t command)
{
  switch (command) {
    case PING_SERVER: return "PING_SERVER";
    case GET_CONF: return "GET_CONF";
    case SET_CONF: return "SET_CONF";
    case GET_STATUS: return "GET_STATUS";
    case GET_SERVER_INFO: return "GET_SERVER_INFO";
    case GET_DATA: return "GET_DATA";
    case GET_CONF_AND_STATUS: return "GET_CONF_AND_STATUS";
    case LOAD_PACSI_PED_DISABLED: return "LOAD_PACSI_PED_DISABLED";
    case LOAD_PACSI_PED_ENABLED: return "LOAD_PACSI_PED_ENABLED";
    default: return "COMMAND_CODE_UNKNOWN";
  }
}

string XpolComms::_statusCode2Str(statusCode_t status)
{
  switch (status) {
    case STATUS_SRV_ERR: return "STATUS_SRV_ERR";
    case STATUS_OK: return "STATUS_OK";
    case STATUS_CFG_TRANSITION: return "STATUS_CFG_TRANSITION";
    case STATUS_LACK_CONTROL: return "STATUS_LACK_CONTROL";
    case STATUS_UNKNOWN_CMD: return "STATUS_UNKNOWN_CMD";
    case STATUS_UNKNOWN_DATA_TYPE: return "STATUS_UNKNOWN_DATA_TYPE";
    case STATUS_WRONG_DATA_SIZE: return "STATUS_WRONG_DATA_SIZE";
    case STATUS_NO_DATA: return "STATUS_NO_DATA";
    case STATUS_WRONG_ARCHIVE: return "STATUS_WRONG_ARCHIVE";
    case STATUS_INVALID_INDEX: return "STATUS_INVALID_INDEX";
    case STATUS_INVALID_PARAM: return "STATUS_INVALID_PARAM";
    case STATUS_INVALID_TEXT: return "STATUS_INVALID_TEXT";
    case STATUS_NETCMD_BUSY: return "STATUS_NETCMD_BUSY";
    default: return "STATUS_CODE_UNKNOWN";
  }
}

string XpolComms::fieldId2Str(fieldId_t id)
{
  switch (id) {
    case IQ_V: return "IQ_V";
    case IQ_COHERED_CLUTFILT_V: return "IQ_COHERED_CLUTFILT_V";
    case IQ_H: return "IQ_H";
    case IQ_COHERED_CLUTFILT_H: return "IQ_COHERED_CLUTFILT_H";
    case RX_POWER_UNAV_V0: return "RX_POWER_UNAV_V0";
    case RX_POWER_UNAV_V1: return "RX_POWER_UNAV_V1";
    case RX_POWER_UNAV_V2: return "RX_POWER_UNAV_V2";
    case RX_POWER_UNAV_H0: return "RX_POWER_UNAV_H0";
    case RX_POWER_UNAV_H1: return "RX_POWER_UNAV_H1";
    case RX_POWER_UNAV_H2: return "RX_POWER_UNAV_H2";
    case TOT_SUMMED_POWER_UNAV_V: return "TOT_SUMMED_POWER_UNAV_V";
    case TOT_SUMMED_POWER_UNAV_H: return "TOT_SUMMED_POWER_UNAV_H";
    case PULSE_PAIR_UNAV_V0_V1: return "PULSE_PAIR_UNAV_V0_V1";
    case PULSE_PAIR_UNAV_V1_V2: return "PULSE_PAIR_UNAV_V1_V2";
    case PULSE_PAIR_UNAV_H0_H1: return "PULSE_PAIR_UNAV_H0_H1";
    case PULSE_PAIR_UNAV_H1_H2: return "PULSE_PAIR_UNAV_H1_H2";
    case X_CORR_UNAV_V_H: return "X_CORR_UNAV_V_H";
    case POWER_SPEC_UNAV_V0: return "POWER_SPEC_UNAV_V0";
    case POWER_SPEC_UNAV_V1: return "POWER_SPEC_UNAV_V1";
    case POWER_SPEC_UNAV_H0: return "POWER_SPEC_UNAV_H0";
    case POWER_SPEC_UNAV_H1: return "POWER_SPEC_UNAV_H1";
    case X_SPEC_UNAV_V0_H0: return "X_SPEC_UNAV_V0_H0";
    case X_SPEC_UNAV_V1_H1: return "X_SPEC_UNAV_V1_H1";
    case RX_POWER_AV_V0: return "RX_POWER_AV_V0";
    case RX_POWER_AV_V1: return "RX_POWER_AV_V1";
    case RX_POWER_AV_V2: return "RX_POWER_AV_V2";
    case RX_POWER_AV_H0: return "RX_POWER_AV_H0";
    case RX_POWER_AV_H1: return "RX_POWER_AV_H1";
    case RX_POWER_AV_H2: return "RX_POWER_AV_H2";
    case TOT_SUMMED_POWER_AV_V: return "TOT_SUMMED_POWER_AV_V";
    case TOT_SUMMED_POWER_AV_H: return "TOT_SUMMED_POWER_AV_H";
    case PULSE_PAIR_AV_V0_V1: return "PULSE_PAIR_AV_V0_V1";
    case PULSE_PAIR_AV_V1_V2: return "PULSE_PAIR_AV_V1_V2";
    case PULSE_PAIR_AV_H0_H1: return "PULSE_PAIR_AV_H0_H1";
    case PULSE_PAIR_AV_H1_H2: return "PULSE_PAIR_AV_H1_H2";
    case X_CORR_AV_V_H: return "X_CORR_AV_V_H";
    case POWER_SPEC_AV_V0: return "POWER_SPEC_AV_V0";
    case POWER_SPEC_AV_V1: return "POWER_SPEC_AV_V1";
    case POWER_SPEC_AV_H0: return "POWER_SPEC_AV_H0";
    case POWER_SPEC_AV_H1: return "POWER_SPEC_AV_H1";
    case X_SPEC_AV_V0_H0: return "X_SPEC_AV_V0_H0";
    case X_SPEC_AV_V1_H1: return "X_SPEC_AV_V1_H1";
    case IF_PHASOR_ESTIMATE: return "IF_PHASOR_ESTIMATE";
    case FUSED_PRODUCTS_DRX_DATA: return "FUSED_PRODUCTS_DRX_DATA";
    case FUSED_PRODUCTS_PROC_DATA: return "FUSED_PRODUCTS_PROC_DATA";
    default: return "XPOL_ID_UNKNOWN";
  }
}

string XpolComms::scanMode2Str(scanMode_t scan)
{
  switch (scan) {
    case SCAN_MODE_NONE: return "SCAN_MODE_NONE";
    case SCAN_MODE_CUSTOM: return "SCAN_MODE_CUSTOM";
    case SCAN_MODE_SOFT_STOP: return "SCAN_MODE_SOFT_STOP";
    case SCAN_MODE_POINT: return "SCAN_MODE_POINT";
    case SCAN_MODE_SLEW: return "SCAN_MODE_SLEW";
    case SCAN_MODE_PPI: return "SCAN_MODE_PPI";
    case SCAN_MODE_RHI: return "SCAN_MODE_RHI";
    case SCAN_MODE_AZ_RASTER: return "SCAN_MODE_AZ_RASTER";
    case SCAN_MODE_EL_RASTER: return "SCAN_MODE_EL_RASTER";
    case SCAN_MODE_VOLUME: return "SCAN_MODE_VOLUME";
    case SCAN_MODE_PAUSED: return "SCAN_MODE_PAUSED";
    default: return "SCAN_MODE_UNKNOWN";
  }
}

string XpolComms::dataDomain2Str(dataDomain_t domain)
{
  switch (domain) {
    case DATA_DOMAIN_TIME: return "TIME";
    case DATA_DOMAIN_FREQ: return "FREQUENCY";
    default: return "DATA_DOMAIN_UNDEFINED";
  }
}

string XpolComms::dataUnits2Str(dataUnits_t units)
{
  switch (units) {
    case DATA_UNITS_DIG_COUNTS: return "DIG_COUNTS";
    case DATA_UNITS_DIG_COUNTS_SQUARED: return "DIG_COUNTS_SQUARED";
    case DATA_UNITS_VOLTAGE: return "Volts";
    case DATA_UNITS_VOLTAGE_SQUARED: return "VoltsSq";
    case DATA_UNITS_MWATT: return "mWatt";
    case DATA_UNITS_DBM: return "dBm";
    case DATA_UNITS_DBZ: return "dBZ";
    default: return "DATA_UNITS_UNDEFINED";
  }
}

string XpolComms::drxMode2Str(drxMode_t mode)
{
  switch (mode) {
    case DRX_MODE_BURST: return "BURST";
    case DRX_MODE_GATED: return "GATED";
    case DRX_MODE_PRI: return "PRI";
    default: return "DRX_MODE_UNDEFINED";
  }
}

string XpolComms::drxTrig2Str(drxTrig_t trig)
{
  switch (trig) {
    case DRX_TRIG_INTERNAL: return "INTERNAL";
    case DRX_TRIG_EXTERNAL: return "EXTERNAL";
    default: return "DRX_TRIG_UNDEFINED";
  }
}

string XpolComms::drxState2Str(drxState_t state)
{
  switch (state) {
    case DRX_STATE_IDLE: return "IDLE";
    case DRX_STATE_RUN: return "RUN";
    default: return "DRX_STATE_UNDEFINED";
  }
}

string XpolComms::drxSource2Str(drxSource_t source)
{
  switch (source) {
    case DRX_SOURCE_RAW_A2D: return "RAW_A2D";
    case DRX_SOURCE_RAMP: return "RAMP";
    case DRX_SOURCE_DIGITAL_TUNER: return "DIGITAL_TUNER";
    default: return "DRX_SOURCE_UNDEFINED";
  }
}

string XpolComms::procMode2Str(procMode_t mode)
{
  switch (mode) {
    case PROC_MODE_PP: return "PROC_MODE_PP";
    case PROC_MODE_PP_SUM: return "PROC_MODE_PP_SUM";
    case PROC_MODE_DUAL_PP: return "PROC_MODE_DUAL_PP";
    case PROC_MODE_DUAL_PP_SUM: return "PROC_MODE_DUAL_PP_SUM";
    case PROC_MODE_FFT: return "PROC_MODE_FFT";
    case PROC_MODE_UNKNOWN: return "PROC_MODE_UNKNOWN";
    default: return "PROC_MODE_UNDEFINED";
  }
}

string XpolComms::serverMode2Str(serverMode_t mode)
{
  switch (mode) {
    case SERVER_MODE_PP: return "SERVER_MODE_PP";
    case SERVER_MODE_DUAL_PP: return "SERVER_MODE_DUAL_PP";
    case SERVER_MODE_FFT: return "SERVER_MODE_FFT";
    case SERVER_MODE_FFT2: return "SERVER_MODE_FFT2";
    case SERVER_MODE_FFT2I: return "SERVER_MODE_FFT2I";
    case SERVER_MODE_UNKNOWN: return "SERVER_MODE_UNKNOWN";
    default: return "SERVER_MODE_UNDEFINED";
  }
}

/////////////////////////////////////////////////////////
// Print status

void XpolComms::printStatus(const XpolStatus &status,
                            ostream &out)
  
{

  out << "==========================================" << endl;
  out << "XPOL STATUS:" << endl;
  out << "  unixTime: " << DateTime::strm(status.unixTimeSecs) << endl;
  out << "  microSecs: " << status.timeMicroSecs << endl;
  out << "  radarTemp[0]: " << status.radarTemps[0] << endl;
  out << "  radarTemp[1]: " << status.radarTemps[1] << endl;
  out << "  radarTemp[2]: " << status.radarTemps[2] << endl;
  out << "  radarTemp[3]: " << status.radarTemps[3] << endl;
  out << "  inclinometerRoll: " << status.inclinometerRoll << endl;
  out << "  inclinometerPitch: " << status.inclinometerPitch << endl;
  out << "  fuel: " << status.fuel << endl;
  out << "  cpuTempC: " << status.cpuTempC << endl;
  out << "  scanMode: " << scanMode2Str(status.scanMode) << endl;
  out << "  txPowerMw: " << status.txPowerMw << endl;
  out << "==========================================" << endl;

}

void XpolComms::printStatus(ostream &out)

{
  printStatus(_xpolStatus, out);
}

/////////////////////////////////////////////////////////
// Print conf

void XpolComms::printConf(const XpolConf &conf,
                          ostream &out)
  
{

  out << "==========================================" << endl;
  out << "XPOL CONF:" << endl;
  out << "  siteInfo: " << conf.siteInfo << endl;
  out << "  azOffset: " << conf.azOffset << endl;
  out << "  spare1: " << conf.spare1 << endl;
  out << "  clutFilterWidthMPerSec: " << conf.clutFilterWidthMPerSec << endl;
  out << "  clutAvInterval: " << conf.clutAvInterval << endl;
  out << "  productsPerSec: " << conf.productsPerSec << endl;
  out << "  fftLength: " << conf.fftLength << endl;
  out << "  fftWindowType: " << conf.fftWindowType << endl;
  out << "  reserved1: " << conf.reserved1 << endl;
  out << "  autoFileRollNRecords: " << conf.autoFileRollNRecords << endl;
  out << "  autoFileRollNScans: " << conf.autoFileRollNScans << endl;
  out << "  autoFileRollFileSizeMb: " << conf.autoFileRollFileSizeMb << endl;
  out << "  autoFileRollElapsedTimeSec: " << conf.autoFileRollElapsedTimeSec << endl;
  out << "  autoFileRollTypeBitFlags: " << conf.autoFileRollTypeBitFlags << endl;
  out << "  reserved2: " << conf.reserved2 << endl;
  out << "  filterBandwidthMhz: " << conf.filterBandwidthMhz << endl;
  out << "  freqTrackAdjThreshPerc: " << conf.freqTrackAdjThreshPerc << endl;
  out << "  freqTrackMode: " << conf.freqTrackMode << endl;
  out << "  reserved3: " << conf.reserved3 << endl;
  out << "  groupIntervalUsec: " << conf.groupIntervalUsec << endl;
  out << "  hDbzPerDbmOffset: " << conf.hDbzPerDbmOffset << endl;
  out << "  hNoisePowerDbm: " << conf.hNoisePowerDbm << endl;
  out << "  loFreqErrorMhz: " << conf.loFreqErrorMhz << endl;
  out << "  loFreqMhz: " << conf.loFreqMhz << endl;
  out << "  maxSampledRangeM: " << conf.maxSampledRangeM << endl;
  out << "  nGates: " << conf.nGates << endl;
  out << "  nGroupPulses: " << conf.nGroupPulses << endl;
  out << "  postDecimationLevel: " << conf.postDecimationLevel << endl;
  out << "  postAveragingInterval: " << conf.postAveragingInterval << endl;
  out << "  priUsecUnit1: " << conf.priUsecUnit1 << endl;
  out << "  priUsecUnit2: " << conf.priUsecUnit2 << endl;
  out << "  priUsecUnitTotal: " << conf.priUsecUnitTotal << endl;
  out << "  primOnBoardDecLevel: " << conf.primOnBoardDecLevel << endl;
  out << "  pulseLenM: " << conf.pulseLenM << endl;
  out << "  reserved4: " << conf.reserved4 << endl;
  out << "  gateSpacingM: " << conf.gateSpacingM << endl;
  out << "  reserved5: " << conf.reserved5 << endl;
  out << "  rangeResMPerGate: " << conf.rangeResMPerGate << endl;
  out << "  recordMoments: " << conf.recordMoments << endl;
  out << "  recordRaw: " << conf.recordRaw << endl;
  out << "  recordingEnabled: " << conf.recordingEnabled << endl;
  out << "  serverMode: " << conf.serverMode << endl;
  out << "  reserved6: " << conf.reserved6 << endl;
  out << "  serverState: " << conf.serverState << endl;
  out << "  reserved7: " << conf.reserved7 << endl;
  out << "  softwareDecLevel: " << conf.softwareDecLevel << endl;
  out << "  sumPowers: " << conf.sumPowers << endl;
  out << "  totAveragingInterval: " << conf.totAveragingInterval << endl;
  out << "  txDelayNanoSec: " << conf.txDelayNanoSec << endl;
  out << "  txDelayPulseWidthMult: " << conf.txDelayPulseWidthMult << endl;
  out << "  txPulseCenterNanoSec: " << conf.txPulseCenterNanoSec << endl;
  out << "  txPulseCenterOffsetNanoSec: " << conf.txPulseCenterOffsetNanoSec << endl;
  out << "  txSampleSwitchDelayNanoSec: " << conf.txSampleSwitchDelayNanoSec << endl;
  out << "  txSampleSwitchHoldoffNanoSec: " << conf.txSampleSwitchHoldoffNanoSec << endl;
  out << "  useClutFilter: " << conf.useClutFilter << endl;
  out << "  vDbzPerDbmOffset: " << conf.vDbzPerDbmOffset << endl;
  out << "  vNoisePowerDbm: " << conf.vNoisePowerDbm << endl;
  out << "  zeroRangeGateIndex: " << conf.zeroRangeGateIndex << endl;
  out << "==========================================" << endl;

}

void XpolComms::printConf(ostream &out)

{
  printConf(_xpolConf, out);
}

/////////////////////////////////////////////////////////
// Print drx spec

void XpolComms::printDrxSpec(const DrxSpec &spec,
                             ostream &out)
  
{

  out << "    DRX SPEC:" << endl;
  out << "      numInputChannels: " << spec.numInputChannels << endl;
  out << "      minSampleClockFreqHz: " << spec.minSampleClockFreqHz << endl;
  out << "      maxSampleClockFreqHz: " << spec.maxSampleClockFreqHz << endl;
  out << "      minCenterFreqHz: " << spec.minCenterFreqHz << endl;
  out << "      maxCenterFreqHz: " << spec.maxCenterFreqHz << endl;
  out << "      minNumDmaDescriptors: " << spec.minNumDmaDescriptors << endl;
  out << "      maxNumDmaDescriptors: " << spec.maxNumDmaDescriptors << endl;
  out << "      minNumDmaDescripPackets: " << spec.minNumDmaDescripPackets << endl;
  out << "      maxNumDmaDescripPackets: " << spec.maxNumDmaDescripPackets << endl;
  out << "      minDmaPacketSizeBytes: " << spec.minDmaPacketSizeBytes << endl;
  out << "      maxDmaPacketSizeBytes: " << spec.maxDmaPacketSizeBytes << endl;
  out << "      dnaPacketSizeGranBytes: " << spec.dnaPacketSizeGranBytes << endl;
  out << "      minBurstSizeBytes: " << spec.minBurstSizeBytes << endl;
  out << "      maxBurstSizeBytes: " << spec.maxBurstSizeBytes << endl;
  out << "      burstSizeGranBytes: " << spec.burstSizeGranBytes << endl;
  out << "      minNumBurstsPerPciIntr: " << spec.minNumBurstsPerPciIntr << endl;
  out << "      maxNumBurstsPerPciIntr: " << spec.maxNumBurstsPerPciIntr << endl;
  out << "      minSkipCountSamples: " << spec.minSkipCountSamples << endl;
  out << "      maxSkipCountSamples: " << spec.maxSkipCountSamples << endl;
  out << "      minCicDecLevel: " << spec.minCicDecLevel << endl;
  out << "      maxCicDecLevel: " << spec.maxCicDecLevel << endl;
  out << "      minFirDecLevel: " << spec.minFirDecLevel << endl;
  out << "      maxFirDecLevel: " << spec.maxFirDecLevel << endl;
  out << "      minPostDecLevel: " << spec.minPostDecLevel << endl;
  out << "      maxPostDecLevel: " << spec.maxPostDecLevel << endl;
  out << "      maxFirFilterLen: " << spec.maxFirFilterLen << endl;
  out << "      minFirGain: " << spec.minFirGain << endl;
  out << "      maxFirGain: " << spec.maxFirGain << endl;
  out << "      fullScaleFirGainLevel: " << spec.fullScaleFirGainLevel << endl;
  out << "      minAnalogVoltageInputV: " << spec.minAnalogVoltageInputV << endl;
  out << "      maxAnalogVoltageInputV: " << spec.maxAnalogVoltageInputV << endl;
  out << "      analogImpedanceOhms: " << spec.analogImpedanceOhms << endl;
  out << "      minDigitizedCountValue: " << spec.minDigitizedCountValue << endl;
  out << "      maxDigitizedCountValue: " << spec.maxDigitizedCountValue << endl;
  out << "      adResBitsPerSample: " << spec.adResBitsPerSample << endl;
  out << "      digitizedCountSizeBits: " << spec.digitizedCountSizeBits << endl;

}

/////////////////////////////////////////////////////////
// Print drx info

void XpolComms::printDrxInfo(const DrxInfo &info,
                             ostream &out)
  
{

  out << "------------------------------------------" << endl;
  out << "  DRX INFO:" << endl;
  out << "    manufacturerCode: " << info.manufacturerCode << endl;
  out << "    manufacturerName: " << info.manufacturerName << endl;
  out << "    modelCode: " << info.modelCode << endl;
  out << "    modelName: " << info.modelName << endl;
  printDrxSpec(info.drxSpec, out);
  out << "------------------------------------------" << endl;

}

/////////////////////////////////////////////////////////
// Print drx conf

void XpolComms::printDrxConf(const DrxConf &conf,
                             ostream &out)
  
{

  out << "------------------------------------------" << endl;
  out << "  DRX CONF:" << endl;
  out << "    mode: " << drxMode2Str(conf.mode) << endl;
  out << "    trig: " << drxTrig2Str(conf.trig) << endl;
  out << "    pciBusFreqMhz: " << conf.pciBusFreqMhz << endl;
  out << "    a2dSampleFreqHz: " << conf.a2dSampleFreqHz << endl;
  out << "    numDmaDescriptors: " << conf.numDmaDescriptors << endl;
  out << "    numDmaPacketsPerDesc: " << conf.numDmaPacketsPerDesc << endl;
  out << "    dmaPacketSize: " << conf.dmaPacketSize << endl;
  out << "    blockSize: " << conf.blockSize << endl;
  out << "    reserved1: " << conf.reserved1 << endl;
  out << "    numBlocksPerPciIntr: " << conf.numBlocksPerPciIntr << endl;
  out << "    reserved2: " << conf.reserved2 << endl;
  out << "    reserved3: " << conf.reserved3 << endl;
  out << "    state[0]: " << drxState2Str(conf.state[0]) << endl;
  out << "    state[1]: " << drxState2Str(conf.state[1]) << endl;
  out << "    source[0]: " << drxSource2Str(conf.source[0]) << endl;
  out << "    source[1]: " << drxSource2Str(conf.source[1]) << endl;
  out << "    skipCount[0]: " << conf.skipCount[0] << endl;
  out << "    skipCount[1]: " << conf.skipCount[1] << endl;
  out << "    cicDecimation[0]: " << conf.cicDecimation[0] << endl;
  out << "    cicDecimation[1]: " << conf.cicDecimation[1] << endl;
  out << "    firDecimation[0]: " << conf.firDecimation[0] << endl;
  out << "    firDecimation[1]: " << conf.firDecimation[1] << endl;
  out << "    postDecimation[0]: " << conf.postDecimation[0] << endl;
  out << "    postDecimation[1]: " << conf.postDecimation[1] << endl;
  out << "    ncoFreqHz[0]: " << conf.ncoFreqHz[0] << endl;
  out << "    ncoFreqHz[1]: " << conf.ncoFreqHz[1] << endl;
  out << "------------------------------------------" << endl;

}

/////////////////////////////////////////////////////////
// Print data product info

void XpolComms::printDataProdInfo(const DataProdInfo &info, ostream &out)

{

  out << "------------------------------------------" << endl;
  out << "  DATA PRODUCT INFO:" << endl;
  out << "    typeCode: " << info.typeCode << endl;
  out << "    shortName: " << info.shortName << endl;
  out << "    longName: " << info.longName << endl;
  out << "    drxChannel: " << info.drxChannel << endl;
  out << "    posDeviceIndex: " << info.posDeviceIndex << endl;
  out << "    gpsDeviceIndex: " << info.gpsDeviceIndex << endl;
  out << "    dataDomain: " << dataDomain2Str(info.dataDomain) << endl;
  out << "    dataUnits: " << dataUnits2Str(info.dataUnits) << endl;
  out << "    nInterleavedTracks: " << info.nInterleavedTracks << endl;
  out << "    matrixDim: " << info.matrixDim << endl;
  out << "------------------------------------------" << endl;

}

/////////////////////////////////////////////////////////
// Print server info

void XpolComms::printServerInfo(const ServerInfo &info,
                                ostream &out)
  
{

  out << "==========================================" << endl;
  out << "SERVER INFO:" << endl;
  out << "  projectName: " << info.projectName << endl;
  printDrxInfo(info.drxInfo, out);
  out << "  productCount: " << info.productCount << endl;
  for (size_t ii = 0; ii < info.dataProdInfo.size(); ii++) {
    out << "  PRODUCT number: " << ii << endl;
    printDataProdInfo(info.dataProdInfo[ii], out);
  }
  out << "==========================================" << endl;

}

void XpolComms::printServerInfo(ostream &out)

{
  printServerInfo(_serverInfo, out);
}

/////////////////////////////////////////////////////////
// Print pedestal info

void XpolComms::printPedestal(const Pedestal &ped, ostream &out)
  
{

  out << "==========================================" << endl;
  out << "PEDESTAL:" << endl;
  out << "  azPosDeg: " << ped.azPosDeg << endl;
  out << "  elPosDeg: " << ped.elPosDeg << endl;
  out << "  azVelDegPerSec: " << ped.azVelDegPerSec << endl;
  out << "  elVelDegPerSec: " << ped.elVelDegPerSec << endl;
  out << "  azCurrentAmps: " << ped.azCurrentAmps << endl;
  out << "  elCurrentAmps: " << ped.elCurrentAmps << endl;
  out << "==========================================" << endl;


}

/////////////////////////////////////////////////////////
// Print georef info

void XpolComms::printGeoref(const Georef &geo, ostream &out)
  
{

  out << "==========================================" << endl;
  out << "GEOREF:" << endl;
  out << "  latDeg: " << geo.latDeg << endl;
  out << "  lonDeg: " << geo.lonDeg << endl;
  out << "  altMeters: " << geo.altMeters << endl;
  out << "  headingMagnetic: " << string(geo.headingMagnetic? "Y":"N") << endl;
  out << "  headingDeg: " << geo.headingDeg << endl;
  out << "  speedKmh: " << geo.speedKmh << endl;
  out << "==========================================" << endl;

}

/////////////////////////////////////////////////////////
// Print data request

void XpolComms::printDataRequest(const DataRequest &req, ostream &out)
  
{

  out << "==========================================" << endl;
  out << "DATA REQUEST:" << endl;
  out << "  productTypeCode: " << fieldId2Str((fieldId_t) req.productTypeCode) << endl;
  out << "  blockOffset: " << req.blockOffset << endl;
  out << "  blockStepLength: " << req.blockStepLength << endl;
  out << "  blockSeenStepLength: " << req.blockSeenStepLength << endl;
  out << "  maxNumBlocks: " << req.maxNumBlocks << endl;
  out << "  matrixExtent1[0]: " << req.matrixExtent1[0] << endl;
  out << "  matrixExtent1[1]: " << req.matrixExtent1[1] << endl;
  out << "  matrixExtent1[2]: " << req.matrixExtent1[2] << endl;
  out << "  matrixExtent1[3]: " << req.matrixExtent1[3] << endl;
  out << "  matrixExtent2[0]: " << req.matrixExtent2[0] << endl;
  out << "  matrixExtent2[1]: " << req.matrixExtent2[1] << endl;
  out << "  matrixExtent2[2]: " << req.matrixExtent2[2] << endl;
  out << "  matrixExtent2[3]: " << req.matrixExtent2[3] << endl;
  out << "  expectedArchiveIndex: " << req.expectedArchiveIndex << endl;
  out << "  formatFlags: " << req.formatFlags << endl;
  out << "==========================================" << endl;

}

/////////////////////////////////////////////////////////
// Print data response

void XpolComms::printDataResponse(const DataResponse &resp, ostream &out)
  
{

  out << "==========================================" << endl;
  out << "DATA RESPONSE:" << endl;
  out << "  productTypeCode: " << resp.productTypeCode << endl;
  out << "  unixTimeSecs: " << DateTime::strm(resp.unixTimeSecs) << endl;
  out << "  timeNanoSecs: " << resp.timeNanoSecs << endl;
  printDrxConf(resp.drxConf, out);
  out << "  drxFirFilterGain: " << resp.drxFirFilterGain << endl;
  out << "  averagingIntervalLen: " << resp.averagingIntervalLen << endl;
  out << "  archiveIndex: " << resp.archiveIndex << endl;
  out << "  blockIndex: " << resp.blockIndex << endl;
  out << "  nBlocks: " << resp.nBlocks << endl;
  out << "  blockSize: " << resp.blockSize << endl;
  out << "  numBlockDim: " << resp.numBlockDim << endl;
  out << "  blockDims[0]: " << resp.blockDims[0] << endl;
  out << "  blockDims[1]: " << resp.blockDims[1] << endl;
  out << "  blockDims[2]: " << resp.blockDims[2] << endl;
  out << "  blockDims[3]: " << resp.blockDims[3] << endl;
  printPedestal(resp.pedestal, out);
  printGeoref(resp.georef, out);
  out << "==========================================" << endl;

}

void XpolComms::printDataResponse(ostream &out)

{
  printDataResponse(_dataResponse, out);
}
  
