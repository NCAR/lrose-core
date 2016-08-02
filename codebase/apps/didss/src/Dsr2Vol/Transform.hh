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
// Transform.hh
//
// Abstract base class for all transform objects
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////
//
// Transform performs the geometric transformation from radar coords
// into the required coords for the MDV file.
//
//////////////////////////////////////////////////////////////

#ifndef Transform_H
#define Transform_H

#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <rapformats/DsRadarMsg.hh>
#include "Params.hh"
#include "FieldInfo.hh"
#include "Beam.hh"
#include "BeamGeom.hh"
#include "OutputMdv.hh"
using namespace std;

typedef enum {
  SCAN_MODE_UNKNOWN,
  SCAN_MODE_SURVEILLANCE,
  SCAN_MODE_SECTOR,
  SCAN_MODE_PPI,
  SCAN_MODE_RHI,
  SCAN_MODE_VERT
} scan_mode_t;

////////////////////////////////
// Transform abstract base class

class Transform

{
  
public:

  // constructor specifies output URL
  
  Transform(const Params &params,
            const string &mdv_url,
            const vector<FieldInfo> &fields);
  
  // destructor

  virtual ~Transform();

  // get geom type

  output_grid_geom_t getGeomType() { return _geomType; }

  // set geom type

  void setGeomType(output_grid_geom_t gtype) { _geomType = gtype; }

  // free up output fields
  
  void freeOutputFields();

  // write volume
  
  virtual int writeVol(const DsRadarParams &radarParams,
                       const DsRadarCalib *radarCalib,
                       const string &statusXml,
                       int volNum,
		       time_t startTime, time_t endTime,
                       scan_mode_t scanMode = SCAN_MODE_UNKNOWN) = 0;
  
  // in-line interp function for performance
  //
  // Default replacement val is for DBZ
  
  static inline fl32 interp(fl32 val1, fl32 val2, fl32 wt1,
			    bool interpDbAsPower,
			    bool isVel, double nyquist,
			    bool allowInterp,
			    bool replaceMissing = false,
			    fl32 replaceVal = -40.0) {
    
    // take care of missing data conditions
    
    if (val1 == missFl32 && val2 == missFl32) {
      
      return missFl32;
      
    } else if (val1 == missFl32) {

      if (replaceMissing) {
	val1 = replaceVal;
      } else {
	return val2;
      }
      
    } else if (val2 == missFl32) {
      
      if (replaceMissing) {
	val2 = replaceVal;
      } else {
	return val1;
      }

    }
    
    // can we interpolate?
    
    if (!allowInterp) {
      if (wt1 >= 0.5) {
	return val1;
      } else {
	return val2;
      }
    }

    // both values are valid, so interpolate

    if (interpDbAsPower) {
      
      double power1 = pow(10.0, val1 / 10.0);
      double power2 = pow(10.0, val2 / 10.0);
      return 10.0 * log10((power1 + power2) / 2.0);
      
    } else if (isVel) {

      double diff = fabs(val1 - val2);
      if (diff < nyquist) {
	return (val1 * wt1) + (val2 * (1.0 - wt1));
      } else {
	if (val1 > val2) {
	  val2 += 2.0 * nyquist;
	} else {
	  val1 += 2.0 * nyquist;
	}
	double val = (val1 * wt1) + (val2 * (1.0 - wt1));
	if (val > nyquist) {
	  val -= 2.0 * nyquist;
	} else if (val < -nyquist) {
	  val += 2.0 * nyquist;
	}
	return val;
      }

    } else {

      return (val1 * wt1) + (val2 * (1.0 - wt1));

    }

  }

  const static double missFl32;

protected:

  void _allocOutputFields();

  // members
  
  const Params &_params;
  string _mdvUrl;
  const vector<FieldInfo> &_fields;
  output_grid_geom_t _geomType;

  // fields
  
  int _nFieldsOut;
  fl32 **_outputFields;
  fl32 *_coverage;

  // grid params
  
  int _nx, _ny, _nz;
  double _dx, _dy, _dz;
  double _minx, _miny, _minz;

  // gates
  
  int _nGates;
  double _startRange;
  double _gateSpacing;
  double _angularRes;

  // radar params

  double _radarLat;
  double _radarLon;
  double _radarAlt;
  double _beamWidth;
  
private:

};

#endif


