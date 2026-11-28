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
// Beam.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2003
//
///////////////////////////////////////////////////////////////
//
// Object for assembling a Beam
//
////////////////////////////////////////////////////////////////

#include "Beam.hh"
#include "Fields.hh"
#include "InputUdp.hh"
#include <toolsa/udatetime.h>

using namespace std;

//////////////////////////////////////
// constructor

Beam::Beam(const Params &params) :
  _params(params)
  
{
  clear();
}

//////////////////////////////////////
// destructor

Beam::~Beam()
  
{
  
}

/////////////
// clear out

void Beam::clear()
{
  _isComplete = false;
  _seqNum = -1;
  _nFields = 0;
  _nGates = 0;
  _nPacketsPerField = 0;
  _nPacketsTotal = 0;
  _nPacketsAdded = 0;
  for (int i = 0; i < N_MOMENTS_MAX; i++) {
    if (_bufs[i].getLen() > 0) {
      _bufs[i].reset();
    }
  }
}

///////////////
// set headers

void Beam::setVolHdr(const volume_header_t &volHdr)
{
  _volHdr = volHdr;
  _nFields = _volHdr.nMoments;
  _nGates = _volHdr.nBins;
  _nPacketsPerField = 1 +
    (_volHdr.nBins - 1) / _volHdr.nBinsPacket;
  _nPacketsTotal = _nPacketsPerField * _nFields;
  _byteWidth = Fields::getByteWidth(_volHdr.momData[0]);

  _prf = (double) _volHdr.prf;
  _wavelength = (double) _volHdr.wavelengthCmOver100 / 10000.0;
  _nyquistVel = (_wavelength * _prf) / 4.0;
  _unambigRange = (3.0e8 / (2.0 * _prf)) / 1000.0; 

}

void Beam::setBeamHdr(const beam_header_t &beamHdr)
{
  _beamHdr = beamHdr;
}

////////////////
// add a packet

void Beam::addBeamPacket(const beam_header_t &beamHdr)

{

  _bufs[beamHdr.momDataIndex].add(beamHdr.momData, beamHdr.momDataSize);
  _nPacketsAdded++;
  
  if (_nPacketsAdded == _nPacketsTotal) {

    _isComplete = true;

    // pad out buffers if appropriate

    int nbytesExpected = _nGates * _byteWidth;
    for (int i = 0; i < _nFields; i++) {
      if ((int) _bufs[i].getLen() < nbytesExpected) {
	int nTooFew = nbytesExpected - _bufs[i].getLen();
	ui08 dummy[nTooFew];
	memset(dummy, 0, nTooFew);
	_bufs[i].add(dummy, nTooFew);
      }
    } // i

  }

}

/////////////////////////
// convert data to 8-bit

void Beam::convertDataTo8Bit(int fieldNum,
			     MemBuf &out) const
  
{
  out.reset();
  if (_byteWidth == 1) {
    out.add(_bufs[fieldNum].getPtr(), _bufs[fieldNum].getLen());
  } else {
    Fields::convertTo8Bit(_volHdr.momData[fieldNum], _nyquistVel,
			  _bufs[fieldNum], out);
  }

}

///////////
// printing

void Beam::printVolHdr(ostream &out) const
{
  InputUdp::printVolHdr(_volHdr, "  ", out);
}

void Beam::printBeamHdr(ostream &out) const
{
  InputUdp::printBeamHdr(_beamHdr, "  ", out);
}

void Beam::printBeamSummary(ostream &out) const
{
  printBeamHdr(out);
  for (int i = 0; i < _volHdr.nMoments; i++) {
    out << "    " << "Field name, len: "
	<< Fields::getName(_volHdr.momData[i])
	<< ", " << _bufs[i].getLen() << endl;
  }
}

//////////////////////
// get various values

time_t Beam::getTime() const
{
  date_time_t btime;
  btime.year = _volHdr.sweepStartTime.year;
  btime.month = _volHdr.sweepStartTime.month;
  btime.day = _volHdr.sweepStartTime.day;
  btime.hour = 0;
  btime.min = 0;
  btime.sec = _volHdr.sweepStartTime.sec + _beamHdr.timeOffsetSecs;
  uconvert_to_utime(&btime);
  return (btime.unix_time + _params.time_correction_secs);
}

double Beam::getEl() const
{
  double el = (_beamHdr.endElev / 65536.0) * 360.0;
  if (el > 180.0) {
    el -= 360.0;
  }
  return el;
}

double Beam::getAz() const
{
  double azStart = (_beamHdr.startAz / 65536.0) * 360.0;
  double azEnd = (_beamHdr.endAz / 65536.0) * 360.0;
  if (azEnd < azStart) {
    azEnd += 360.0;
  }
  double azMean = (azStart + azEnd) / 2.0;
  if (azMean < 0) {
    azMean += 360.0;
  } else if (azMean >= 360.0) {
    azMean -= 360.0;
  }
  return azMean;

}

