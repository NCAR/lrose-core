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

/************************************************************************
 * GridPoint.hh: class implementing grid index points.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GridPoint_HH
#define GridPoint_HH

#include <iostream>

#include "GridOffset.hh"

class GridPoint
{
 public:

  // Constructors

  GridPoint(int x = 0, int y = 0);
  GridPoint(GridPoint *point);
  GridPoint(GridPoint *point, GridOffset *offset);
  
  // Destructor

  ~GridPoint(void);
  

  ////////////////////
  // Access methods //
  ////////////////////


  inline int getIndex(const int nx, const int ny) const
  {
    return (y * nx) + x;
  }
  
  inline void setPoint(const int x, const int y)
  {
    this->x = x;
    this->y = y;
  }
  
  inline void setPoint(const GridPoint *point)
  {
    x = point->x;
    y = point->y;
  }
  
  inline void setPoint(const GridPoint &point)
  {
    x = point.x;
    y = point.y;
  }
  
  inline void setPoint(const GridPoint *point, const GridOffset *offset)
  {
    x = point->x + offset->x_offset;
    y = point->y + offset->y_offset;
  }
  
  inline void setPoint(const GridPoint &point, const GridOffset &offset)
  {
    x = point.x + offset.x_offset;
    y = point.y + offset.y_offset;
  }
  

  /////////////////////
  // Utility methods //
  /////////////////////

  // Rotate the point about the origin by the given angle.  The angle
  // value must be given in degrees.

  void rotate(const double angle);
  

  ///////////////
  // Operators //
  ///////////////

  bool operator==(const GridPoint &other) const
  {
    return (this->x == other.x &&
	    this->y == other.y);
  }
  

  bool operator!=(const GridPoint &other) const
  {
    return (this->x != other.x ||
	    this->y != other.y);
  }
  

  ////////////////////
  // Public members //
  ////////////////////

  // The actual offset values

  int x;
  int y;
  

 private:

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("GridPoint");
  }
  
};


#endif
