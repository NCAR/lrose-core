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
// Beam.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2003
//
///////////////////////////////////////////////////////////////

#include "Beam.hh"
#include "Transform.hh"
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <iostream>

using namespace std;

const double Beam::missFl32 = -9999.0;

/////////////////////////////////////////////////
// construct from radarMsg

Beam::Beam(DsRadarMsg &radarMsg,
	   const Params &params,
	   const vector<FieldInfo> &fields,
	   double nyquist_vel) :
  _fields(fields),
  nyquistVel(nyquist_vel),
  fieldData(NULL),
  censorFlag(NULL)
  
{

  const DsRadarBeam &rbeam = radarMsg.getRadarBeam();
  const DsRadarParams &rparams = radarMsg.getRadarParams();
  vector<DsFieldParams *> &fparams = radarMsg.getFieldParams();
  
  time = rbeam.dataTime;

  elev = rbeam.elevation;
  if (params.use_target_elev) {
    elev = rbeam.targetElev;
  }
  if (elev > 180) {
    elev -= 360.0;
  }
  az = rbeam.azimuth;
  targetAz = rbeam.targetAz;
  targetEl = rbeam.targetElev;
  
  if (rbeam.scanMode > 0) {
    scanMode = rbeam.scanMode;
  } else {
    scanMode = rparams.scanMode;
  }
  tiltNum = rbeam.tiltNum;
  volNum = rbeam.volumeNum;
  isIndexed = rbeam.beamIsIndexed;
  angularResolution = rbeam.angularResolution;
  antennaTransition = rbeam.antennaTransition;

  prf = rparams.pulseRepFreq;
  nyquistVel = rparams.unambigVelocity;
  
  nGates = rparams.numGates;
  if (params.remove_test_pulse) {
    nGates -= params.ngates_test_pulse;
  }
  startRange = rparams.startRange;
  gateSpacing = rparams.gateSpacing;
  accept = true;

  byteWidth = rbeam.byteWidth;
  missing = 1;
  fieldData = ucalloc2(_fields.size(), nGates, byteWidth);
  censorFlag = (int *) ucalloc(nGates, sizeof(int));
  
  // copy in the field data
  // order changes from gate-by-gate to field-by-field
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    for (size_t ii = 0; ii < fparams.size(); ii++) {
      if (_fields[ifield].dsrName == fparams[ii]->name) {

	if (byteWidth == 4) {

	  fl32 *inData = (fl32 *) rbeam.data() + ii;
	  fl32 *outData = (fl32 *) fieldData[ifield];
	  for (int igate = 0; igate < nGates;
	       igate++, inData += fparams.size(), outData++) {
	    *outData = *inData;
	  } // igate

	} else if (byteWidth == 2) {

	  ui16 *inData = (ui16 *) rbeam.data() + ii;
	  ui16 *outData = (ui16 *) fieldData[ifield];
	  for (int igate = 0; igate < nGates;
	       igate++, inData += fparams.size(), outData++) {
	    *outData = *inData;
	  } // igate

	} else {

	  // byte width 1
	  
	  ui08 *inData = (ui08 *) rbeam.data() + ii;
	  ui08 *outData = (ui08 *) fieldData[ifield];
	  for (int igate = 0; igate < nGates;
	       igate++, inData += fparams.size(), outData++) {
	    *outData = *inData;
	  } // igate

	}

	break;

      } // if (!strcmp ...
    } // ii
  } // ifield

}
  
/////////////////////////////////////////////////
// construct by interpolating bewteen 2 beams
// Interpolated beam has (weight1)       from beam1,
//                       (1.0 - weight1) from beam2

Beam::Beam(const Beam &beam1, const Beam &beam2, double weight1) :
  _fields(beam1._fields),
  fieldData(NULL),
  censorFlag(NULL)

{

  // compute the min number of gates in the beams
  
  int minGates;
  if (beam1.nGates < beam2.nGates) {
    minGates = beam1.nGates;
  } else {
    minGates = beam2.nGates;
  }
  
  // create the missing beam
  
  time = beam1.time + (beam2.time - beam1.time) / 2;
  double weight2 = 1.0 - weight1;
  az = beam1.az * weight1 + beam2.az * weight2;
  elev = beam1.elev * weight1 + beam2.elev * weight2;
  startRange = beam1.startRange;
  gateSpacing = beam1.gateSpacing;
  nGates = minGates;
  byteWidth = beam1.byteWidth;
  missing = beam1.missing;
  fmissing = beam1.fmissing;
  nyquistVel = beam1.nyquistVel;
  
  fieldData =
    ucalloc2(beam1._fields.size(), nGates, byteWidth);
  
  censorFlag = (int *) ucalloc(nGates, sizeof(int));

  for (size_t ifield = 0; ifield < beam1._fields.size(); ifield++) {
    
    const FieldInfo &fld = beam1._fields[ifield];
    bool interpDbAsPower = fld.interpDbAsPower;
    bool isVel = fld.isVel;
    bool allowInterp = fld.allowInterp;
    
    for (int igate = 0; igate < nGates; igate++) {
      fl32 val1 = beam1.getValue(ifield, igate);
      fl32 val2 = beam2.getValue(ifield, igate);
      fl32 val = Transform::interp(val1, val2, weight1, interpDbAsPower,
				   isVel, nyquistVel, allowInterp);
      setValue(ifield, igate, val);
    }
    
  } // ifield

}
 
/////////////////////////////////////////////////
// destructor

Beam::~Beam()

{
  if (fieldData != NULL) {
    ufree2((void **) fieldData);
  }
  if (censorFlag != NULL) {
    ufree(censorFlag);
  }
}

