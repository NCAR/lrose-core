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
   *
   * @param[in] file_path Path for HDF file to process.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   */

  HdfFile(const string &file_path,
	  const bool debug_flag = false,
	  const bool verbose_flag = false);
  

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
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the scan time range from the file.
   *
   * @param[out] begin_time The beginning scan time for this file.
   * @param[out] end_time The ending scan time for this file.
   *
   * @return Returns true on success, false on failure.
   */

  bool getScanTimeRange(DateTime &begin_time, DateTime &end_time);
  

  /**
   * @brief Get the scan time vector.
   *
   * @return Returns a reference to the scan time vector.  The vector
   *         will be empty if there was an error reading it.
   */

  inline const vector< DateTime > &getScanTimes()
  {
    _readScanTimes();
    
    return _scanTimes;
  }
  

  /**
   * @brief Get the sun magnitudes from the file.
   *
   * @param[out] sun_mags The sun magnitudes.
   *
   * @return Returns true on success, false on failure.
   */

  bool getSunMags(vector< double > &sun_mags);
  

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
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Maximum number of characters in a single field name in
   *        a TRMM file.
   */

  static const int MAX_FIELD_NAME_LEN;
  

  /**
   * @brief Maximum number of characters in a field list.
   */

  static const int MAX_FIELD_LIST_LEN;
  

  /**
   * @brief The name of the scan time VDATA in the TRMM files
   */

  static const string SCAN_TIME_VDATA_NAME;
  

  /**
   * @brief The list of fields in the scan_time VDATA section for the first
   *        type of scan time storage.
   */

  static const string SCAN_TIME_FIELD_LIST1;
  

  /**
   * @brief The list of fields in the scan_time VDATA section for the second
   *        type of scan time storage.
   */

  static const string SCAN_TIME_FIELD_LIST2;
  

  /**
   * @brief The name of the solar calibration VDATA in the TRMM file.
   */

  static const string SOLAR_CAL_VDATA_NAME;
  

  /**
   * @brief The name of the sun magnitude field in the solar calibration
   *        VDATA.
   */

  static const string SOLAR_CAL_SUN_MAG_FIELD_NAME;
  

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
   * @brief Flag indicating whether the scan time information has been
   *        read from this file yet.
   */

  bool _scanTimesRead;
  

  /**
   * @brief List of scan times for each scan in the file.
   */

  vector< DateTime > _scanTimes;
  

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
   * @brief Read the scan time data from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readScanTimes();
  

  /**
   * @brief Read the scan times from the TRMM file using format 1.
   *
   * @param[in] vdata_id VDATA identifier for the scan_time data.
   * @param[in] n_records The number of records in the VDATA.
   * @param[in] interlace Interlace mode used by the VDATA.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readScanTimes1(const int32 vdata_id,
		       const int32 n_records, const int32 interlace);
  

  /**
   * @brief Read the scan times from the TRMM file using format 2.
   *
   * @param[in] vdata_id VDATA identifier for the scan_time data.
   * @param[in] n_records The number of records in the VDATA.
   * @param[in] interlace Interlace mode used by the VDATA.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readScanTimes2(const int32 vdata_id,
		       const int32 n_records, const int32 interlace);
  

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
