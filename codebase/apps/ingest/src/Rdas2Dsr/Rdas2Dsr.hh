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
// Rdas2Dsr.hh
//
// Rdas2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////
//
// Rdas2Dsr reads radar data from RDAS and writes it to
// an FMQ in DsRadar format
//
////////////////////////////////////////////////////////////////

#ifndef Rdas2Dsr_HH
#define Rdas2Dsr_HH

#include <string>
#include <toolsa/Socket.hh>
#include <Fmq/DsRadarQueue.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

#define RDAS_BEAM_NSTATUS 8
#define RDAS_BEAM_N32 80

// beam definition

typedef struct {
  si32 cookie;
  si32 version;
  si32 struct_len;
  si32 count_data_included;
  si32 radar_id;
  si32 year;
  si32 month;
  si32 day;
  si32 hour;
  si32 min;
  si32 sec;
  si32 msec;
  si32 ngates;
  si32 nfields;
  si32 nsamples;
  si32 polarization_code;
  si32 beam_count;
  si32 tilt_count;
  si32 end_of_tilt_flag;
  si32 end_of_vol_flag;
  si32 flag_status1;
  si32 flag_status2;
  si32 field_codes[12];
  si32 spare_ints[10];
  fl32 az;
  fl32 el;
  fl32 el_target;
  fl32 alt_km;
  fl32 lat_deg;
  fl32 lat_frac_deg;
  fl32 lon_deg;
  fl32 lon_frac_deg;
  fl32 gate_spacing;
  fl32 start_range;
  fl32 pulse_width;
  fl32 prf;
  fl32 analog_status[RDAS_BEAM_NSTATUS];
  fl32 spare_floats[16];
  char status_string[64];
} rdas_beam_hdr_t;

////////////////////////
// This class

class Rdas2Dsr {
  
public:

  // constructor

  Rdas2Dsr (int argc, char **argv);

  // destructor
  
  ~Rdas2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsRadarQueue _rQueue;
  rdas_beam_hdr_t _beamHdr;
  ui16 *_beamData;
  
  Socket _socket;
  bool _socketIsOpen;
  int _beamCount;
  int _volNum;
  int _tiltNum;
  int _scanType;

  int _prevNGates;
  double _prevStartRange;
  double _prevGateSpacing;
  double _prevAtmosAtten;
  double _prevCalSlope;
  double _prevCalOffset;

  // radar parameters

  int _siteNum;
  string _siteName;
  string _polarizationStr;

  int _samplesPerAz;
  int _samplesPerGate;
  int _samplesPerBeam;

  double _altitude;
  double _latitude;
  double _longitude;

  double _startRange;
  double _gateSpacing;

  double _prf;
  double _frequency;
  double _wavelength;
  double _pulseWidth;

  double _horizBeamWidth;
  double _vertBeamWidth;

  double _peakPower;
  double _receiverMds;
  double _receiverGain;
  double _antGain;
  double _systemGain;

  double _radarConst;
  double _calSlope;
  double _calOffset1km;
  double _mdsCount;
  double _atmosAtten;

  int _rCorrNGates;
  double _rCorrStartRange;
  double _rCorrGateSpacing;
  double *_range;
  double *_rangeCorrection;

  // count table
  
  const static int _maxCount = 32768;
  double *_calibLut;
  bool _calibLutReady;
  
  // functions

  int _openSocket();
  void _closeSocket();
  int _readIncoming();
  int _readBeam();
  int _readParams();
  int _readCalib();
  int _loadCalibTable(const char *calibBuf);

  int _handleBeam(const rdas_beam_hdr_t &hdr,
		  int nGates,
		  const ui16 *counts);

  int _writeRadarAndFieldParams(int radarId,
				int nGates,
				double start_range,
				double gate_spacing);

  int _writeBeam(time_t beamTime,
		 int nGates,
		 const rdas_beam_hdr_t &hdr,
		 const fl32 *dbz,
		 const fl32 *snr);

  int _findXmlField(const char *xml_buf,
		    const char *field_name,
		    string &val,
		    const char* &startNext);
  
  int _findParamValStr(const char *xml_buf,
		       const char *param_name,
		       string &val);
  
  int _findParamInt(const char *xml_buf,
		    const char *param_name,
		    int &val);

  int _findParamDouble(const char *xml_buf,
		       const char *param_name,
		       double &val);

  int _findFieldInt(const char *xml_buf,
		    const char *param_name,
		    int &val);

  int _findFieldDouble(const char *xml_buf,
		       const char *param_name,
		       double &val);

  void _printRadarCharacteristics(ostream &out);

  void _computeRangeCorrTable(int nGates);

};

#endif
