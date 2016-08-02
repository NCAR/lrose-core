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
// RadarSpectra.cc
//
//  C++ class for dealing with z-v probability calibration
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2006
//////////////////////////////////////////////////////////////

#include <rapformats/RadarSpectra.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <iomanip>
using namespace std;

const double RadarSpectra::missingDouble = -9999.0;

///////////////
// constructor

RadarSpectra::RadarSpectra()
  
{
  
  _versionNum = 1;
  clear();

}

/////////////
// destructor

RadarSpectra::~RadarSpectra()

{

}

//////////////////////////
// clear all data members

void RadarSpectra::clear()

{

  _channel = CHANNEL_HC;

  _timeSecs = 0;
  _doubleTime = 0;
  
  _nSamples = 0;
  _nGates = 0;
  _startRange = 0;
  _gateSpacing = 0;

  _elevDeg = 0;
  _azDeg = 0;

  _prt = 0;
  _wavelengthCm = 0;
  _nyquist = 0;

  _noiseDbm = 0;
  _receiverGainDb = 0;
  _baseDbz1km = 0;

  _staggeredPrtMode = false;
  _prtShort = 0;
  _prtLong = 0;
  _nGatesPrtShort = 0;
  _nGatesPrtLong = 0;
  _staggeredM = 0;
  _staggeredN = 0;

  _iq.clear();

}

//////////////////////////
// add iq data for a gate

void RadarSpectra::addGateIq(const vector<radar_iq_t> &iq)

{

  for (int ii = 0; ii < (int) iq.size(); ii++) {
    _iq.push_back(iq[ii]);
  }

  _nGates = _iq.size() / _nSamples;

}

//////////////////////////
// get iq data for a gate

vector<RadarSpectra::radar_iq_t> RadarSpectra::getGateIq(int gateNum) const

{
  
  int startPt = gateNum * _nSamples;
  int endPt = startPt + _nSamples;
  
  vector<radar_iq_t> iq;
  
  for (int ii = startPt; ii <= endPt; ii++) {
    if (ii < (int) _iq.size()) {
      iq.push_back(_iq[ii]);
    }
  }

  return iq;

}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void RadarSpectra::assemble()
  
{

  // free up mem buffer
  
  _memBuf.free();

  // fill up header

  header_t hdr;
  MEM_zero(hdr);

  hdr.version_num = _versionNum;
  hdr.channel = _channel;
  
  hdr.time_secs = _timeSecs;
  hdr.partial_secs = (fl32) (_doubleTime - (double) _timeSecs);
  
  hdr.n_samples = _nSamples;
  hdr.n_gates = _nGates;
  hdr.start_range = _startRange;
  hdr.gate_spacing = _gateSpacing;
  
  hdr.elev_deg = _elevDeg;
  hdr.az_deg = _azDeg;
  
  hdr.prt = _prt;
  hdr.wavelength_cm = _wavelengthCm;
  hdr.nyquist = _nyquist;

  hdr.noiseDbm = _noiseDbm;
  hdr.receiverGainDb = _receiverGainDb;
  hdr.baseDbz1km = _baseDbz1km;

  hdr.staggered_prt_mode = _staggeredPrtMode;
  hdr.prt_short = _prtShort;
  hdr.prt_long = _prtLong;
  hdr.n_gates_prt_short = _nGatesPrtShort;
  hdr.n_gates_prt_long = _nGatesPrtLong;
  hdr.staggered_m = _staggeredM;
  hdr.staggered_n = _staggeredN;

  STRncopy(hdr.radar_name, _radarName.c_str(), radarNameLen);
  STRncopy(hdr.notes, _notes.c_str(), notesLen);

  // byte swap

  BE_from_array_32(&hdr, sizeof(hdr) - radarNameLen - notesLen);

  // add header
  
  _memBuf.add(&hdr, sizeof(hdr));

  // add data

  for (int ii = 0; ii < (int) _iq.size();  ii++) {

    radar_iq_t iq = _iq[ii];
    BE_from_array_32(&iq, sizeof(iq));
    _memBuf.add(&iq, sizeof(iq));

  }

}

///////////////////////////////////////////////////////////
// disassemble the header
// Allows user to peek into header.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int RadarSpectra::disassembleHdr(const void *buf, int len)

{

  if (len < (int) sizeof(header_t)) {
    cerr << "ERROR - RadarSpectra::disassembleHdr" << endl;
    cerr << "  Cannot disassemble header" << endl;
    cerr << "  Required len:" << sizeof(header_t) << endl;
    cerr << "  Available len:" << len << endl;
    return -1;
  }

  // copy header and swap

  header_t hdr;
  memcpy(&hdr, buf, sizeof(hdr));
  BE_to_array_32(&hdr, sizeof(hdr) - radarNameLen - notesLen);

  // set members

  _versionNum = hdr.version_num;
  _channel = (polarization_channel_t) hdr.channel;
  
  _timeSecs = hdr.time_secs;
  _doubleTime = (double) _timeSecs + (double) hdr.partial_secs;
  
  _nSamples = hdr.n_samples;
  _nGates = hdr.n_gates;
  _startRange = hdr.start_range;
  _gateSpacing = hdr.gate_spacing;
  
  _elevDeg = hdr.elev_deg;
  _azDeg = hdr.az_deg;
  
  _prt = hdr.prt;
  _wavelengthCm = hdr.wavelength_cm;
  _nyquist = hdr.nyquist;

  _noiseDbm = hdr.noiseDbm;
  _receiverGainDb = hdr.receiverGainDb;
  _baseDbz1km = hdr.baseDbz1km;

  _staggeredPrtMode = hdr.staggered_prt_mode;
  _prtShort = hdr.prt_short;
  _prtLong = hdr.prt_long;
  _nGatesPrtShort = hdr.n_gates_prt_short;
  _nGatesPrtLong = hdr.n_gates_prt_long;
  _staggeredM = hdr.staggered_m;
  _staggeredN = hdr.staggered_n;

  // ensure radar name and notes are null terminated

  hdr.radar_name[radarNameLen - 1] = '\0';
  _radarName = hdr.radar_name;

  hdr.notes[notesLen - 1] = '\0';
  _notes = hdr.notes;

  return 0;

}

///////////////////////////////////////////////////////////
// disassemble full object
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int RadarSpectra::disassemble(const void *buf, int len)

{

  // load buffer into memBuf

  _memBuf.free();
  _memBuf.add(buf, len);

  // set header

  if (disassembleHdr(buf, len)) {
    return -1;
  }

  // compute min required size of buffer

  int npts = _nGates * _nSamples;
  int minBufSize = sizeof(header_t) + npts * sizeof(radar_iq_t);

  if (len < minBufSize) {
    cerr << "ERROR - RadarSpectra::disassembleHdr" << endl;
    cerr << "  Cannot disassemble data" << endl;
    cerr << "  Required len:" << minBufSize << endl;
    cerr << "  Available len:" << len << endl;
    return -1;
  }

  _iq.clear();
  char *inPtr = (char *) buf + sizeof(header_t);
  for (int ii = 0; ii < npts; ii++, inPtr += sizeof(radar_iq_t)) {
    radar_iq_t iq;
    memcpy(&iq, inPtr, sizeof(iq));
    BE_to_array_32(&iq, sizeof(iq));
    _iq.push_back(iq);
  }

  return 0;

}

//////////////////////
// printing object

void RadarSpectra::printHeader(ostream &out, string spacer /* = ""*/ ) const

{

  out << "=============================================" << endl;
  out << spacer << "Radar beam spectra" << endl;
  out << spacer << "  Version number : " << _versionNum << endl;
  int msecs = (int) ((_doubleTime - _timeSecs) * 1000.0 + 0.5);
  out << spacer << "  timeSecs : " << DateTime::strm(_timeSecs) << endl;
  char timeStr[128];
  sprintf(timeStr, "%s.%.3d", DateTime::strm(_timeSecs).c_str(), msecs);
  out << spacer << "  time : " << timeStr << endl;
  
  out << spacer << "  nSamples : " << _nSamples << endl;
  out << spacer << "  nGates : " << _nGates << endl;
  out << spacer << "  startRange : " << _startRange << endl;
  out << spacer << "  gateSpacing : " << _gateSpacing << endl;
  out << spacer << "  elevDeg : " << _elevDeg << endl;
  out << spacer << "  azDeg : " << _azDeg << endl;
  out << spacer << "  prt : " << _prt << endl;
  out << spacer << "  wavelengthCm : " << _wavelengthCm << endl;
  out << spacer << "  nyquist : " << _nyquist << endl;
  out << spacer << "  noiseDbm : " << _noiseDbm << endl;
  out << spacer << "  receiverGainDb : " << _receiverGainDb << endl;
  out << spacer << "  baseDbz1km : " << _baseDbz1km << endl;
  out << spacer << "  staggeredPrtMode : " << _staggeredPrtMode << endl;
  out << spacer << "  prtShort : " << _prtShort << endl;
  out << spacer << "  prtLong : " << _prtLong << endl;
  out << spacer << "  nGatesPrtShort : " << _nGatesPrtShort << endl;
  out << spacer << "  nGatesPrtLong : " << _nGatesPrtLong << endl;
  out << spacer << "  staggeredM : " << _staggeredM << endl;
  out << spacer << "  staggeredN : " << _staggeredN << endl;
  out << spacer << "  radarName : " << _radarName << endl;
  out << spacer << "  notes : " << _notes << endl;

  out << endl;
  
}

void RadarSpectra::print(ostream &out, string spacer /* = ""*/ ) const

{

  printHeader(out, spacer);
  
  for (int igate = 0; igate < _nGates; igate++) {
    double range = igate * _gateSpacing + _startRange;
    out << "=============================================" << endl;
    out << spacer << "  Gate num : " << igate << endl;
    out << spacer << "  Range : " << range << endl;
    out << spacer << "  Index, I, Q .... " << endl;
    vector<radar_iq_t> gateIq = getGateIq(igate);
    for (int jj = 0; jj < (int) gateIq.size(); jj++) {
      char text[256];
      sprintf(text, "%4d %15.3e %15.3e",  jj, gateIq[jj].ival, gateIq[jj].qval);
      out << "    " << text << endl;
    }
  }

  out << endl;
  
}

