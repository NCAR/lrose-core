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
// File2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
///////////////////////////////////////////////////////////////

#ifndef File2Fmq_H
#define File2Fmq_H

#include <string>
#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>
using namespace std;

////////////////////////
// This class

class File2Fmq {
  
public:

  // constructor
  
  File2Fmq(const Params &params,
	   DsRadarQueue &r_queue,
	   const char *input_path,
	   int vol_num);
  
  // destructor
  
  ~File2Fmq();

  // read data in from file
  
  int read();

  // write out the data
  
  int write();

protected:
  
private:
  
  const Params &_params;
  DsRadarQueue &_rQueue;
  string _inputPath;
  int _volNum;
  
  vector<string> _headers;

  int _version;
  int _fileType;

  // date / time

  int _year;
  int _month;
  int _day;
  int _hour;
  int _min;
  int _sec;
  int _time;
  int _useNow;
  
  // dimensions

  int _nGates;
  int _nFields;
  int _nTilts;
  int _nBytesData;

  // azimuth angles

  int _nAz;
  double _minAz;
  double _deltaAz;
  
  // elevation angles

  vector<double> _elevations;

  // radar params
  
  int _radarId;
  string _radarName;
  int _samplesPerBeam;
  Params::polar_t _polarization; // polarization type
  double _altitude;              // km
  double _latitude;              // degrees
  double _longitude;             // degrees
  double _gateSpacing;           // km
  double _startRange;            // km
  double _beamWidth;             // half-power beam width - degrees
  double _pulseWidth;            // nano-secs
  double _prf;                   // /s
  double _wavelength;            // cm
  double _radarConstant;         // dB
  double _xmitPeakPwr;           // watts
  double _receiverMds;           // dBm
  double _receiverGain;          // dB
  double _antennaGain;           // dB
  double _systemGain;            // dB
  double _maxRange;

  // all data fields scale and bias

  double _dbzScale, _dbzBias;
  double _velScale, _velBias;
  double _widthScale, _widthBias;
  double _zdrScale, _zdrBias;
  
  // actual data field

  string _fieldName;
  string _fieldUnits;
  double _scale;
  double _bias;
  int _compressed;

  // field data array
  
  ui08 *_fieldData;

  void _printHeaders(ostream &out);
  void _printMetaData(ostream &out);

  int _findHeader(const string &tag, string &hdr);

  int _interpretHeaders();

  int _writeParams();

  int _writeBeams(int tilt_num);

  int _findNameInList(const string &name,
		      const string &list);
  
  
};

#endif

