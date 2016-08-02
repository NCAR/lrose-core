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
 *   $Date: 2016/03/04 02:22:09 $
 *   $Id: MaliSat2Mdv.hh,v 1.2 2016/03/04 02:22:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MaliSat2Mdv: MaliSat2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2014
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MaliSat2Mdv_HH
#define MaliSat2Mdv_HH

#include <netcdf.hh>
#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

class MaliSat2Mdv
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

  ~MaliSat2Mdv(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MaliSat2Mdv *Inst(int argc, char **argv);
  static MaliSat2Mdv *Inst();
  

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

  static const string LAT_DIM_NAME;
  static const string LON_DIM_NAME;
  
  static const string LATMIN_ATT_NAME;
  static const string LONMIN_ATT_NAME;
  static const string LATRES_ATT_NAME;
  static const string LONRES_ATT_NAME;
  
  static const string TEMP_VAR_NAME;
  static const string TEMP_UNITS_ATT_NAME;
  static const string TEMP_MISSING_VALUE_ATT_NAME;
  
  static const double MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MaliSat2Mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  DsTrigger *_dataTrigger;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MaliSat2Mdv(int argc, char **argv);
  

  /*********************************************************************
   * _getTempField() - Get the temperature field from the given netCDF file and put
   *                   it into the given MDV file.
   *
   * Returns true on success, false on failure.
   */

  bool _getTempField(Mdvx &mdvx,
		     const NcFile &nc_file,
		     const string &input_file_path) const;
  

  /*********************************************************************
   * _getDataTime() - Get the data time and accumulation period from the
   *                  input file name.
   *
   * Returns the data time on success, DateTime::NEVER on failure.
   */

  DateTime _getDataTime(const string &input_file_path) const;
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   */

  bool _initTrigger();
  

  /*********************************************************************
   * _processFile() - Process the given file.
   */

  bool _processFile(const string &input_file_path);
  

  /*********************************************************************
   * _updateFieldHeader() - Update the field header for the ACCUM field 
   *                        based on information in the netCDF file.
   *
   * Returns true on success, false on failure.
   */

  bool _updateFieldHeader(Mdvx::field_header_t &field_hdr,
			  const NcFile &nc_file,
			  const string &input_file_path,
			  const string &units) const;
  

  /*********************************************************************
   * _updateMasterHeader() - Update the master header in the given MDV
   *                         file using information from the given netCDF
   *                         file and file path.
   *
   * Returns true on success, false on failure.
   */

  bool _updateMasterHeader(Mdvx &mdvx,
			   const DateTime &valid_time,
			   const string &input_file_path) const;
  

};


#endif
