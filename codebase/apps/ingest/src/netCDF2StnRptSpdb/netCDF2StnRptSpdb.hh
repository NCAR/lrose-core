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
/************************************************************************
 * netCDF2StnRptSpdb: netCDF2StnRptSpdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef netCDF2StnRptSpdb_HH
#define netCDF2StnRptSpdb_HH

#include <string>

#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "InputVariable.hh"
#include "Params.hh"
using namespace std;


class netCDF2StnRptSpdb
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

  ~netCDF2StnRptSpdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static netCDF2StnRptSpdb *Inst(int argc, char **argv);
  static netCDF2StnRptSpdb *Inst();
  

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

  static netCDF2StnRptSpdb *_instance;

  //a netCDF2StnRptSpdb object contains InputVariable objects
  //InputVariable objects do the work of getting the values of the given variables

  InputVariable _inputVar_lat;
  InputVariable _inputVar_lon;
  InputVariable _inputVar_alt;
  InputVariable _inputVar_temp;
  InputVariable _inputVar_dew_point;
  InputVariable _inputVar_relhum;
  InputVariable _inputVar_windspd;
  InputVariable _inputVar_winddir;
  InputVariable _inputVar_windgust;
  InputVariable _inputVar_pres;
  InputVariable _inputVar_liquid_accum;
  InputVariable _inputVar_visibility;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;

  
  /////////////////////
  // Private methods //
  /////////////////////


  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */
  
  netCDF2StnRptSpdb(int argc, char **argv);
  

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

  /*********************************************************************
   * _initInputVariable() - initialize an input variable from the param file
   *
   * 
   */

  void _initInputVariable(InputVariable &inputVar, 
			  const Params::input_variable_t &param_input) const;
  

};


#endif
