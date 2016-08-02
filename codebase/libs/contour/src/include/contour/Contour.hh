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
// Contour.hh
//
// Class representing a contour of a rectangular grid.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#ifndef Contour_H
#define Contour_H

#include <iostream>
#include <map>

#include <contour/ContourLevel.hh>
#include <dataport/port_types.h>

class BinarySmoother;
class ContourSmoother;
class DouglasPeuckerSmoother;


class Contour
{

  friend class BinarySmoother;
  friend class ContourSmoother;
  friend class DouglasPeuckerSmoother;
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  Contour(const bool& debug_flag = false);

  // destructor
  
  ~Contour();


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * addPolyline() - Add the given polyline to the given level of the
   *                 contour.
   */

  void addPolyline(const double& level_value,
		   const ContourPolyline& polyline);
  

  /*********************************************************************
   * clear() - Clear out the current object information.
   */

  void clear()
  {
    _contourLevels.erase(_contourLevels.begin(),
			 _contourLevels.end());
    
  }
  

  /*********************************************************************
   * getNumLevels() -  Retrieve the number of levels.
   */

  int getNumLevels()
  {
    return _contourLevels.size();   
  }
  

  ///////////////////////
  // Iteration methods //
  ///////////////////////

  /*********************************************************************
   * getFirstLevel() - Get the first level in the contour
   */

  const ContourLevel *getFirstLevel() const
  {
    _contourLevelsIter = _contourLevels.begin();
    
    if (_contourLevelsIter == _contourLevels.end())
      return 0;
    
    return &(*_contourLevelsIter).second;
  }
  

  /*********************************************************************
   * getNextLevel() - Get the next level in the contour
   */

  const ContourLevel *getNextLevel() const
  {
    if (_contourLevelsIter == _contourLevels.end())
      return 0;
    
    ++_contourLevelsIter;
    
    if (_contourLevelsIter == _contourLevels.end())
      return 0;
    
    return &(*_contourLevelsIter).second;
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
  
  // The levels in the contour

  mutable map< double, ContourLevel > _contourLevels;
  mutable map< double, ContourLevel >::iterator _contourLevelsIter;
  
};

#endif
