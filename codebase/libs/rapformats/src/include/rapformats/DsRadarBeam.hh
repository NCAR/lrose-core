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
/////////////////////////////////////////////////////////////////////////////
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  April 1998
//
/////////////////////////////////////////////////////////////////////////////
//
// This class handles the storage of a radar beam in a generic
// manner. The type of data in the beam fields (ui08, ui16 or
// fl32), the byte_width (1, 2 or 4), the scale and bias for
// ui08 and ui16 data, are all stored in DsRadarParams.
// Therefore to interpret a beam, you need the associated
// params object.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _DS_RADAR_BEAM_INC_
#define _DS_RADAR_BEAM_INC_

#include <string>
#include <ctime>
#include <rapformats/ds_radar.h>
using namespace std;

class DsRadarBeam
{
public:
  
  DsRadarBeam();
  DsRadarBeam( const DsRadarBeam &other );
  ~DsRadarBeam();
  
  DsRadarBeam& operator=( const DsRadarBeam &other );

  // copy header and data

  void copy( const DsRadarBeam &other );

  // copy the header members only
  
  void copyHeader( const DsRadarBeam &other );

  // get time as a double
  double getDoubleTime() const;

  // get pointer to beam data

  ui08 *data() const { return (ui08 *) _data; }
  void *getData() const { return _data; }

  // get pointer to beam buffer
  // buffer contains header + data

  ui08 *beam() const { return (ui08 *) _beam; }
  void *getBeam() const { return _beam; }
  DsBeamHdr_t *getBeamHdr() const { return (DsBeamHdr_t *) _beam; }

  // get length of data, in bytes

  int dataLen() const { return _dataLen; }
  int getDataNbytes() const { return _dataLen; }

  // get length of beam buffer, in bytes

  int beamLen() const { return _beamLen; }
  int getBeamNBytes() const { return _beamLen; }

  // load up data, in_data_len is in bytes

  void loadData( const void *in_data, int in_data_len, int byte_width );
  void loadData( const ui08 *in_data, int in_data_len );
  
  // decode beam message, load up object

  void decode( int nGatesIn, DsBeamHdr_t *bhdr_msg,
	       int nFields, int nGatesOut );

  // encode returns pointer to start of beam
  // Also use get methods for details of the buffer

  void *encode();
  
  // printing

  void print( FILE *out=stdout ) const;

  void print( ostream &out ) const;

  // print data as raw bytes
  
  void printByteData( FILE *out=stdout ) const;

  // print byte data as floats
  
  void printFloatData( FILE *out,
		       const char *field_name,
		       int field_num,
		       int num_fields,
		       double scale,
		       double bias,
		       int byteWidth = 1,
		       int missingVal = 0 ) const;
  
  void printUi08AsFloats( FILE *out,
			  const char *field_name,
			  int field_num,
			  int num_fields,
			  double scale,
			  double bias,
			  int missingVal = 0 ) const;
  
  // print short data as floats
  
  void printUi16AsFloats( FILE *out,
			  const char *field_name,
			  int field_num,
			  int num_fields,
			  double scale,
			  double bias,
			  int missingVal = 0 ) const;
  
  // print fl32 data

  void printFl32( FILE *out,
		  const char *field_name,
		  int field_num,
		  int num_fields,
		  int missingVal = -9999 ) const;
  
  time_t dataTime;
  int nanoSecs;
  time_t referenceTime;

  int byteWidth;

  int volumeNum;
  int tiltNum;
  int scanMode; // e.g. DS_RADAR_SECTOR_MODE
  bool antennaTransition; // true = in transition
  
  float azimuth;
  float elevation;

  float targetElev;
  float targetAz; // for RHI mode

  bool beamIsIndexed;
  float angularResolution; // for indexed beams
  int nSamples; // number of time series hits in dwell

  float measXmitPowerDbmH;  // measured H power in dBm
  float measXmitPowerDbmV;  // measured V power in dBm 

private:
  
  void *_beam;
  void *_data;
  int _beamLen;
  int _dataLen;

  void _alloc( int in_data_len );

};

#endif
