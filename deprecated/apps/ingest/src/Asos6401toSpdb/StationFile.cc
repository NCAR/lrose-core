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
//   $Date: 2016/03/07 01:22:59 $
//   $Id: StationFile.cc,v 1.2 2016/03/07 01:22:59 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * StationFile: Base class for classes that control files with station
 *              location information.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "StationFile.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

StationFile::StationFile(const bool debug_flag) :
  _debug(debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

StationFile::~StationFile()
{
}


/*********************************************************************
 * getStationLoc() - Get the location for the indicated station.
 *
 * Returns true on success, false on failure.
 * Returns the station location in the lat, lon and alt arguments.
 */

bool StationFile::getStationLoc(const string &station_id,
				double &lat, double &lon, double &alt) const
{
  static const string method_name = "StationFile::getStationLoc()";
  
  map< string, station_info_t >::const_iterator station_iter;
  
  // Get the station from the file

  if ((station_iter = _stationList.find(station_id)) == _stationList.end())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Station <" << station_id << "> not found in station list" << endl;
    
    return false;
  }
  
  // Extract the needed information

  lat = (*station_iter).second.lat;
  lon = (*station_iter).second.lon;
  alt = (*station_iter).second.alt;
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
