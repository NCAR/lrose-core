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
 *   $Date: 2016/03/07 01:23:04 $
 *   $Id: Nmsu2Spdb.hh,v 1.3 2016/03/07 01:23:04 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Nmsu2Spdb: Nmsu2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef Nmsu2Spdb_HH
#define Nmsu2Spdb_HH

#include <string>

#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class Nmsu2Spdb
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~Nmsu2Spdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Nmsu2Spdb *Inst(int argc, char **argv);
  static Nmsu2Spdb *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Nmsu2Spdb *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Objects needed for token parsing

  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  static const double UNAVAIL;

  typedef enum
  {
    MON_TOKEN,
    DAY_TOKEN,
    YEAR_TOKEN,
    HOUR_TOKEN,
    AIR_TEMP_DEGF_TOKEN,
    PERCENT_REL_HUM_TOKEN,
    INCHES_PRECIP_TOKEN,
    MPH_WIND_SPEED_TOKEN,
    DEG_WIND_DIRECTION_TOKEN,
    SOLAR_RAD_TOKEN,
    SOIL_TEMP_DEGF_TOKEN,
    SOIL_MOIST_TOKEN,
    RADIATION_TOKEN,
    NUM_TOKENS
  } token_list_t;

  char **_tokens;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */
  
  Nmsu2Spdb(int argc, char **argv);
  
  /*********************************************************************
   * _convertDouble()
   * 
   * Returns the token converted to a double, resetting unavail data to
   * STATION_NAN.
   */
  
  double _convertDouble(const char* token) const;

  /*********************************************************************
   * _processData() - Process data in the given file, starting at the
   *                  given line number.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const string& file_name);
  

  /*********************************************************************
   * _writeToDatabase() - Write the given record to the SPDB database.
   *
   * Returns true on success, false on failure.
   */

  bool _writeToDatabase(station_report_t& station);
  

};


#endif

