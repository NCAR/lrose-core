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
//   $Date: 2016/03/04 02:22:12 $
//   $Id: VlevelPressHandler.cc,v 1.2 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * VlevelPressHandler: Class that gets pressure values from a vlevel
 *                     header.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "VlevelPressHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

VlevelPressHandler::VlevelPressHandler() :
  PressHandler()
{
}

  
/*********************************************************************
 * Destructor
 */

VlevelPressHandler::~VlevelPressHandler()
{
}


/*********************************************************************
 * init() - Initialize the handler.  Must be called before any other
 *          methods.
 *
 * Returns true on success, false on failure.
 */

bool VlevelPressHandler::init(const Mdvx::vlevel_header_t vlevel_hdr)
{
  _vlevelHdr = vlevel_hdr;

  return true;
}


/*********************************************************************
 * getPressureValue() - Get the indicated pressure value.
 *
 * Returns true if the pressure value is available, false otherwise.
 */

bool VlevelPressHandler::getPressureValue(const int plane_index, const int z,
					  fl32 &pressure) const
{
  pressure = _vlevelHdr.level[z] * 100.0;
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
