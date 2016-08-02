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
// Beam object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2003
//
///////////////////////////////////////////////////////////////

#ifndef Beam_H
#define Beam_H

#include <cmath>
#include <vector>
#include <dataport/port_types.h>
#include <rapformats/DsRadarMsg.hh>
#include "FieldInfo.hh"
#include "Params.hh"

using namespace std;

// beam geometry container class

class Beam {

public:

  // construct from DsRadarMsg

  Beam(DsRadarMsg &radarMsg,
       const Params &params,
       const vector<FieldInfo> &fields,
       double nyquist_vel);
  
  // construct by interpolating bewteen 2 beams
  
  Beam(const Beam &beam1, const Beam &beam2, double weight1);
  
  // destructor

  ~Beam();

  // get field value as an fl32, with missFl32 (-9999) as the
  // missing value
  
  inline fl32 getValue(int ifield, int igate) const {
    
    const FieldInfo &fld = _fields[ifield];

    double dval;

    if (byteWidth == 4) {
      
      fl32 **data = (fl32 **) fieldData;
      fl32 val = data[ifield][igate];
      if (val == (fl32) fld.missingDataValue) {
	return missFl32;
      } else {
	dval = val;
      }
      
    } else if (byteWidth == 2) {

      ui16 **data = (ui16 **) fieldData;
      ui16 val = data[ifield][igate];
      if (val == fld.missingDataValue) {
	return missFl32;
      } else {
	dval = val * fld.scale + fld.bias;
      }
      
    } else {

      ui08 **data = (ui08 **) fieldData;
      ui08 val = data[ifield][igate];
      if (val == fld.missingDataValue) {
	return missFl32;
      } else {
	dval = val * fld.scale + fld.bias;
      }
      
    }

    return (fl32) dval;

  }

  // set field value
  
  inline void setValue(int ifield, int igate, fl32 val) {
    
    const FieldInfo &fld = _fields[ifield];
    
    if (byteWidth == 4) {
      fl32 **data = (fl32 **) fieldData;
      data[ifield][igate] = val;
    } else if (byteWidth == 2) {
      int shortVal = (int) ((val - fld.bias) / fld.scale + 0.5);
      if (shortVal < 0) {
	shortVal = 0;
      } else if (shortVal > 65535) {
	shortVal = 65535;
      }
      ui16 **data = (ui16 **) fieldData;
      data[ifield][igate] = (ui16) shortVal;
    } else {
      int byteVal = (int) ((val - fld.bias) / fld.scale + 0.5);
      if (byteVal < 0) {
	byteVal = 0;
      } else if (byteVal > 255) {
	byteVal = 255;
      }
      ui08 **data = (ui08 **) fieldData;
      data[ifield][igate] = (ui08) byteVal;
    }

  }
  
  // set field value as missing
  
  inline void setMissing(int ifield, int igate) {
    
    const FieldInfo &fld = _fields[ifield];
    
    if (byteWidth == 4) {
      fl32 **data = (fl32 **) fieldData;
      data[ifield][igate] = (fl32) fld.missingDataValue;
    } else if (byteWidth == 2) {
      ui16 **data = (ui16 **) fieldData;
      data[ifield][igate] = (ui16) fld.missingDataValue;
    } else {
      ui08 **data = (ui08 **) fieldData;
      data[ifield][igate] = (ui08) fld.missingDataValue;
    }

  }
  
  // data members

  const static double missFl32;
  const vector<FieldInfo> &_fields;

  time_t time;
  fl32 az, elev;
  fl32 prf;
  fl32 startRange;
  fl32 gateSpacing;
  int nGates;
  bool accept;
  int byteWidth;
  int missing;
  fl32 fmissing;
  int scanMode;
  double nyquistVel;
  void **fieldData;

};

#endif

