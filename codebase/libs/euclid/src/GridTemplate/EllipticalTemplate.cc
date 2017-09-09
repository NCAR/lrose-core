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
 * EllipticalTemplate.cc: class implementing an elliptical template to be
 *                        applied on gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include <euclid/EllipticalTemplate.hh>
#include <euclid/GridOffset.hh>
#include <euclid/GridTemplate.hh>
#include <toolsa/toolsa_macros.h>

using namespace std;


/**********************************************************************
 * Constructors
 */

EllipticalTemplate::EllipticalTemplate(double rotation_angle,
				       double major_axis,
				       double minor_axis) :
  GridTemplate()
{
  setEllipse(rotation_angle, major_axis, minor_axis);
}


EllipticalTemplate::EllipticalTemplate(const EllipticalTemplate& rhs)
{
  // First, copy the base class information

  vector< GridOffset* >::const_iterator offset_iter;
  
  for (offset_iter = rhs._offsetList.begin();
       offset_iter != rhs._offsetList.end(); ++offset_iter)
    _offsetList.push_back(new GridOffset(*offset_iter));
  
  _pointInGridBase = rhs._pointInGridBase;
  _pointInGridNumX = rhs._pointInGridNumX;
  _pointInGridNumY = rhs._pointInGridNumY;
  _pointInGridReturn = rhs._pointInGridReturn;
  
  // Now, copy this class' information

  _rotationAngle = rhs._rotationAngle;
  _majorAxis = rhs._majorAxis;
  _minorAxis = rhs._minorAxis;
  
}


/**********************************************************************
 * Destructor
 */

EllipticalTemplate::~EllipticalTemplate(void)
{
  // Do nothing
}
  

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void EllipticalTemplate::printOffsetList(FILE *stream)
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Elliptical template:");
  fprintf(stream, "    rotation angle = %f\n", _rotationAngle);
  fprintf(stream, "    major axis = %f\n", _majorAxis);
  fprintf(stream, "    minor axis = %f\n", _minorAxis);
  
  fprintf(stream, " grid points:\n");
  GridTemplate::printOffsetList(stream);
}


/**********************************************************************
 * setEllipse() - Reset the template with a new ellipse.
 */

void EllipticalTemplate::setEllipse(const double rotation_angle,
				    const double major_axis,
				    const double minor_axis)
{
  // Save the template specifications making sure that the major axis is
  // larger than the minor axis.

  _rotationAngle = rotation_angle;
  if (major_axis > minor_axis)
  {
    _majorAxis = major_axis;
    _minorAxis = minor_axis;
  }
  else
  {
    _majorAxis = major_axis;
    _minorAxis = minor_axis;
  }
  
  // Clear out the old offset list

  _offsetList.erase(_offsetList.begin(), _offsetList.end());
  
  // Create the offsets list.  If the ellipse is small then just make the
  // template cover the current grid square

  if (_majorAxis < 1.0)
  {
    _addOffset(0,0);
    return;
  }

  double theta = -1.0 * rotation_angle * DEG_TO_RAD;
    
  double sin_theta = sin(theta);
  double cos_theta = cos(theta);
  
  int major_axis_int = (int)_majorAxis;
  
  for (int x_offset = -major_axis_int; x_offset <= major_axis_int; x_offset++)
  {
    for (int y_offset = -major_axis_int; y_offset <= major_axis_int; y_offset++)
    {
      // Rotate the grid point so we can see if the point lies
      // in the "standard" ellipse orientation.

      double rotated_x = ((double)x_offset * cos_theta) +
	((double)y_offset * sin_theta);

      double rotated_y = (-1.0 * (double)x_offset * sin_theta) +
	((double)y_offset * cos_theta);

      // Use the ellipse equation to see if the point is inside
      // the ellipse.  Because the axis values are given in the
      // number of pixels across the ellipse and we need to use
      // the distance between center points, each axis value
      // must be decremented before it is used.  Also note that
      // I'm using the form of the ellipse equation that produces
      // a vertical ellipse.  I'm doing this because the example
      // in the paper is oriented this direction.

      double semimajor_axis = MAX((double)(_majorAxis - 1) / 2.0, 0.01);
      double semiminor_axis = MAX((double)(_minorAxis - 1) / 2.0, 0.01);
      
      double ellipse_value =
	((rotated_x * rotated_x) / (semiminor_axis * semiminor_axis)) +
	((rotated_y * rotated_y) / (semimajor_axis * semimajor_axis));
	
      if (ellipse_value <= 1.0)
	_addOffset(x_offset, y_offset);
      
    } /* endfor - y_offset */
    
  } /* endfor - x_offset */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
