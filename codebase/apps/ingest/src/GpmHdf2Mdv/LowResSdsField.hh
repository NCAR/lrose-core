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
 * @file LowResSdsField.hh
 *
 * @class LowResSdsField
 *
 * Class representing a TRMM SDS field that is stored at full resolution.
 *  
 * @date 10/31/2008
 *
 */

#ifndef LowResSdsField_HH
#define LowResSdsField_HH

#include <string>

#include "SdsDataField.hh"

using namespace std;

/** 
 * @class LowResSdsField
 */

class LowResSdsField : public SdsDataField
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
   */

  LowResSdsField(const string &sds_field_name,
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

  virtual ~LowResSdsField(void);
  

protected:

  ///////////////////////
  // Protected methods //
  ///////////////////////

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
				const vector< int > &dimensions);
  

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
				const fl64 *raw_data);
  

};


#endif
