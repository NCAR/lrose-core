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
 *
 * @file GpmHdf2Mdv.hh
 *
 * @class GpmHdf2Mdv
 *
 * GpmHdf2Mdv program object.
 *  
 * @date 10/30/2008
 *
 */

#ifndef GpmHdf2Mdv_HH
#define GpmHdf2Mdv_HH

#include "SdsDataField.hh"

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "RadConvert.hh"

using namespace std;

/** 
 * @class GpmHdf2Mdv
 */

class GpmHdf2Mdv
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Destructor
   */

  ~GpmHdf2Mdv(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the GpmHdf2Mdv instance.
   */

  static GpmHdf2Mdv *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the GpmHdf2Mdv instance.
   */

  static GpmHdf2Mdv *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /**
   * @brief Run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static GpmHdf2Mdv *_instance;
  
  /**
   * @brief Program name.
   */

  char *_progName;

  /**
   * @brief Command line arguments.
   */

  Args *_args;

  /**
   * @brief Parameter file parameters.
   */

  Params *_params;
  
  /**
   * @brief Data triggering object.
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief Projection to use for output MDV file.
   */

  MdvxPjg _outputProj;
  
  /**
   * @brief The object used to convert radiance values to brightness
   *        temperature values.
   */

  RadConvert _radConvert;
  
  /**
   * @brief List of Input SDS data field handlers.
   */

  vector< SdsDataField* > _dataHandlers;
  

  /**
   * @brief Flag indicating whether the solar calibration data needs to
   *        be loaded in the Geolocation object.
   */

  bool _loadSolarCalibData;
  

  /**
   * @brief Flag indicating whether the brightness temperature table needs
   *        to be loaded in the radiance conversion object.
   */

  bool _loadBtTable;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note Private because this is a singleton object.
   */

  GpmHdf2Mdv(int argc, char **argv);
  

  /**
   * @brief Add the scan delta time field to the MDV file.
   *
   * @param[in,out] mdvx The MDV file.
   * @param[in] geolocation The geolocation and scan time information from
   *                        the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addMdvDeltaTimeField(DsMdvx &mdvx,
			     const HdfFile &hdf_file) const;
  

  /**
   * @brief Add the scan time field to the MDV file.
   *
   * @param[in,out] mdvx The MDV file.
   * @param[in] geolocation The geolocation and scan time information from
   *                        the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addMdvScanTimeField(DsMdvx &mdvx,
			    const HdfFile &hdf_file) const;
  

  /**
   * @brief Add the solar magnitude field to the MDV file.
   *
   * @param[in,out] mdvx The MDV file.
   * @param[in] geolocation The geolocation information from
   *                        the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addMdvSolarMagField(DsMdvx &mdvx,
			    const HdfFile &hdf_file) const;
  

  /**
   * @brief Add the solar zenith field to the MDV file.
   *
   * @param[in,out] mdvx The MDV file.
   * @param[in] geolocation The geolocation information from
   *                        the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addMdvSolarZenithField(DsMdvx &mdvx,
			       const HdfFile &hdf_file) const;
  

  /**
   * @brief Add the geolocation fields to the given MDV file.
   *
   * @param[in,out] mdvx The MDV file.
   * @param[in] geolocation The geolocation information for each
   *                        point in the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addMdvGeolocationFields(DsMdvx &mdvx,
				const HdfFile &hdf_file) const;
  

  /**
   * @brief Create the indicated blank MDV field.
   *
   * @param[in] field_name Field name.
   * @param[in] field_units Field units.
   *
   * @return Returns a pointer to the blank MDV field on success, 0 on failure.
   */

  MdvxField *_createBlankMdvField(const string &field_name,
				  const string &field_units) const;
  

  /**
   * @brief Create the indicated blank MDV time field.
   *
   * @param[in] field_name Field name.
   * @param[in] field_units Field units.
   *
   * @return Returns a pointer to the blank MDV field on success, 0 on failure.
   */

  MdvxField *_createBlankMdvTimeField(const string &field_name,
				      const string &field_units) const;
  

  /**
   * @brief Create the field-related vectors for the given output field.
   *
   * @param[in] output_field Output field information from the parameter file.
   * @param[out] field_info List of output fields for this SDS field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _createFieldVector(Params::output_field_t output_field,
			  vector< FieldInfo > &field_info);
  

  /**
   * @brief Create the vertical levels vector for the given output field.
   *
   * @param[in] output_field Output field information from the parameter file.
   * @param[out] vert_levels List of vertical levels for this field.
   * @param[out] dz_constant Flag indicating whether dz is constant.
   * @return Returns true on success, false on failure.
   */

  bool _createVertLevelsVector(Params::output_field_t output_field,
			       vector< double > &vert_levels,
			       bool &dz_constant) const;
  

  /**
   * @brief Initialize the data handler objects.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initDataHandlers();
  

  /**
   * @brief Initialize the output projection.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputProjection();
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Parse a string containing a list of doubles into a vector.
   *
   * @param[in] double_list_string String containing list of double values,
   *                               comma-delimited.
   * @param[out] double_list Vector of corresponding double values.
   *
   * @return Returns true on success, false on failure.
   */

  static bool _parseDoubleList(const char *double_list_string,
			       vector< double > &double_list);


  /**
   * @brief Parse a string containing a list of strings into a vector.
   *
   * @param[in] string_list_string String containint list of strings,
   *                               comma-delimited.
   * @param[out] string_list Vector of corresponding string values.
   *
   * @return Returns true on success, false on failure.
   */

  static bool _parseStringList(const char *string_list_string,
			       vector< string > &string_list);
  

  /**
   * @brief Process the given file.
   *
   * @param[in] file_path Path for the input file to process.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processFile(const string &file_path);
  

  /**
   * @brief Set the master header information in the given MDV file.
   *
   * @param[in,out] mdvx MDV file.
   * @param[in] begin_time Scan begin time.
   * @param[in] end_time Scan end time.
   * @param[in] input_path Input file path.
   */

  void _setMasterHeader(DsMdvx &mdvx,
			const DateTime &begin_time,
			const DateTime &end_time,
			const string &input_path) const;
  

};


#endif
