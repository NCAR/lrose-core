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
// ReadFromUdp.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Read UDP stream that is created by the WRITE_TO_UDP mode
// of this application.
// Creates APAR time series format stream,
// and write out to an FMQ
//
////////////////////////////////////////////////////////////////

#ifndef ReadFromUdp_HH
#define ReadFromUdp_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"

class IwrfTsPulse;
using namespace std;

////////////////////////
// This class

class ReadFromUdp {
  
public:

  // constructor

  ReadFromUdp(const string &progName,
              const Params &params,
              vector<string> &inputFileList);

  // destructor
  
  ~ReadFromUdp();

  // run 

  int Run();
  
protected:
  
private:
  
  string _progName;
  const Params &_params;
  vector<string> _inputFileList;

  // output UDP

  int _udpFd;
  int _errCount;

  // pulse details

  ui64 _dwellSeqNum;
  ui64 _pulseSeqNum;
  ui64 _sampleSeqNum; // for UDP only
  vector<IwrfTsPulse *> _dwellPulses;
  
  // functions

  int _convert2Udp(const string &inputPath);
  int _processDwell(vector<IwrfTsPulse *> &dwellPulses);

  void _fillIqData(IwrfTsPulse *iwrfPulse,
                   int channelNum,
                   vector<si16> &iqData);

  int _sendPulse(ui64 sampleNumber,
                 ui64 pulseNumber,
                 si64 secondsTime,
                 ui32 nanoSecs,
                 ui64 dwellNum,
                 ui32 beamNumInDwell,
                 ui32 visitNumInBeam,
                 double uu,
                 double vv,
                 bool isXmitH,
                 bool isCoPolRx,
                 int nGates,
                 vector<si16> &iqApar);

  void _addAparHeader(ui64 sampleNumber,
                      ui64 pulseNumber,
                      si64 secondsTime,
                      ui32 nanoSecs,
                      ui32 pulseStartIndex,
                      ui64 dwellNum,
                      ui32 beamNumInDwell,
                      ui32 visitNumInBeam,
                      double uu,
                      double vv,
                      bool isFirstPktInPulse,
                      bool isXmitH,
                      bool isCoPolRx,
                      int nSamples,
                      MemBuf &buf);

  int _openOutputUdp();
  int _writeBufToUdp(const MemBuf &buf);
  
};

#endif
