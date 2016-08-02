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
 * MdvGrid.cc: MdvGrid object code.  This object manipulates a grid
 *             object for the Mdv class.  This was created as a
 *             separate object so that grids could be compared between
 *             fields and MDV objects using the "==" operator.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cerrno>
#include <cassert>
#include <cstdlib>

#include <Mdv/mdv/mdv_print.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>

#include <Mdv/mdv/Mdv.h>
using namespace std;

/*
 * Global variables
 */



/*********************************************************************
 * Constructors
 */

MdvGrid::MdvGrid(const double minx,
		 const double miny,
		 const double minz,
		 const double delta_x,
		 const double delta_y,
		 const double delta_z,
		 const int nx,
		 const int ny,
		 const int nz,
		 const int proj_type,
		 MdvDebugLevel debug_level)
{
  static const char *routine_name = "Constructor";
  
  if (debug_level >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the debug level.  Do this first in case any other
  // routines need it.

  _debugLevel = debug_level;

  // Save the grid parameters

  _minX = minx;
  _minY = miny;
  _minZ = minz;
  
  _deltaX = delta_x;
  _deltaY = delta_y;
  _deltaZ = delta_z;
  
  _nX = nx;
  _nY = ny;
  _nZ = nz;
  
  _projType = proj_type;
  
  // Set the tolerance value

  _tolerance = _setTolerance();
  
}


MdvGrid::MdvGrid(const MdvGrid *grid_ptr)
{
  static const char *routine_name = "Constructor";
  
  if (grid_ptr->_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the grid parameters

  _minX = grid_ptr->_minX;
  _minY = grid_ptr->_minY;
  _minZ = grid_ptr->_minZ;
  
  _deltaX = grid_ptr->_deltaX;
  _deltaY = grid_ptr->_deltaY;
  _deltaZ = grid_ptr->_deltaZ;
  
  _nX = grid_ptr->_nX;
  _nY = grid_ptr->_nY;
  _nZ = grid_ptr->_nZ;
  
  _projType = grid_ptr->_projType;
  
  // Set the tolerance value

  _tolerance = grid_ptr->_tolerance;
  
  // Save the debug level

  _debugLevel = grid_ptr->_debugLevel;
}


/*********************************************************************
 * Destructor
 */

MdvGrid::~MdvGrid(void)
{
  static const char *routine_name = "Destructor";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
}


/*********************************************************************
 * print() - Print the MDV grid information to the indicated stream.
 */

void MdvGrid::print(char *filename)
{
  static const char *routine_name = "print";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  FILE *output_file;
  
  // Open the output file.
  
  if ((output_file = ta_fopen_uncompress(filename, "w")) == (FILE *)NULL)
  {
    fprintf(stderr,
	    "%s::%s: ERROR:  Error opening file <%s> for output\n",
	    _className(), routine_name, filename);
    return;
  }
  
  // Print the information

  print(output_file);
  
  // Close the output file

  fclose(output_file);
  
  return;
}


void MdvGrid::print(FILE *stream)
{
  static const char *routine_name = "print";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Print the information

  fprintf(stream,
	  "\nGrid Information:\n");
  fprintf(stream,
	  "   min X = %f\n", _minX);
  fprintf(stream,
	  "   min Y = %f\n", _minY);
  fprintf(stream,
	  "   min Z = %f\n", _minZ);
  fprintf(stream,
	  "   delta X = %f\n", _deltaX);
  fprintf(stream,
	  "   delta Y = %f\n", _deltaY);
  fprintf(stream,
	  "   delta Z = %f\n", _deltaZ);
  fprintf(stream,
	  "   num X = %d\n", _nX);
  fprintf(stream,
	  "   num Y = %d\n", _nY);
  fprintf(stream,
	  "   num Z = %d\n", _nZ);
  fprintf(stream,
	  "   projection type = %s\n", MDV_proj2string(_projType));
  fprintf(stream, "\n");
  
  return;
}


/*********************************************************************
 * operator==() - Determine if two grids are equivalent, within the
 *                specified tolerance.
 */

int MdvGrid::operator==(const MdvGrid &grid)
{
  static const char *operator_name = "==";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), operator_name);
  
  // The grid projection must be the same

  if (_projType != grid._projType)
  {
    if (_debugLevel >= MDV_DEBUG_MSGS)
      fprintf(stdout,
	      "%s::%s Grid projections don't match\n",
	      _className(), operator_name);
    
    return(FALSE);
  }
  
  // The grid size must match exactly

  if (_nX != grid._nX ||
      _nY != grid._nY ||
      _nZ != grid._nZ)
  {
    if (_debugLevel >= MDV_DEBUG_MSGS)
      fprintf(stdout,
	      "%s::%s Grid size doesn't match\n",
	      _className(), operator_name);
    
    return(FALSE);
  }
  
  // The grid deltas must be within tolerance.  If there is only a single
  // plane of data, don't check the Z values.

  if (_nZ > 1)
  {
    if (_deltaX > grid._deltaX + _tolerance ||
	_deltaX < grid._deltaX - _tolerance ||
	_deltaY > grid._deltaY + _tolerance ||
	_deltaY < grid._deltaY - _tolerance ||
	_deltaZ > grid._deltaZ + _tolerance ||
	_deltaZ < grid._deltaZ - _tolerance)
    {
      if (_debugLevel >= MDV_DEBUG_MSGS)
	fprintf(stdout,
		"%s::%s Grid deltas not within tolerance\n",
		_className(), operator_name);
    
      return(FALSE);
    }
  }
  else
  {
    if (_deltaX > grid._deltaX + _tolerance ||
	_deltaX < grid._deltaX - _tolerance ||
	_deltaY > grid._deltaY + _tolerance ||
	_deltaY < grid._deltaY - _tolerance)
    {
      if (_debugLevel >= MDV_DEBUG_MSGS)
	fprintf(stdout,
		"%s::%s Grid deltas not within tolerance\n",
		_className(), operator_name);
    
      return(FALSE);
    }
  }
  // The grid origins must be within tolerance

  if (_nZ > 1)
  {
    if (_minX > grid._minX + _tolerance ||
	_minX < grid._minX - _tolerance ||
	_minY > grid._minY + _tolerance ||
	_minY < grid._minY - _tolerance ||
	_minZ > grid._minZ + _tolerance ||
	_minZ < grid._minZ - _tolerance)
    {
      if (_debugLevel >= MDV_DEBUG_MSGS)
	fprintf(stdout,
		"%s::%s Grid origins not within tolerance\n",
		_className(), operator_name);
    
      return(FALSE);
    }
  }
  else
  {
    if (_minX > grid._minX + _tolerance ||
	_minX < grid._minX - _tolerance ||
	_minY > grid._minY + _tolerance ||
	_minY < grid._minY - _tolerance)
    {
      if (_debugLevel >= MDV_DEBUG_MSGS)
	fprintf(stdout,
		"%s::%s Grid origins not within tolerance\n",
		_className(), operator_name);
    
      return(FALSE);
    }
  }

  // If we get here, the grids matched

  return(TRUE);
}


/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * _setTolerance() - Set the tolerance value to be used for this grid.
 *                   A default value is used unless the environment
 *                   variable MDV_GRID_TOLERANCE is set to a non-negative
 *                   value.
 */

double MdvGrid::_setTolerance(void)
{
  static const char *routine_name = "_setTolerance";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // See if the environment variable is set.

  char *env_ptr;
  
  if ((env_ptr = getenv("MDV_GRID_TOLERANCE")) == NULL)
    return(0.0000001);
  
  // Convert the environment variable to a number

  double tolerance;
  
  if ((tolerance = atof(env_ptr)) < 0.0)
    tolerance = 0.0;
  
  return(tolerance);
}
