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
// ContourLevel.hh
//
// Class representing a single level of a contour of a
// rectangular grid.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#ifndef ContourLevel_H
#define ContourLevel_H

#include <iostream>
#include <vector>

#include <contour/ContourPolyline.hh>

class BinarySmoother;
class ContourSmoother;
class DouglasPeuckerSmoother;

class ContourLevel
{
  
  friend class BinarySmoother;
  friend class ContourSmoother;
  friend class DouglasPeuckerSmoother;
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /*********************************************************************
   * Constructors
   */

  ContourLevel(const double& level_value = 0.0,
	       const bool& debug_flag = false);


  /*********************************************************************
   * Destructor
   */
  
  ~ContourLevel();


  ///////////////////////
  // Iteration methods //
  ///////////////////////

  /*********************************************************************
   * getFirstPolyline() - Retrieve the first polyline in the level.
   *
   * Returns a pointer to the first polyline in the level, or 0 if there
   * are no polylines in the level.
   */

  ContourPolyline *getFirstPolyline() const
  {
    _contourPolylinesIter = _contourPolylines.begin();
    
    if (_contourPolylinesIter == _contourPolylines.end())
      return 0;
    
    return &(*_contourPolylinesIter);
  }

  
  /*********************************************************************
   * getNextPolyline() - Retrieve the next polyline in the level.
   *
   * Returns a pointer to the next polyline in the level, or 0 if there
   * are no more polylines in the level.
   */

  ContourPolyline *getNextPolyline() const
  {
    if (_contourPolylinesIter == _contourPolylines.end())
      return 0;
    
    ++_contourPolylinesIter;
    
    if (_contourPolylinesIter == _contourPolylines.end())
      return 0;
    
    return &(*_contourPolylinesIter);
  }

  
  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * addPolyline() - Add the given polyline to the contour level.
   */

  void addPolyline(const ContourPolyline& polyline)
  {
    _contourPolylines.push_back(polyline);
  }

  
  /*********************************************************************
   * getNumPolylines() - Retrieve the number of polylines in the level.
   */

  int getNumPolylines() const
  {
    return _contourPolylines.size();
  }

  
  /*********************************************************************
   * getLevelValue() - Retrieve the value of the level.
   */

  double getLevelValue() const
  {
    return _levelValue;
  }

  
  ////////////////////
  // Output methods //
  ////////////////////

  /*********************************************************************
   * print() - Print the contents of the contour object to the given
   *           stream.
   */

  void print(ostream &out) const;
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debugFlag;
  
  double _levelValue;
  
  mutable vector< ContourPolyline > _contourPolylines;
  mutable vector< ContourPolyline >::iterator _contourPolylinesIter;
  

};

#endif
