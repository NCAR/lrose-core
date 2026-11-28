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
 * @file MonthlySdsField.hh
 *
 * @class MonthlySdsField
 *
 * Class representing a monthly climotology SDS field.
 *  
 * @date 11/18/2008
 *
 */

#ifndef MonthlySdsField_HH
#define MonthlySdsField_HH

#include <string>

#include <Mdv/DsMdvx.hh>

#include "SdsDataField.hh"

using namespace std;

/** 
 * @class MonthlySdsField
 */

class MonthlySdsField : public SdsDataField
{
 public:

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
   * @param[in] output_url The output URL for this field.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  MonthlySdsField(const string &sds_field_name,
		  const string &prog_name,
		  const string &output_url,
		  const bool debug_flag = false,
		  const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~MonthlySdsField(void);
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef enum
  {
    LAT_DIM_NUMBER,
    LON_DIM_NUMBER,
    MONTH_DIM_NUMBER,
    NUM_DIMENSIONS
  } dimensions_t;
  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Year to use for the monthly climatology output.  This must match
   *        what is expected by the DsMdvServer for the data to be served
   *        correctely.
   */

  static const int CLIMO_YEAR;
  

  /**
   * @brief Day to use for the monthly climatology output.  This must match
   *        what is expected by the DsMdvServer for the data to be served
   *        correctely.
   */

  static const int CLIMO_DAY;
  

  /**
   * @brief Hour to use for the monthly climatology output.  This must match
   *        what is expected by the DsMdvServer for the data to be served
   *        correctely.
   */

  static const int CLIMO_HOUR;
  

  /**
   * @brief Minute to use for the monthly climatology output.  This must match
   *        what is expected by the DsMdvServer for the data to be served
   *        correctely.
   */

  static const int CLIMO_MINUTE;
  

  /**
   * @brief Second to use for the monthly climatology output.  This must match
   *        what is expected by the DsMdvServer for the data to be served
   *        correctely.
   */

  static const int CLIMO_SECOND;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

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
				const vector< int > &dimensions) const;
  

  /**
   * @brief Create the climatology field for the given month.
   *
   * @param[in] raw_data The raw data from the HDF file.
   * @param[in] raw_proj The projection for the raw data.
   * @param[in] month_index The index for the desired month in the raw data.
   * @param[in] field_name_long The long field name for this field.
   * @param[in] field_units The units for this field.
   * @param[in] missing_data_value The missing data value for this field.
   *
   * @return Returns a pointer to the climo field on success, 0 on failure.
   */

  MdvxField *_createClimoField(const float64 *raw_data,
			       const Pjg &raw_proj,
			       const int month_index,
			       const string &field_name_long,
			       const string &field_units,
			       const double missing_data_value) const;
  

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
			       const double missing_data_value);


  /**
   * @brief Set the master header for thie given output file.
   *
   * @param[in,out] out_mdvx The output MDV file.
   * @param[in] month The month for the file.
   * @param[in] input_path The path for the input file used to create this
   *                       MDV file.
   */

  void _setMasterHeader(DsMdvx &out_mdvx,
			const int month,
			const string &input_path);
  

};


#endif
