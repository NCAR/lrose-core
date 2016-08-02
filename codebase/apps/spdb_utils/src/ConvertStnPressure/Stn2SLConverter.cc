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
//   $Id: Stn2SLConverter.cc,v 1.2 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Stn2SLConverter: Class for converting the pressure values in a station
 *                  report from station pressure to sea level pressure.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <rapmath/math_macros.h>

#include "Stn2SLConverter.hh"

using namespace std;


const double Stn2SLConverter::GAMMA = 6.5;
const double Stn2SLConverter::GRAVITY = 9.80616;
const double Stn2SLConverter::R = 287.04;


/**********************************************************************
 * Constructor
 */

Stn2SLConverter::Stn2SLConverter (const bool debug_flag) :
  Converter(debug_flag)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

Stn2SLConverter::~Stn2SLConverter(void)
{
  // Do nothing
}
  

/*********************************************************************
 * updatePressure() - Update the pressure field in the given station
 *                    report.
 */

void Stn2SLConverter::updatePressure(station_report_t &stn_report)
{
  // Convert the given pressure value, assumed to be sea level pressure,
  // to station pressure.  This is based on the sea-level reduction code
  // from GEMPAK.

  // Make sure the needed fields exist.  If any one is missing, set the
  // pressure value to missing since we can't calculate it.

  if (stn_report.temp == STATION_NAN ||
      stn_report.dew_point == STATION_NAN ||
      stn_report.pres == STATION_NAN ||
      stn_report.alt == STATION_NAN)
  {
    stn_report.pres = STATION_NAN;
    return;
  }
  
  // Calculate the station pressure value

  double elevation = stn_report.alt;   // units???
  double temp_k = TEMP_C_TO_K(stn_report.temp);
  double vapr = 6.112 *
    exp((17.67 * stn_report.dew_point) / (stn_report.dew_point + 243.5));
  double e = vapr * (1.001 + (stn_report.pres - 100.0) / 900.0 * 0.0034);
  double mixr = 0.62197 * (e / (stn_report.pres - e)) * 1000.0;
  double virt_temp = temp_k *
    ((1.0 + (0.001 * mixr) / 0.62197) / (1.0 + (0.001 * mixr)));
  double deltv = GAMMA * elevation / 1000.0;
  double avg_virt_temp = (virt_temp + (virt_temp - deltv)) / 2.0;
  double slpressure = stn_report.pres *
    exp((GRAVITY * elevation) / (R * avg_virt_temp));
  
  stn_report.pres = slpressure;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
