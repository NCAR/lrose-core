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
 *   $Date: 2016/03/07 01:23:11 $
 *   $Id: satWinds2Spdb.hh,v 1.2 2016/03/07 01:23:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * satWinds2Spdb: satWinds2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef satWinds2Spdb_HH
#define satWinds2Spdb_HH

#include <string>

#include <dsdata/DsTrigger.hh>
#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class satWinds2Spdb
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

  ~satWinds2Spdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static satWinds2Spdb *Inst(int argc, char **argv);
  static satWinds2Spdb *Inst();
  

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

  static satWinds2Spdb *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;

  // Need a DsTrigger object

  DsTrigger *_dataTrigger;
  
  // Objects needed for token parsing

  static const int TOKEN_YEAR_BEGIN;
  static const int TOKEN_YEAR_LEN;

  static const int TOKEN_MONTH_BEGIN;
  static const int TOKEN_MONTH_LEN;

  static const int TOKEN_DAY_BEGIN;
  static const int TOKEN_DAY_LEN;

  static const int TOKEN_HOUR_BEGIN;
  static const int TOKEN_HOUR_LEN;

  static const int TOKEN_MIN_BEGIN;
  static const int TOKEN_MIN_LEN;

  static const int TOKEN_LAT_BEGIN;
  static const int TOKEN_LAT_LEN;

  static const int TOKEN_LON_BEGIN;
  static const int TOKEN_LON_LEN;

  static const int TOKEN_PRESSURE_BEGIN;
  static const int TOKEN_PRESSURE_LEN;

  static const int TOKEN_WIND_SPEED_BEGIN;
  static const int TOKEN_WIND_SPEED_LEN;

  static const int TOKEN_WIND_DIRECTION_BEGIN;
  static const int TOKEN_WIND_DIRECTION_LEN;

  static const int MINIMUM_INPUT_LENGTH;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  satWinds2Spdb(int argc, char **argv);
  
  /*********************************************************************
   * _convertDouble()
   * 
   * Pulls indicated token out of input line, converts it to a double, and
   * returns true upon success
   */
  
  bool _convertDouble(const char* inputLine, const int begin, const int numChars, fl32 &return_value) const;

  /*********************************************************************
   * _convertDate()
   *
   * Pulls date information out of input line, calls convertInt() for each token,
   * returns a DateTime object
   */

  static DateTime _convertDate(const char* inputLine);

  /*********************************************************************
   * _convertInt()
   *
   * Pulls indicated token out of input line, converts it to an int, and
   * returns true upon success
   */

  static bool _convertInt(const char* inputLine, const int begin, const int numChars, int &return_value);


  /*********************************************************************
   * _processData() - Process data in the given file
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

