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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:29:42 $
//   $Id: IconDef.cc,v 1.2 2016/03/04 02:29:42 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * IconDef.cc : IconDef methods.  This class represents a stroked
 *              icon definition for a Symprod object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <vector>

#include <toolsa/mem.h>

#include "IconDef.hh"
using namespace std;



// Global constants


/*********************************************************************
 * Constructors
 */

IconDef::IconDef(const string &icon_name,
		 vector< GridPoint > &icon_points) :
  _iconName(icon_name),
  _numPoints(icon_points.size())
{
  // Create the array of points

  _points = (Symprod::ppt_t *)umalloc(_numPoints * sizeof(Symprod::ppt_t));
  
  vector< GridPoint >::iterator point;
  int i;
  int up = Symprod::PPT_PENUP;
  
  for (point = icon_points.begin(), i = 0;
       point != icon_points.end();
       ++point, ++i)
  {
    
    if (point->x == 32767) {
      _points[i].x = up;
    } else {
      _points[i].x = point->x;
    }
    
    if (point->y == 32767) {
      _points[i].y = up;
    } else {
      _points[i].y = point->y;
    }
  }
  
}


IconDef::IconDef(const IconDef &rhs) :
  _iconName(rhs._iconName),
  _numPoints(rhs._numPoints)
{
  // Create the array of points

  _points = (Symprod::ppt_t *)umalloc(_numPoints * sizeof(Symprod::ppt_t));
  
  int i;
  
  for (i = 0; i < _numPoints; ++i)
  {
    _points[i].x = rhs._points[i].x;
    _points[i].y = rhs._points[i].y;
  }
  
}


/*********************************************************************
 * Destructor
 */

IconDef::~IconDef()
{
  ufree(_points);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
