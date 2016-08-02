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
// PlanTransform.hh
//
// Transforms for plan view objects (CART, PPI, POLAR)
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////
//
// PlanTransform performs the geometric transformation from radar coords
// into the required plan-view coords for the MDV file.
//
//////////////////////////////////////////////////////////////

#ifndef PlanTransform_H
#define PlanTransform_H

#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <rapformats/DsRadarMsg.hh>
#include "Params.hh"
#include "FieldInfo.hh"
#include "Beam.hh"
#include "BeamGeom.hh"
#include "OutputMdv.hh"
#include "Transform.hh"
using namespace std;

// lookup table struct

typedef struct {
  si16 ix, iy, iz;
  si16 igate, iaz, ielev;
  fl32 wtGate, wtAz, wtElev;
} plan_transform_lut_t;

#define PSEUDO_DIAM 17066.0 // for curvature correction

////////////////////////////////
// PlanTransform abstract base class

class PlanTransform : public Transform

{
  
public:

  PlanTransform (const Params &params,
		 const string &mdv_url);
  
  virtual ~PlanTransform();
  
  // public functions
  
  virtual int setGeom(const vector<double> vlevel_array,
		      int n_az,
		      int n_az_per_45,
		      double delta_az,
		      int n_gates,
		      double start_range,
		      double gate_spacing,
		      double beam_width,
		      double radar_lat,
		      double radar_lon,
		      double radar_alt) = 0;

  virtual void calcLookup() = 0;
  
  virtual void doTransform(const vector<FieldInfo> &fields,
			   double nyquist,
			   Beam ***beam_array) = 0;

  virtual int writeVol(const DsRadarParams &radarParams,
		       time_t startTime, time_t endTime,
		       const vector<FieldInfo> &fields);

  void freeLut();
  
protected:

  // elev angles

  int _nElev;
  vector<double> _elevArray;

  // azimuth spacing

  int _naz;
  double _deltaAz;

  // lookup table

  vector<plan_transform_lut_t> _lut;
  
  // arrays for converting the lookup for the first
  // sector to the other 7 sectors
  
  static const int _azSign[8];
  static const int _xSign[8];
  static const int _ySign[8];
  static const bool _swapXy[8];
  int _azOffset[8];
  
private:

};

#endif


