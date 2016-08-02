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
 *   $Id: Mdv2Grib.hh,v 1.9 2016/03/04 02:22:09 dixon Exp $
 *   $Revision: 1.9 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Mdv2Grib: Mdv2Grib program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Mdv2Grib_HH
#define Mdv2Grib_HH

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

#include "GribWriter.hh"

using namespace std;


class Mdv2Grib
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

  ~Mdv2Grib(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Mdv2Grib *Inst(int argc, char **argv);
  static Mdv2Grib *Inst();
  

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

  int run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Mdv2Grib *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;

  // Input trigger object

  DsTrigger *_dataTrigger;
  
  // Input path object

  DsInputPath * _inputPath;
  
  // GRIB writer object

  GribWriter _gribWriter;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Mdv2Grib(int argc, char **argv);
  

  /*********************************************************************
   * _getFieldType() - Get the GribWriter field type given the field type
   *                   from the parameter file.
   */

  GribWriter::field_type_t _getFieldType(const Params::field_type_t field_type) const;
  

  /*********************************************************************
   * _getOutputPath() - Construct the output file path
   */

  void _getOutputPath(const DsMdvx &input_mdv,
		      string &grib_file_path, string &rel_data_path) const;
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(const TriggerInfo &trigger_info);
  

  /*********************************************************************
   * _readMdvFile() - Read the MDV file for the given time.
   */

  bool _readMdvFile(DsMdvx &input_mdv,
		    const TriggerInfo &trigger_info) const;
  

  /*********************************************************************
   * _writeField() - Write the given field to the GRIB file.
   */

  bool _writeField(const Mdvx::master_header_t &master_hdr,
		   const MdvxField &mdv_field) const;
  

  /*********************************************************************
   * _setReadField() - Set the fields to read in the MDV request
   */

  bool _setReadFields(DsMdvx &input_mdv) const;
  

};


#endif
