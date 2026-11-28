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
 *   $Date: 2016/03/07 01:23:07 $
 *   $Id: WestTx2Spdb.hh,v 1.2 2016/03/07 01:23:07 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * WestTx2Spdb: WestTx2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sep 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef WestTx2Spdb_HH
#define WestTx2Spdb_HH

#include <string>

#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class WestTx2Spdb
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

  ~WestTx2Spdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static WestTx2Spdb *Inst(int argc, char **argv);
  static WestTx2Spdb *Inst();
  

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

  static WestTx2Spdb *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Objects needed for token parsing

  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  static const double UNAVAIL;
  static const double UNAVAILABLE;
  
  typedef enum
  {
    ID_TOKEN = 0,
    JULIAN_DAY_TOKEN,
    LOCAL_TIME_TOKEN,
    STATION_ID_TOKEN,
    WINDSPD_10M_SCALER_TOKEN,
    WINDSPD_10M_VECTOR_TOKEN,
    WINDDIR_10M_TOKEN,
    WINDDIR_SD_TOKEN,
    WINDSPD_SD_TOKEN,
    PEAKWINDSPD_10M_TOKEN,
    TEMP_1POINT5_TOKEN,
    TEMP_9M_TOKEN,
    TEMP_2M_TOKEN,
    RH_1POINT5_TOKEN,
    PRESSURE_TOKEN,
    RAINFALL_IN_TOKEN,
    DEWPOINT_DEGC_TOKEN,
    WINDSPD_2M_TOKEN,
    SOLAR_RADIATION_TOKEN,
    NUM_TOKENS
  } token_list_t;
  
  char **_tokens;


  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  WestTx2Spdb(int argc, char **argv);
  

  /*********************************************************************
   * _convertDouble()
   * 
   *
   * Returns the token converted to a double, resetting unavail data to
   * STATION_NAN.
   */

  double _convertDouble(const char* token);

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

