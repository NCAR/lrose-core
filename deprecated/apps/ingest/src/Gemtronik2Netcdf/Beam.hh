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
//////////////////////////////////////////////////////////
// Beam - class that holds beam data
//
// $Id: Beam.hh,v 1.5 2016/03/07 01:23:00 dixon Exp $
//
/////////////////////////////////////////////////////////

#ifndef _BEAM_DATA_
#define _DEAM_DATA_

#include <vector>
#include <deque>

#include "Params.hh"
#include "Gemtronik2Netcdf.hh"
#include "ReadGemtronik.hh"
#include "InterestMap.hh"

class Beam 
{

public:

  //
  // Constructor
  //
  Beam(Params *params, float elev);

  //
  // Destructor
  //
  ~Beam();

  //
  // Create interest maps.
  //   These are static on the class, and should be created before any
  //   beams are constructed. They are also created only once for
  //   all instances of the class.
  //
  //   params = tdrp parameters
  //
  //   returns 0 on success, -1 on failure
  //
  static int createInterestMaps( Params *params );

  //
  // Compute REC
  //   beams        = deque (or list) of beams that will be used along
  //                  with the current beam to calculate the REC values
  //                  for the given beam
  //   midBeamIndex = the index in the deque of the current 
  //                  (or middle beam)
  //
  void computeRec( const deque< Beam* > beams, int midBeamIndex );


  float computeMinDbz0();

  //
  //
  void addData(ReadGemtronik::VolumeEnum_t  dataType, int nbins, 
	       float datamin, float datascale, float startrange, 
	       float rangestep, float snr_thresh, unsigned char *data);

  void mergeDbz(Beam *prevBeam);

  Beam* getClosestBeam(float currentAz);

  //
  // Constants
  //   DBZ_DIFF_SQ_MAX = maximum allowable value of reflectivity
  //                     difference squared
  //   MISSING_DBL     = bad data value for variables of type double
  //   M_TO_KM         = multiplier used to convert meters to
  //                     kilometers
  static const double DBZ_DIFF_SQ_MAX;
  static const double MISSING_DBL;
  static const double M_TO_KM;
  static const short  DBZ_BAD;
  static const short  VEL_BAD;
  static const short  SW_BAD;
  static const short  ZDR_BAD;
  static const short  PHI_BAD;
  static const short  RHO_BAD;
  static const short  SNR_BAD;
  static const short  PR_BAD;
  static const short  REC_BAD;

  //
  // Get methods
  const ui08*  getDbz() const { return _dbz; }
  const ui08*  getVel() const{ return _vel; };
  const ui08*  getWidth() const{ return _width; }
  const ui08*  getZdr() const { return _zdr; }
  const ui08*  getPhi() const{ return _phi; };
  const ui08*  getRho() const{ return _rho; }
  const ui08*  getSnr() const{ return _snr; }
  const ui08*  getPr() const{ return _pr; }
  const ui08*  getRec() const{ return _rec; }

  float  getAzimuth() const { return _az; }
  int    getNVelGates() const { return _nVelGates; }
  int    getNReflGates() const { return _nReflGates; }
  float  getVelGateWidth() const { return _velGateWidth; }
  float  getReflGateWidth() const { return _reflGateWidth; }
  float  getRangeToFirstVelGate() const { return _rangeToFirstVelGate; }
  float  getRangeToFirstReflGate() const { return _rangeToFirstReflGate; }
  float  getDbzScale() const { return _dbzScale; }
  float  getDbzBias() const { return _dbzBias; }
  short  getDbzBad() const { return DBZ_BAD; }
  float  getVelScale() const { return _velScale; }
  float  getVelBias() const { return _velBias; }
  short  getVelBad() const { return VEL_BAD; }
  float  getSwScale() const { return _swScale; }
  float  getSwBias() const { return _swBias; }
  short  getSwBad() const { return SW_BAD; }
  float  getZdrScale() const { return _zdrScale; }
  float  getZdrBias() const { return _zdrBias; }
  short  getZdrBad() const { return ZDR_BAD; }
  float  getPhiScale() const { return _phiScale; }
  float  getPhiBias() const { return _phiBias; }
  short  getPhiBad() const { return PHI_BAD; }
  float  getRhoScale() const { return _rhoScale; }
  float  getRhoBias() const { return _rhoBias; }
  short  getRhoBad() const { return RHO_BAD; }
  float  getSnrScale() const { return _snrScale; }
  float  getSnrBias() const { return _snrBias; }
  short  getSnrBad() const { return SNR_BAD; }
  float  getPrScale() const { return _prScale; }
  float  getPrBias() const { return _prBias; }
  short  getPrBad() const { return PR_BAD; }
  float  getRecScale() const { return _recScale; }
  float  getRecBias() const { return _recBias; }
  short  getRecBad() const { return REC_BAD; }

private:

  Params *_params;

  //
  // Flags
  bool _recDone;
  bool _saveZdr;
  bool _savePhi;
  bool _saveRho;

  //
  // Information about this beam
  float  _az;
  float  _snrThresh;
  int    _nReflGates;
  int    _nVelGates;
  int    _nSwGates;
  float  _rangeToFirstReflGate;
  float  _rangeToFirstVelGate;
  float  _rangeToFirstSwGate;
  float  _reflGateWidth;
  float  _velGateWidth;
  float  _swGateWidth;
  float  _dbzScale;
  float  _dbzBias;
  float  _velScale;
  float  _velBias;
  float  _swScale;
  float  _swBias;
  float  _zdrScale;
  float  _zdrBias;
  float  _phiScale;
  float  _phiBias;
  float  _rhoScale;
  float  _rhoBias;
  float  _snrScale;
  float  _snrBias;
  float  _prScale;
  float  _prBias;
  float  _recScale;
  float  _recBias;

  //
  // data values
  unsigned char* _dbz;
  unsigned char* _vel;
  unsigned char* _width;
  unsigned char* _zdr;
  unsigned char* _phi;
  unsigned char* _rho;
  unsigned char* _snr;
  unsigned char* _pr;
  unsigned char* _rec;

  //
  // Variables used for the rec computations
  //   _recVarsReady     = flag indicating whether the variables
  //                       that are necessary for computing the
  //                       rec have been calculated or not. 
  //   _recDbzDiffSq     = reflectivity difference squared
  //   _recDbzSpinChange = reflectivity spin change
  // 
  bool    _recVarsReady;
  double *_recDbzDiffSq;
  double *_recDbzSpinChange;


  //
  // Interest maps
  //   Note that these maps are static on the class, since they should
  //   only be computed once
  //    
  //   dbzTextureInterestMap = interest map for dbz texture field
  //   dbzSpinInterestMap    = interest map for dbz spin field
  //   velInterestMap        = interest map for velocity field
  //   widthInterestMap      = interest map for spectrum width field
  //   velSdevInterestMap    = interest map for standard deviation of
  //                           velocity field
  //
  static InterestMap *_dbzTextureInterestMap;
  static InterestMap *_dbzSpinInterestMap;
  static InterestMap *_velInterestMap;
  static InterestMap *_widthInterestMap;
  static InterestMap *_velSdevInterestMap;

  //
  // Store the data
  void _storeData( unsigned char* dataIn, int numGatesIn, unsigned char** dataOut);

  //
  // Look through the deque (or list) of beams that could be used
  // for the computation of the rec for the current beam. Decide
  // how many beams on each side of the current (or middle) beam
  // should be used in the rec computation. Will look for thinks like
  // too big of an azimuth gap between beams, etc.
  //
  //   beams = list of beams for the rec computation associated with
  //           current beam
  //   midBeamIndex = index of middle beam (or our current beam)
  //                  in the list
  //   minIndex     = minimum index of valid beams for the computation,
  //                  output argument
  //   maxIndex     = maximum index of valid beams for the computation,
  //                  output argument
  //
  void _computeRecBeamLimits(const deque<Beam *> beams,
			     int midBeamIndex,
			     int &minIndex,
			     int &maxIndex) const;
  
  //
  // Compute the variables necessary for the rec computation
  //
  void _computeRecVars();

  //
  // Calculate snr
  //   Allocates and fills out the _snr array
  //   using the _dbz data.
  void _computeSnr();

  //
  // Calculate power ratio
  //   Allocates and fills out the _pr array
  //   using the _snr data.
  void _computePR(float unambRange);

  //
  // Compute the difference in degrees between two azimuths
  //   az1 = first azimuth in degrees
  //   az2 = second azimuth in degrees
  //
  double _computeAzDiff(double az1, double az2) const;

  //
  // Convert the interest map to a vector of points
  //   map     = interest map
  //   nPoints = number of points in map
  //   pts     = vector of points
  //
  //   returns 0 on success and -1 on failure
  //
  //   Note that this is a static function that gets called
  //   by computeInterestMaps. Thus this is only used when
  //   the interest maps are created. This is called multiple
  //   times for the multiple interest map, but never gets
  //   called again after the interest maps are computed
  //   for all instances of this class
  //
  static int _convertInterestMapToVector(const Params::interestMapPoint_t *map,
					 int nPoints,
					 vector<InterestMap::ImPoint> &pts);

};

#endif

