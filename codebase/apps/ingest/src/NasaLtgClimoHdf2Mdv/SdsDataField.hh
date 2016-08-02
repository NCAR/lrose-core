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

#include "HdfFile.hh"

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
   *
   * @param[in] sds_field_name The name of the field in the SDS portion of
   *                           the file.
   * @param[in] prog_name The program name.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  SdsDataField(const string &sds_field_name,
	       const string &prog_name,
	       const string &output_url,
	       const bool debug_flag = false,
	       const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~SdsDataField(void);
  

  /**
   * @brief Read the SDS array data from the file and create the MDV
   *        files for this data.
   *
   * @param[in] hdf_file Input HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool createMdvFiles(HdfFile &hdf_file);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Maximum number of characters in a single field name in
   *        a TRMM file.
   */

  static const int MAX_FIELD_NAME_LEN;
  

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
   * @brief Program name.
   */

  string _progName;
  
  /**
   * @brief SDS field name.
   */

  string _sdsFieldName;
  

  /**
   * @brief Output URL.
   */

  string _outputUrl;
  

  ///////////////////////////////
  // Protected virtual methods //
  ///////////////////////////////

  /**
   * @brief Check the dimensions of the incoming data against what we are
   *        expecting.
   *
   * @param[in] num_lats The number of latitude values in the HDF file.
   * @param[in] num_lons The number of longitude values in the HDF file.
   * @param[in] dimensions The dimensions found for this data in the TRMM
   *                       file.
   *
   * @return Returns true if the dimensions of the data are okay, false
   *         otherwise.
   */

  virtual bool _checkDimensions(const int num_lats, const int num_lons,
				const vector< int > &dimensions) const = 0;
  

  /**
   * @brief Create the MDV files for this data.
   *
   * @param[in] input_path The input path of the raw data used for
   *                       information in the master header of the output
   *                       files.
   * @param[in] raw_data The raw data for the field.
   * @param[in] raw_proj The projection information for the raw data.
   * @param[in] dimensions The dimensions of the raw data.
   * @param[in] field_name_long The long field name for this field.
   * @param[in] field_units The units for this field.
   * @param[in] missing_data_value The missing data value for this field.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool _createMdvFiles(const string &input_path,
			       const float64 *raw_data,
			       const Pjg &raw_proj,
			       const vector< int > &dimensions,
			       const string &field_name_long,
			       const string &field_units,
			       const double missing_data_value) = 0;


};


#endif
