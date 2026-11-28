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
 * @file FieldInfo.hh
 *
 * @class FieldInfo
 *
 * Class for maintaining information about a data field within an SDS field.
 *  
 * @date 11/7/2008
 *
 */

#ifndef FieldInfo_HH
#define FieldInfo_HH

#include <string>
#include <vector>

#include "RadConvert.hh"

using namespace std;

/** 
 * @class FieldInfo
 */

class FieldInfo
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

  FieldInfo(const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~FieldInfo(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**
   * @brief Calculate the MDV data value given the raw TRMM file value.
   *
   * @param[in] raw_data_value The raw data value as read in from the TRMM
   *                           file.
   * @param[in] rad_convert Object for converting radiance values to
   *                        brightness temperature.
   * @param[in] sun_mag Sun magnitude.
   * @param[in] solar_zenith Solar zenith.
   * @param[in] cos_solar_zenith Cosine of the solar zenith.
   *
   * @return Returns the MDV file value for the given TRMM file value.
   */

  double calcMdvValue(const double raw_data_value,
		      const RadConvert &rad_convert,
		      const double sun_mag,
		      const double solar_zenith,
		      const double cos_solar_zenith) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Add a value to the the missing data value list.
   *
   * @param[in] missing_value The new missing data value for this field.
   */

  inline void addMissingValue(const double &missing_value)
  {
    _missingValues.push_back(missing_value);
  }
  

  /**
   * @brief Add a value to the the bad data value list.
   *
   * @param[in] bad_value The new bad data value for this field.
   */

  inline void addBadValue(const double &bad_value)
  {
    _badValues.push_back(bad_value);
  }
  

  /**
   * @brief Set the MDV field name.
   *
   * @param[in] field_name The MDV field name for this field.
   */

  inline void setMdvFieldName(const string &field_name)
  {
    _mdvFieldName = field_name;
  }
  

  /**
   * @brief Set the MDV units string.
   *
   * @param[in] field_name The MDV units string for this field.
   */

  inline void setMdvFieldUnits(const string &field_units)
  {
    _mdvFieldUnits = field_units;
  }
  

  /**
   * @brief Set the data scale.
   *
   * @param[in] scale The data scale for this field.
   */

  inline void setScale(const double &scale)
  {
    _scale = scale;
  }
  

  /**
   * @brief Set the data bias.
   *
   * @param[in] bias The data bias for this field.
   */

  inline void setBias(const double &bias)
  {
    _bias = bias;
  }
  

  /**
   * @brief Set the radiance conversion type.
   *
   * @param[in] rad_convert_type The radiance conversion type for this field.
   */

  inline void setRadConvertType(const RadConvert::convert_type_t rad_convert_type)
  {
    _radConvertType = rad_convert_type;
  }
  

  /**
   * @brief Get the MDV field name.
   *
   * @return Returns the MDV field name for this field.
   */

  inline string getMdvFieldName() const
  {
    return _mdvFieldName;
  }
  

  /**
   * @brief Get the MDV units string.
   *
   * @return Returns the MDV units string for this field.
   */

  inline string getMdvFieldUnits() const
  {
    return _mdvFieldUnits;
  }
  

  /**
   * @brief Get the data scale.
   *
   * @return Returns the data scale for this field.
   */

  inline double getScale() const
  {
    return _scale;
  }
  

  /**
   * @brief Get the data bias.
   *
   * @return Returns the data bias for this field.
   */

  inline double getBias() const
  {
    return _bias;
  }
  

  /**
   * @brief Get the radiance conversion type.
   *
   * @return Returns the radiance conversion for this field.
   */

  inline RadConvert::convert_type_t getRadConvertType() const
  {
    return _radConvertType;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  

  /**
   * @brief Field name to use in the MDV file.
   */

  string _mdvFieldName;
  

  /**
   * @brief Units string to use in the MDV file.
   */

  string _mdvFieldUnits;
  

  /**
   * @brief Scale to use when converting the data from the value stored
   *        in the TRMM file to the true data value.
   */

  double _scale;
  
  
  /**
   * @brief Bias to use when converting the data from the value stored
   *        in the TRMM file to the true data value.
   */

  double _bias;
  

  /**
   * @brief List of alues used to indicate missing data in the TRMM data
   *       file for this field.
   */

  vector< double > _missingValues;
  

  /**
   * @brief List of alues used to indicate bad data in the TRMM data
   *       file for this field.
   */

  vector< double > _badValues;
  

  /**
   * @brief Radiance conversion types for this MDV field.
   */

  RadConvert::convert_type_t _radConvertType;
  

};


#endif
