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
 * @file RadConvert.hh
 *
 * @class RadConvert
 *
 * Class for converting radiance to brightness temperature.
 *  
 * @date 4/1/2009
 *
 */

#ifndef RadConvert_HH
#define RadConvert_HH

#include <string>
#include <vector>

using namespace std;


/** 
 * @class RadConvert
 */

class RadConvert
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief Value indicating an invalid brightness temperature.
   */

  static const double BT_INVALID;
  

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    CONVERT_RAD_NONE,
    CONVERT_RAD_VIS,
    CONVERT_RAD_CH3,
    CONVERT_RAD_CH4,
    CONVERT_RAD_CH5
  } convert_type_t;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  RadConvert(const bool debug_flag = false, const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  ~RadConvert(void);
  

  /**
   * @brief Initialize the radiance conversion object.
   *
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const bool debug_flag = false, const bool verbose_flag = false);
  

  /**
   * @brief Load the brightness temperature table.
   *
   * @param[in] table_file_path The full path of the file containing the
   *                            brightness temperature table.
   *
   * @return Returns true on success, false on failure.
   */

  bool loadTable(const string &table_file_path);
  

  /**
   * @brief Get the brightness temperature value for this radiance
   *        value.
   *
   * @param[in] radiance Radiance value.
   * @param[in] convert_rad Type of radiance conversion to perform.
   * @param[in] sun_mag The sun magnitude at this point.
   * @param[in] solar_zenith The solar zenith at this point.
   * @param[in] cos_solar_zenith The cosine of the solar zenith at this point.
   *
   * @return Returns the brightness temperature value on success,
   *         BT_INVALID on failure.
   */

  double getBT(const double radiance, const convert_type_t convert_rad,
	       const double sun_mag,
	       const double solar_zenith, const double cos_solar_zenith) const;
  

 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const double CH1_SOLAR_IRRADIANCE;
  static const double EARTH_SUN_MEAN_DISTANCE;


  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double temp;
    double ch3_rad;
    double ch4_rad;
    double ch5_rad;
  } bt_entry_t;
  
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag
   */

  bool _debug;
  

  /**
   * @brief Verbose flag
   */

  bool _verbose;
  

  /**
   * @brief Flag indicating whether this object has been initialized.
   */

  bool _tableInitialized;
  

  /**
   * @brief The brightness temperature table.
   */

  vector< double > _tempTable;
  

  /**
   * @brief The channel 3 radiance table.
   */

  vector< double > _rad3Table;
  

  /**
   * @brief The channel 4 radiance table.
   */

  vector< double > _rad4Table;
  

  /**
   * @brief The channel 5 radiance table.
   */

  vector< double > _rad5Table;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Check the brightness temperature table for internal
   *        consistency.
   *
   * @return Returns true if the table is okay, false otherwise.
   */

  bool _checkTable() const;
  

  /**
   * @brief Get the brightness temperature value for this radiance
   *        value from the given table.
   *
   * @param[in] radiance Radiance value.
   * @param[in] rad_table The radiance table.
   *
   * @return Returns the brightness temperature value on success,
   *         BT_INVALID on failure.
   */

  double _getBTFromTable(const double radiance,
			 const vector< double > &rad_table) const;
  

  /**
   * @brief Calculate the brightness temperature (albedo???) for this
   *        given visible radiance.
   *
   * @param[in] radiance Radiance value.
   * @param[in] sun_mag Sun magnitude.
   * @param[in] solar_zenith Solar zenith.
   * @param[in] cos_solar_zenith Cosine of the solar zenith.
   *
   * @return Returns the BT value for this radiance on success,
   *         BT_INVALID on failure.
   */

  double _getVisBT(const double radiance,
		   const double sun_mag,
		   const double solar_zenith,
		   const double cos_solar_zenith) const;
  

  /**
   * @brief Print the table contents to the indicated stream
   *
   * @param[in,out] out The output stream to use
   */

  void _printTable(ostream &out) const;
  

};


#endif
