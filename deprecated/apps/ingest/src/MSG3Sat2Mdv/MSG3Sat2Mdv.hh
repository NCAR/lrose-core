
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
 * @file MSG3Sat2Mdv.hh
 * @brief Converts satellite NetCDF file of known format to Mdv.This is not
 *         a general netCDF file converter.
 *
 * @class  MSG3Sat2Mdv
 * @brief  Converts satellite NetCDF file of known format to Mdv. This is not
 *         a general netCDF file converter.
 */

#ifndef MSG3Sat2Mdv_H
#define MSG3Sat2Mdv_H


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

class MSG3Sat2Mdv {
 
public:
/**
   * Constructor 
   * Initialize data members, set up archive or real-time triggering based on
   * user defined parameters. Exit if driver does not initialize properly.
   * @param[in] argc  Number of command line arguments
   * @param[in] argv  Typical real-time and archive mode command lines are:
   *                  'MSG3Sat2Mdv -params MSG3Sat2Mdv.params' or
   *                  'MSG3Sat2Mdv -params MSG3Sat2Mdv.params -f [satFilePath]'
   */
  MSG3Sat2Mdv (int argc, char **argv);

  /**
   * Destructor  Free memory, unregister with the process mapper
   */
  ~MSG3Sat2Mdv();

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
   * Missing or bad data value for geolocation data
   */
  const static float GEO_MISSING;

  /**
   *  Missing or bad data for satellite data field
   */
  const static float SAT_MISSING;

  /**
   * Contants used as upper bound for determining max and mins. Used for debugging only.
   */
  const static float VERY_LARGE_NUM;

  /**
   * Contants used as upper lower for determining max and mins. Used for debugging only.
   */
  const static float VERY_SMALL_NUM;

  const static double EPSILON;
  const static double C1;
  const static double C2;

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
   * Mdv object made up of master header, field headers, vlevel headers, and field data 
   */
  DsMdvx _mdv;
  
  /**
   * Header for Mdv object
   */ 
  Mdvx::master_header_t _master_hdr;

  /**
   * Header for Mdv field object
   */ 
  Mdvx::field_header_t _field_hdr;
  
  /**
   * Header for vertical level component of Mdv object
   */ 
  Mdvx::vlevel_header_t _vlevel_hdr;

  /**
   * Output data projection object handles mapping satellite data to user specified
   * projection
   */
  MdvxPjg _outputProj;

  /**
   * Number of columns of satellite data
   */
  int _nx;

  /**
   * Number of rows of satellite data
   */
  int _ny;
  
  /**
   * Granule start time
   */
  DateTime _startTime;
  
  /**
   * Granule end time
   */
  DateTime _endTime;
  
  /**
   * Call methods to read the netCDF, create the Mdv object, and write the output file
   * @param[in] inputPath  path of input netCDF file returned by the data trigger.
   * @return  0 if successful, 1 otherwise
   */
  int _processData(char *inputPath); 

  /**
   * Read the netCDF file. The application expects latitude and longitude 
   * arrays stored as floats, and other satellite data field stored as shorts with scale and
   * bias. The names of these fields,and names of the data dimensions are entered as user 
   * defined parameters.
   * @param[in] filename  Name or path the netCDF file
   * @return   0 if successful, 1 otherwise
   */
  int _netcdf2MdvFields(char *filename);

  /**
   * Convert Mdv data to user specified storage type, apply compression, and write
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
   *  Adjust the start, end, and centroid times in Mdv header.
   */
  void _fieldHeaderAdjustNames( char *fieldNameLong, char *fieldNameShort, char *units );
  
  /**
   * Set ouput data projection object based on user parameters
   */
  void _setProjection();
  
  /**
   * Map satellite data to user specified projection
   */
  void _mapSatData2MdvArray(float* mdvDataVals, float *satDataVals, 
			    float*latitudes, float *longitudes);  

  void processSolarAngles(const time_t satTime, const float latitude, const float longitude,
			  float &sunEarthDist, float &azimuth);

  int getJulianDay(const time_t uTime);

};

#endif

