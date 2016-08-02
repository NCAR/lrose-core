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
// AlternatingVelocity.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Compute secondary velocity estimate for alternating mode radars,
// using (a) the alternating mode velocity and (b) the velocity estimated
// from H and V pulses separately. (b) is less noisy than (a),
// but folds at half the nyquist. So we use (a) to unfold (b) to 
// produce a less noisy field which folds at the full nyquist.
// 
// We call this derived product velAlt.
//
///////////////////////////////////////////////////////////////

#ifndef AlternatingVelocity_H
#define AlternatingVelocity_H

#include <string>
#include <vector>
#include <radar/MomentsFields.hh>
class IwrfCalib;
using namespace std;

class AlternatingVelocity {
  
public:

  // constructor
  
  AlternatingVelocity ();

  // destructor
  
  ~AlternatingVelocity();

  // compute velAlt
  //
  // Assumes that the following fields have been computed
  // and are stored in mfields array:
  //
  //   vel - alternating mode velocity
  //   vel_hv - velocity from H and V time series
  //   noise_flag - presence of noise
  //   cpa - clutter phase alignment
  //
  // The following fields will be set by this call:
  //
  //   velAlt
  //   velAlt_fold_interval
  //   velAlt_fold_confidence
  //   vel_diff (vel_hv - velAlt)
  
  void computeVelAlt(int nGates,
                   MomentsFields *mfields,
                   double nyquist);

  // set to load up test fields
  //
  // If this is set to true, the following test fields
  // will be loaded by computeVelAlt():
  //
  //   test2: meanConfidence for gap run
  //   test3: min velocity for a gap run
  //   test4: max velocity for a gap run
  //   test5: corrected velocity for run with incorrect unfold

  void setLoadTestFields(bool state) { _loadTestFields = state; }
  
protected:
private:

  bool _loadTestFields;

  // moments field data

  int _nGates;
  MomentsFields *_mfields;

  // class for computational fields

  class CompFields {
  public:
    int foldInterval;
    int unfoldInterval;
    double velDiff;
    double foldConfidence;
    double meanConfidence;
    double minVelRun;
    double maxVelRun;
    double unfoldedRun;
    double velAlt;
    bool velZero;
    bool velNyquist;
    double fracZero;
    double fracNyquist;
    bool fixFlag;
    int startGate;
    int endGate;
  };

  vector<CompFields> _compFields;
  
  // utility class for gate runs

  class GateRun {
  public:
    GateRun(int start, int end) :
            start(start), end(end)
    {
      len = (end - start) + 1;
      if (start == 0) {
        // fill from back
        mid = 0;
      } else {
        mid = (start + end) / 2;
      }
      minVel = 0.0;
      maxVel = 0.0;
      meanConfidence = 0.0;
    }
    int start, mid, end;
    int len;
    double minVel, maxVel;
    double meanConfidence;
  };

  // velocity unfolding
  
  double _nyquist, _halfNyquist, _twiceNyquist;
  vector<GateRun> _gaps;

  // private methods
  
  void _fixAltModeVel();
  void _findGapRuns();
  void _computeFoldInterval(GateRun &run);
  void _correctBadFold(GateRun &run);
  
};

#endif
