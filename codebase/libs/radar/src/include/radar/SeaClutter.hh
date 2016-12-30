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
// SeaClutter.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2016
//
///////////////////////////////////////////////////////////////
//
// Locate gates contaminated with sea clutter
//
///////////////////////////////////////////////////////////////

#ifndef SeaClutter_H
#define SeaClutter_H

#include <pthread.h>
#include <vector>
#include <radar/InterestMap.hh>
#include <radar/PhidpProc.hh>
#include <toolsa/TaArray.hh>
using namespace std;

class SeaClutter {
  
public:

  // constructor
  
  SeaClutter();

  // destructor
  
  ~SeaClutter();

  // set debugging state

  void setDebug(bool state) { _debug = state; }

  //////////////////////////////////////////////////////

  // set size of kernel for computing stats

  void setNGatesKernel(int val) {
    _nGatesKernel = val;
  }

  // max elevation angle for sea clutter

  void setMaxElevDeg(double val) { _maxElevDeg = val; }

  // only include gates that exceed a given SNR threshold
  
  void setMinSnrDb(double val) { _minSnrDb = val; }

  // set interest maps for sea clutter location

  // mean rhohv in range

  void setInterestMapRhohvMean
    (const vector<InterestMap::ImPoint> &pts,
     double weight);

  // standard deviation of zdr in range

  void setInterestMapZdrSdev
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  // standard deviation of phidp in range

  void setInterestMapPhidpSdev
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  // gradient of reflectivity in elevation polar space

  void setInterestMapDbzElevGradient
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  // interest threshold for seaclutter identification

  void setClutInterestThreshold(double val);

  // set radar props
  
  void setRadarHtM(double val) { _radarHtM = val; }
  void setWavelengthM(double val) { _wavelengthM = val; }
  
  ///////////////////////////////////////////////////////////////
  // set the ray properties
  // must be called before locate()
  // Set the nyquist if it is known.
  
  void setRayProps(time_t timeSecs, 
                   double nanoSecs,
                   double elevation, 
                   double azimuth,
                   int nGates,
                   double startRangeKm,
                   double gateSpacingKm);

  ///////////////////////////////////////////////////////////////
  // set the missing value for fields
  
  void setFieldMissingVal(double val) { _missingVal = val; }
  
  ///////////////////////////////////////////////////////////////
  // set the fields as available
  
  void setSnrField(double *vals);
  void setDbzField(double *vals, double noiseDbzAt100km);
  void setRhohvField(double *vals);
  void setPhidpField(double *vals);
  void setZdrField(double *vals);
  void setDbzElevGradientField(double *vals);
  
  //////////////////////////////////////////////
  // perform clutter location
  //
  // Must call setRayProps first, and set the fields.
  //
  // Min fields required:
  //
  //   snr or dbz
  //   rhohv
  //   phidp
  //   zdr
 
  int locate();

  // get input fields - after setting ray props and fields

  const double *getSnr() const { return _snr; }
  const double *getDbz() const { return _dbz; }
  const double *getRhohv() const { return _rhohv; }
  const double *getPhidp() const { return _phidp; }
  const double *getZdr() const { return _zdr; }
  const double *getDbzElevGradient() const { return _dbzElevGradient; }
  
  // get results - after running locate
  // these arrays span the gates from 0 to nGates-1
  
  const bool *getClutFlag() const { return _clutFlag; }
  
  const double *getSnrMean() const { return _snrMean; }
  const double *getRhohvMean() const { return _rhohvMean; }
  const double *getPhidpSdev() const { return _phidpSdev; }
  const double *getZdrSdev() const { return _zdrSdev; }
  
  const double *getRhohvMeanInterest() const { return _rhohvMeanInterest; }
  const double *getPhidpSdevInterest() const { return _phidpSdevInterest; }
  const double *getZdrSdevInterest() const { return _zdrSdevInterest; }
  const double *getDbzElevGradientInterest() const { return _dbzElevGradientInterest; }

  ////////////////////////////////////
  // print parameters for debugging
  
  void printParams(ostream &out);

protected:
private:

  // debugging
  
  bool _debug;

  // radar properties

  double _radarHtM;
  double _wavelengthM;

  // ray properties

  time_t _timeSecs;
  double _nanoSecs;
  double _elevation;
  double _azimuth;
  int _nGates;

  double _startRangeKm;
  double _gateSpacingKm;

  // field missing value

  double _missingVal;

  // compute phidp sdev

  PhidpProc _phidpProc;

  // arrays for input and computed data
  // and pointers to those arrays

  TaArray<double> _snr_;
  double *_snr;
  bool _snrAvail;
  
  TaArray<double> _dbz_;
  double *_dbz;
  bool _dbzAvail;
  
  TaArray<double> _rhohv_;
  double *_rhohv;
  bool _rhohvAvail;
  
  TaArray<double> _phidp_;
  double *_phidp;
  bool _phidpAvail;
  
  TaArray<double> _zdr_;
  double *_zdr;
  bool _zdrAvail;

  TaArray<double> _dbzElevGradient_;
  double *_dbzElevGradient;
  bool _dbzElevGradientAvail;

  // results

  TaArray<bool> _clutFlag_;
  bool *_clutFlag;

  TaArray<double> _snrMean_;
  double *_snrMean;

  TaArray<double> _rhohvMean_;
  double *_rhohvMean;

  TaArray<double> _phidpSdev_;
  double *_phidpSdev;

  TaArray<double> _zdrSdev_;
  double *_zdrSdev;

  TaArray<double> _rhohvMeanInterest_;
  double *_rhohvMeanInterest;

  TaArray<double> _phidpSdevInterest_;
  double *_phidpSdevInterest;

  TaArray<double> _zdrSdevInterest_;
  double *_zdrSdevInterest;

  TaArray<double> _dbzElevGradientInterest_;
  double *_dbzElevGradientInterest;

  // gate limits for computing stats along a ray

  vector<size_t> _startGate;
  vector<size_t> _endGate;

  // fuzzy logic for clutter detection

  int _nGatesKernel;
  double _minSnrDb;
  double _maxElevDeg; // max elevation angle for sea clutter

  InterestMap *_interestMapRhohvMean;
  InterestMap *_interestMapPhidpSdev;
  InterestMap *_interestMapZdrSdev;
  InterestMap *_interestMapDbzElevGradient;
  
  double _weightRhohvMean;
  double _weightPhidpSdev;
  double _weightZdrSdev;
  double _weightDbzElevGradient;
  
  double _clutInterestThreshold;

  // private methods
  
  void _computeSnrFromDbz(double noiseDbzAt100km);
  void _computeMeanInRange(const double *vals, double *means);
  void _computeSdevInRange(const double *vals, double *sdevs);
  void _createDefaultInterestMaps();

};

#endif
