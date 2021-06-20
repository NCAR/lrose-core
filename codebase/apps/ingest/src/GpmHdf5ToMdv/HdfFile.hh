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

#include <Ncxx/Hdf5xx.hh>
#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>

using namespace std;



/** 
 * @class HdfFile
 */

class HdfFile
{
 public:
  //////////////////////
  // Public constants //
  //////////////////////

  static const double MISSING_VALUE;

  typedef struct
  {
    double lat;
    double lon;
    time_t scan_time;
    double sun_mag;
    double solar_zenith;
    double cos_solar_zenith;
  } location_t;

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] file_path Path for TRMM HDF file to process.
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
   * @brief Tell the class to read the solar calibration data from the
   *        HDF file.  Must be called before calling init() to have any
   *        effect.
   */

  inline void setReadMagData()
  {
    _readSunMagDataFlag = true;
  }

  /**
   * @brief Initialize the TRMM HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool init();
  
  /**
   * @brief Reads basic information from the trmm file (numScans, numPixels, geolocations)
   *
   * @return Returns true on success, false on failure.
   */

  bool readDataHeaders();

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the number of scans in the TRMM data.
   *
   * @return Returns the number of scans in the data.
   */

  inline int getNumScans() const
  {
    return _numScans;
  }

  /**
   * @brief Get the number of pixels in the TRMM data.
   *
   * @return Returns the number of pixels in the data.
   */

  inline int getNumPixels() const
  {
    return _numPixels;
  }

  /**
   * @brief Get the location information for the given scan number and pixel number.
   *
   * @return Returns the latitude value.
   */

  inline location_t getLocationInfo(const int i) const
  {
    if(i >= 0 && i < _numScans)
      return _locations[i];
    location_t missingLocation = {-9999.99, -9999.99, 0, -9999.99, -9999.99, -9999.99};
    return missingLocation;
  }

  /**
   * @brief Get the latitude for the given data index.
   *
   * @return Returns the latitude value.
   */

  inline double getLat(const int i) const
  {
    return _locations[i].lat;
  }
  

  /**
   * @brief Get the longitude for the given data index.
   *
   * @return Returns the longitude value.
   */

  inline double getLon(const int i) const
  {
    return _locations[i].lon;
  }
  

  /**
   * @brief Get the scan time for the given data index.
   *
   * @return Returns the scan time.
   */

  inline time_t getScanTime(const int i) const
  {
    return _locations[i].scan_time;
  }
  

  /**
   * @brief Get the sun magnitude for the given data index.
   *
   * @return Returns the sun magnitude.
   */

  inline double getSunMag(const int i) const
  {
    return _locations[i].sun_mag;
  }
  

  /**
   * @brief Get the solar zenith for the given data index.
   *
   * @return Returns the solar zenith.
   */

  inline double getSolarZenith(const int i) const
  {
    return _locations[i].solar_zenith;
  }
  

  /**
   * @brief Get the cosine of the solar zenith for the given data index.
   *
   * @return Returns the cosine of the solar zenith.
   */

  inline double getCosSolarZenith(const int i) const
  {
    return _locations[i].cos_solar_zenith;
  }
  
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
    _readScanTimeData();
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
   * @brief Name of the geolocation SDS fields in the TRMM datasets.
   */

  static const string LATITUDE_SDS_FIELD_NAME;
  static const string LONGITUDE_SDS_FIELD_NAME;
  //static const string GEOLOCATION_SDS_FIELD_NAME;

  /**
   * @brief Name of the local direction SDS field in the TRMM datasets.
   *        This is the field than contains the solar zenith values.
   */

  static const string LOCAL_DIR_SDS_FIELD_NAME;

  /**
   * @brief The number of pixels represented by a single angle in the local
   *        direction data in the TRMM datasets.
   */

  static const int LOCAL_DIR_PIXELS_PER_ANGLE;
  
  /**
   * @brief The names of the scan time components 
   */

  static const string SCAN_TIME_YEAR;
  static const string SCAN_TIME_MONTH;
  static const string SCAN_TIME_DAY;
  static const string SCAN_TIME_HOUR;
  static const string SCAN_TIME_MINUTE;
  static const string SCAN_TIME_SECOND;
  


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

  si32 _fileId;
  

  /**
   * @brief HDF SDS interface identifier.
   */

  si32 _sdId;
  
  /**
   * @brief Flag indicating whether to try to retieve the solar calibration
   *        data from the HDF file.
   */

  bool _readSunMagDataFlag;
  
  /**
   * @brief Number of scans in the TRMM data.
   */

  int _numScans;
  

  /**
   * @brief Number of pixels in the TRMM data.
   */

  int _numPixels;

  /**
   * @brief Flag indicating whether the scan time information has been
   *        read from this file yet.
   */

  bool _scanTimesRead;
  

  /**
   * @brief List of scan times for each scan in the file.
   */

  vector< DateTime > _scanTimes;
  
  /**
   * @brief List of locations of data points in this TRMM file.  The locations
   *        are ordered by [scan][pixel].
   */

  vector< location_t > _locations;

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
   * @brief Read the geolocation data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readGeolocationData();
  

  /**
   * @brief Read the scan time data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readScanTimeData();
  

  /**
   * @brief Read the solar magnitude data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readSunMagData();
  

  /**
   * @brief Read the solar zenith data from the HDF file.
   *
   * @param[in] hdf_file The HDF file object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readSolarZenithData();
  
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
			 si32 sds_id,
			 si32 num_elements,
			 si32 *edges, si32 *stride,
			 si32 *start) const;
  
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

  fl64 *_readFl64Data(const string &field_name,
                      si32 sds_id,
                      si32 num_elements,
                      si32 *edges, si32 *stride,
                      si32 *start) const;
  
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
		      si32 sds_id,
		      si32 num_elements,
		      si32 *edges, si32 *stride,
		      si32 *start) const;
  
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
		       si32 sds_id,
		       si32 num_elements,
		       si32 *edges, si32 *stride,
		       si32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as si32.
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
		       si32 sds_id,
		       si32 num_elements,
		       si32 *edges, si32 *stride,
		       si32 *start) const;
  
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
		       si32 sds_id,
		       si32 num_elements,
		       si32 *edges, si32 *stride,
		       si32 *start) const;
  
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
			si32 sds_id,
			si32 num_elements,
			si32 *edges, si32 *stride,
			si32 *start) const;
  
  /**
   * @brief Read data for a field that is stored as usi32.
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
                        si32 sds_id,
                        si32 num_elements,
                        si32 *edges, si32 *stride,
                        si32 *start) const;
  
};


#endif
