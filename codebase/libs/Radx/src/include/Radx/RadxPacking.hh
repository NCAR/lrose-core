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
// RadxPacking.hh
//
// Packing geometry object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////

#ifndef RadxPacking_HH
#define RadxPacking_HH

#include <cassert>
#include <vector>
#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// This class provides the methods for handling the
/// ray/range geometry for RadxField and RadxVol.

class RadxPacking {
  
public:

  /// constructor
  
  RadxPacking();

  /// copy constructor
  
  RadxPacking(const RadxPacking &rhs);

  /// destructor

  virtual ~RadxPacking();

  /////////////////////////////////////////////////////////////////
  /// assignment
  
  RadxPacking& operator=(const RadxPacking &rhs);
  
  /////////////////////////////////////////////////////////////////
  /// copy the geometry from another object
  
  void copyPacking(const RadxPacking &master);
  
  /////////////////////////////////////////////////////////////////
  /// clear the geometry data

  virtual void clearPacking();

  ////////////////////////////////////////////////////////////////
  /// \name Derived class set methods:
  
  /////////////////////////////////////////////////////////
  /// add a ray with nGates
  
  void addToPacking(size_t nGates);
  
  /////////////////////////////////////////////////////////////////
  /// set geom for a vector of rays
  
  void setPacking(const vector<size_t> &rayNGates);

  //@}
  
  ////////////////////////////////////////////////////////////////
  /// \name get methods:

  /// get the number of rays
  
  inline size_t getNRays() const { return _rayNGates.size(); }
  
  /// get nPoints - the sum of the number of gates in all rays
  
  inline size_t getNPoints() const { return _nPoints; }
  
  /// get the number of gates in a ray

  inline size_t getRayNGates(size_t rayNum) const { 
    assert (rayNum < _rayNGates.size());
    return _rayNGates[rayNum];
  }
  
  /// get the max number of gates in any ray

  inline size_t getMaxNGates() const { return _maxNGates; }
  
  /// do the number of gates vary by ray?
  
  inline bool getNGatesVary() const { return _nGatesVary; }
  
  /// get the index to the data in a ray

  inline size_t getRayStartIndex(size_t rayNum) const { 
    assert (rayNum < _rayNGates.size());
    return _rayStartIndex[rayNum];
  }
  
  /// get the number of gates in each ray

  inline const vector<size_t> &getRayNGates() const { 
    return _rayNGates; 
  }

  /// get the start index for the data in each ray
  
  inline const vector<size_t> &getRayStartIndex() const { 
    return _rayStartIndex;
  }

  //@}

  /// print geometry
  
  void printSummary(ostream &out) const;
  void printFull(ostream &out) const;
  
protected:

  // data
  
  size_t _nPoints; ///< sum of number of gates in each ray
  mutable size_t _maxNGates; ///< max number of gates in any ray
  mutable bool _nGatesVary; ///< the number of gates is variable
  vector<size_t> _rayNGates; ///< number of gates per ray
  vector<size_t> _rayStartIndex; ///< index to start of data in ray

  /// make copy of data members

  RadxPacking & _copy(const RadxPacking &rhs); 

  // methods
  
  void _init();

private:
  
};

#endif

