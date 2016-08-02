
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 * @file PolarSat2Mdv.hh
 * @brief Converts satellite NetCDF file of known format to Mdv.This is not
 *         a general netCDF file converter.
 *        
 *        See http://www.nws.noaa.gov/om/notification/tin12-45viirsaaa.htm for 
 *        file details
 *
 *        See Joint Polar Satellite System (JPSS) Operational Algorithm Description
 *        (OAD) Document for VIIRS Ground Track Mercator (GTM) Imagery Environmental 
 *        Data Record (EDR) Software for discussion on geolocation information 
 *        contained in the file.
 *
 * @class  PolarSat2Mdv
 * @brief  Converts satellite NetCDF file of known format to Mdv. This is not
 *         a general netCDF file converter.
 *     
 *         See http://www.nws.noaa.gov/om/notification/tin12-45viirsaaa.htm for 
 *         file details
 *
 *         See Joint Polar Satellite System (JPSS) Operational Algorithm Description
 *         (OAD) Document for VIIRS Ground Track Mercator (GTM) Imagery Environmental 
 *         Data Record (EDR) Software for discussion on geolocation information 
 *         contained in the file.
 */

#ifndef PolarSat2Mdv_H
#define PolarSat2Mdv_H


#include <string>
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include <dsdata/TriggerInfo.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

class PolarSat2Mdv {
 

public:
/**
   * Constructor 
   * Initialize data members, set up archive or real-time triggering based on
   * user defined parameters. Exit if driver does not initialize properly.
   * @param[in] argc  Number of command line arguments
   * @param[in] argv  Typical real-time and archive mode command lines are:
   *                  'PolarSat2Mdv -params PolarSat2Mdv.params' or
   *                  'PolarSat2Mdv -params PolarSat2Mdv.params -f [satFilePath]'
   */
  PolarSat2Mdv (int argc, char **argv);

  /**
   * Destructor  Free memory, unregister with the process mapper
   */
  ~PolarSat2Mdv();

  /**
   * Run For each data trigger, process input data
   */
  int Run();

  
  /**
   * Flag to indicate object initialized properly
   */
  bool isOK;

protected:
  
private:
 
  /**
   * Value of Pi used in spherical geometry calculations
   */
  const static float MYPI;

  /**
   * Missing or bad data value for geolocation data
   */
  const static float GEO_MISSING;

  /**
   *  Missing or bad data for satellite data field
   */
  const static float SAT_MISSING;

  /**
   * Radius of the earth in kilometers
   */
  const static float R_km;
  
  /**
   * Minimum value considered legal for a denominator
   */
  const static float EPSILON;

  /**
   * Contants used as upper bound for determining max and mins. Used for debugging only.
   */
  const static float VERY_LARGE_NUM;

  /**
   * Contants used as upper lower for determining max and mins. Used for debugging only.
   */
  const static float VERY_SMALL_NUM;

  /**
   * This is the number of sparse geolocaton data points in the cross track of the granule.
   * It includes the ground track point and a point to the east and west of the ground track.
   * This number is known to be and expected to be 3. This algorithm will not work for sparse
   * data of different cross track dimension.
   */
  const static int   NUM_SPARSE_PTS_CROSS_TRACK;

  /**
   * Used in debug messages
   */
  string _progName;
 
  /**
   * Object to parse command line arguments
   */
  Args _args;
  
  /**
   * Object containing user defined parameters
   */
  Params _params;
  
  /**
   * File trigger. Note that in real-time mode this trigger requires input files to be 
   * named yyyymmdd/hhmmss.nc
   */
  DsInputPath *_fileTrigger;
  
  /**
   * Header for Mdv object
   */ 
  Mdvx::master_header_t _master_hdr;
  
  /**
   * Header for vertical level component of Mdv object
   */ 
  Mdvx::vlevel_header_t _vlevel_hdr;

  /**
   * Header for Mdv field data
   */ 
  Mdvx::field_header_t _field_hdr;

  /**
   * Output data projection object handles mapping satellite data to user specified
   * projection
   */
  MdvxPjg _outputProj;

  /**
   * Flag to indicate that processed satellite data exists in Mdv data array
   * Data may be stored if user chooses to collect multiple granules per Mdv file
   */
  bool _haveDataInMdvArray;

  /**
   *  Time span of data in Mdv object.
   */ 
  int _dataTimeSpan;

  /**
   * Array holds latitudes of every data point in the granule. These are calculated
   * from the sparse geo location information contained in the file.
   */
  float *_latitudes;
  
  /**
   * Array holds longitudes of every data point in the granule.These are calculated
   * from the sparse geo location information contained in the file.
   */
  float *_longitudes;
  
  /**
   * Pointer to array of satellite data field values (with scale and bias applied)
   * at resolution in data file
   */
  float * _satDataVals;
  
  /**
   * Pointer to array of satellite data field values on user specified projection
   * for mdv data file.
   */
  float * _mdvDataVals;
  
  /**
   * Granule start time
   */
  DateTime _startTime;
  
  /**
   * Granule end time
   */
  DateTime _endTime;
  
  /**
   * Number of data points in granule which are cross track to ground track
   */
  int _numPtsCrossTrack; 
  
  /**
   * Number of data points in granule ground track
   */
  int _numPtsAlongTrack;
  
  /**
   * Free allocated memory created for Mdv object
   */ 
  void _clearMdvData();

  /**
   * Free allocated memory created reading NetCDF file
   */
  void _clearNcfData();
  
  /**
   * Call methods to read the netCDF, create the Mdv object, and write the output file
   * @param[in] inputPath  path of input netCDF file returned by the data trigger.
   * @return  0 if successful, 1 otherwise
   */
  int _processData(char *inputPath); 

  /**
   * Read the netCDF file. The application expects nx3 sparse latitude and longitude 
   * arrays stored as floats, one satellite data field stored as shorts with scale and
   * bias. The names of these fields, and the ground track and cross track granule 
   * data dimensions are entered as user defined parameters.
   * @param[in] filename  Name or path the netCDF file
   * @return   0 if successful, 1 otherwise
   */
  int _readNetcdf(char *filename);

  /**
   * An angle measure in degrees [0,360] from point0 to point1 is returned
   * @param[in] lat0Deg
   * @param[in] lon0Deg
   * @param[in] lat1Deg
   * @param[in] lon1Deg
   * @return angle measure between points
   */ 
  float _bearing(float lat0Deg, float lon0Deg, float lat1Deg, float lon1Deg);

  /**
   * Given a starting point latitude and longitude, a great circle distance on the earths surface,
   * and a bearing, find the end point latitude and longitude. 
   * @param[in] startLatDeg  Starting point latitude in interval[-90,90]
   * @param[in] startLonDeg  Starting point latitude  in interval[-180,180]
   * @param[in] bearingDeg  Bearing in degrees from starting point to end point in interval[0,360]
   * @param[in] distanceKm  Distance between granule data points along great circle arc 
   *                        on the earths surface 
   * @param[out] endLatDeg  Latitude of end point in degrees in the interval [-90,90]
   * @param[out] endLonDeg  Longitude of end point in degrees in the interval[-180,180]
   */ 
  void _startPtBearingDist2LatLon(float startLatDeg, float startLonDeg, float bearingDeg, 
				  float distanceKm, float &endLatDeg, float &endLonDeg);

  /**
   * Fill out arrays of latitude and longitude for every point in granule.
   * First compute bearing to cross track end points along each point of the ground track.
   * Given that the cross track data dimension is known, and the distance between granule data
   * points is .375 km, use the bearing along cross tracks to compute all data point
   * latitides and longitudes using spherical trigonometery
   * @param[in] sparseLats An nx3 array of ground track latitudes and latitudes of cross track end points
   * @param[in] sparseLons An nx3 array of ground track logitudes and longitudes of cross track end points
   * @return 0 if successful, otherwise
   */
  int _sparseToFullGeolocationData(float sparseLats[][3], float sparseLons[][3]);

  /**
   * Create a master header, field header, vlevel header, and convert granule data to 
   * user specified projection.  Add all parts to the Mdvx object
   * @param[in]  mdv
   */
  void _createMdv(DsMdvx &mdv);
  
  /**
   * Convert Mdv data ( need to add user specified storage type, apply compression), and write
   * to user specified directory. 
   * @param[in]  mdv
   * @return  0 if successful, 1 otherwise
   */
  int _writeMdv();
  
  /**
   *  Set members of header for Mdv object.
   */
  void _setMasterHeader();

  /**
   *  Adjust the start, end, and centroid times in Mdv header.
   */
  void _masterHdrAdjustTime();

  /**
   * Set members of vertical level object which is part of Mdv object. 
   */
  void _setVlevelHdr();

  /**
   * Set members of Mdv field header.  
   */
  void _setFieldHdr();
  
  /**
   * Set ouput data projection object based on user parameters
   */
  void _setProjection();
  
  /**
   * Map satellite data to user specified projection
   */
  void _mapSatData2MdvArray();  
};

#endif

