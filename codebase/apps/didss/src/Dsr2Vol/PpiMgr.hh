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
// PpiMgr.hh
//
// Manager for PPI (plan view) volume
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2007
//
///////////////////////////////////////////////////////////////

#ifndef PpiMgr_H
#define PpiMgr_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Args.hh"
#include "Params.hh"
#include "FieldInfo.hh"
#include "Beam.hh"
#include "BeamGeomMgr.hh"
#include "PlanTransform.hh"
#include "Antenna.hh"
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>

using namespace std;

typedef struct {
  int num;
  double sumElev;
  double meanElev;
  int nBeams;
  double fractionFilled;
  bool inUse;
} tilt_t;

typedef struct {
  double elev;
  int nBeams;
} tilt_peak_t;

// class for comparing tilts, for sorting

class TiltCompare {
public:
  bool operator()(const tilt_t &a, const tilt_t &b) const {
    return a.meanElev < b.meanElev;
  }
};

////////////////////////
// This class

class PpiMgr {
  
public:

  // constructor

  PpiMgr (const string &prog_name,
          const Params &params,
          const vector<FieldInfo> &fields,
          const vector<Beam *> &beamsStored,
          const BeamGeomMgr &beamGeomMgr);

  // destructor
  
  ~PpiMgr();
  
  // process PPI volume
  
  void processPpi(int volNum,
                  scan_mode_t scanMode,
		  bool scan_info_from_headers,
                  const DsRadarParams &predomRadarParams,
                  const DsRadarCalib *radarCalib,
                  const string &statusXml,
                  time_t startTime, time_t endTime,
                  double beamWidth, double nyquist,
                  double radarLat, double radarLon, double radarAlt);

protected:
private:

  // members
  
  string _progName;
  const Params &_params;
  const vector<FieldInfo> &_fields;

  // storing the beams

  const vector<Beam *> &_beamsStored;
  vector<Beam *> _beamsInterp;
  const Beam ***_regularArray;

  // current scan mode
  
  bool _scanInfoFromHeaders;
  double _fractionOfFullCircle;
  
  // beam geometry

  const BeamGeomMgr &_beamGeomMgr;

  // azimuth spacing

  int _naz;
  int _nazPer45;
  double _deltaAzRequested;
  double _deltaAzUsed;

  // elevation histogram
  
  int _nElevHist;
  int _elevHistOffset;
  double _elevHistIntv;
  int _elevHistSearchWidth;
  int *_elevHist;
  int _nElev;
  vector<double> _elevTable;     // elevations in use
  vector<double> _prevElevTable; // elevations in use
  vector<int> _tiltTable;        // tilt numbers in use

  // lookup table for tilt indices
  // converts a tiltNum into an index
  // in the _elevTable
  
  vector<int> _tiltIndexLookup;

  // Transformation objects

  vector<PlanTransform *> _transforms;
  
  // radar info;
  
  double _radarLat, _radarLon, _radarAlt;
  double _beamWidth;
  double _nyquist;

  // functions
  
  bool _setDeltaAz(double angular_res);
  void _loadElevTable();
  void _loadElevTableFromTilts();
  void _loadElevTableFromHist();
  void _calculateElevTable();
  
  void _loadRegularArrayFromTilts();
  void _loadRegularArrayFromHist();

  int _computeElevHist();

  void _bridgeMissingInAzimuth();
  void _bridgeMissingInElevation();
  Beam *_interpBeam(const Beam *beam1, const Beam *beam2,
		      double weight1);
  
  void _printElevs();

  void _clearAll();
  void _clearBeamsInterp();
  void _clearElevHist();
  void _freeRegularArray();
  
};

#endif

