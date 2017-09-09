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
/*********************************************************************
 * GridPoint.cc: class implementing grid index points.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include <euclid/GridOffset.hh>
#include <euclid/GridPoint.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

/**********************************************************************
 * Constructors
 */

GridPoint::GridPoint(int x, int y)
{
  setPoint(x, y);
}


GridPoint::GridPoint(GridPoint *point)
{
  setPoint(point);
}


GridPoint::GridPoint(GridPoint *point, GridOffset *offset)
{
  setPoint(point, offset);
}


/**********************************************************************
 * Destructor
 */

GridPoint::~GridPoint(void)
{
}
  

/**********************************************************************
 * rotate() - Rotate the point about the origin by the given angle.
 *            The angle value must be given in degrees.
 */

void GridPoint::rotate(const double angle)
{
  double angle_rad = angle * M_PI / 180.0;
  double cosa = cos(angle_rad);
  double sina = sin(angle_rad);

  double new_x = x * cosa + y * sina;
  double new_y = -x * sina + y * cosa;

  x = (int)(new_x + 0.5);
  y = (int)(new_y + 0.5);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
