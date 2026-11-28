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
 * @file HdfFile.hh
 *
 * @class HdfFile
 *
 * Class controlling access to a TRMM HDF file.
 *  
 * @date 10/31/2008
 *
 */

#ifndef HdfFile_HH
#define HdfFile_HH

#include <string>
#include <vector>

#include <hdf/hdf.h>
#include <hdf/mfhdf.h>

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include <toolsa/DateTime.hh>

using namespace std;

/** 
 * @class HdfFile
 */

class HdfFile
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

  HdfFile(const string &file_path,
	  const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~HdfFile(void);
  

  /**
   * @brief Initialize the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool init();
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /**
   * @brief Read the indicated SDS data from the TRMM file.
   *
   * @param[in] field_name The SDS field name for the data.
   * @param[out] dimensions The dimensions of the SDS field.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *readSdsData(const string &field_name,
		    vector< int > &dimensions) const;
  

  /**
   * @brief Read the indicated SDS data from the TRMM file.
   *
   * @param[in] field_name The SDS field name for the data.
   * @param[out] dimensions The dimensions of the SDS field.
   * @param[out] field_long_name The field long name as read from the HDF file.
   * @param[out] field_units The field units as read from the HDF file.
   * @param[out] missing_data_value The field missing data value as read
   *                                from the HDF file.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *readSdsData(const string &field_name,
		    vector< int > &dimensions,
		    string &field_long_name,
		    string &field_units,
		    double &missing_data_value) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the file path for this file.
   */

  string getFilePath() const
  {
    return _filePath;
  }
  

  /**
   * @brief Get the number of latitudes in the file.
   *
   * @return Returns the number of latitudes in the file.
   */

  int getNumLats() const
  {
    return _numLatitudes;
  }
  

  /**
   * @brief Get the number of longitudes in the file.
   *
   * @return Returns the number of longitudes in the file.
   */

  int getNumLons() const
  {
    return _numLongitudes;
  }
  

  /**
   * @brief Get the projection information for the file.
   *
   * @return Returns the projection for the file.
   */

  Pjg getProj() const
  {
    return _proj;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The name of the latitude variable in the SDS data section of
   *        the HDF file.
   */

  static const string LAT_VAR_NAME;
  

  /**
   * @brief The name of the longitude variable in the SDS data section of
   *        the HDF file.
   */

  static const string LON_VAR_NAME;
  

  /**
   * @brief Field long name attribute name in the HDF file.
   */

  static const string FIELD_LONG_NAME_ATTR_NAME;
  

  /**
   * @brief Field units attribute name in the HDF file.
   */

  static const string FIELD_UNITS_ATTR_NAME;
  

  /**
   * @brief Field missing value attribute name in the HDF file.
   */

  static const string FIELD_MISSING_VALUE_ATTR_NAME;
  

  /**
   * @brief Maximum number of characters in a single field name in
   *        an HDF file.
   */

  static const int MAX_FIELD_NAME_LEN;
  

  /**
   * @brief Latitude offset value.
   */

  static const double LAT_OFFSET;
  

  /**
   * @brief Longitude offset value.
   */

  static const double LON_OFFSET;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  

  /**
   * @brief The path to the TRMM HDF file.
   */

  string _filePath;
  

  /**
   * @brief HDF file identifier.
   */

  int32 _fileId;
  

  /**
   * @brief HDF SDS interface identifier.
   */

  int32 _sdId;
  

  /**
   * @brief The number of latitudes in the file.
   */

  int _numLatitudes;
  

  /**
   * @brief The latitude values in the file.
   */

  float *_latitudes;
  

  /**
   * @brief The number of longitudes in the file.
   */

  int _numLongitudes;
  

  /**
   * @brief The longitude values in the file.
   */

  float *_longitudes;
  

  /**
   * @brief The projection of the input data.
   */

  Pjg _proj;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Extrac the date information from the TRMM filename.
   *
   * @return Returns the file date on success, DateTime::NEVER on failure.
   */

  DateTime _getDateFromFilename() const;
  

  /**
   * @brief Get the latitude and longitude information from the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getLatLon();
  

  /**
   * @brief Get the specified float attribute from the HDF file.
   *
   * @param[in] sds_id SDS identifier.
   * @param[in] attr_name Attribute name.
   * @param[out] float_attr Float attribute value.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getFloatAttribute(const int32 sds_id,
			  const string &attr_name,
			  double &float_attr) const;
  

  /**
   * @brief Get the specified string attribute from the HDF file.
   *
   * @param[in] sds_id SDS identifier.
   * @param[in] attr_name Attribute name.
   * @param[out] string_attr String attribute value.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getStringAttribute(const int32 sds_id,
			   const string &attr_name,
			   string &string_attr) const;
  

  /**
   * @brief Read data for a field that is stored as float32.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readFloat32Data(const string &field_name,
			 int32 sds_id,
			 int32 num_elements,
			 int32 *edges, int32 *stride,
			 int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as float64.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readFloat64Data(const string &field_name,
			 int32 sds_id,
			 int32 num_elements,
			 int32 *edges, int32 *stride,
			 int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as int8.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readInt8Data(const string &field_name,
		      int32 sds_id,
		      int32 num_elements,
		      int32 *edges, int32 *stride,
		      int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as int16.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readInt16Data(const string &field_name,
		       int32 sds_id,
		       int32 num_elements,
		       int32 *edges, int32 *stride,
		       int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as int32.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readInt32Data(const string &field_name,
		       int32 sds_id,
		       int32 num_elements,
		       int32 *edges, int32 *stride,
		       int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as uint8.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readUint8Data(const string &field_name,
		       int32 sds_id,
		       int32 num_elements,
		       int32 *edges, int32 *stride,
		       int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as uint16.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readUint16Data(const string &field_name,
			int32 sds_id,
			int32 num_elements,
			int32 *edges, int32 *stride,
			int32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as uint32.
   *
   * @param[in] field_name SDS field name.
   * @param[in] sds_id The SDS identifier for this field.
   * @param[in] num_elements Number of elements to read in this field.
   * @param[in] edges Edges to read in this data (see HDF4 documentation).
   * @param[in] stride Stride to read in this data (see HDF4 documentation).
   * @param[in] start Starting elements to read in this data (see HDF4
   *                  documentation.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readUint32Data(const string &field_name,
			int32 sds_id,
			int32 num_elements,
			int32 *edges, int32 *stride,
			int32 *start) const;
  
};


#endif
