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
/*********************************************************************
 * UfRecord.hh
 *
 * Object represting a single UF record
 *
 *********************************************************************/

#ifndef UF_RECORD_HH
#define UF_RECORD_HH

#include "UfRadar.hh"
#include "DateTime.hh"
#include <vector>
using namespace std;

class UfRecord

{

public:

  // constructor
  
  UfRecord();
  
  // destructor
  
  virtual ~UfRecord();

  // set debugging on

  void setDebug(bool state) { _debug = state; }
 
  // clear all data
  
  void clearData();

  // free up data

  void free();

  // disassemble the object from a raw UF buffer
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int nBytes);

  // field translation

  typedef struct {
    string input_name;
    string uf_name;
    int scale;
  } field_tranlation_t;
  
  // load ray data
  // Returns 0 on success, -1 on failure
  
  int loadRayData(int volume_num,
                  int sweep_num,
                  bool is_rhi,
                  int ray_num_in_vol,
                  const DateTime &ray_time,
                  double lat,
                  double lon,
                  double alt_m,
                  double el,
                  double az,
                  int n_gates,
                  int n_samples,
                  double start_range_m,
                  double gate_spacing_m,
                  const double *snr_db,
                  const double *vel,
                  const double *width,
                  const double *ht_km);
  
  // Write record to open file
  // Returns 0 on success, -1 on failure
  
  int write(FILE *fp);

  // print record
  
  void print(ostream &out,
	     bool print_headers,
	     bool print_data);

  // print data members derived from a disassemble

  void printDerived(ostream &out) const;

  // check for all missing data
  // returns true if all data is missing

  bool allDataMissing();

  // set methods

  void setInstrumentName(const string &val) { _instrumentName = val; }
  void setPolarizationCode(int val) { _polarizationCode = val; }
  void setRadarConstant(double val) { _radarConstant = val; }
  void setNoisePowerDbm(double val) { _noisePowerDbm = val; }
  void setReceiverGainDb(double val) { _receiverGainDb = val; }
  void setPeakPowerDbm(double val) { _peakPowerDbm = val; }
  void setAntennaGainDb(double val) { _antennaGainDb = val; }
  void setPulseWidthUs(double val) { _pulseWidthUs = val; }
  void setHorizBeamWidthDeg(double val) { _horizBeamWidthDeg = val; }
  void setVertBeamWidthDeg(double val) { _vertBeamWidthDeg = val; }
  void setWavelengthCm(double val) { _wavelengthCm = val; }
  void setPrf(double val) { _prf = val; }
  void setNyquistVel(double val) { _nyquistVel = val; }

  // data
  
private:

  bool _debug;

  UF_mandatory_header_t _manHdr;
  UF_data_header_t _dataHdr;
  vector<UF_field_info_t> _fieldInfo;
  vector<UF_field_header_t> _fieldHdrs;
  vector<string> _fieldNames;
  vector<MemBuf> _shortData;

  string _instrumentName;
  int _numGates;
  int _numSamples;
  int _polarizationCode;
  double _startRangeKm;
  double _gateSpacingKm;
  double _radarConstant;
  double _noisePowerDbm;
  double _receiverGainDb;
  double _peakPowerDbm;
  double _antennaGainDb;
  double _pulseWidthUs;
  double _horizBeamWidthDeg;
  double _vertBeamWidthDeg;
  double _wavelengthCm;
  double _prf;
  double _nyquistVel;

  int _volNum;
  int _tiltNum;
  double _targetAngle;

  double _elevation;
  double _azimuth;
  time_t _beamTime;

  void _init();

};

#endif

