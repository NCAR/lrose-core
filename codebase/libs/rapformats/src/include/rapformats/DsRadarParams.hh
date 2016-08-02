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
////////////////////////////////////////////////////////////////////////////////
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  April 1998
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_RADAR_PARAMS_INC_
#define _DS_RADAR_PARAMS_INC_


#include <string>
#include <rapformats/ds_radar.h>
using namespace std;


class DsRadarParams
{
public:

  DsRadarParams();
  DsRadarParams( const DsRadarParams &inputParams ) { copy( inputParams ); }
  ~DsRadarParams(){};

  DsRadarParams& operator=( const DsRadarParams &inputParams );

  bool operator==( const DsRadarParams &otherParams ) const;
  bool operator!=( const DsRadarParams &otherParams ) const
                 { return !operator==( otherParams ); }
   
  void copy( const DsRadarParams &inputParams );
  void copy( const DsRadarParams_t &inputParams );
  void print( FILE *out=stdout ) const;
  void print(ostream &out) const;

  void decode( DsRadarParams_t *rparams );
  void encode( DsRadarParams_t *rparams ) const;
  void loadStruct( DsRadarParams_t *rparams ) const;

  inline  int   getNumGates() const { return numGates; }
  inline  int   getNumFields() const { return numFields; }

  inline  void  setNumGates(int nGates) { numGates = nGates; }

  static string radarType2Str(int rtype);
  static string scanMode2Str(int mode);
  static string followMode2Str(int mode);
  static string polType2Str(int ptype);
  static string prfMode2Str(int mode);

  int     radarId;           // unique number
  int     radarType;         // use radar type defs in ds_radar.h
  int     numFields;         // number of fields
  int     numGates;          // number of range gates
  int     samplesPerBeam;    // number of pulses per data beam
  int     scanType;          // the current scan strategy
  int     scanMode;          // see ds_radar.h
  int     followMode;        // see ds_radar.h
  int     polarization;      // see ds_radar.h
  int     prfMode;           // see ds_radar.h

  float   radarConstant;     // radar constant
  float   altitude;          // km
  float   latitude;          // degrees
  float   longitude;         // degrees
  float   gateSpacing;       // km
  float   startRange;        // km
  float   horizBeamWidth;    // degrees
  float   vertBeamWidth;     // degrees
  float   pulseWidth;        // micro-seconds
  float   pulseRepFreq;      // pulse repitition freq (/s)
  float   prt;               // sec
  float   prt2;              // sec - for dual and staggered prt
  float   wavelength;        // cm
  float   xmitPeakPower;     // calibrated xmit power - watts
  float   receiverMds;       // dBm
  float   receiverGain;      // dB
  float   antennaGain;       // dB
  float   systemGain;        // dB
  float   unambigVelocity;   // m/s
  float   unambigRange;      // km

  float measXmitPowerDbmH;  // measured H power in dBm
  float measXmitPowerDbmV;  // measured V power in dBm 

  string  radarName;         // name of radar site
  string  scanTypeName;      // name of scanType

};

#endif
