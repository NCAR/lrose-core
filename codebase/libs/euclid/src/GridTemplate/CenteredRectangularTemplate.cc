/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1995, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: mccabe $
//   $Locker:  $
//   $Date: 2017/10/30 18:56:20 $
//   $Id: CenteredRectangularTemplate.cc,v 1.1 2017/10/30 18:56:20 mccabe Exp $
//   $Revision: 1.1 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CenteredRectangularTemplate.cc: class implementing a Rectangular template 
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

#include <euclid/CenteredRectangularTemplate.hh>
#include <euclid/GridOffset.hh>
#include <euclid/GridTemplate.hh>
using namespace std;

/**********************************************************************
 * Constructor
 */

CenteredRectangularTemplate::CenteredRectangularTemplate(double length, double width) :
  GridTemplate()
{
  // Save the radius

  _length = length;
  _width = width;
  
  // Create the offsets list

  for (int y = 0; y <= (int)(_width * 0.5); y++)
  {
    for (int x = 0; x <= (int)(_length * 0.5); x++)
    {
	_addOffset(x, y);
	
	if (x != 0 && y != 0)
	    _addOffset(-x, -y);
	
	if (x != 0)
	    _addOffset(-x, y);
	
	if (y != 0)
	    _addOffset(x, -y);
      
    } /* endfor - x */
    
  } /* endfor - y */
  
}


/**********************************************************************
 * Destructor
 */

CenteredRectangularTemplate::~CenteredRectangularTemplate(void)
{
  // Do nothing
}
  

/**********************************************************************
 * printOffsetList() - Print the offset list to the given stream.  This
 *                     is used for debugging.
 */

void CenteredRectangularTemplate::printOffsetList(FILE *stream)
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Centered Rectangular template:");
  fprintf(stream, "    length = %f\n", _length);
  fprintf(stream, "    width = %f\n", _width);
  
  fprintf(stream, " grid points:\n");

  GridTemplate::printOffsetList(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
