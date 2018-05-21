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
 * GridTemplate.hh: class implementing a template to be applied to
 *                  gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GridTemplate_HH
#define GridTemplate_HH

#include <vector>
#include <cstdio>
#include "GridOffset.hh"
#include "GridPoint.hh"

using namespace std;

class GridTemplate
{
 public:

  // Constructors

  GridTemplate(void);
  GridTemplate(const GridTemplate& rhs);
  
  // Destructor

  ~GridTemplate(void);
  
  // Methods for iterating through the template within the grid centered
  // on the given point.  To use these methods, first call getFirstInGrid()
  // to get the first point.  Then call getNextInGrid() to get all remaining
  // points until a (GridPoint *)NULL is returned.  Any time getFirstInGrid()
  // is called, the iteration will be cleared and will start over again.
  //
  // base_x and base_y give the coordinates of the point around which the
  // template is to be applied.
  //
  // These routines return a point to a static object which must NOT be
  // deleted by the calling routine.

  GridPoint *getFirstInGrid(const int &base_x, const int &base_y,
			    const int &nx, const int &ny) const;
  
  GridPoint *getNextInGrid(void) const;
  
  // Printing methods

  void printOffsetList(FILE *stream);
  
  // Access methods

  inline void addOffset(const GridOffset &offset)
  {
    _offsetList.push_back(new GridOffset(offset.x_offset,
					 offset.y_offset));
  }
    
  inline void addOffset(const int x_offset, const int y_offset)
  {
    _offsetList.push_back(new GridOffset(x_offset, y_offset));
  }
    
  int size(void) const
  {
    return _offsetList.size();
  }
  
  int getNumPts(void) const
  {
    return _offsetList.size();
  }
  
 protected:

  // The offsets that make up the circle

  vector< GridOffset* > _offsetList;

  // Iterator for finding points within a grid

  mutable vector< GridOffset* >::const_iterator _pointInGridIterator;
  mutable GridPoint _pointInGridBase;
  mutable int _pointInGridNumX;
  mutable int _pointInGridNumY;
  mutable GridPoint _pointInGridReturn;
  
  // Add the given offset to the offset list

  void _addOffset(int x_offset, int y_offset);

};


#endif
