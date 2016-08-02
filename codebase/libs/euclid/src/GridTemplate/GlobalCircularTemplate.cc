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
 * GlobalGridTemplate: class implementing a circular template to be
 *                      applied on gridded data.
 * GlobalCircularTemplate.cc: Extends to work on global lat lon grids.
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

#include <euclid/GlobalCircularTemplate.hh>
#include <euclid/GridOffset.hh>
#include <euclid/GlobalGridTemplate.hh>
using namespace std;

/**********************************************************************
 * Constructor
 */

GlobalCircularTemplate::GlobalCircularTemplate(double radius) :
  GlobalGridTemplate()
{
  // Save the radius

  _radius = radius;
  
  // Create the offsets list

  for (int y = 0; y <= (int)radius; y++)
  {
    for (int x = 0; x <= (int)radius; x++)
    {
      double double_x = (double)x;
      double double_y = (double)y;
      
      double distance = sqrt((double_x * double_x) + (double_y * double_y));
      
      if (distance <= _radius)
      {
	_addOffset(x, y);
	
	if (x != 0 && y != 0)
	  _addOffset(-x, -y);
	
	if (x != 0)
	  _addOffset(-x, y);
	
	if (y != 0)
	  _addOffset(x, -y);
      }
      
    } /* endfor - x */
    
  } /* endfor - y */
  
}


/**********************************************************************
 * Destructor
 */

GlobalCircularTemplate::~GlobalCircularTemplate(void)
{
  // Do nothing
}
  

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void GlobalCircularTemplate::printOffsetList(FILE *stream)
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Circular template with radius %f grid spaces:\n",
	  _radius);
  
  GlobalGridTemplate::printOffsetList(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
