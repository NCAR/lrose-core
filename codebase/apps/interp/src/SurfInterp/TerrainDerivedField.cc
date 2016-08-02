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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:50:09 $
//   $Id: TerrainDerivedField.cc,v 1.2 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TerrainDerivedField: Class for calculating the terrain derived field.
 *                      Terrain is not actually a derived field, but it
 *                      fits into the SurfInterp processing best as a
 *                      derived field, so we'll just call it one an ignore
 *                      the surface fields when calculating it.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <rapmath/math_macros.h>

#include "TerrainDerivedField.hh"

using namespace std;

const string TerrainDerivedField::FIELD_NAME = "Terrain";
const string TerrainDerivedField::FIELD_UNITS = "Ft";


/*********************************************************************
 * Constructors
 *
 * Note that the terrain pointer points to an object owned by the caller
 * and must be deleted by the caller after deleting this object.
 */

TerrainDerivedField::TerrainDerivedField(Terrain *terrain,
					 const Pjg &output_proj,
					 const bool output_field_flag,
					 const bool debug_flag) :
  DerivedField(FIELD_NAME, FIELD_UNITS,
	       output_proj, output_field_flag, debug_flag),
  _terrain(terrain)
{
}

  
/*********************************************************************
 * Destructor
 */

TerrainDerivedField::~TerrainDerivedField()
{
}


/*********************************************************************
 * derive() - Calculate the derivation.
 *
 * Returns true on success, false on failure.
 */

bool TerrainDerivedField::derive(map< int, StnInterpField* > &interp_field_list,
				 map< int, DerivedField* > &der_field_list)
{
  // Retrieve the terrain data and copy it into our derived grid.

  fl32 *terrain_data;
  fl32 terrain_bad_value;
  
  if ((terrain_data = _terrain->getTerrain(terrain_bad_value)) == 0)
    return false;
  
  // Allocate space for the terrain data

  int grid_size = _outputProj.getNx() * _outputProj.getNy();
  
  delete [] _derivedGrid;
  _derivedGrid = new fl32[grid_size];
  
  _missingDataValue = terrain_bad_value;
  
  // Convert the terrain file values which are in meters to the desired
  // values which are in feet.

  for (int i = 0; i < grid_size; ++i)
  {
    if (terrain_data[i] == terrain_bad_value)
      _derivedGrid[i] = _missingDataValue;
    else
      _derivedGrid[i] = terrain_data[i] / M_PER_FT;
  } /* endfor - i */
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
