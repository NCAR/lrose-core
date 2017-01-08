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
// PhidpFilt.hh
//
// Filter utilities for phidp
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2017
//
///////////////////////////////////////////////////////////////

#ifndef PhidpFilt_hh
#define PhidpFilt_hh

#include <cstdio>
#include <vector>
using namespace std;

////////////////////////
// This class

class PhidpFilt {
  
public:

  // constructor

  PhidpFilt();
  
  // destructor
  
  ~PhidpFilt();

  // set range geometry
  
  void setRangeGeometry(double startRangeKm, double gateSpacingKm) {
    _startRangeKm = startRangeKm;
    _gateSpacingKm = gateSpacingKm;
  }

  // compute texture for PHIDP
  // takes account of folding
  // nGates and phidp passed in
  // computes sdev and load into sdevPhidp array
  // sdevPhidp array must be allocated by caller
  //
  // If computeTrendRmse is true, we compute the rmse of
  // the deviation from the trend mean
  //
  // If computeTrendRmse is false, we compute the
  // standard deviation of the phidp
  //
  // if meanPhidp is not nullptr, the mean value is stored there.
  // if texturePhidp is not nullptr, the texture value is stored there.
  
  void computePhidpSdev(int nGatesData,
                        int nGatesKernel,
                        const double *phidp,
                        double missingValue,
                        bool computeTrendRmse = false,
                        double *meanPhidp = NULL,
                        double *texturePhidp = NULL);
  
protected:
  
private:

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // phidp computations

  class PhidpState {
  public:
    void init(double missingValue) {
      missing = true;
      phidp = missingValue;
      xx = 0.0;
      yy = 0.0;
      meanxx = 0.0;
      meanyy = 0.0;
      phidpMean = missingValue;
      phidpTexture = missingValue;
    }
    bool missing;
    double phidp;
    double xx;
    double yy;
    double meanxx;
    double meanyy;
    double phidpMean;
    double phidpTexture;
  };
  
  vector<PhidpState> _phidpStates;
  bool _phidpFoldsAt90;
  double _phidpFoldVal, _phidpFoldRange;

  // private functions
  
  void _computeFoldingRange(int nGates);

};

#endif

