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
 * @file NegBuoyancy.hh
 *
 * @class NegBuoyancy
 *
 * NegBuoyancy is the top level application class.
 *  
 * @date 6/10/2010
 *
 */

#ifndef NegBuoyancy_HH
#define NegBuoyancy_HH

#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <physics/AdiabatTempLookupTable.hh>

#include "Args.hh"
#include "Params.hh"

#include "Input.hh"

/**
 * @class NegBuoyancy
 */

class NegBuoyancy
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

  ~NegBuoyancy(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the singleton object.
   */

  static NegBuoyancy *Inst(int argc, char **argv);

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the singleton object.
   */

  static NegBuoyancy *Inst();
  

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

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief The value to use to flag missing data in the CAPE field.
   */

  static const fl32 CAPE_MISSING_DATA_VALUE;

  /**
   * @brief The value to use to flag missing data in the CIN field.
   */

  static const fl32 CIN_MISSING_DATA_VALUE;

  /**
   * @brief The value to use to flag missing datain the height fields.
   */

  static const fl32 HEIGHT_MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer
   */

  static NegBuoyancy *_instance;
  
  /**
   * @brief The program name.
   */

  char *_progName;

  /**
   * @brief The command line arguments.
   */

  Args *_args;

  /**
   * @brief The parameter file parameters.
   */

  Params *_params;
  
  /**
   * @brief Data trigger object.
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief Input object.
   */

  Input *_input;
    /**
   * @brief Look up table for adibat temperature values.
   */

  AdiabatTempLookupTable _lookupTable;
  
  /**
   * @brief The input pressure field.
   */

  MdvxField *_pressureField;
  
  /**
   * @brief The input temperature field.
   */

  MdvxField *_temperatureField;
  
  /**
   * @brief The input mixing ratio field.
   */

  MdvxField *_mixingRatioField;
  
  /**
   * @brief The input height field.
   */

  MdvxField *_heightField;
  
  /**
   * @brief The input terrain field.
   */

  MdvxField *_terrainField;
  
  /**
   * @brief The calculated bounding pressure field.
   */

  MdvxField *_boundingPresField;
  
  /**
   * @brief The calculated CAPE field.
   */

  MdvxField *_capeField;
  
  /**
   * @brief The calculated CIN field.
   */

  MdvxField *_cinField;
  
  /**
   * @brief The calculated LCL relative height field.
   */

  MdvxField *_lclRelHtField;
  
  /**
   * @brief The calculated LFC relative height field.
   */

  MdvxField *_lfcRelHtField;
  
  /**
   * @brief The calculated EL relative height field.
   */

  MdvxField *_elRelHtField;
  
  /**
   * @brief The calculated LCL height above ground field.
   */

  MdvxField *_lclField;
  
  /**
   * @brief The calculated LFC height above ground field.
   */

  MdvxField *_lfcField;
  
  /**
   * @brief The calculated minimum bouyancy level field.
   */

  MdvxField *_kbminField;
  
  /**
   * @brief The calculated minimum buoyancy field.
   */

  MdvxField *_bminField;
  
  /**
   * @brief The calculated minimum buoyancy height field.
   */

  MdvxField *_zbminField;
  
  /**
   * @brief The calculated LCL temperature field.
   */

  MdvxField *_tlcField;
  
  /**
   * @brief The calculated lifted parcel virtual temperature field.
   */

  MdvxField *_tliftField;
  
  /**
   * @brief The calculated original height of lifted parcel field.
   */

  MdvxField *_zparField;
  
  /**
   * @brief The calculated original model level of lifted parcel field.
   */

  MdvxField *_kparField;
  
  /**
   * @brief The calculated maximum buoyancy field.
   */

  MdvxField *_bmaxField;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor -- private because this is a singleton object
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   */

  NegBuoyancy(int argc, char **argv);
  

  /**
   * @brief Calculate the output fields using the current data.
   *
   * @return Returns true on success, false on failure.
   */
  
  bool _calculateOutputFields();
  

  /**
   * @brief Clear out the internal data fields.
   */

  void _clearData();
  

  /**
   * @brief Create the given 2D field.
   *
   * @param[in] field_name Field name.
   * @param[in] field_name_long Long field name.
   * @param[in] field_code Field GRIB code.
   * @param[in] units Field units.
   * @param[in] missing_data_value Missing data value.
   * @param[in] sample_field_hdr Sample field header to be used as the
   *                             basis for this field's header.
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   */
  
  MdvxField *_createField(const string &field_name,
			  const string &field_name_long,
			  const int field_code,
			  const string &units,
			  const fl32 missing_data_value,
			  const Mdvx::field_header_t &sample_field_hdr);
  

  /**
   * @brief Create the given 3D field.
   *
   * @param[in] field_name Field name.
   * @param[in] field_name_long Long field name.
   * @param[in] field_code Field GRIB code.
   * @param[in] units Field units.
   * @param[in] missing_data_value Missing data value.
   * @param[in] sample_field_hdr Sample field header to be used as the
   *                             basis for this field's header.
   * @param[in] sample_vlevel_hdr Sample vlevel header to be used as the
   *                              basis for this field's vlevel header.
   * @param[in] min_vlevel Minimum vertical level in the created field.
   * @param[in] max_vlevel Maximum vertical level in the created field.
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   */
  
  MdvxField *_createField(const string &field_name,
			  const string &field_name_long,
			  const int field_code,
			  const string &units,
			  const fl32 missing_data_value,
			  const Mdvx::field_header_t &sample_field_hdr,
			  const Mdvx::vlevel_header_t &sample_vlevel_hdr,
			  const int min_vlevel, const int max_vlevel);
  

  /**
   * @brief Allocate space for all of the calculated fields and initialize
   *        the data values as "missing".
   *
   * @param[in] min_calc_level Minimum vertical level for calculations.
   * @param[in] max_calc_level Maximum vertical level for calculations.
   *
   * @return Returns true on success, false on failure.
   */

  bool _createOutputFields(const int min_calc_level,
			   const int max_calc_level);
  

  /**
   * @brief Initialize the input object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initInput();
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process the data for the given time.
   *
   * @param[in] trigger_info The trigger information for the data.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const TriggerInfo &trigger_info);
  

  /**
   * @brief Read in all of the input fields.
   *
   * @param[in] trigger_info The trigger information for the data.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readInputFields(const TriggerInfo &trigger_info);
  

  /**
   * @brief Update the master header for the output file.
   *
   * @param[in,out] output_file The file in which to update the master header.
   * @param[in] data_time The new data time for the master header.
   * @param[in] field_hdr A field header containing information to use
   *                      in the master header.
   */
  
  void _updateMasterHeader(DsMdvx &output_file,
			   const DateTime &data_time,
			   const Mdvx::field_header_t &field_hdr);
  

};


#endif
