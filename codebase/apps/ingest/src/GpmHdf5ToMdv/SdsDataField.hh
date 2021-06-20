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
 * @file SdsDataField.hh
 *
 * @class SdsDataField
 *
 * Base class for a data field that is stored in the SDS section of a
 * TRMM file.
 *  
 * @date 10/31/2008
 *
 */

#ifndef SdsDataField_HH
#define SdsDataField_HH

#include <string>
#include <vector>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <dataport/port_types.h>

#include "FieldInfo.hh"
#include "HdfFile.hh"
#include "RadConvert.hh"

using namespace std;

/** 
 * @class SdsDataField
 */

class SdsDataField
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief Value to use to flag bad data in the MDV volume.
   */

  static const double MDV_BAD_DATA_VALUE;
  
  /**
   * @brief Value to use to flag missing data in the MDV volume.
   */

  static const double MDV_MISSING_DATA_VALUE;
  

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  SdsDataField(const string &sds_field_name,
	       const vector< FieldInfo > field_info,
	       const int num_vert_levels,
	       const vector< double > vert_levels,
	       const Mdvx::vlevel_type_t vert_level_type,
	       const bool dz_constant,
	       const bool invert_vert_levels,
	       RadConvert &rad_convert,
	       const bool debug_flag = false,
	       const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~SdsDataField(void);
  

  /**
   * @brief Read the SDS array data from the TRMM file and create the MDV
   *        fields.
   *
   * @param[in,out] mdv_fields Vector of MDV fields created from the SDS
   *                           array.
   * @param[in] geolocation Geolocation information for the TRMM file.
   * @param[in] hdf_file The TRMM file.
   *
   * @return Returns true on success, false on failure.
   */

  bool createMdvFields(vector< MdvxField* > &mdv_fields,
		       const MdvxPjg &output_proj,
		       const HdfFile &hdf_file);
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;

  /**
   * @brief Radiance to brightness temperature conversion object.  This 
   *        object is owned by the calling class and must not be deleted
   *        here.
   */

  RadConvert &_radConvert;
  
  /**
   * @brief SDS field name.
   */

  string _sdsFieldName;
  

  /**
   * @brief Information about the data fields in the SDS array.
   */

  vector< FieldInfo > _fieldInfo;
  

  /**
   * @brief Number of vertical levels in the data.
   */

  int _numVertLevels;
  

  /**
   * @brief List of vertical levels in the data.
   */

  vector< double > _vertLevels;
  

  /**
   * @brief Type of vertical levels in the data.
   */

  Mdvx::vlevel_type_t _vertLevelType;
  

  /**
   * @brief Flag indicating whether the distance between the vertical levels
   *        is constant.
   */

  bool _dzConstant;
  

  /**
   * @bool Difference in heights of vertical levels.  Used only if
   *       _dzConstant is true.  Otherwise, set to 0.0.
   */

  double _gridDz;
  

  /**
   * @brief Flag indicating whether the vertical levels in the input file
   *        should be inverted before writing to the output file.
   */

  bool _invertVertLevels;
  

  ///////////////////////////////
  // Protected virtual methods //
  ///////////////////////////////

  /**
   * @brief Check the dimensions of the incoming data against what we are
   *        expecting.
   *
   * @param[in] geolocation Geolocation information for this TRMM file.
   *                        This gives us the expected number of scans
   *                        and pixels in the data.
   * @param[in] dimensions The dimensions found for this data in the TRMM
   *                       file.
   *
   * @return Returns true if the dimensions of the data are okay, false
   *         otherwise.
   */

  virtual bool _checkDimensions(const HdfFile &hdf_file,
				const vector< int > &dimensions) = 0;
  

  /**
   * @brief Create the MDV fields for this TRMM data.
   *
   * @param[out] mdv_fields The MDV fields created from this data.
   * @param[in] output_proj The desired output projection for the MDV data.
   * @param[in] geolocation The geolocation information for the TRMM data.
   * @param[in] raw_data The raw data for the TRMM field as read from the
   *                     file.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool _createMdvFields(vector< MdvxField* > &mdv_fields,
				const MdvxPjg &output_proj,
				const HdfFile &hdf_file,
				const fl64 *raw_data) = 0;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Create the described MDV field, filling in the data with
   *        missing data values.
   *
   * @param[in] field_name Field name for the field.
   * @param[in] field_units Units for the field.
   * @param[in] output_proj Projection to use for the field.
   * @param[in] vlevel_type The vertical level type for the field.
   * @param[in] vert_levels List of vertical levels for the field.
   *
   * @return Returns a pointer to the blank field on success, 0 on failure.
   */

  MdvxField *_createBlankMdvField(const string &field_name,
				  const string &field_units,
				  const MdvxPjg &output_proj,
				  const Mdvx::vlevel_type_t vlevel_type,
				  const vector< double > vert_levels);
  

};


#endif
