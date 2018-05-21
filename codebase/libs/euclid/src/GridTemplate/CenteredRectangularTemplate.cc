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
//   $Date: 2017/11/08 20:59:03 $
//   $Id: CenteredRectangularTemplate.cc,v 1.2 2017/11/08 20:59:03 mccabe Exp $
//   $Revision: 1.2 $
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

CenteredRectangularTemplate::CenteredRectangularTemplate(double length, double height) :
  GridTemplate()
{
  _length = length;
  _height = height;

  if(int(length) % 2 == 0) {
    cerr << "WARNING: Using even value for length (" << length
	 << ") will actually use length = " << length+1 << endl;
  }

  if(int(height) % 2 == 0) {
    cerr << "WARNING: Using even value for height (" << height
	 << ") will actually use height = " << height+1 << endl;
  }  
  
  // Create the offsets list

  for (int y = 0; y <= (int)(_height * 0.5); y++)
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
  fprintf(stream, "    height = %f\n", _height);
  
  fprintf(stream, " grid points:\n");

  GridTemplate::printOffsetList(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
