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
// PhidpProc.hh
//
// Phidp processing
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2016
//
///////////////////////////////////////////////////////////////

#ifndef PhidpProc_hh
#define PhidpProc_hh

#include <vector>
#include <toolsa/TaArray.hh>
using namespace std;

////////////////////////
// This class

class PhidpProc {
  
public:
  
  // constructor
  
  PhidpProc();
  
  // destructor
  
  ~PhidpProc();

  // set range geometry prior to other computations
  
  void setRangeGeometry(double startRangeKm, double gateSpacingKm) {
    _startRangeKm = startRangeKm;
    _gateSpacingKm = gateSpacingKm;
  }

  // compute standard deviation of phidp in range
  // computed around the circle
  //   nGatesData: number of gates in data
  //   nGatesSdev: number of gates over which we compute the sdev
  //   missingVal: missingDataValue
  
  void computePhidpSdev(int nGatesData, 
                        int nGatesSdev,
                        double *phidp,
                        double missingVal);
  
  // get methods, after calling computeSdev
  
  int getNGatesData() const { return _nGatesData; }
  int getNGatesSdev() const { return _nGatesSdev; }
  const double *getPhidp() const { return _phidp; }
  const double *getPhidpSdev() const { return _phidpSdev; }
  double getMissingVal() const { return _missingVal; }

protected:
  
private:

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // phidp state for computations

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
      phidpSdev = missingValue;
    }
    bool missing;
    double phidp;
    double xx;
    double yy;
    double meanxx;
    double meanyy;
    double phidpMean;
    double phidpSdev;
  };

  vector<PhidpState> _phidpStates;
  bool _phidpFoldsAt90;
  double _phidpFoldVal, _phidpFoldRange;

  // sdev data

  int _nGatesData;
  int _nGatesSdev;
  TaArray<double> _phidp_;
  TaArray<double> _phidpSdev_;

  double *_phidp;
  double *_phidpSdev;
  double _missingVal;

  // private functions
  
  void _computePhidpFoldingRange();

};

#endif

