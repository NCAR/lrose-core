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
//   $Date: 2016/03/03 18:47:40 $
//   $Id: WxHazardFactory.cc,v 1.3 2016/03/03 18:47:40 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * WxHazardFactory.cc: Class used for creating specific types of
 *                     WxHazard objects from an SPDB buffer.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <iostream>
#include <unistd.h>

#include <dataport/port_types.h>
#include <spdbFormats/ConvRegionHazard.hh>
#include <spdbFormats/WxHazard.hh>
#include <spdbFormats/WxHazardFactory.hh>
using namespace std;


// Global variables

  
/*********************************************************************
 * Constructor
 */

WxHazardFactory::WxHazardFactory(bool debug_flag)
{
  _debugFlag = debug_flag;
}


/*********************************************************************
 * Destructor
 */

WxHazardFactory::~WxHazardFactory()
{
}


/*********************************************************************
 * create() - Create the appropriate weather hazard object from the
 *            given SPDB buffer.
 */

WxHazard *WxHazardFactory::create(ui08 *buffer)
{
  const char *routine_name = "create()";
  
  // Read the hazard header from the buffer.

  WxHazard::spdb_hazard_header_t *header =
    (WxHazard::spdb_hazard_header_t *)buffer;
  WxHazard::spdbHazardHeaderToNative(header);
  
  switch (header->hazard_type)
  {
  case WxHazard::CONVECTIVE_REGION_HAZARD :
    return new ConvRegionHazard(buffer);
    
  default:
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Invalid hazard type " << header->hazard_type <<
      " in SPDB buffer" << endl;
    
    return (WxHazard *)NULL;
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

