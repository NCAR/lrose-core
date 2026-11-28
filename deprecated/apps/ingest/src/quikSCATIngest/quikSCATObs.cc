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
 * quikSCATObs: Class that manipulates a quikSCAT observation.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <physics/physics.h>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

#include "quikSCATObs.hh"

using namespace std;

/*********************************************************************
 * Constructors
 */

quikSCATObs::quikSCATObs(DateTime time, double lat, double lon, double wind_speed, double wind_dir, bool rain_flag, bool nsol_flag) :
  _obsTime(time),
  _latitude(lat),
  _longitude(lon),
  _windSpeed(wind_speed),
  _windDir(wind_dir),
  _rainFlag(rain_flag),
  _nsolFlag(nsol_flag)
{
}
  
/*********************************************************************
 * Destructor
 */

quikSCATObs::~quikSCATObs()
{
}

/*********************************************************************
 * print() - Print the object values to the given stream
 */

void quikSCATObs::print(ostream &out) const
{
  static const string method_name = "quikSCATObs::print()";

  out << endl;
  out << "    ====================" << endl;
  out << "    quikSCAT Observation" << endl;
  out << "    ====================" << endl;
  out << "    obs time: " << _obsTime << endl;
  out << "    latitude: " << _latitude << endl;
  out << "    longitude: " << _longitude << endl;
  out << "    wind speed or magnitude in m/s: " << _windSpeed << " m/s" << endl;
  out << "    wind direction or phase in degrees: " << _windDir << " degrees" << endl;
  out << "    rain flag: " << _rainFlag << endl;
  out << "    nsol flag: " << _nsolFlag << endl;
}

/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
