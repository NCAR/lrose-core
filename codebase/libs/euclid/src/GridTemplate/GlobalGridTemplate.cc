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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GlobalGridTemplate.hh: class implementing a template to be applied to
 *                  global lat lon gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * Nov 2006
 *
 * Jason Craig
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include <euclid/GlobalGridTemplate.hh>
#include <euclid/GridOffset.hh>

using namespace std;

/**********************************************************************
 * Constructors
 */

GlobalGridTemplate::GlobalGridTemplate(void)
{
  // Do nothing
}


GlobalGridTemplate::GlobalGridTemplate(const GlobalGridTemplate& rhs)
{
  vector< GridOffset* >::const_iterator offset_iter;
  
  for (offset_iter = rhs._offsetList.begin();
       offset_iter != rhs._offsetList.end(); ++offset_iter)
    _offsetList.push_back(new GridOffset(*offset_iter));
  
  _pointInGridBase = rhs._pointInGridBase;
  _pointInGridNumX = rhs._pointInGridNumX;
  _pointInGridNumY = rhs._pointInGridNumY;
  _pointInGridReturn = rhs._pointInGridReturn;
  
}


/**********************************************************************
 * Destructor
 */

GlobalGridTemplate::~GlobalGridTemplate(void)
{
  // Reclaim the space for the offset list

  vector< GridOffset* >::iterator list_iter;
  for (list_iter = _offsetList.begin(); list_iter != _offsetList.end();
       ++list_iter)
    delete *list_iter;
  
  _offsetList.erase(_offsetList.begin(), _offsetList.end());
  
}
  

/**********************************************************************
 * getFirstInGrid() - Get the first template grid point within the given
 *                    grid.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GlobalGridTemplate::getFirstInGrid(const int &base_x, const int &base_y,
					const int &nx, const int &ny) const
{
  // Set up the iterator and save the grid information

  _pointInGridIterator = _offsetList.begin();
  
  _pointInGridBase.x = base_x;
  _pointInGridBase.y = base_y;
  
  _pointInGridNumX = nx;
  _pointInGridNumY = ny;
  
  // Send back the first point

  return getNextInGrid();
}
  

/**********************************************************************
 * getNextInGrid() - Get the next template grid point within the grid.
 *                   Returns NULL when there are no more points in the
 *                   grid.
 *
 * Returns a pointer to a static object which must NOT be deleted by the
 * calling routine.
 */

GridPoint *GlobalGridTemplate::getNextInGrid(void) const
{
  while (_pointInGridIterator != _offsetList.end())
  {
    GridOffset *offset = *_pointInGridIterator;
    
    _pointInGridIterator++;
    
    _pointInGridReturn.x = _pointInGridBase.x + offset->x_offset;
    _pointInGridReturn.y = _pointInGridBase.y + offset->y_offset;

    //    if (_pointInGridReturn.x >= 0 &&
    //_pointInGridReturn.x < _pointInGridNumX &&
    //_pointInGridReturn.y >= 0 &&
    //_pointInGridReturn.y < _pointInGridNumY)
    //{
      return &_pointInGridReturn;
      //}
    
  }
  
  return (GridPoint *)NULL;
}
  

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void GlobalGridTemplate::printOffsetList(FILE *stream)
{
  vector< GridOffset* >::iterator ol_iterator;
  
  for (ol_iterator = _offsetList.begin();
       ol_iterator != _offsetList.end();
       ol_iterator++)
  {
    GridOffset *offset = *ol_iterator;

    double x = (double)offset->x_offset;
    double y = (double)offset->y_offset;
    
    double distance = sqrt((x * x) + (y * y));
    
    fprintf(stream, " %4d %4d   %f\n",
	    offset->x_offset, offset->y_offset, distance);
    
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addOffset() - Add the given offset to the offset list.
 */

void GlobalGridTemplate::_addOffset(int x_offset, int y_offset)
{
  GridOffset *offset = new GridOffset(x_offset, y_offset);
  
  _offsetList.push_back(offset);
  
  return;
}


