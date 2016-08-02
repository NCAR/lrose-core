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
/////////////////////////////////////////////////////////////
// ChillSdb2Dsr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// ChillSdb2Dsr reads CHILL moments from the SDB server,
// reformats it into DsRadar format and writes to a DsRadar FMQ
//
// Also optionally computes calibration from SDB packets, and
// writes calibration XML files to a specified directory.
//
////////////////////////////////////////////////////////////////

#ifndef ChillSdb2Dsr_HH
#define ChillSdb2Dsr_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <didss/DsMessage.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/chill_types.h>
#include <rapformats/DsRadarSweep.hh>
#include <rapformats/DsRadarPower.hh>
#include <Fmq/DsRadarQueue.hh>

using namespace std;

////////////////////////
// This class

class ChillSdb2Dsr {
  
public:

  // constructor

  ChillSdb2Dsr(int argc, char **argv);

  // destructor
  
  ~ChillSdb2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double piCubed;
  static const double lightSpeed;
  static const double kSquared;

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // input data socket

  Socket _sock;
  time_t _reconnectTime;

  // output radar queue

  DsRadarQueue *_rQueue;
  DsRadarMsg _msg;

  // chill housekeeping types
  
  radar_info_t _radarInfo;
  scan_seg_t _scanSeg;
  processor_info_t _procInfo;
  iwrf_xmit_power_t _powerUpdate;
  iwrf_event_notice_t _eventNotice;
  cal_terms_t _calTerms;
  xmit_info_t _xmitInfo;
  antenna_correction_t _antCorr;
  xmit_sample_t _xmitSample;
  phasecode_t _phaseCode;
  sdb_version_hdr_t _version;
  sdb_track_info_t _track;
  ray_header_t _rayHdr;

  bool _radarInfoAvail;
  bool _scanSegAvail;
  bool _procInfoAvail;
  bool _powerUpdateAvail;
  bool _eventNoticeAvail;
  bool _calTermsAvail;
  bool _xmitInfoAvail;
  bool _antCorrAvail;
  bool _xmitSampleAvail;
  bool _phaseCodeAvail;
  bool _versionAvail;
  bool _trackAvail;
  
  bool _paramsPending;
  bool _calibPending;
  time_t _calibTime;
  time_t _prevCalibXmlWriteTime;
  
  double _az;
  double _el;
  double _prevAz;
  double _prevEl;

  int _nBeamsWritten;
  int _nBeamsSinceParams;
  int _volNum;
  int _tiltNum;
  
  // functions
  
  void _sleepBeforeReconnect(int errCount);
  int _readFromServer();
  int _seekAhead(int nBytes);
  int _reSync();
  int _peekAtBuffer(void *buf, int nbytes);
  
  int _readStruct(int nbytes, int structSize, void *buf);
  int _readRadarInfo(int nbytes);
  int _readScanSeg(int nbytes);
  int _readProcInfo(int nbytes);
  int _readPowerUpdate(int nbytes);
  int _readEventNotice(int nbytes);
  int _readCalTerms(int nbytes);
  int _readXmitInfo(int nbytes);
  int _readAntCorr(int nbytes);
  int _readXmitSample(int nbytes);
  int _readPhaseCode(int nbytes);
  int _readVersion(int nbytes);
  int _readTrack(int nbytes);
  int _readRay(int nbytes);
  void _computeAngles();
  string _hskId2String(int id);

  // DSR output

  int _openFmq();
  int _writeParams(int nFields);
  void _addField(const string &name, const string &units,
                 vector<DsFieldParams*> &fp);
  int _writeBeam(const char *gateData,
                 int nFields, int nBytes);
  int _writeCalib();

  // CALIBRATION output
  
  int _writeCalibXml();
  double _computeRadarConstant(double xmitPowerDbm,
                               double antennaGainDb,
                               double twoWayWaveguideLossDb,
                               double twoWayRadomeLossDb);
                                     
};

#endif
