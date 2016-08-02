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
//   $Date: 2016/03/07 01:39:55 $
//   $Id: SL2StnConverter.cc,v 1.2 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SL2StnConverter: Class for converting the pressure values in a station
 *                  report from sea level pressure to station pressure.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <physics/PhysicsLib.hh>

#include "SL2StnConverter.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

SL2StnConverter::SL2StnConverter (const bool debug_flag) :
  Converter(debug_flag)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

SL2StnConverter::~SL2StnConverter(void)
{
  // Do nothing
}
  

/*********************************************************************
 * updatePressure() - Update the pressure field in the given station
 *                    report.
 */

void SL2StnConverter::updatePressure(station_report_t &stn_report)
{
  // Convert the given pressure value, assumed to be sea level pressure,
  // to station pressure.  This is based on the sea-level reduction code
  // from GEMPAK.

  // Make sure the needed fields exist.  If any one is missing, set the
  // pressure value to missing since we can't calculate it.

  if (stn_report.pres == STATION_NAN ||
      stn_report.alt == STATION_NAN)
  {
    stn_report.pres = STATION_NAN;
    return;
  }
  
  // Calculate the station pressure value

  stn_report.pres = PhysicsLib::SL2StnPressure(stn_report.pres,
					       stn_report.alt);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
