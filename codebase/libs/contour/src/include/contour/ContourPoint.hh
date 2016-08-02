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
// ContourPoint.hh
//
// Class representing a single point in a contour
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#ifndef ContourPoint_H
#define ContourPoint_H

#include <iostream>

class ContourPoint
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  ContourPoint(const double& x, const double& y,
	       const bool& debug_flag = false);


  // destructor
  
  ~ContourPoint();


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getX() - Retrieve the x-component of the point.
   */

  double getX() const
  {
    return _x;
  }
  

  /*********************************************************************
   * getY() - Retrieve the y-component of the point.
   */

  double getY() const
  {
    return _y;
  }
  

  //////////////////////////
  // Comparison operators //
  //////////////////////////

  inline bool operator==(const ContourPoint& rhs) const
  {
    if (this->_x == rhs._x && this->_y == rhs._y)
      return true;
    
    return false;
  }
  

  inline bool operator!=(const ContourPoint& rhs) const
  {
    if (this->_x != rhs._x || this->_y != rhs._y)
      return true;
    
    return false;
  }
  

  inline bool operator<(const ContourPoint& rhs) const
  {
    if (this->_x < rhs._x)
      return true;
    
    if (this->_x > rhs._x)
      return false;
    
    if (this->_y < rhs._y)
      return true;
    
    return false;
  }
  

  inline bool operator<=(const ContourPoint& rhs) const
  {
    return (*this < rhs || *this == rhs);
  }
  

  inline bool operator>(const ContourPoint& rhs) const
  {
    if (this->_x > rhs._x)
      return true;
    
    if (this->_y < rhs._y)
      return false;
    
    if (this->_x > rhs._x)
      return true;
    
    return false;
  }
  

  inline bool operator>=(const ContourPoint& rhs) const
  {
    return (*this > rhs || *this == rhs);
  }
  

protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  double _x;
  double _y;
  

  /////////////////////
  // Private methods //
  /////////////////////

};

#endif
