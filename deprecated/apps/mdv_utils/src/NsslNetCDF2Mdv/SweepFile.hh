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
/**
 * @file SweepFile.hh
 * @brief Class for controlling access to the netCDF sweep files.
 * @class SweepFile
 * @brief Class for controlling access to the netCDF sweep files.
 &*/

#ifndef SweepFile_H
#define SweepFile_H

#include "AzimuthLut.hh"
#include "NsslData.hh"
#include "FieldSpec.hh"
#include <string>
#include <vector>

class Mdvx;
class MdvxField;
class DateTime;
class Nc3Error;
class Nc3File;
class Nc3Var;

class SweepFile
{
  
public:

  /**
   * Constructor
   */
  SweepFile(const NsslData &data, const FieldSpec &spec,
	    const std::string &num_gates_dim_name,
	    bool data_driven_beamwidth = false,
	    int num_allowed_output_beamwidth = 0,
	    double *allowed_output_beamwidth = NULL,
	    double output_beamwidth = 1.0,
	    bool force_negative_longitude = false);
  
  /**
   * Destructor
   */
  ~SweepFile();


  /***
   * get the volume number from the sweep file.
   *
   * @param[out] volume_number
   * @return true on success, false on failure
   */
  bool getVolumeNumber(int &volume_number) const;
  
  /**
   * Get the elevation angle from the sweep file
   *
   * @param[out] elev  The angle
   * @return true on success, false on failure
   */
  bool getElevAngle(double &elev) const;

  /**
   * Get the radar name from the sweep file
   * @param[out] name
   * @return true on success, false on failure
   */
  bool getRadarName(std::string &name) const;

  /**
   * Get the volume start time from the sweep file.
   * @param[out] volume_start_time
   * @return true on success, false on failure
   */
  bool getVolumeStartTime(DateTime &volume_start_time) const;
  
  /**
   * Initialize the SweepFile.  This method MUST be called
   * before any other methods are called.
   *
   * @return true on success, false on failure
   */
  bool initialize(void);
  

  /**
   * Add the sweep into the given MDV object.
   * @param[in,out] mdv_file
   * @return true on success, false on failure
   */
  bool addSweepToMdv(Mdvx &mdv_file);
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  // Names of dimensions in the sweep file
  static const std::string NUM_AZIMUTHS_DIM_NAME;
  
  // Names of variables in the sweep file
  static const std::string GATE_SPACING_VAR_NAME;
  static const std::string AZIMUTH_VAR_NAME;
  static const std::string AZIMUTH_SPACING_VAR_NAME;
  
  // Names of attributes in the sweep file
  static const std::string VOL_NUM_GLOBAL_ATT_NAME;
  static const std::string VOLUME_START_TIME_GLOBAL_ATT_NAME;
  static const std::string LATITUDE_GLOBAL_ATT_NAME;
  static const std::string LONGITUDE_GLOBAL_ATT_NAME;
  static const std::string ALTITUDE_GLOBAL_ATT_NAME;
  static const std::string START_RANGE_GLOBAL_ATT_NAME;
  static const std::string TARGET_ELEV_GLOBAL_ATT_NAME;
  static const std::string RADAR_NAME_GLOBAL_ATT_NAME;

  // Names of global attributes in the sweep file
  static const float FLOAT_MISSING_DATA_VALUE;
  static const int INT_MISSING_DATA_VALUE;

  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  NsslData _data;
  FieldSpec _spec;

  std::string _numGatesDimName;


  Nc3Error *_ncError;
  Nc3File * _sweepFile;
  
  // Data dimensions

  int _numGates;
  int _numInputAzimuths;
  int _numOutputAzimuths;

  bool _dataDrivenBeamWidth;
  std::vector<double> _allowedOutputBeamWidth;
  double _outputBeamWidth;
  AzimuthLut _azimuthLut;
  
  bool _forceNegativeLongitude;
  
  bool _objectInitialized;
  
  bool _createAzimuthLookupTable();
  void _updateMasterHeader(Mdvx &mdv_file) const;
  bool _createMdvFields(Mdvx &mdv_file);
  MdvxField *_createMdvField(void) const;
  bool _updateMdvFieldData(MdvxField &field)const;

  bool _getDimensions();
  Nc3Var *_getFieldVar(const std::string &field_name) const;
  int _getGlobalIntAtt(const std::string &att_name) const;
  std::string _getGlobalStringAtt(const std::string &att_name) const;
  float _getGlobalFloatAtt(const std::string &att_name) const;
  float _getScalarVarFloat(const std::string &variable_name,
			   const int data_index = 0) const;
  int _getScalarVarInt(const std::string &variable_name,
		       const int data_index = 0) const;
  float _getVarFloatAtt(const Nc3Var &variable,
			const std::string &att_name) const;
  std::string _getVarStringAtt(const Nc3Var &variable,
			       const std::string &att_name) const;
};

#endif
