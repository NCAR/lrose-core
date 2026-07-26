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
// KdpQuadFit.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// with help from ChatGpt 5.
//
// July 2026
//
///////////////////////////////////////////////////////////////
//
// Fit a local quadratic to PHIDP, for each point.
// Compute KDP as the slope at the center of the quadratic.
//
///////////////////////////////////////////////////////////////

#ifndef KdpQuadFit_hh
#define KdpQuadFit_hh

#include <string>
#include <vector>
#include <array>
#include <limits>
#include <iostream>
using namespace std;
class KdpQuadFitParams;

////////////////////////
// This class

class KdpQuadFit {
  
public:

  /**
   * Constructor
   */

  KdpQuadFit();
  
  /**
   * Destructor
   */

  virtual ~KdpQuadFit();

  // compute KDP using quadratic
  // returns 0 on success, -1 on failure
  // call get() methods for results
  
  int compute(const std::vector<double>& phidpDeg,
              double gateSpacingKm,
              int halfWidth,
              const std::vector<double>* quality = nullptr);
  
  // get the results after calling compute
  
  const std::vector<double> &getKdpDegPerKm() const { return _kdpDegPerKm; }
  const std::vector<double> &getPhidpFitDeg() const { return _phidpFitDeg; }
  const std::vector<double> &getResidualStdDeg() const { return _residualStdDeg; }
  const std::vector<int> getNValid() const { return _nValid; }

  // missing value
  
  static constexpr double missingValue() {
    return std::numeric_limits<double>::quiet_NaN();
  }
  
protected:
  
private:

  // data
  
  std::vector<double> _kdpDegPerKm;
  std::vector<double> _phidpFitDeg;
  std::vector<double> _residualStdDeg;
  std::vector<int> _nValid;

  // structs
  
  typedef struct LocalFit_t {
    bool valid = false;
    double intercept = missingValue();
    double slopeDegPerKm = missingValue();
    double curvature = missingValue();
    double residualStdDeg = missingValue();
    int nValid = 0;
  } LocalFit;

  // Unwrap a PHIDP vector whose values are in degrees.
  // Missing values do not update the previous valid phase.

  std::vector<double> _unwrapDegrees(const std::vector<double>& phaseDeg);

  // Solve a 3-by-3 linear system using Gaussian elimination with pivoting.
  
  int _solve3x3(std::array<std::array<double, 3>, 3> matrix,
                std::array<double, 3> rhs,
                std::array<double, 3>& solution);
  
  // Local weighted quadratic fit.
  //
  // x is measured in km relative to the center gate. Using centered x values
  // improves numerical conditioning and makes coefficient[1] the derivative
  // at the center gate.

  LocalFit _fitLocalQuadratic(const std::vector<double>& phidpUnwrapped,
                              const std::vector<double>* quality,
                              std::size_t center,
                              int halfWidth,
                              double gateSpacingKm);

};

#endif

