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
// RhiTransform.hh
//
// Cartesian POLAR transform object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////
//
// Transform performs the geometric transformation from radar coords
// into the required POLAR coords for the MDV file.
//
//////////////////////////////////////////////////////////////

#ifndef RhiTransform_H
#define RhiTransform_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Transform.hh"
#include "OutputMdv.hh"
#include "FieldInfo.hh"
#include "Beam.hh"

using namespace std;

typedef struct {
  double peakAz;
  double meanAz;
  double minElev;
  double maxElev;
  int nbeams;
} rhi_t;
  
class RhiTransform : public Transform

{
  
public:
  
  RhiTransform (const Params &params,
		const string &mdv_url,
		double oversampling_ratio,
		bool interp_in_elevation,
		const vector<Beam *> &beams,
		const vector<FieldInfo> &fields,
		int n_gates,
		double start_range,
		double gate_spacing,
		double beam_width,
		double radar_lat,
		double radar_lon,
		double radar_alt);
  
  virtual ~RhiTransform();
  
  // load RHI from beams
  // Returns 0 on success, -1 on failure
  
  int load();
  
  // write the output volume
  
  virtual int writeVol(const DsRadarParams &radarParams,
		       time_t startTime, time_t endTime,
		       const vector<FieldInfo> &fields);

protected:

  double _oversamplingRatio;
  bool _interpInElevation;

  const vector<Beam *> &_beams;
  const vector<FieldInfo> &_fields;
  
  // azimuth histogram

  int _nAzHist;
  double _azHistIntv;
  int _azHistSearchWidth;
  int *_azHist;
  vector<rhi_t> _rhiTable; // RHI table in use
  int _nRhi;
  int _nElRhi;
  double _minElRhi, _deltaElRhi;

  vector<Beam *> _beamsInterp;
  vector<vector<Beam *> > _rhiVec;
  Beam ***_rhiArray;
  
  // MDV params
  
  vector<double> _vlevels;

  // functions

  int _loadRhiTable();
  void _computeAzHist();
  void _loadRhiVector();
  void _loadRhiRegularArray();
  void _loadMdvOutput();

  int _azIndex(int i);
  
  void _clearAll();
  void _clearAzHist();
  void _clearRhiArray();
  void _clearBeam(Beam *beam);
  void _clearBeamsInterp();

};

// class to compare RHIs by azimuth

class RhiComparePeakAz {
public:
  bool operator() (rhi_t const &a, rhi_t const &b) const;
};

// class to compare beams by elevation

class BeamCompareElev {
public:
  bool operator() (Beam* const &a, Beam* const &b) const;
};

#endif

