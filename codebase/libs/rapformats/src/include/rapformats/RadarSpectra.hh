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
// RadarSpectra.hh
//
// C++ class for storing radar spectra, gate-by-gate
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Ocy 2008
//////////////////////////////////////////////////////////////

#ifndef _RadarSpectra_hh
#define _RadarSpectra_hh

#include <cstdio>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>

using namespace std;

class RadarSpectra {

public:

  typedef enum {
    CHANNEL_HC = 1,
    CHANNEL_VC = 2,
    CHANNEL_HX = 3,
    CHANNEL_VX = 4
  } polarization_channel_t;

  static const int radarNameLen = 32;
  static const int notesLen = 160;

  typedef struct {

    si32 version_num;
    si32 channel;

    si32 time_secs;
    fl32 partial_secs;

    si32 n_samples;

    si32 n_gates;
    fl32 start_range;
    fl32 gate_spacing;

    fl32 elev_deg;
    fl32 az_deg;

    fl32 prt; // secs
    fl32 wavelength_cm;
    fl32 nyquist; // m/s
    
    fl32 noiseDbm;
    fl32 receiverGainDb;
    fl32 baseDbz1km;

    // staggered mode

    si32 staggered_prt_mode;
    fl32 prt_short;
    fl32 prt_long;
    si32 n_gates_prt_short;
    si32 n_gates_prt_long;
    si32 staggered_m;
    si32 staggered_n;

    si32 spare[41];

    char radar_name[radarNameLen];
    char notes[notesLen];

  } header_t;
  
  typedef struct {

    fl32 ival;
    fl32 qval;

  } radar_iq_t;
  
  // constructor
  
  RadarSpectra();

  // destructor

  ~RadarSpectra();

  //////////////////////// set methods /////////////////////////

  // clear all data members

  void clear();

  //////////////////////// set methods /////////////////////////

  void setChannel(polarization_channel_t val) {
    _channel = val;
  }
  
  void setTimeSecs(time_t val) {
    _timeSecs = val;
  }
  
  void setDoubleTime(double val) {
    _doubleTime = val;
  }
  
  void setNSamples(int val) {
    _nSamples = val;
  }

  void setNGates(int val) {
    _nGates = val;
  }

  void setStartRange(double val) {
    _startRange = val;
  }

  void setGateSpacing(double val) {
    _gateSpacing = val;
  }

  void setElevDeg(double val) {
    _elevDeg = val;
  }

  void setAzDeg(double val) {
    _azDeg = val;
  }

  void setPrt(double val) {
    _prt = val;
  }

  void setWavelengthCm(double val) {
    _wavelengthCm = val;
  }

  void setNyquist(double val) {
    _nyquist = val;
  }
  
  void setNoiseDbm(double val) {
    _noiseDbm = val;
  }
  
  void setReceiverGainDb(double val) {
    _receiverGainDb = val;
  }
  
  void setBaseDbz1km(double val) {
    _baseDbz1km = val;
  }

  void setStaggeredPrtMode(bool state) {
    _staggeredPrtMode = state;
  }

  void setPrtShort(double val) {
    _prtShort = val;
  }

  void setPrtLong(double val) {
    _prtLong = val;
  }

  void setNGatesPrtShort(int val) {
    _nGatesPrtShort = val;
  }

  void setNGatesPrtLong(int val) {
    _nGatesPrtLong = val;
  }

  void setStaggeredM(int val) {
    _staggeredM = val;
  }

  void setStaggeredN(int val) {
    _staggeredN = val;
  }

  void setRadarName(const string &val) {
    _radarName = val;
  }
  
  void setNotes(const string &val) {
    _notes = val;
  }
  
  // add data for a gate
  
  void addGateIq(const vector<radar_iq_t> &iq);

  //////////////////////// get methods /////////////////////////

  int getVersionNum() const { return _versionNum; }
  polarization_channel_t getChannel() const { return _channel; }

  time_t getTimeSecs() const { return  _timeSecs; }
  double getDoubleTime() const { return  _doubleTime; }

  int getNSamples() const { return _nSamples; }
  int getNGates() const { return _nGates; }
  double getStartRange() const { return  _startRange; }
  double getGateSpacing() const { return  _gateSpacing; }

  double getElevDeg() const { return  _elevDeg; }
  double getAzDeg() const { return  _azDeg; }

  double getPrt() const { return  _prt; }
  double getWavelengthCm() const { return  _wavelengthCm; }
  double getNyquist() const { return  _nyquist; }

  double getNoiseDbm() const { return  _noiseDbm; }
  double getNoise() const { return  pow(10.0, _noiseDbm/10.0); }
  double getReceiverGainDb() const { return  _receiverGainDb; }
  double getReceiverGain() const { return  pow(10.0, _receiverGainDb/10.0); }
  double getBaseDbz1km() const { return  _baseDbz1km; }

  bool isStaggeredPrtMode() const { return _staggeredPrtMode; }
  double getPrtShort() const { return  _prtShort; }
  double getPrtLong() const { return  _prtLong; }
  int getNGatesPrtShort() const { return _nGatesPrtShort; }
  int getNGatesPrtLong() const { return _nGatesPrtLong; }
  int getStaggeredM() const { return _staggeredM; }
  int getStaggeredN() const { return _staggeredN; }

  const string &getRadarName() const { return _radarName; }
  const string &getNotes() const { return _notes; }
  
  // get data for a gate

  vector<radar_iq_t> getGateIq(int gateNum) const;
  
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();
  
  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }
  
  // disassemble the header
  // Allows user to peek into header.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassembleHdr(const void *buf, int len);

  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void printHeader(ostream &out, string spacer = "") const;
  void print(ostream &out, string spacer = "") const;
  
  // missing value

  static const double missingDouble;

protected:

  int _versionNum;
  polarization_channel_t _channel;

  time_t _timeSecs;
  double _doubleTime;
  
  int _nSamples;

  int _nGates;
  double _startRange;
  double _gateSpacing;

  double _elevDeg;
  double _azDeg;

  double _prt;
  double _wavelengthCm;
  double _nyquist;

  double _noiseDbm;
  double _receiverGainDb;
  double _baseDbz1km;

  bool _staggeredPrtMode;
  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  int _staggeredM;
  int _staggeredN;

  string _radarName;
  string _notes;
  
  vector<radar_iq_t> _iq;

  // buffer for assemble / disassemble

  MemBuf _memBuf;

private:

};


#endif

