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
// RadxRangeGeom.hh
//
// Range geometry object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#ifndef RadxRangeGeom_HH
#define RadxRangeGeom_HH

#include <string>
#include <vector>
#include <iostream>
#include <Radx/Radx.hh>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// BASE CLASS FOR RANGE GEOMETRY
/// 
/// This class provides the methods for handling the
/// range geometry for Radx classes

class RadxRangeGeom {
  
public:

  /// constructor
  
  RadxRangeGeom();

  /// copy constructor
  
  RadxRangeGeom(const RadxRangeGeom &rhs);

  /// destructor

  virtual ~RadxRangeGeom();

  /////////////////////////////////////////////////////////////////
  /// assignment
  
  RadxRangeGeom& operator=(const RadxRangeGeom &rhs);
  
  /////////////////////////////////////////////////////////////////
  /// copy the geometry from another object
  
  void copyRangeGeom(const RadxRangeGeom &master);

  /////////////////////////////////////////////////////////////////
  /// clear the geometry data

  virtual void clearRangeGeom();

  /////////////////////////////////////////////////////////////////
  /// set geometry
  
  /// set geometry for constant gate spacing
  
  virtual void setRangeGeom(double startRangeKm,
                            double gateSpacingKm);
  
  //@}

  ////////////////////////////////////////////////////////////////
  /// \name Base class get methods:

  /// has it been set

  inline bool getRangeGeomSet() const { return _rangeGeomSet; }

  /// get the start range, in km

  inline double getStartRangeKm() const { return _startRangeKm; }
  
  /// get the gate spacing, in km

  inline double getGateSpacingKm() const { return _gateSpacingKm; }

  /// get the gate range, in km

  inline double getGateRangeKm(const size_t gate_index) const
    { return _startRangeKm + (_gateSpacingKm * ((double)gate_index + 0.5)); }

  /// get the gate index for the given range

  inline size_t getGateIndex(const double range_km) const
    { return (size_t)(((range_km - _startRangeKm)/ _gateSpacingKm) + 0.5); }

  //@}

  /// print geometry
  
  virtual void print(ostream &out) const;

  /// convert to XML

  void convertToXml(string &xml, int level = 0) const;
  
protected:

  // data

  bool _rangeGeomSet;
  double _startRangeKm; ///< range to center of first gate in km
  double _gateSpacingKm; ///< gate spacing, in km
  
  // methods
  
  void _init();
  RadxRangeGeom & _copy(const RadxRangeGeom &rhs); 

private:
  
};

#endif

