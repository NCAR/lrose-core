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
// msg31beamData.hh
//
// Deals with message 31 beam data. You pass in params,
// compression type flag, uncompressed data length
// and the buffer that has the original beam data in it.
// The class then puts together the array of data values.
// A method allows resampling onto another spacing.
// 
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef MSG_31_BEAM_H
#define MSG_31_BEAM_H

#include <dataport/port_types.h>

#include "Params.hh"

using namespace std;

class msg31beamData {
  
public:

  // Constructor, from hostname and port, socket IO assumed.
  msg31beamData ( Params *TDRP_params, int compressionFlags, 
	     int compressedLen, unsigned char *buffer );

  // Get status.
  bool isOk();

  // Get number of gates.
  int getNgates();

  // Get data at ith gate (first gate is 0)
  fl32 getGate(int iGate );

  // Get range to first gate
  double getRangeFirstGate();

  // Get gate spacing
  double getGateSpacing();

  // Get field name
  string getFieldName();

  // See if we want this field.
  bool isWanted();

  // Get field number. Depends on what fields we've asked for.
  int getFieldNum();

  // Resample data onto new spacing.
  void resampleData( double firstGateRangeKm,
		     double lastGateRangeKm,
		     double spacingKm);

  // Access to internal members. Set to 0 initially and then
  // overwritten.
  double getLat();
  double getLon();
  double getAlt();
  double getNyquistVel();
  int getVCP();

  // Destructor.
  ~msg31beamData ();

  const static float badVal;

protected:
  
private:

  Params *_params;
  bool _ok;
  bool _wanted;
  double _gateSpacing;
  double _rangeFirstGate;
  int _nGates;
  string _fieldName;
  fl32 *_data;
  int _fieldNum;
  int _vcp;

  void _bSwap(void *i);
  void _bSwapLong(void *i);

  bool _equal(double a, double b);

  double _lat, _lon, _alt;
  double _nyquistVel;

};

#endif





