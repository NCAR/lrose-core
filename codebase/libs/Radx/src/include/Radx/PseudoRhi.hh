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
// PseudoRhi object
//
// RadxRay objects from PPI volume combined in a stack one
// above the other to form a pseudo RHI
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2016
//
///////////////////////////////////////////////////////////////

#ifndef PseudoRhi_HH
#define PseudoRhi_HH

#include <vector>
#include <ostream>
class RadxRay;
using namespace std;

//////////////////////////////////////////////////////////////////////
/// CLASS FOR STORING PSEUDO RHI of RadxRay objects

class PseudoRhi {

public:

  /// Constructor
  
  PseudoRhi();
  
  /// Copy constructor
  
  PseudoRhi(const PseudoRhi &rhs);

  /// Destructor
  
  virtual ~PseudoRhi();

  /// Assignment
  
  PseudoRhi& operator=(const PseudoRhi &rhs);
  
  //////////////////////////////////////////////////////////////////
  /// \name Set methods
  //@{

  /// Set debugging on/off. Off by default.

  void setDebug(bool val);

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Add methods:
  //@{

  /// Add a ray to the volume.
  ///
  /// The ray must be created with new() before calling this method.
  /// PseudoRhi takes responsibility for freeing the object.
  
  void addRay(RadxRay *ray);

  //@}

  //////////////////////////////////////////////////////////////////
  //@{

  /// Compute the mean azimuth angle from the rays
  /// After calling, make call to getMeanAzimuth().

  void computeMeanAzimuthFromRays();

  /// compute the max number of gates
  /// Also determines if number of gates vary by ray.
  /// After this call, use the following to get the results:
  ///   size_t getMaxNGates()
  ///   bool nGatesVary()
  
  void computeMaxNGates() const;
  
  /// Sort the rays by elevation angle, lowest to highest.
  /// Also sets the az of lowest ray,
  /// mean azimuth and max nGates.
  
  void sortRaysByElevation();

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name get methods:
  //@{

  /// get member values

  double getLowLevelAzimuth() const { return _lowLevelAzimuth; }
  double getMeanAzimuth() const { return _meanAzimuth; }

  size_t getMaxNGates() const { return _maxNGates; }
  bool getNGatesVary() const { return _nGatesVary; }

  /// Get vector of rays.

  inline const vector<RadxRay *> &getRays() const { return _rays; }
  inline vector<RadxRay *> &getRays() { return _rays; }
  
  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Clearing data:
  //@{

  /// Clear all of the data in the object.
  
  void clear();
  
  /// Remove all ray on object.

  void clearRays();

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Printing:
  //@{

  /// Print metadata on volume object.
  
  void print(ostream &out) const;
  
  /// Print ray metadata

  void printWithRayMetaData(ostream &out) const;

  /// Print summary of each ray

  void printRaySummary(ostream &out) const;

  /// Print full metadata, and actual field data.

  void printWithFieldData(ostream &out) const;

  //@}

protected:
  
private:

  // debug state

  bool _debug;

  // metadata

  double _lowLevelAzimuth;
  double _meanAzimuth;

  mutable size_t _maxNGates;
  mutable bool _nGatesVary;

  // rays

  vector<RadxRay *> _rays;

  // private methods
  
  void _init();
  PseudoRhi & _copy(const PseudoRhi &rhs);
  void _clearRays();
  
  /// sorting rays by time or azimuth

  class RayPtr {
  public:
    RadxRay *ptr;
    RayPtr(RadxRay *p) : ptr(p) {}
  };
  class SortByRayElevation {
  public:
    bool operator()(const RayPtr &lhs, const RayPtr &rhs) const;
  };

};

#endif
