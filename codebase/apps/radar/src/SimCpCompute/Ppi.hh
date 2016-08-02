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
// Ppi.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#ifndef Ppi_HH
#define Ppi_HH

#include <vector>
#include <iostream>
#include "MomentData.hh"
#include "Params.hh"
#include "Beam.hh"
using namespace std;

////////////////////////
// This class

class Ppi {
  
public:

  typedef enum {
    HORIZ,
    VERT,
    BOTH,
    UNKNOWN
  } xmit_mode_t;
  
  // constructor
  
  Ppi(const Params &params);
  
  // destructor
  
  ~Ppi();
  
  // clear data
  
  void clearData();

  // add a beam
  
  void addBeam(Beam *beam);

  // does an azimuth already exist

  bool azExists(double az);

  // is the ppi complete - i.e. all azimuths exist?

  bool complete();

  // compute properties of this ppi
  
  void computeProps();

  // compute cross-polar power diffs from previous ppi
  // Returns 0 on success, -1 on failure.
  // On success, meanVxMinusHx is set.

  int computeCpDiff(const Ppi &prev,
                    double &meanVcMinusHc,
                    double &meanVxMinusHx);

  // print

  void print(ostream &out);

  // get methods
  
protected:
private:
  
  const Params &_params;

  bool _startFound;
  bool _propsComputed;
  bool _diffsComputed;

  xmit_mode_t _xmitMode;

  int _nAz;
  double _startAz;
  double _endAz;
  double _deltaAz;
  double _meanEl;
  double _meanTime;
  
  double _validCount;
  double _bothValidCount;
  
  double _meanHc;
  double _meanHx;
  double _meanVc;
  double _meanVx;

  double _meanHcMinusHx;
  double _meanHcMinusVc;
  double _meanHcMinusVx;
  double _meanVcMinusHx;
  double _meanVcMinusVx;
  double _meanHxMinusVx;

  double _minHcMinusHx;
  double _minHcMinusVc;
  double _minHcMinusVx;
  double _minVcMinusHx;
  double _minVcMinusVx;
  double _minHxMinusVx;

  double _maxHcMinusHx;
  double _maxHcMinusVc;
  double _maxHcMinusVx;
  double _maxVcMinusHx;
  double _maxVcMinusVx;
  double _maxHxMinusVx;

  double _meanHcMinusPrevVc;
  double _meanVcMinusPrevHc;

  double _meanPrevHcMinusVc;
  double _meanPrevVcMinusHc;

  double _meanHxMinusPrevVx;
  double _meanVxMinusPrevHx;

  double _meanPrevHxMinusVx;
  double _meanPrevVxMinusHx;

  Beam **_beams;

  int _computeAzIndex(double az);

};

#endif

