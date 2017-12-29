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
// RadxRemap.hh
//
// Remap range geometry
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////

#ifndef RadxRemap_HH
#define RadxRemap_HH

#include <string>
#include <vector>
#include <iostream>
#include <Radx/Radx.hh>
using namespace std;

class RadxRemap {
  
public:

  /// constructor
  
  RadxRemap();
  
  /// copy constructor
  
  RadxRemap(const RadxRemap &rhs);

  /// destructor

  virtual ~RadxRemap();
  
  /////////////////////////////////////////////////////////////////
  /// assignment
  
  RadxRemap& operator=(const RadxRemap &rhs);
  
  /////////////////////////////////////////////////////////////////
  /// Check if gate spacing is constant, given range array.
  /// Range array is distance to center of each gate.
  ///
  /// Returns true if gate spacing is constant, false otherwise.
  
  static bool checkGateSpacingIsConstant(const vector<double> &rangeArray);
  
  /////////////////////////////////////////////////////////////////
  /// Compute lookup from range array.
  ///
  /// Range array is distance to center of each gate.
  ///
  /// If constant, startRange and gateSpacing are derived from the
  /// vector of ranges.
  ///
  /// If not constant, a lookup vector will be computed for
  /// remapping the non-constant range data onto a constant
  /// delta range.
  ///
  /// Use getGateSpacingIsConstant() to check for constant gate spacing.
  ///     remappingRequired() indicates the need to remapping
  ///     getStartRangeKm() and getGateSpacingKm() if spacing is constant.
  ///     getRangeArray() for saved range array.
  ///     getLookupNearest() for lookup table.
  ///
  /// Returns 0 on success, -1 on error

  int computeRangeLookup(const vector<double> &rangeArray);

  /////////////////////////////////////////////////////////////////
  /// check if gate spacing is constant
  /// Used after call to computeRangeLookup()
  ///
  /// Returns true if gate spacing is constant, false otherwise.

  bool getGateSpacingIsConstant() const { return _gateSpacingIsConstant; }
  
  /// check if the geometry is different given start range and gate spacing
  /// uses an absolute difference of 0.00001 km as the check
  
  static bool checkGeometryIsDifferent(double startRangeKm0,
                                       double gateSpacingKm0,
                                       double startRangeKm1,
                                       double gateSpacingKm1);
  
  ////////////////////////////////////////////////////////////////
  /// Prepare for interpolation, given a difference in geometry.
  ///
  /// Computes lookup tables and weights.
  /// Prepares for both interpolationg and nearest neighbor remapping.
  ///
  /// After running this, you can use:
  ///     remappingRequired() indicates the need to remapping
  ///     getNGatesInterp()
  ///     getStartRangeKm() and getGateSpacingKm() if spacing is constant.
  ///     getRangeArray() for saved range array.
  ///     getLookupNearest() for lookup table.
  ///     getIndexBefore()
  ///     getIndexAfter()
  ///     getWtBefore()
  ///     getWtAfter()

  void prepareForInterp(int oldNGates,
                        double startRangeKm0,
                        double gateSpacingKm0,
                        double startRangeKm1,
                        double gateSpacingKm1);

  //@}

  ////////////////////////////////////////////////////////////////
  /// \name Base class get methods:

  /// is remapping required?

  inline bool remappingRequired() const { return _remappingRequired; }

  /// get the number of gates in lookup table for nearest neighbor

  inline size_t getNGatesNearest() const { return _nearest.size(); }
  
  /// get the number of gates in range array

  inline size_t getNGatesIn() const { return _rangeArray.size(); }
  
  /// get the number of gates after interp

  inline size_t getNGatesInterp() const { return _nGatesInterp; }
  
  /// get the start range, in km

  inline double getStartRangeKm() const { return _startRangeKm; }
  
  /// get the gate spacing, in km

  inline double getGateSpacingKm() const { return _gateSpacingKm; }

  /// get the range vector

  inline const vector<double> &getRangeArray() const { return _rangeArray; }
  
  /// get the lookup vector for nearest neighbor

  inline const vector<int> &getLookupNearest() const { return _nearest; }
  
  /// get the lookup vectors for interpolation
  
  inline const vector<int> &getIndexBefore() const { return _indexBefore; }
  inline const vector<int> &getIndexAfter() const { return _indexAfter; }
  inline const vector<double> &getWtBefore() const { return _wtBefore; }
  inline const vector<double> &getWtAfter() const { return _wtAfter; }
  
  //@}

  //////////////////////////////////////////////////////////////
  /// \name Printing
  //@{
  
  /// print
  
  virtual void print(ostream &out) const;
  
  //@}

protected:

  // data
  
  bool _gateSpacingIsConstant; ///< set by computeRangeLookup()
  double _startRangeKm; ///< range to center of first gate in km
  double _gateSpacingKm; ///< gate spacing, in km
  bool _remappingRequired; ///< must remap onto constant spacing
  vector<double> _rangeArray; ///< range to center of each gate

  // nearest neighbor lookup

  vector<int> _nearest; ///< lookup table for nearest neighbor remapping

  // interpolation lookups

  vector<int> _indexBefore; ///< index of point before remap range
  vector<int> _indexAfter; ///< index of point beyond remap range
  vector<double> _wtBefore; ///< interp wt for point before remap range
  vector<double> _wtAfter; ///< interp wt for point beyond remap range
  size_t _nGatesInterp; ///< n gates after interp

  /// make copy of data members
  
  RadxRemap & _copy(const RadxRemap &rhs); 
  
  // methods
  
  void _init();

private:
  
};

#endif

