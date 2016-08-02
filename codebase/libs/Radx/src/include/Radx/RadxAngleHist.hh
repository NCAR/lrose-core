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
// RadxAngleHist.hh
//
// Setting scan info from a histogram of scan angles
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#ifndef RadxAngleHist_H
#define RadxAngleHist_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <Radx/RadxRay.hh>
#include <Radx/RadxArray.hh>

using namespace std;

typedef struct {
  double sumAngle;
  double meanAngle;
  int nBeams;
  bool inUse;
} sweep_t;

typedef struct {
  double angle;
  int nBeams;
} sweep_peak_t;

////////////////////////
// This class

class RadxAngleHist {
  
public:

  // constructor
  
  RadxAngleHist();
  
  // destructor
  
  ~RadxAngleHist();

  // set debugging

  void setDebug() { _debug = true; }
  void setVerbose() { _verbose = true; }

  /// set the histogram interval
  /// default is 0.1 deg
  /// 
  /// If the scan strategy has angles very close together you
  /// may need to use a fine resolution

  void setHistInterval(double val) { _histIntv = val; }

  /// Set the histogram search width
  /// The default is 3.0 deg
  ///
  /// This is the width of the search in looking for peaks in the
  /// histogram. When looking for peaks, the program searches by this
  /// number of bins on either side of the search bin. For example, if
  /// the hist_angle_resolution is 0.1 and the hist_angle_search_width
  /// is 3, the program will search 3 bins, or 0.3 degrees, on either
  /// side of the search bin. It looks for a peak with values equal to
  /// or below the peak in the adjacent bins and less than the peak in
  /// bins further out.

  void setHistSearchWidth(int val) { _histSearchWidth = val; }

  /// Set the min number of rays in a sweep
  /// The default is 10
  /// If the histogram suggests that a sweep has fewer rays
  /// than this, we combine it with other sweeps.
  
  void setMinRaysInSweep(int val) { _minRaysInSweep = val; }

  /// set the sweep numbers in the input beams
  /// NOTE: you can use RadxVol::checkIsRhi() to determine whether
  /// a volume is an RHI or not.
  
  void fillInSweepNumbers(const vector<RadxRay *> &rays);

  /////////////////////////////////////////////////////
  /// check if rays are predominantly in RHI mode
  /// Returns true if RHI, false otherwise

  static bool checkIsRhi(const vector<RadxRay *> &rays);

  /////////////////////////////////////////////////////
  /// check if all rays are missing the sweep numbers
  /// Returns true if all sweep numbers are missing.

  static bool checkSweepsNumbersAllMissing(const vector<RadxRay *> &rays);

protected:
private:

  // members
  
  bool _debug;
  bool _verbose;

  // histogram

  bool _isRhi;

  double _histIntv;
  int _histSearchWidth;
  bool _targetAnglesValid;
  int _minRaysInSweep;

  int _nHist;
  int _histOffset;

  vector<int> _hist;
  vector<double> _angleTable;     // angles in use
  vector<int> _sweepTable;        // sweep numbers in use

  // lookup table for sweep indices
  // converts a sweepNum into an index
  // in the _angleTable
  
  vector<int> _sweepIndexLookup;
  
  // functions
  
  void _loadAngleTableFromHist(const vector<RadxRay *> rays);
  int _computeHist(const vector<RadxRay *> rays);
  void _printAngles();
  void _clearHist();
  
};

#endif

