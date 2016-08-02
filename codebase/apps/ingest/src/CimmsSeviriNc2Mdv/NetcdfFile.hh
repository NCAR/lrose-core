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
 * @file NetcdfFile.hh
 *
 * @class NetcdfFile
 *
 * Class controlling access to a CIMMS netCDF file.
 *  
 * @date 11/17/2008
 *
 */

#ifndef NetcdfFile_HH
#define NetcdfFile_HH

#include <string>
#include <vector>

#include <netcdf.hh>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "GridHandler.hh"

using namespace std;

/** 
 * @class NetcdfFile
 */

class NetcdfFile
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

  NetcdfFile(const string &file_path,
	     const bool debug_flag = false,
	     const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~NetcdfFile(void);
  

  /**
   * @brief Initialize the netCDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool init();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the band data from the netCDF file, returning it as MDV
   *        fields.
   *
   * @param[out] fields The MDV fields containing the band data.
   * @param[in] mdv_proj The projection for the MDV file.
   * @param[in] grid_handler The data grid handler.
   * @param[in] output_grib_code The GRIB code to use for the fields
   *                             in the output MDV file.
   *
   * @return Returns true on success, false on failure.
   */
  
  bool getBandsAsMdv(vector< MdvxField* > &fields,
		     const MdvxPjg &mdv_proj,
		     GridHandler &grid_handler,
		     const int output_grib_code) const;
  

  /**
   * @brief Get the image time from the file.
   *
   * @return Returns the image time from the file on success,
   *         DateTime::NEVER on failure.
   */

  DateTime getImageTime() const;
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Name of the image date variable in the netCDF file.
   */

  static const string IMAGE_DATE_VAR_NAME;
  
  /**
   * @brief Name of the image time variable in the netCDF file.
   */

  static const string IMAGE_TIME_VAR_NAME;
  
  /**
   * @brief Name of the X dimension in the netCDF file.
   */

  static const string X_DIM_NAME;

  /**
   * @brief Name of the Y dimension in the netCDF file.
   */

  static const string Y_DIM_NAME;
  
  /**
   * @brief Name of the dimension in the netCDF file which indicates the
   *        number of bands included in this file.
   */

  static const string BANDS_DIM_NAME;
  
  /**
   * @brief Name of the latitude variable in the netCDF file.
   */

  static const string LAT_VAR_NAME;

  /**
   * @brief Name of the longitude variable in the netCDF file.
   */

  static const string LON_VAR_NAME;
  
  /**
   * @brief Name of the band numbers variable in the netCDF file.
   */

  static const string BAND_NUMS_VAR_NAME;
  
  /**
   * @brief Name of the bands data variable in the netCDF file.
   */

  static const string DATA_VAR_NAME;

  /**
   * @brief Name of the bands units attribute in the netCDF file.
   */

  static const string BANDS_UNITS_ATT_NAME;


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
   * @brief The path to the CIMMS netCDF file.
   */

  string _filePath;
  

  /**
   * @brief The netCDF file object.
   */

  NcFile _ncFile;
  

  /**
   * @brief The number of elements in the X direction.
   */

  int _nx;
  

  /**
   * @brief The number of elements in the Y direction.
   */

  int _ny;
  

  /**
   * @brief The number of bands in the file.
   */

  int _numBands;
  

  /**
   * @brief The latitude values from this file.
   */

  float *_latitudes;
  

  /**
   * @brief The longitude values from this file.
   */

  float *_longitudes;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Get the data dimensions from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getDimensions();
  

  /**
   * @brief Get the latitude/longitude values from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getLatLon();
  

  /**
   * @brief Update the given MDV field header with the band field 
   *        information.
   *
   * @param[in] band_num The index number of this band in the netCDF file.
   * @param[out] field_hdr The MDV field header.
   * @param[in] mdv_proj The projection to use for the MDV field.
   * @param[in] units The units for the band field.
   * @param[in] output_grib_code The GRIB code to use for these fields.
   *
   * @return Returns true on success, false on failure.
   */

  bool _updateFieldHeader(const int band_num,
			  Mdvx::field_header_t &field_hdr,
			  const MdvxPjg &mdv_proj,
			  const string &units,
			  const int output_grib_code) const;
  
};


#endif
