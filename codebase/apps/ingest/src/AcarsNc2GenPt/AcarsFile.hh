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
 *   $Id: AcarsFile.hh,v 1.2 2016/03/07 01:22:59 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * AcarsFile: Class for controlling access to the netCDF ACARS files.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef AcarsFile_H
#define AcarsFile_H

#include <string>
#include <vector>
#include <netcdf.hh>

#include <Spdb/Spdb.hh>
#include <toolsa/DateTime.hh>

#include "DataField.hh"

using namespace std;


class AcarsFile
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  AcarsFile();
  
  /*********************************************************************
   * Destructor
   */

  ~AcarsFile();


  /*********************************************************************
   * initialize() - Initialize the AcarsFile object.  This method MUST
   *                be called before any other methods are called.
   *
   * Returns true on success, false on failure
   */

  bool initialize(const string &num_recs_dim_name,
		  const string &tail_len_dim_name,
		  const string &missing_data_value_att_name,
		  const string &latitude_var_name,
		  const string &longitude_var_name,
		  const string &altitude_var_name,
		  const string &tail_number_var_name,
		  const string &data_source_var_name,
		  const string &data_times_var_name,
		  const bool debug_flag);
  

  /*********************************************************************
   * addDataField() - Add the given data field to the ACARS object.
   */

  void addDataField(DataField *data_field)
  {
    _dataFields.push_back(data_field);
  }
  

  /*********************************************************************
   * initializeFile() - Initialize the ACARS file information.  This 
   *                    method MUST be called at the beginning of processing
   *                    any file.
   *
   * Returns true on success, false on failure
   */

  bool initializeFile(const string &acars_file_path = "");
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /*********************************************************************
   * writeAsSpdb() - Write out the acars file in SPDB format.
   *
   * Returns true on success, false on failure
   */

  bool writeAsSpdb(const string &spdb_url,
		   const bool save_only_specified_source = false,
		   const int data_source = 0);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * setDebugFlag() - Set the debug flag to the given value.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

  /*********************************************************************
   * setFilePath() - Set the acars file path.
   */

  void setFilePath(const string &acars_file_path)
  {
    _filePath = acars_file_path;
  }
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const double FLOAT_MISSING_DATA_VALUE;
  static const double DOUBLE_MISSING_DATA_VALUE;

  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  string _filePath;
  
  NcFile *_acarsFile;
  
  // netCDF file dimension names

  string _numRecsDimName;
  string _tailLenDimName;
  
  string _missingDataValueAttName;
  
  string _latitudeVarName;
  string _longitudeVarName;
  string _altitudeVarName;
  string _dataSourceVarName;
  string _tailNumberVarName;
  string _dataTimesVarName;
  
  // Data fields to process

  vector< DataField* > _dataFields;
  
  // Initialization flags

  bool _objectInitialized;
  bool _fileInitialized;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _getByteFieldVar() - Get the specified byte values from the
   *                      netCDF file.
   *
   * Returns a pointer to the byte values on success, 0 on failure.
   */

  NcValues *_getByteFieldVar(const string &field_name) const;
  

  /*********************************************************************
   * _getCharFieldVar() - Get the specified character values from the
   *                      netCDF file.
   *
   * Returns a pointer to the character values on success, 0 on failure.
   */

  NcValues *_getCharFieldVar(const string &field_name) const;
  

  /*********************************************************************
   * _getDoubleFieldVar() - Get the specified double values from the
   *                        netCDF file.
   *
   * Returns a pointer to the double values on success, 0 on failure.
   */

  NcValues *_getDoubleFieldVar(const string &field_name,
			       double &missing_data_value) const;
  

  /*********************************************************************
   * _getFloatFieldVar() - Get the specified float values from the
   *                       netCDF file.
   *
   * Returns a pointer to the float values on success, 0 on failure.
   */

  NcValues *_getFloatFieldVar(const string &field_name,
			      float &missing_data_value) const;
  

  /*********************************************************************
   * _getIntFieldVar() - Get the specified integer values from the
   *                     netCDF file.
   *
   * Returns a pointer to the integer values on success, 0 on failure.
   */

  NcValues *_getIntFieldVar(const string &field_name) const;
  

  /*********************************************************************
   * _getVarDoubleAtt() - Get the specified attribute from the given
   *                      netCDF variable as a double.
   *
   * Returns the attribute value retrieved from the netCDF file on
   * success, the global DOUBLE_MISSING_DATA_VALUE on failure.
   */

  double _getVarDoubleAtt(const NcVar &variable,
			  const string &att_name) const;
  

  /*********************************************************************
   * _getVarFloatAtt() - Get the specified attribute from the given
   *                     netCDF variable as a float.
   *
   * Returns the attribute value retrieved from the netCDF file on
   * success, the global FLOAT_MISSING_DATA_VALUE on failure.
   */

  float _getVarFloatAtt(const NcVar &variable,
			const string &att_name) const;
  

  /*********************************************************************
   * _retrieveData() - Retrieve the data from the netCDF file and put it
   *                   into the given SPDB object.
   *
   * Returns true on success, false on failure.
   */

  bool _retrieveData(Spdb &spdb,
		     const bool save_only_specified_source,
		     const int data_source);
  

};

#endif
