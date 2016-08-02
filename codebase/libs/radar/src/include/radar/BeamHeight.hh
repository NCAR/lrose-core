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
// BeamHeight.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2012
//
///////////////////////////////////////////////////////////////
//
// Compute beam height for normal propagation
//
////////////////////////////////////////////////////////////////

#ifndef BeamHeight_HH
#define BeamHeight_HH

using namespace std;

class BeamHeight {
  
public:

  // constructor
  
  BeamHeight();
  
  // destructor
  
  ~BeamHeight();
 
  // Set the pseudo radius ratio
  
  void setPseudoRadiusRatio(double ratio);

  // set height of radar MSL

  void setInstrumentHtKm(double val) { _instHtKm = val; }
  
  // get height of radar MSL
  
  double getInstrumentHtKm() const { return _instHtKm; }

  // compute ht from elevation angle and slant range
  // Side effect: sets gndRangeKm.
  // Call getGndRangeKm() to get ground range after calling computeHtKm().
  
  double computeHtKm(double elDeg, double slantRangeKm) const;
  
  double getGndRangeKm() const { return _gndRangeKm; }

  // compute elevation angle for given ground distance and ht.
  // Side effect: sets slantRangeKm.
  //
  // Call getSlantRangeKm() to get ground range after calling
  // computeElevationdeg().
  
  double computeElevationDeg(double htKm, double gndRangeKm);
  
  double getSlantRangeKm() const { return _slantRangeKm; }

protected:
private:

  static const double _earthRadiusKm;
  double _pseudoRadiusRatio;
  double _pseudoRadiusKm;
  double _pseudoRadiusKmSq;
  double _pseudoDiamKm;

  double _instHtKm;
  double _htKm;
  double _gndRangeKm;
  double _slantRangeKm;

  double _fx(double elRad);
  double _ht(double elRad);
  
};

#endif
