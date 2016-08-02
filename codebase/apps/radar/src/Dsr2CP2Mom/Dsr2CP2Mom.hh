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
// Dsr2CP2Mom.hh
//
// Dsr2CP2Mom object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// Dsr2CP2Mom reads an input radar FMQ, and
// writes CP2Moments UDP data
//
///////////////////////////////////////////////////////////////////////

#ifndef Dsr2CP2Mom_HH
#define Dsr2CP2Mom_HH

#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>

#include "Args.hh"
#include "Params.hh"
#include "CP2Net.hh"
#include "CP2UdpSocket.hh"

using namespace std;

////////////////////////
// This class

class Dsr2CP2Mom {
  
public:

  // constructor

  Dsr2CP2Mom (int argc, char **argv);

  // destructor
  
  ~Dsr2CP2Mom();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // keeping track of number of beams

  long long _nBeamsRead;

  // The maximum message size that we can send
  // on UDP, and the UDP output buffer size
  
  static const int _soMaxMsgSize = 64000;
  static const int _udpBufferSize = 100000000;

  /// The socket that data products are transmitted on.
  
  CP2UdpSocket* _pProductSocket;

  // input FMQs

  DsRadarQueue _sbandQueue;
  DsRadarQueue _xbandQueue;
  DsRadarMsg _sbandMsg, _xbandMsg;
  int _xContents, _sContents;
  double _prevSbandAz, _sbandAz, _xbandAz;
  bool _clockWise;
  bool _xbandAvail;

  // functions
  
  int _run();
  
  int _readMsg(DsRadarQueue &radarQueue,
	       DsRadarMsg &radarMsg,
	       int &contents);

  int _processSbandBeam();
  int _processXbandBeam();

  int _processField(const string &fieldName,
		    int dsrFieldNum,
		    int cp2FieldId,
		    const DsRadarParams &radarParams,
		    const DsFieldParams &fieldParams,
		    const DsRadarBeam &radarBeam,
		    double missingVal);

  int _initUdpOutput();

  void _sendField(CP2Net::CP2ProductHeader& header, 
		  std::vector<double>& data,
		  CP2Net::CP2Packet& packet,
		  bool forceSend);
  
  void _printBeamInfo(const DsRadarMsg &radarMsg,
		      const string &label);

  int _readSband();
  int _readXband();
  
  double _getAz(const DsRadarMsg &radarMsg);
  double _getEl(const DsRadarMsg &radarMsg);
  time_t _getTime(const DsRadarMsg &radarMsg);
  double _getTimeDbl(const DsRadarMsg &radarMsg);

  void _syncQueues(bool allowSbandRead);

};

#endif

