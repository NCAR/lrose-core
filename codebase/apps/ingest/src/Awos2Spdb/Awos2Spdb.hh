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
 *   $Id: Awos2Spdb.hh,v 1.7 2016/03/07 01:22:59 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Awos2Spdb: Awos2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef Awos2Spdb_HH
#define Awos2Spdb_HH

#include <string>

#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class Awos2Spdb
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

  ~Awos2Spdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Awos2Spdb *Inst(int argc, char **argv);
  static Awos2Spdb *Inst();
  

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

  static Awos2Spdb *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Objects needed for token parsing

  static const int TOKEN_DATE_BEGIN_YY;
  static const int TOKEN_DATE_LEN_YY;
  static const int TOKEN_DATE_BEGIN_MON;
  static const int TOKEN_DATE_LEN_MON;
  static const int TOKEN_DATE_BEGIN_DAY;
  static const int TOKEN_DATE_LEN_DAY;
  static const int TOKEN_DATE_BEGIN_HOUR;
  static const int TOKEN_DATE_LEN_HOUR;
  static const int TOKEN_DATE_BEGIN_MIN;
  static const int TOKEN_DATE_LEN_MIN;
  static const int TOKEN_TEMP_BEGIN;
  static const int TOKEN_TEMP_LEN;
  static const int TOKEN_WIND_GUST_BEGIN;
  static const int TOKEN_WIND_GUST_LEN;
  static const int TOKEN_WIND_SPEED_BEGIN;
  static const int TOKEN_WIND_SPEED_LEN;
  static const int TOKEN_DEWPOINT_BEGIN;
  static const int TOKEN_DEWPOINT_LEN;
  static const int TOKEN_PRECIP_BEGIN;
  static const int TOKEN_PRECIP_LEN;
  static const int TOKEN_VISIB_BEGIN;
  static const int TOKEN_VISIB_LEN;
  static const int TOKEN_WIND_DIRECTION_BEGIN;
  static const int TOKEN_WIND_DIRECTION_LEN;

  static const int MINIMUM_INPUT_LENGTH;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Awos2Spdb(int argc, char **argv);
  
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
   * _processData() - Process data in the given file, starting at the
   *                  given line number.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const string& file_name);
  

  /*********************************************************************
   * _addToDatabase() - Add the given record to the SPDB database buffer.
   *
   * Returns true on success, false on failure.
   */

  bool _addToDatabase(DsSpdb &spdb, station_report_t& station);
  

};


#endif

