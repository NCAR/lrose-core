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
// ChillSdbServer.hh
//
// ChillSdbServer object - the driver sets up the server object which
// does the real work.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2007
//
///////////////////////////////////////////////////////////////

#ifndef _ChillSdbServer_HH
#define _ChillSdbServer_HH

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>
#include <radar/chill_types.h>

class Socket;
using namespace std;

class ChillSdbServer {
  
public:

  // constructor

  ChillSdbServer (int argc, char **argv);

  // destructor
  
  ~ChillSdbServer();

  // run 

  int Run();

  // data members

  bool OK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // server

  time_t _lastPrintTime;
  int _numClients;

  // input radar queue

  DsRadarParams _rParams;
  vector<DsFieldParams *> _fParams;

  // output status

  unsigned int _rayNumber;
  int _volNum;
  int _sweepNum;
  double _prevAz;
  double _prevEl;

  // struct for indexing the FMQ data so
  // it can be packed into the gate_mom_data_t struct

  typedef struct {
    int Z;
    int V;
    int W;
    int NCP;
    int ZDR;
    int PHIDP;
    int RHOHV;
    int LDR_H;
    int LDR_V;
    int KDP;
    int Zc;
    int ZDRc;
    int PHIDPf;
    int avg_v_mag;
    int avg_v_phase;
    int avg_h_mag;
    int avg_h_phase;
    int lag0_hc;
    int lag0_vc;
    int lag0_hx;
    int lag0_vx;
    int lag1_hc_mag;
    int lag1_hc_phase;
    int lag1_vc_mag;
    int lag1_vc_phase;
    int lag2_hc_mag;
    int lag2_hc_phase;
    int lag2_vc_mag;
    int lag2_vc_phase;
    int lag0_hv_mag;
    int lag0_hv_phase;
    int rhohv_hcx;
    int rhohv_vcx;
  } mom_index_t;

  mom_index_t _momIndex;

  // private methods

  inline double signof(double a) { return (a == 0) ? 0 : (a<0 ? -1.0 : 1.0); }

  int _initialize();
  int _handleClient(Socket *sock);
  int _writeVersion(Socket *sock);
  int _writeInitialHeaders(Socket *sock);
  int _writeRadarInfo(Socket *sock);
  int _writeScanSegment(Socket *sock);
  int _writeProcessorInfo(Socket *sock);
  int _writePowerUpdate(Socket *sock);
  int _writeCalTerms(Socket *sock);
  int _writeXmitInfo(Socket *sock);
  int _writeBeam(DsRadarMsg &msg, Socket *sock);
  int _handleFlags(DsRadarMsg &msg, Socket *sock);
  int _writeEventNotice(Socket *sock,
                        event_notice_flags_t flags);
  void _saveFieldParams(DsRadarMsg &msg);
  int _findFieldIndex(const string &name);
  void _reapChildren();
  void _clearFieldParams();
  int _getVersionFromString(const char * str);
  void _initMomOutput(gate_mom_data_t *momOutput);
  void _fillBeam(DsRadarMsg &msg, gate_mom_data_t *momArray);
  void _loadScalar(void *fieldData, gate_mom_data_t *momArray,
                   int inIndex, int outOffset, float outMissing);

  void _loadVector(void *fieldsIn,
                   gate_mom_data_t *momOutput,
                   int inIndexMag,
                   int inIndexPhase,
                   int outOffsetRe,
                   int outOffsetIm,
                   float outMissing);
  
};

#endif
