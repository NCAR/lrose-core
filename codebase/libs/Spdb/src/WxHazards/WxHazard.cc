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
//   $Date: 2016/03/03 18:13:05 $
//   $Id: WxHazard.cc,v 1.5 2016/03/03 18:13:05 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * WxHazard.cc: Abstract object representing a weather hazard.  This
 *              is the base class for weather hazard objects stored in
 *              an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <unistd.h>

#include <dataport/bigend.h>
#include <Spdb/WxHazard.hh>
using namespace std;

// Global variables

  
/*********************************************************************
 * Constructor
 */

WxHazard::WxHazard(hazard_type_t hazard_type, bool debug_flag) :
     _debugFlag(debug_flag),
     _hazardType(hazard_type)
{
     // Do nothing
}


/*********************************************************************
 * Destructor
 */

WxHazard::~WxHazard()
{
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * spdbHazardHeaderToBigend() - Swaps the spdb_hazard_header_t
 *                              structure from native format to
 *                              big-endian format.
 */

void WxHazard::spdbHazardHeaderToBigend(spdb_hazard_header_t *header)
{
     BE_from_array_32(header, sizeof(spdb_hazard_header_t));
}


/*********************************************************************
 * spdbHazardHeaderToNative() - Swaps the spdb_hazard_header_t
 *                              structure from big-endian format to
 *                              native format.
 */

void WxHazard::spdbHazardHeaderToNative(spdb_hazard_header_t *header)
{
     BE_to_array_32(header, sizeof(spdb_hazard_header_t));
}

