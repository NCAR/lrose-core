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
//   $Date: 2016/03/04 02:22:12 $
//   $Id: MinXYMover.cc,v 1.2 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MinXYMover: Class that moves an MDV grid in the X/Y direction by simply
 *             changing the grid_minx and grid_miny values in the field
 *             header.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>

#include "MinXYMover.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

MinXYMover::MinXYMover(const double x_offset_km, const double y_offset_km,
		       const bool debug_flag) :
  Mover(x_offset_km, y_offset_km, debug_flag)
{
}


/*********************************************************************
 * Destructor
 */

MinXYMover::~MinXYMover()
{
}


/*********************************************************************
 * moveField() - Move the field data to the appropriate new location.
 *
 * Returns true on success, false on failure.
 */

bool MinXYMover::moveField(MdvxField &field)
{
  // Get the current projection from the field since we need to move the
  // minx and miny values in the proper units

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  MdvxPjg proj(field_hdr);
  
  // Update the header minx/miny values

  field_hdr.grid_minx =
    proj.km2x(proj.x2km(field_hdr.grid_minx) + _xOffsetKm);
  field_hdr.grid_miny =
    proj.km2x(proj.x2km(field_hdr.grid_miny) + _yOffsetKm);

  field.setFieldHeader(field_hdr);
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
