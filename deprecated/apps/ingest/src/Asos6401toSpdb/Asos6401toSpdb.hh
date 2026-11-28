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
 *   $Id: Asos6401toSpdb.hh,v 1.2 2016/03/07 01:22:59 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Asos6401toSpdb: Asos6401toSpdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Asos6401toSpdb_HH
#define Asos6401toSpdb_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "StationFile.hh"

using namespace std;

class Asos6401toSpdb
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

  ~Asos6401toSpdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Asos6401toSpdb *Inst(int argc, char **argv);
  static Asos6401toSpdb *Inst();
  

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

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int MAX_LINE_LEN;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Asos6401toSpdb *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  DsTrigger *_dataTrigger;
  
  StationFile *_stationFile;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Asos6401toSpdb(int argc, char **argv);
  

  /*********************************************************************
   * _decodeReport() - Decode the line from the ASOS file into a station
   *                   report.
   *
   * Return true on success, false on failure
   */

  bool _decodeReport(const char *line,
		     station_report_t &report) const;
  

  /*********************************************************************
   * _getStationLoc() - Get the location for this station from the station
   *                    file.
   *
   * Returns true on success, false on failure.
   */

  bool _getStationLoc(const string &station_id,
		      double &lat, double &lon, double &alt) const;
  

  /*********************************************************************
   * _initStationFile() - Initialize the station file handler
   *
   * Returns true on success, false on failure.
   */

  bool _initStationFile();
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   */

  bool _initTrigger();
  

  /*********************************************************************
   * _processFile() - Process the given file.
   */

  bool _processFile(const string &input_file_path);
  

};


#endif
