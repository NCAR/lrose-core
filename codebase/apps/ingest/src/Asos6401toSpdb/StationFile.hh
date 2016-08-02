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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:22:59 $
 *   $Id: StationFile.hh,v 1.2 2016/03/07 01:22:59 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * StationFile: Base class for classes that control files with station
 *              location information.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef StationFile_H
#define StationFile_H

#include <iostream>
#include <map>
#include <string>

using namespace std;


class StationFile
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  StationFile(const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  virtual ~StationFile();


  /*********************************************************************
   * readFile() - Read the stations from the given file.
   *
   * Returns true on success, false on failure.
   */

  virtual bool readFile(const string &station_file_path) = 0;


  /*********************************************************************
   * getStationLoc() - Get the location for the indicated station.
   *
   * Returns true on success, false on failure.
   * Returns the station location in the lat, lon and alt arguments.
   */

  virtual bool getStationLoc(const string &station_id,
			     double &lat, double &lon, double &alt) const;


protected:
  
  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    string station_id;
    double lat;
    double lon;
    double alt;
  } station_info_t;
  
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  map< string, station_info_t > _stationList;
  
};

#endif
