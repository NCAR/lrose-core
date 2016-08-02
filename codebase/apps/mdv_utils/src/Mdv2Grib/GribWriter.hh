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
 *   $Id: GribWriter.hh,v 1.14 2016/03/04 02:22:09 dixon Exp $
 *   $Revision: 1.14 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GribWriter: Class for writing MDV data to a GRIB file.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GribWriter_HH
#define GribWriter_HH

#include <grib/GribRecord.hh>
#include <grib/GribVertType.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

#include "DataConverter.hh"
#include "Params.hh"

using namespace std;

class GribWriter
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    FIELD_TYPE_NORMAL,
    FIELD_TYPE_ACCUM,
    FIELD_TYPE_AVG
  } field_type_t;
    

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructors
   */

  GribWriter(const bool debug_flag = false, const bool is_forecast_data = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~GribWriter(void);


  /*********************************************************************
   * openFile() - Open the indicated file for output
   */

  bool openFile(const string &file_path);


  /*********************************************************************
   * writeField() - Write the given field to the GRIB file.
   */

  bool writeField(const Mdvx::master_header_t &master_hdr,
                  const MdvxField &mdv_field,
                  const DataConverter &converter,
                  const int grib_tables_version,
                  const int originating_center,
                  const int subcenter_id,
                  const int generating_process_id,
                  const int grid_id,
                  const int grib_code,
                  const int precision,
                  const int max_bit_length,
                  const Params::forecast_interval_type_t forecast_interval,
                  const int time_range_id,
                  const bool override_vert_level,
                  const int vert_level_type,
                  const int vert_level_bottom,
                  const int vert_level_top,
                  const double data_addend,
		  const field_type_t field_type,
		  const int accum_secs);


  /*********************************************************************
   * closeFile() - Close the current output file.
   */

  void closeFile();


  ////////////////////
  // Access methods //
  ////////////////////

  void setDebugFlag(const bool debug_flag) { _debug = debug_flag; }

  void setForecastFlag(const bool forecast_flag) { _is_forecast_data = forecast_flag; }

 private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debug; 
  bool _is_forecast_data;

  string _outputFilePath;
  FILE *_outputFile;

  static const float FEET_TO_METERS;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _convertMdv2GribLevel() - Convert the given MDV vertical level info
   *                           to the equivalent GRIB level info.
   */

  static bool _convertMdv2GribLevelType(const int mdv_level_type,
                                        const double mdv_level_value,
                                        GribVertType::vert_type_t &grib_level_type,
                                        int &grib_level_value_top,
                                        int &grib_level_value_bottom);


  /*********************************************************************
   * _setTimeValues() - Set the time values in the grib record.
   */

  void _setTimeValues(GribRecord &grib_record,
		      const int time_range_id,
		      const int forecast_interval,
		      const field_type_t field_type,
		      const int accum_secs,
		      const Mdvx::master_header_t master_hdr,
		      const Mdvx::field_header_t field_hdr) const;
  

};


#endif
