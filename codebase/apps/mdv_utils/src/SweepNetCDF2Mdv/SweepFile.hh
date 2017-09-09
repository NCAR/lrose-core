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
 *   $Date: 2017/09/09 21:22:28 $
 *   $Id: SweepFile.hh,v 1.12 2017/09/09 21:22:28 dixon Exp $
 *   $Revision: 1.12 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SweepFile: Class for controlling access to the netCDF sweep files.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SweepFile_H
#define SweepFile_H

#include <string>
#include <vector>
#include <Ncxx/Nc3File.hh>

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>
#include <toolsa/DateTime.hh>

#include "AzimuthLut.hh"

using namespace std;


class SweepFile
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  SweepFile(const string &sweep_file_path = "",
            const string &num_gates_dim_name = "maxCells",
	    const string &field_list_var_name = "fields",
	    const string &missing_data_value_att_name = "_fillValue",
	    const bool bias_specified = true,
	    const double output_beamwidth = 1.0,
	    const bool force_negative_longitude = false,
	    const bool override_file_missing_data_value = false,
	    const double file_missing_data_value = -999.0,
	    const bool fix_missing_beams = false,
	    const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  ~SweepFile();


  /*********************************************************************
   * initialize() - Initialize the SweepFile.  This method MUST be called
   *                before any other methods are called.
   *
   * Returns true on success, false on failure
   */

  bool initialize();
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /*********************************************************************
   * addSweepToMdv() - Add the sweep from this file into the given MDV 
   *                   object.
   *
   * Returns true on success, false on failure
   */

  bool addSweepToMdv(Mdvx &mdv_file);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getVolumeNumber() - Get the volume number from the sweep file.
   *
   * Returns true on success, false on failure
   */

  bool getVolumeNumber(int &volume_number) const;
  

  /*********************************************************************
   * getVolumeStartTime() - Get the volume start time from the sweep
   *                        file.
   *
   * Returns true on success, false on failure
   */

  bool getVolumeStartTime(DateTime &volume_start_time) const;
  

  /*********************************************************************
   * setDebugFlag() - Set the debug flag to the given value.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

  /*********************************************************************
   * setFilePath() - Set the sweep file path.
   */

  void setFilePath(const string &sweep_file_path)
  {
    _filePath = sweep_file_path;
  }
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  // Names of dimensions in the sweep file

  static const string NUM_AZIMUTHS_DIM_NAME;
  static const string NUM_FIELDS_DIM_NAME;
  static const string FIELD_NAME_LEN_DIM_NAME;
  
  // Names of variables in the sweep file

  static const string FIELD_LIST_VAR_NAME;
  static const string LATITUDE_VAR_NAME;
  static const string LONGITUDE_VAR_NAME;
  static const string ALTITUDE_VAR_NAME;
  static const string GATE_SPACING_VAR_NAME;
  static const string START_RANGE_VAR_NAME;
  static const string TARGET_ELEV_VAR_NAME;
  static const string BASE_TIME_VAR_NAME;
  static const string TIME_OFFSET_VAR_NAME;
  static const string AZIMUTH_VAR_NAME;
  static const string VOLUME_START_TIME_VAR_NAME;
  
  // Names of attributes in the sweep file

  static const string FIELD_NAME_LONG_ATT_NAME;
  static const string UNITS_ATT_NAME;
  static const string SCALE_ATT_NAME;
  static const string BIAS_ATT_NAME;
  
  // Names of global attributes in the sweep file

  static const string VOL_NUM_GLOBAL_ATT_NAME;
  
  static const float FLOAT_MISSING_DATA_VALUE;
  static const int INT_MISSING_DATA_VALUE;

  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  // NC names that don't seem to be constant across files

  string _numGatesDimName;
  string _fieldListVarName;
  string _missingDataValueAttName;
  bool _biasSpecified;

  string _filePath;
  Nc3Error *_ncError;
  Nc3File *_sweepFile;
  
  // Data dimensions

  int _numGates;
  int _numInputAzimuths;
  int _numOutputAzimuths;

  double _outputBeamWidth;
  AzimuthLut _azimuthLut;
  
  bool _forceNegativeLongitude;
  
  bool _overrideFileMissingDataValue;
  double _fileMissingDataValue;
  
  bool _fixMissingBeams;
  
  bool _objectInitialized;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _createAzimuthLookupTable() - Create the azimuth lookup table, used
   *                               to map the netCDF data index to the
   *                               appropriate azimuth location.
   *
   * Returns true on success, false on failure.
   */

  bool _createAzimuthLookupTable();
  

  /*********************************************************************
   * _createMdvField() - Create the MDV field with the given name based
   *                     on the data in the sweep file.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createMdvField(const string &field_name) const;
  

  /*********************************************************************
   * _createMdvFields() - Create the MDV fields from the data in the
   *                      sweep file.
   *
   * Returns true on success, false on failure.  The MDV fields are
   * stored in the class members.
   */

  bool _createMdvFields(Mdvx &mdv_file);
  

  /*********************************************************************
   * _getDimensions() - Get the data dimensions from the sweep file.
   *
   * Returns true on success, false on failure.  The data dimensions are
   * stored in the class members.
   */

  bool _getDimensions();
  

  /*********************************************************************
   * _getFieldList() - Get the list of fields in the sweep file.
   *
   * Returns the list of fields retrieved from the sweep file.
   */

  vector< string> _getFieldList() const;
  

  /*********************************************************************
   * _getFieldVar() - Get the specified variable from the netCDF file.
   *
   * Returns a pointer to the variable on success, 0 on failure.
   */

  Nc3Var *_getFieldVar(const string &field_name) const;
  

  /*********************************************************************
   * _getGlobalIntAtt() - Get the specified global attribute from the
   *                      netCDF file as an integer.
   *
   * Returns the attribute value retrieved from the netCDF file on
   * success, the global INT_MISSING_DATA_VALUE on failure.
   */

  int _getGlobalIntAtt(const string &att_name) const;
  

  /*********************************************************************
   * _getScalarVarFloat() - Get the specified scalar variable from the
   *                        netCDF file as a float value.
   *
   * Returns the variable value retrieved from the netCDF file on
   * success, the global FLOAT_MISSING_DATA_VALUE on failure.
   */

  float _getScalarVarFloat(const string &variable_name,
			   const int data_index = 0) const;
  

  /*********************************************************************
   * _getScalarVarInt() - Get the specified scalar variable from the
   *                      netCDF file as an integer value.
   *
   * Returns the variable value retrieved from the netCDF file on
   * success, the global INT_MISSING_DATA_VALUE on failure.
   */

  int _getScalarVarInt(const string &variable_name,
		       const int data_index = 0) const;
  

  /*********************************************************************
   * _getVarFloatAtt() - Get the specified attribute from the given
   *                     netCDF variable as a float.
   *
   * Returns the attribute value retrieved from the netCDF file on
   * success, the global FLOAT_MISSING_DATA_VALUE on failure.
   */

  float _getVarFloatAtt(const Nc3Var &variable,
			const string &att_name) const;
  

  /*********************************************************************
   * _getVarStringAtt() - Get the specified attribute from the given
   *                      netCDF variable as a string.
   *
   * Returns the attribute value retrieved from the netCDF file on
   * success, "" on failure.
   */

  string _getVarStringAtt(const Nc3Var &variable,
			  const string &att_name) const;
  

  /*********************************************************************
   * _updateMasterHeader() - Update the master header in the given MDV
   *                         file based on the information in the sweep
   *                         file.
   */

  void _updateMasterHeader(Mdvx &mdv_file) const;
  

  /*********************************************************************
   * _updateMdvFieldData() - Add the new sweep to the MDV data volume.
   *
   * Returns true on success, false on failure.
   */

  bool _updateMdvFieldData(const string &field_name,
			   MdvxField &field)const;
  

};

#endif
