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
 * RectangularTemplate.cc: class implementing a Rectangular template 
 *                      to be applied on gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2007
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include <euclid/RectangularTemplate.hh>
#include <euclid/GridOffset.hh>
#include <euclid/GridTemplate.hh>
using namespace std;

/**********************************************************************
 * Constructor
 */

RectangularTemplate::RectangularTemplate(double length, double width) :
  GridTemplate(),
  _length(length),
  _width(width)
{
  
  // Create the offsets list
  // For the rectangle object, make the start point the bottom
  // left corner instead of the center.

  for (int y = 0; y < (int)(_width); y++)
  {
    for (int x = 0; x < (int)(_length); x++)
    {
	_addOffset(x, y);
	
    } /* endfor x = 0 */
    
  } /* endfor y = 0*/
  
}


/**********************************************************************
 * Destructor
 */

RectangularTemplate::~RectangularTemplate(void)
{
  // Do nothing
}
  

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void RectangularTemplate::printOffsetList(FILE *stream)
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Rectangular template:");
  fprintf(stream, "    length = %f\n", _length);
  fprintf(stream, "    width = %f\n", _width);
  
  fprintf(stream, " grid points:\n");

  GridTemplate::printOffsetList(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
