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
 *   $Date: 2016/03/07 01:23:08 $
 *   $Id: ihop3x3Goes2Mdv.hh,v 1.5 2016/03/07 01:23:08 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ihop3x3Goes2Mdv: ihop3x3Goes2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak/Kay Levesque
 *
 ************************************************************************/

#ifndef ihop3x3Goes2Mdv_HH
#define ihop3x3Goes2Mdv_HH

#include <string>

#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "GridHandler.hh"
#include "Args.hh"
#include "Params.hh"

using namespace std;


class ihop3x3Goes2Mdv
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

  /*********************************************************************
   * Destructor
   */

  ~ihop3x3Goes2Mdv(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static ihop3x3Goes2Mdv *Inst(int argc, char **argv);
  static ihop3x3Goes2Mdv *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();


  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  // Singleton instance pointer

  static ihop3x3Goes2Mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  

  // Objects needed for token parsing

  static const int TOKEN_DATE_BEGIN_YY;
  static const int TOKEN_DATE_LEN_YY;

  static const int TOKEN_DATE_BEGIN_JULIAN_DAY;
  static const int TOKEN_DATE_LEN_JULIAN_DAY;

  static const int TOKEN_DATE_BEGIN_HOUR;
  static const int TOKEN_DATE_LEN_HOUR;

  static const int TOKEN_DATE_BEGIN_MIN;
  static const int TOKEN_DATE_LEN_MIN;

  static const int TOKEN_DATE_BEGIN_SEC;
  static const int TOKEN_DATE_LEN_SEC;

  static const int TOKEN_LAT_BEGIN;
  static const int TOKEN_LAT_LEN;

  static const int TOKEN_LON_BEGIN;
  static const int TOKEN_LON_LEN;

  static const int TOKEN_TEMP_BEGIN;
  static const int TOKEN_TEMP_LEN;

  static const int TOKEN_DEWPOINT_BEGIN;
  static const int TOKEN_DEWPOINT_LEN;

  static const int TOKEN_PRESSURE_BEGIN;
  static const int TOKEN_PRESSURE_LEN;

  static const int TOKEN_HEIGHT_BEGIN;
  static const int TOKEN_HEIGHT_LEN;

  static const int TOKEN_RETRIEVAL_TYPE_BEGIN;
  static const int TOKEN_RETRIEVAL_TYPE_LEN;

  static const int MINIMUM_INPUT_LENGTH;


  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  ihop3x3Goes2Mdv(int argc, char **argv);

  
  /*********************************************************************
   * _convertDouble()
   * 
   * Pulls indicated token out of input line, converts it to a double, and
   * returns true upon success
   */
  
  bool _convertDouble(const char* inputLine, const int begin, const int numChars, double &return_value) const;

  /*********************************************************************
   * _convertDate()
   *
   * Pulls date information out of input line, calls convertInt() for each token,
   * returns a DateTime object
   */

  static DateTime _convertDate(char* inputLine);

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
  
};

#endif
