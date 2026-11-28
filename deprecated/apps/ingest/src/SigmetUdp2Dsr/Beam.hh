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
// Beam.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2003
//
///////////////////////////////////////////////////////////////
//
// Object for assembling a beam from UDP packets
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <string>
#include <dataport/bigend.h>
#include <toolsa/MemBuf.hh>
#include "Params.hh"
#include "data.hh"

using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  Beam(const Params &params);
  ~Beam();

  void clear();
  void setVolHdr(const volume_header_t &volHdr);
  void setBeamHdr(const beam_header_t &beamHdr);
  void addBeamPacket(const beam_header_t &beamHdr);

  void convertDataTo8Bit(int fieldNum, MemBuf &out) const;
  
  const volume_header_t &getVolHdr() const { return _volHdr; }
  const beam_header_t &getBeamHdr() const { return _beamHdr; }
  bool isComplete() const { return _isComplete; }

  int getSeqNum() const { return _seqNum; }
  int getNFields() const { return _nFields; }
  int getNGates() const { return _nGates; }
  int getByteWidth() const { return _byteWidth; }

  time_t getTime() const;
  double getEl() const;
  double getAz() const;
  double getPrf() const { return _prf; }
  double getWavelength() const { return _wavelength; }
  double getNyquistVel() const { return _nyquistVel; }
  double getUnambigRange() const { return _unambigRange; }

  const MemBuf &getFieldBuf(int ifield) const { return _bufs[ifield]; }

  void printVolHdr(ostream &out) const;
  void printBeamHdr(ostream &out) const;
  void printBeamSummary(ostream &out) const;
  
protected:
private:
  
  const Params &_params;

  bool _isComplete;

  int _seqNum;
  int _nFields;
  int _nGates;
  int _byteWidth;
  int _nPacketsPerField;
  int _nPacketsTotal;
  int _nPacketsAdded;

  volume_header_t _volHdr;
  beam_header_t _beamHdr;

  double _prf;
  double _wavelength;
  double _nyquistVel;
  double _unambigRange;
  
  MemBuf _bufs[N_MOMENTS_MAX];

};

#endif

