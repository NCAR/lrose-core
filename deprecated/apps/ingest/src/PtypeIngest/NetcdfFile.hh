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
 * Class controlling access to a HRRR 15 minute netCDF file.
 *  
 * @date 12/02/2009
 *
 */

#ifndef NetcdfFile_HH
#define NetcdfFile_HH

#include <string>
#include <vector>

#include <Ncxx/Ncxx.hh>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/DateTime.hh>

#include "NetcdfField.hh"

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
	     const int forecast_interval_secs,
	     const Mdvx::compression_type_t output_compression_type,
	     const Mdvx::scaling_type_t output_scaling_type,
	     const double output_scale,
	     const double output_bias,
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
  

  /**
   * @brief Add a field to be processed.
   *
   * @param[in] nc_field Field to be processed.
   *
   * @note Note that the calling method retains control of the field object
   *       and must delete the pointer when it is no longer needed.
   */

  void addField(NetcdfField *field)
  {
    _fieldList.push_back(field);
  }
  

  ////////////////////
  // Output methods //
  ////////////////////

  /**
   * @brief Create the output MDV files at the given URL.
   *
   * @param[in] output_url Output MDV URL.
   *
   * @return Returns true on success, false on failure.
   */

  bool createMdvFiles(const string &output_url) const;
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Name of the X dimension in the netCDF file.
   */

  static const string X_DIM_NAME;

  /**
   * @brief Name of the Y dimension in the netCDF file.
   */

  static const string Y_DIM_NAME;
  
  /**
   * @brief Name of the time dimension in the netCDF file.
   */

  static const string TIME_DIM_NAME;
  
  /**
   * @brief Name of the center latitude global attribute in the netCDF file.
   */

  static const string MINX_ATT_NAME;
  
  /**
   * @brief Name of the center longitude global attribute in the netCDF file.
   */

  static const string MINY_ATT_NAME;
  
  /**
   * @brief Name of the latitude 1 global attribute in the netCDF file.
   */

  static const string LAT1_ATT_NAME;
  
  /**
   * @brief Name of the latitude 2 global attribute in the netCDF file.
   */

  static const string LAT2_ATT_NAME;
  
  /**
   * @brief Name of the standard longitude global attribute in the netCDF file.
   */

  static const string STAND_LON_ATT_NAME;

  /**
   * @brief Name of the dx global attribute in the netCDF file.
   */

  static const string DX_ATT_NAME;
  
  /**
   * @brief Name of the dy global attribute in the netCDF file.
   */

  static const string DY_ATT_NAME;
  
  /**
   * @brief Name of the year global attribute in the netCDF file.
   */

  static const string BASE_TIME_ATT_NAME;
  

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
   * @brief The forecast interval for the data in the input files in
   *        seconds.
   */

  int _forecastIntervalSecs;
  
  /**
   * @brief List of fields to extract from the netCDF file.
   *
   * @note These pointers are owned by the calling method and should not
   *       be deleted here.
   */

  vector< NetcdfField* > _fieldList;
  
  /**
   * @brief Type of compression to use for the MDV fields when writing
   *        them to the output files.
   */

  Mdvx::compression_type_t _outputCompressionType;
  
  /**
   * @brief Type of scaling to use for the MDV fields when writing
   *        them to the output files.
   */

  Mdvx::scaling_type_t _outputScalingType;
  
  /**
   * @brief Scale value to use when scaling output.
   */

  double _outputScale;
  
  /**
   * @brief Bias value to use when scaling output.
   */

  double _outputBias;
  
  /**
   * @brief The path to the netCDF file.
   */

  string _filePath;
  

  /**
   * @brief The netCDF file object.
   */

  NcxxFile _ncFile;
  

  /**
   * @brief The number of elements in the X direction.
   */

  int _nx;
  

  /**
   * @brief The number of elements in the Y direction.
   */

  int _ny;
  

  /**
   * @brief The number of forecast times in the data.
   */

  int _numTimes;
  

  /**
   * @brief The projection of the data in the netCDF file.
   */

  MdvxProj _inputProj;
  

  /**
   * @brief The base time of the data in the netCDF file.
   */

  DateTime _baseTime;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Create the MDV file for the indicated forecast.
   *
   * @param[in] output_url The URL for the MDV files.
   * @param[in] forecast_index The index of this forecast in the input file.
   * @param[in] forecast_secs The forecast lead time in seconds.
   * @param[in] forecast_time The forecast valid time.
   *
   * @return Returns true on success, false on failure.
   */

  bool _createMdvFile(const string &output_url,
		      const int forecast_index,
		      const int forecast_secs,
		      const DateTime &forecast_time) const;
  

  /**
   * @brief Get the base time from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getBaseTime();
  

  /**
   * @brief Get the specified data dimension from the file.
   *
   * @param[in] dim_name The dimension name.
   *
   * @return Returns the size of the dimension on success, -1 on failure.
   */

  inline int _getDimension(const string &dim_name) const
  {
    NcxxDim dim = _ncFile.getDim(dim_name);
    if(dim.isNull()) {
      return -1;
    }
    return dim.getSize();
  }
  
  /**
   * @brief Get the data dimensions from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getDimensions();
  

  /**
   * @brief Get the specified global attribute as a float value.
   *
   * @param[in] att_name The global attribute name.
   *
   * @return Returns the attribute value on success,
   *         NC_FILL_FLOAT on failure.
   */

  float _getGlobalAttAsFloat(const string &att_name) const
  {
    try {
      NcxxGroupAtt att = _ncFile.getAtt(att_name);
      float val;
      att.getValues(&val);
      return val;
    } catch (NcxxException& e) {
      return NC_FILL_FLOAT;
    }
  }
  
  /**
   * @brief Get the specified global attribute as an integer value.
   *
   * @param[in] att_name The global attribute name.
   *
   * @return Returns the attribute value on success,
   *         NC_FILL_INT on failure.
   */

  int _getGlobalAttAsInt(const string &att_name) const
  {
    try {
      NcxxGroupAtt att = _ncFile.getAtt(att_name);
      int val;
      att.getValues(&val);
      return val;
    } catch (NcxxException& e) {
      return NC_FILL_INT;
    }
  }
  
  /**
   * @brief Get the specified variable attribute as a string value.
   *
   * @param[in] var The variable pointer.
   * @param[in] att_name The global attribute name.
   *
   * @return Returns the attribute value on success, "" on failure.
   */

  string _getVarAttAsString(const NcxxVar &var, const string &att_name) const
  {
    try {
      NcxxGroupAtt att = _ncFile.getAtt(att_name);
      string val;
      att.getValues(val);
      return val;
    } catch (NcxxException& e) {
      return "";
    }
  }
  
  /**
   * @brief Get the input projection from the file.
   *
   * @return Returns true on success, false on failure.
   *
   * @note This method assumes that _nx and _ny were already retrieved
   *       from the file with _getDimensions().
   */

  bool _getProjection();
  

  /**
   * @brief Set the master header values in the given MDV file.
   *
   * @param[in] forecast_time The forecast time for the file.
   */

  void _setMasterHeader(DsMdvx &mdvx,
			const DateTime &forecast_time) const;
  

};


#endif
