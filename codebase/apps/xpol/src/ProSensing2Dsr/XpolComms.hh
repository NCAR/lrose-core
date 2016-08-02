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
////////////////////////////////////////////////////////////////////
// ProSensing Communication XpolComms
//
// Mike Dixon, EOL, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// April 2012
//
////////////////////////////////////////////////////////////////////

#ifndef XPOL_COMMS_HH
#define XPOL_COMMS_HH

#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/RadarComplex.hh>
using namespace std;

class XpolComms {

public:

  // constructor

  XpolComms();
  
  // destructor

  virtual ~XpolComms();

  // close any open connections to server

  void close();

  // enumerated types

  typedef enum {
    IQ_V = 0,
    IQ_COHERED_CLUTFILT_V = 1,
    IQ_H = 2,
    IQ_COHERED_CLUTFILT_H = 3,
    RX_POWER_UNAV_V0 = 4,
    RX_POWER_UNAV_V1 = 5,
    RX_POWER_UNAV_V2 = 6,
    RX_POWER_UNAV_H0 = 7,
    RX_POWER_UNAV_H1 = 8,
    RX_POWER_UNAV_H2 = 9,
    TOT_SUMMED_POWER_UNAV_V = 10,
    TOT_SUMMED_POWER_UNAV_H = 11,
    PULSE_PAIR_UNAV_V0_V1 = 12,
    PULSE_PAIR_UNAV_V1_V2 = 13,
    PULSE_PAIR_UNAV_H0_H1 = 14,
    PULSE_PAIR_UNAV_H1_H2 = 15,
    X_CORR_UNAV_V_H = 16,
    POWER_SPEC_UNAV_V0 = 17,
    POWER_SPEC_UNAV_V1 = 18,
    POWER_SPEC_UNAV_H0 = 19,
    POWER_SPEC_UNAV_H1 = 20,
    X_SPEC_UNAV_V0_H0 = 21,
    X_SPEC_UNAV_V1_H1 = 22,
    RX_POWER_AV_V0 = 23,
    RX_POWER_AV_V1 = 24,
    RX_POWER_AV_V2 = 25,
    RX_POWER_AV_H0 = 26,
    RX_POWER_AV_H1 = 27,
    RX_POWER_AV_H2 = 28,
    TOT_SUMMED_POWER_AV_V = 29,
    TOT_SUMMED_POWER_AV_H = 30,
    PULSE_PAIR_AV_V0_V1 = 31,
    PULSE_PAIR_AV_V1_V2 = 32,
    PULSE_PAIR_AV_H0_H1 = 33,
    PULSE_PAIR_AV_H1_H2 = 34,
    X_CORR_AV_V_H = 35,
    POWER_SPEC_AV_V0 = 36,
    POWER_SPEC_AV_V1 = 37,
    POWER_SPEC_AV_H0 = 38,
    POWER_SPEC_AV_H1 = 39,
    X_SPEC_AV_V0_H0 = 40,
    X_SPEC_AV_V1_H1 = 41,
    IF_PHASOR_ESTIMATE = 42,
    FUSED_PRODUCTS_DRX_DATA = 43,
    FUSED_PRODUCTS_PROC_DATA = 44
  } fieldId_t;

  typedef enum {
    SCAN_MODE_NONE = 0,
    SCAN_MODE_CUSTOM = 1,
    SCAN_MODE_SOFT_STOP = 2,
    SCAN_MODE_POINT = 3,
    SCAN_MODE_SLEW = 4,
    SCAN_MODE_PPI = 5,
    SCAN_MODE_RHI = 6,
    SCAN_MODE_AZ_RASTER = 7,
    SCAN_MODE_EL_RASTER = 8,
    SCAN_MODE_VOLUME = 9,
    SCAN_MODE_PAUSED = 10
  } scanMode_t;

  typedef enum {
    DATA_DOMAIN_UNDEFINED = 0,
    DATA_DOMAIN_TIME = 1,
    DATA_DOMAIN_FREQ = 2
  } dataDomain_t;

  typedef enum {
    DATA_UNITS_UNDEFINED = 0,
    DATA_UNITS_DIG_COUNTS = 1,
    DATA_UNITS_DIG_COUNTS_SQUARED = 2,
    DATA_UNITS_VOLTAGE = 3,
    DATA_UNITS_VOLTAGE_SQUARED = 4,
    DATA_UNITS_MWATT = 5,
    DATA_UNITS_DBM = 6,
    DATA_UNITS_DBZ = 7
  } dataUnits_t;

  typedef enum {
    DRX_MODE_BURST = 0,
    DRX_MODE_GATED = 1,
    DRX_MODE_PRI = 2
  } drxMode_t;

  typedef enum {
    DRX_TRIG_INTERNAL = 0,
    DRX_TRIG_EXTERNAL = 1
  } drxTrig_t;

  typedef enum {
    DRX_STATE_IDLE = 0,
    DRX_STATE_RUN = 1
  } drxState_t;

  typedef enum {
    DRX_SOURCE_RAW_A2D = 0,
    DRX_SOURCE_RAMP = 1,
    DRX_SOURCE_DIGITAL_TUNER = 2
  } drxSource_t;

  typedef enum {
    PROC_MODE_PP = 0,
    PROC_MODE_PP_SUM = 1,
    PROC_MODE_DUAL_PP = 2,
    PROC_MODE_DUAL_PP_SUM = 3,
    PROC_MODE_FFT = 4,
    PROC_MODE_UNKNOWN = 5
  } procMode_t;

  typedef enum {
    SERVER_MODE_PP = 0,
    SERVER_MODE_DUAL_PP = 1,
    SERVER_MODE_FFT = 2,
    SERVER_MODE_FFT2 = 3,
    SERVER_MODE_FFT2I = 4,
    SERVER_MODE_UNKNOWN = 5
  } serverMode_t;

  // data classes

  class XpolStatus {
  public:
    time_t unixTimeSecs;
    int timeMicroSecs;
    int radarTemps[4];
    int inclinometerRoll;
    int inclinometerPitch;
    int fuel;
    double cpuTempC;
    scanMode_t scanMode;
    double txPowerMw;
  };

  class XpolConf {
  public:
    string siteInfo;
    double azOffset;
    int spare1;
    double clutFilterWidthMPerSec;
    int clutAvInterval;
    double productsPerSec;
    int fftLength;
    int fftWindowType;
    int reserved1;
    int autoFileRollNRecords;
    int autoFileRollNScans;
    int autoFileRollFileSizeMb;
    int autoFileRollElapsedTimeSec;
    int autoFileRollTypeBitFlags;
    int reserved2;
    double filterBandwidthMhz;
    double freqTrackAdjThreshPerc;
    int freqTrackMode;
    int reserved3;
    int groupIntervalUsec;
    double hDbzPerDbmOffset;
    double hNoisePowerDbm;
    double loFreqErrorMhz;
    double loFreqMhz;
    double maxSampledRangeM;
    int nGates;
    int nGroupPulses;
    int postDecimationLevel;
    int postAveragingInterval;
    int priUsecUnit1;
    int priUsecUnit2;
    int priUsecUnitTotal;
    int primOnBoardDecLevel;
    double pulseLenM;
    int reserved4;
    double gateSpacingM;
    int reserved5;
    double rangeResMPerGate;
    int recordMoments;
    int recordRaw;
    int recordingEnabled;
    int serverMode;
    int reserved6;
    int serverState;
    int reserved7;
    int softwareDecLevel;
    int sumPowers;
    int totAveragingInterval;
    int txDelayNanoSec;
    int txDelayPulseWidthMult;
    int txPulseCenterNanoSec;
    int txPulseCenterOffsetNanoSec;
    int txSampleSwitchDelayNanoSec;
    int txSampleSwitchHoldoffNanoSec;
    int useClutFilter;
    double vDbzPerDbmOffset;
    double vNoisePowerDbm;
    double zeroRangeGateIndex;
  };

  class DrxSpec {
  public:
    int numInputChannels;
    int minSampleClockFreqHz;
    int maxSampleClockFreqHz;
    int minCenterFreqHz;
    int maxCenterFreqHz;
    int minNumDmaDescriptors;
    int maxNumDmaDescriptors;
    int minNumDmaDescripPackets;
    int maxNumDmaDescripPackets;
    int minDmaPacketSizeBytes;
    int maxDmaPacketSizeBytes;
    int dnaPacketSizeGranBytes;
    int minBurstSizeBytes;
    int maxBurstSizeBytes;
    int burstSizeGranBytes;
    double minNumBurstsPerPciIntr;
    double maxNumBurstsPerPciIntr;
    int minSkipCountSamples;
    int maxSkipCountSamples;
    int minCicDecLevel;
    int maxCicDecLevel;
    int minFirDecLevel;
    int maxFirDecLevel;
    int minPostDecLevel;
    int maxPostDecLevel;
    int maxFirFilterLen;
    double minFirGain;
    double maxFirGain;
    double fullScaleFirGainLevel;
    double minAnalogVoltageInputV;
    double maxAnalogVoltageInputV;
    double analogImpedanceOhms;
    int minDigitizedCountValue;
    int maxDigitizedCountValue;
    int adResBitsPerSample;
    int digitizedCountSizeBits;
  };

  class DrxInfo {
  public:
    int manufacturerCode;
    string manufacturerName;
    int modelCode;
    string modelName;
    DrxSpec drxSpec;
  };
  
  class DrxConf {
  public:
    drxMode_t mode;
    drxTrig_t trig;
    int pciBusFreqMhz;
    int a2dSampleFreqHz;
    int numDmaDescriptors;
    int numDmaPacketsPerDesc;
    int dmaPacketSize;
    int blockSize;
    int reserved1;
    double numBlocksPerPciIntr;
    int reserved2;
    int reserved3;
    drxState_t state[2];
    drxSource_t source[2];
    int skipCount[2];
    int cicDecimation[2];
    int firDecimation[2];
    int postDecimation[2];
    int ncoFreqHz[2];
  };
  
  class DataProdInfo {
  public:
    int typeCode;
    string shortName;
    string longName;
    int drxChannel;
    int posDeviceIndex;
    int gpsDeviceIndex;
    dataDomain_t dataDomain;
    dataUnits_t dataUnits;
    int nInterleavedTracks;
    int matrixDim;
  };

  class ServerInfo {
  public:
    string projectName;
    DrxInfo drxInfo;
    int productCount;
    vector<DataProdInfo> dataProdInfo;
  };

  class Pedestal {
  public:
    double azPosDeg;
    double elPosDeg;
    double azVelDegPerSec;
    double elVelDegPerSec;
    double azCurrentAmps;
    double elCurrentAmps;
  };

  class Georef {
  public:
    double latDeg;
    double lonDeg;
    double altMeters;
    bool headingMagnetic;
    double headingDeg;
    double speedKmh;
  };

  class DataRequest {
  public:
    int productTypeCode;
    int blockOffset;
    int blockStepLength;
    int blockSeenStepLength;
    int maxNumBlocks;
    int matrixExtent1[4];
    int matrixExtent2[4];
    int expectedArchiveIndex;
    int formatFlags;
  };

  class DataResponse {
  public:
    int productTypeCode;
    time_t unixTimeSecs;
    int timeNanoSecs;
    DrxConf drxConf;
    double drxFirFilterGain;
    int averagingIntervalLen;
    int archiveIndex;
    long long blockIndex;
    int nBlocks;
    int blockSize;
    int numBlockDim;
    int blockDims[4];
    Pedestal pedestal;
    Georef georef;
  };

  // set methods

  void setDebug(bool state = true) { _debug = state; }
  void setVerbose(bool state = true) { _verbose = state; }

  void setServerHost(const string &host) { _serverHost = host; }
  void setServerPort(int port) { _serverPort = port; }
  
  void setOverrideAzOffset(double azOffsetDeg) {
    _overrideAzOffset = true;
    _azOffsetDeg = azOffsetDeg;
  }

  // read data from server

  int pingServer();
  int readConf(bool withStatus = false);
  int readStatus();
  int readServerInfo();
  int readData(fieldId_t fieldId);
  int readMetaData();

  // get ray properties

  double getRayDTimeSecs() const {
    return _dataResponse.unixTimeSecs + _dataResponse.timeNanoSecs / 1.0e9;
  }
  time_t getRayTimeSecs() const { return _dataResponse.unixTimeSecs; }
  int getRayNanoSecs() const { return _dataResponse.timeNanoSecs; }
  double getRayAzDeg() const {
    double az = _dataResponse.pedestal.azPosDeg + _xpolConf.azOffset;
    if (az > 360) {
      az -= 360.0;
    } else if (az < 0) {
      az += 360.0;
    }
    return az;
  }
  double getRayElDeg() const { return _dataResponse.pedestal.elPosDeg; }
  int getNGates() const { return _xpolConf.nGates; }

  // get results

  const XpolStatus &getStatus() const { return _xpolStatus; }
  const XpolConf &getConf() const { return _xpolConf; }
  const ServerInfo &getServerInfo() const { return _serverInfo; }
  const DataResponse &getDataResponse() const { return _dataResponse; }
  const void *getDataPtr() const { return _dataMemBuf.getPtr(); }
  int getDataNBytes() const { return _dataMemBuf.getLen(); }

  procMode_t getProcMode() const { return _procMode; }
  serverMode_t getServerMode() const { return _serverMode; }
  string getServerModeStr() const { return serverMode2Str(_serverMode); }

  bool getMetaDataHasChanged() const { return _metaDataHasChanged; }

  const double *getRecvPwrV0() const { return _recvPwrV0; }
  const double *getRecvPwrV1() const { return _recvPwrV1; }
  const double *getRecvPwrV2() const { return _recvPwrV2; }
  const double *getRecvPwrH0() const { return _recvPwrH0; }
  const double *getRecvPwrH1() const { return _recvPwrH1; }
  const double *getRecvPwrH2() const { return _recvPwrH2; }
  const double *getTotPwrV() const { return _totPwrV; }
  const double *getTotPwrH() const { return _totPwrH; }

  const RadarComplex_t *getDopPpV0V1() const { return _dopPpV0V1; }
  const RadarComplex_t *getDopPpV1V2() const { return _dopPpV1V2; }
  const RadarComplex_t *getDopPpH0H1() const { return _dopPpH0H1; }
  const RadarComplex_t *getDopPpH1H2() const { return _dopPpH1H2; }
  const RadarComplex_t *getCrossCorrVH() const { return _crossCorrVH; }
  
  // converting enums to strings
  
  static string fieldId2Str(fieldId_t id);
  static string scanMode2Str(scanMode_t scan);
  static string dataDomain2Str(dataDomain_t domain);
  static string dataUnits2Str(dataUnits_t units);
  static string drxMode2Str(drxMode_t mode);
  static string drxTrig2Str(drxTrig_t trig);
  static string drxState2Str(drxState_t state);
  static string drxSource2Str(drxSource_t source);
  static string procMode2Str(procMode_t mode);
  static string serverMode2Str(serverMode_t mode);
  
  // printing data classes

  static void printStatus(const XpolStatus &status, ostream &out);
  void printStatus(ostream &out);
  
  static void printConf(const XpolConf &conf, ostream &out);
  void printConf(ostream &out);

  static void printDrxSpec(const DrxSpec &spec, ostream &out);
  static void printDrxInfo(const DrxInfo &info, ostream &out);
  static void printDrxConf(const DrxConf &conf, ostream &out);

  static void printServerInfo(const ServerInfo &info, ostream &out);
  void printServerInfo(ostream &out);

  static void printDataProdInfo(const DataProdInfo &info, ostream &out);

  static void printPedestal(const Pedestal &ped, ostream &out);
  static void printGeoref(const Georef &geo, ostream &out);
  static void printDataRequest(const DataRequest &req, ostream &out);

  static void printDataResponse(const DataResponse &resp, ostream &out);
  void printDataResponse(ostream &out);
  
protected:

  typedef enum {
    PING_SERVER = 0x01,
    GET_CONF = 0x02,
    SET_CONF = 0x03,
    GET_STATUS = 0x04,
    GET_SERVER_INFO = 0x05,
    GET_DATA = 0x07,
    GET_CONF_AND_STATUS = 0x08,
    LOAD_PACSI_PED_DISABLED = 0x200,
    LOAD_PACSI_PED_ENABLED = 0x201
  } commandCode_t;

  typedef enum {
    STATUS_SRV_ERR = 65,
    STATUS_OK = 66,
    STATUS_CFG_TRANSITION = 67,
    STATUS_LACK_CONTROL = 68,
    STATUS_UNKNOWN_CMD = 69,
    STATUS_UNKNOWN_DATA_TYPE = 70,
    STATUS_WRONG_DATA_SIZE = 71,
    STATUS_NO_DATA = 72,
    STATUS_WRONG_ARCHIVE = 73,
    STATUS_INVALID_INDEX = 74,
    STATUS_INVALID_PARAM = 75,
    STATUS_INVALID_TEXT = 76,
    STATUS_NETCMD_BUSY = 77
  } statusCode_t;


  bool _debug;
  bool _verbose;
  bool _overrideAzOffset;
  double _azOffsetDeg;

  string _serverHost;
  int _serverPort;
  Socket _socket;
  bool _socketIsOpen;

  bool _swap;

  MemBuf _readMemBuf;
  char *_readBuf;
  int _readPos;

  MemBuf _writeMemBuf;
  char *_writeBuf;
  int _writePos;

  int _prevBlockIndex;
  int _archiveIndex;
  bool _metaDataHasChanged;
  
  XpolStatus _xpolStatus;
  XpolConf _xpolConf;
  ServerInfo _serverInfo;
  DataResponse _dataResponse;

  procMode_t _procMode;
  serverMode_t _serverMode;

  MemBuf _dataMemBuf;

  MemBuf _recvPwrV0Buf;
  MemBuf _recvPwrV1Buf;
  MemBuf _recvPwrV2Buf;
  MemBuf _recvPwrH0Buf;
  MemBuf _recvPwrH1Buf;
  MemBuf _recvPwrH2Buf;
  MemBuf _totPwrVBuf;
  MemBuf _totPwrHBuf;

  MemBuf _dopPpV0V1Buf;
  MemBuf _dopPpV1V2Buf;
  MemBuf _dopPpH0H1Buf;
  MemBuf _dopPpH1H2Buf;
  MemBuf _crossCorrVHBuf;
  
  double *_recvPwrV0;
  double *_recvPwrV1;
  double *_recvPwrV2;
  double *_recvPwrH0;
  double *_recvPwrH1;
  double *_recvPwrH2;
  double *_totPwrV;
  double *_totPwrH;

  RadarComplex_t *_dopPpV0V1;
  RadarComplex_t *_dopPpV1V2;
  RadarComplex_t *_dopPpH0H1;
  RadarComplex_t *_dopPpH1H2;
  RadarComplex_t *_crossCorrVH;
  
  // methods

  static string _commandCode2Str(commandCode_t command);
  static string _statusCode2Str(statusCode_t status);

  void _loadStatus(XpolStatus &status);
  void _loadConf(XpolConf &conf);
  void _loadServerInfo(ServerInfo &serverInfo);
  void _loadDrxSpec(DrxSpec &drxSpec);
  void _loadDrxConf(DrxConf &drxConf);
  void _loadDrxInfo(DrxInfo &drxInfo);
  void _loadDataProdInfo(DataProdInfo &prodInfo);
  void _loadPedestal(Pedestal &ped);
  void _loadGeoref(Georef &geo);
  void _loadDataResponse(DataResponse &resp);
  int _loadDataBuffer();
  void _allocCovarBuffers();
  void _loadCovarBuffers();
  
  // get members from read buffer

  si32 _getSi32(const void *buf, int offsetBytes);
  fl32 _getFl32(const void *buf, int offsetBytes);
  fl64 _getFl64(const void *buf, int offsetBytes);
  
  si32 _getNextSi32();
  si64 _getNextSi64();
  fl32 _getNextFl32();
  fl64 _getNextFl64();
  string _getNextString(int len);

  void _putNextSi32(si32 val);
  void _putNextFl32(fl32 val);
  void _putNextFl64(fl64 val);
  void _putNextString(const string &val, int len);

  void _putDataRequest(DataRequest &req);

  void _allocReadBuf(int len);
  void _allocWriteBuf(int len);
  void _resetWriteBuf();
  
  // get structs from buffer
  
  int _openSocket();
  void _closeSocket();
  int _communicateWithServer();
  int _communicateWithServer(void *sendBuf,
                             int sendLen,
                             void *rcvBuf,
                             int rcvLen);
  int _readFromServer();
  int _readFromServer(void *rcvBuf, int rcvLen);

private:

};

#endif

