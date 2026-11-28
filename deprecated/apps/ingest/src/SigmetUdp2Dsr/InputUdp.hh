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
// InputUdp.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////

#ifndef InputUdp_hh
#define InputUdp_hh

#include "Params.hh"
#include "Beam.hh"
#include "data.hh"
#include <string>
#include <deque>
#include <toolsa/MemBuf.hh>
#include <dataport/bigend.h>

using namespace std;

////////////////////////
// This class

class InputUdp {
  
public:
  
  // constructor - initializes for given size
  
  InputUdp(const string &prog_name,
	   const Params &params);
  
  // destructor
  
  ~InputUdp();

  // open and close

  int openUdp();
  void closeUdp();

  // read data from socket, puts into buffer
  // Returns number of bytes read.
  // Use getBuf() for access to buffer.
  
  int readPacket();

  // test if volume header has just been read. If so,
  // use getVolHdr() to get a reference to it.

  bool volHdrIsNew() { return _volHdrIsNew; }
  bool EndVol() { return _EndVol; }
  const volume_header_t &getVolHdr() { return _volHdr; }

  // test if new beam has just been assembled.
  // If so, use getBeam() to get a reference to it.

  bool beamIsNew() { return _beamIsNew; }
  const Beam &getBeam() { return _beam; }

  static void volHdrFromBe(volume_header_t &hdr);
  static void beamHdrFromBe(beam_header_t &hdr);

  static void printVolHdr(const volume_header_t &hdr,
			  const string &spacer,
			  ostream &out);
  
  static void printBeamHdr(const beam_header_t &hdr,
			  const string &spacer,
			  ostream &out);
  
protected:
  
private:

  string _progName;
  const Params &_params;
  int _udpFd;
  static const int maxUdpBytes = 1524;
  ui08 _buf[maxUdpBytes];

  bool _volHdrAvail;
  bool _volHdrIsNew;
  bool _EndVol;
  volume_header_t _volHdr;
  int _byteWidth;
  int _nPacketsPerField;
  static const int _nBeamsSave = 5;
  int _nBeamPacketsSave;

  bool _beamIsNew;
  Beam _beam;

  deque<ui08 *> _rayPackets;

  int _assembleBeam();

};

#endif

