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
  double sumAz;
  double peakAz;
  double meanAz;
  double minElev;
  double maxElev;
  int tiltNum;
  int nBeams;
  double fractionFilled;
  bool inUse;
} az_t;
  
class RhiTransform : public Transform

{
  
public:
  
  RhiTransform (const Params &params,
		const string &mdv_url,
		const vector<FieldInfo> &fields,
		double oversampling_ratio,
		bool interp_in_elevation,
		const vector<Beam *> &beamsStored,
		bool scan_info_from_headers,
		int n_gates,
		double start_range,
		double gate_spacing,
                double angular_res,
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
                       const DsRadarCalib *calib,
                       const string &statusXml,
                       int volNum,
                       time_t startTime, time_t endTime,
                       scan_mode_t scanMode = SCAN_MODE_UNKNOWN);

protected:

  double _oversamplingRatio;
  bool _interpInElevation;
  bool _scanInfoFromHeaders;

  const vector<Beam *> &_beamsStored;
  
  // azimuth histogram

  int _nAzHist;
  double _azHistIntv;
  int _azHistSearchWidth;
  int *_azHist;
  vector<az_t> _azTable; // RHI table in use
  int _nEl;
  double _minEl, _maxEl, _deltaEl;

  vector<Beam *> _beamsInterp;
  vector<vector<Beam *> > _beamVec;
  Beam ***_regArray;
  
  // MDV params
  
  vector<double> _vlevels;

  // functions

  void _loadAzTable();
  void _loadAzTableFromTilts();
  void _loadAzTableFromHist();
  void _computeAzHist();
  void _loadBeamVectorFromTilts();
  void _loadBeamVectorFromHist();
  void _sortBeamVector();
  void _loadRegBeamArrayIndexed();
  void _loadRegBeamArrayNonIndexed();
  void _loadMdvOutput();

  int _azIndex(int i);
  
  void _clearAll();
  void _clearAzHist();
  void _clearRegArray();
  void _clearBeam(Beam *beam);
  void _clearBeamsInterp();

};

// class to compare RHIs by azimuth

class RhiCompareAz {
public:
  bool operator() (az_t const &a, az_t const &b) const;
};

// class to compare beams by elevation

class BeamCompareElev {
public:
  bool operator() (Beam* const &a, Beam* const &b) const;
};

#endif

