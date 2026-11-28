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
 *   $Id: NwsStationFile.hh,v 1.2 2016/03/07 01:22:59 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NwsStationFile: Class for classes that control National Weather
 *                 Service-format files with station location information.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NwsStationFile_H
#define NwsStationFile_H

#include <iostream>
#include <string>

#include "StationFile.hh"

using namespace std;


class NwsStationFile : public StationFile
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    ALT_UNITS_METERS,
    ALT_UNITS_FEET
  } alt_units_t;
  
  
  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  NwsStationFile(const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  virtual ~NwsStationFile();


  /*********************************************************************
   * init() - Initialize the object.
   *
   * Returns true on success, false on failure.
   */

  virtual bool init(const alt_units_t alt_units);


  /*********************************************************************
   * readFile() - Read the stations from the given file.
   *
   * Returns true on success, false on failure.
   */

  virtual bool readFile(const string &station_file_path);


protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  alt_units_t _altitudeUnits;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _degMinSec2Deg() - Convert the given lat/lon string from deg-min-sec
   *                    to degrees.
   *
   * Returns the converted value.
   */

  static double _degMinSec2Deg(const string &deg_min_sec_str);
  

};

#endif
