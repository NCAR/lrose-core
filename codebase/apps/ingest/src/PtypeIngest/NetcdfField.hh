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
 * @file NetcdfField.hh
 *
 * @class NetcdfField
 *
 * Class for dealing with a field in the netCDF file.
 *  
 * @date 12/02/2009
 *
 */

#ifndef NetcdfField_HH
#define NetcdfField_HH

#include <string>

#include <netcdf.hh>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/DateTime.hh>

using namespace std;

/** 
 * @class NetcdfField
 */

class NetcdfField
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

  NetcdfField(const string &nc_field_name,
	      const double nc_missing_data_value,
	      const bool transform_data,
	      const double transform_multiplier,
	      const double transform_constant,
	      const string &transform_units,
	      const bool replace_data,
	      const double replace_nc_value,
	      const double replace_mdv_value,
	      const bool debug_flag = false,
	      const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~NetcdfField(void);
  

  /**
   * @brief Set the remapping projection.
   *
   * @param[in] remap_proj The remapping projection to use on output.
   */

  inline void setRemapOutput(const MdvxProj &remap_proj)
  {
    _remapProj = remap_proj;
    _remapOutput = true;
  }
  

  /**
   * @brief Create an MDV field out of the specified field in the
   *        netCDF file.
   *
   * @param[in] nc_file The netCDF file.
   * @param[in] input_proj The projection of the data in the netCDF file.
   * @param[in] forecast_index The index of this forecast in the input file.
   * @param[in] forecast_secs The forecast lead time in seconds.
   * @param[in] forecast_time The forecast valid time.
   *
   * @return Returns a pointer to the MDV field on success, 0 on failure.
   */

  MdvxField *createMdvField(const NcFile &nc_file,
			    const MdvxProj &input_proj,
			    const int forecast_index,
			    const int forecast_secs,
			    const DateTime &forecast_time);
  

  ////////////////////
  // Access methods //
  ////////////////////

  inline string getFieldName() const
  {
    return _ncFieldName;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Name of the long field name attribute for the variables in the
   *        netCDF file.
   */

  static const string LONG_FIELD_NAME_ATT_NAME;
  
  /**
   * @brief Name of the units attributed for the variables in the netCDF
   *        file.
   */

  static const string UNITS_ATT_NAME;
  

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
   * @brief The name of the field in the netCDF file.
   */

  string _ncFieldName;
  

  /**
   * @brief The projection of the data in the netCDF file.
   */

  MdvxProj _inputProj;
  

  /**
   * @brief The missing data value in the netCDF file.
   */

  double _ncMissingDataValue;
  

  /**
   * @brief Flag indicating whether to transform the data before putting it
   *        in the MDV field.
   */

  bool _transformData;
  

  /**
   * @brief Multiplier to use when transforming data.
   */

  double _transformMultiplier;
  

  /**
   * @brief Constant to use when transforming data.
   */

  double _transformConstant;
  

  /**
   * @brief Units of the MDV data after applying the transform.
   */

  string _transformUnits;
  

  /**
   * @brief Flag indicating whether to replace a data value from the netCDF
   *        file with a different value in the MDV file.
   */

  bool _replaceData;
  

  /**
   * @brief Value in the netCDF file to replace.
   */

  double _replaceNcValue;
  

  /**
   * @brief Value to use in the MDV file.
   */

  double _replaceMdvValue;
  

  /**
   * @brief Flag indicating whether to remap the output.
   */

  bool _remapOutput;
  

  /**
   * @brief Projection to use when remapping the output.  Used only if
   *        _remapOutput is true.  This must be a MdvxProj object rather
   *        than a MdvxProj object because that is the only projection object
   *        the the MdvxField remapping method understands.
   */

  MdvxProj _remapProj;
  

  /**
   * @brief Look up table used when remapping the output.
   */

  MdvxRemapLut _remapLut;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Get the specified variable attribute as a string value.
   *
   * @param[in] var The variable pointer.
   * @param[in] att_name The global attribute name.
   *
   * @return Returns the attribute value on success, "" on failure.
   */

  string _getVarAttAsString(const NcVar &var, const string &att_name) const
  {
    NcAtt *att;
  
    if ((att = var.get_att(att_name.c_str())) == 0)
    {
      return "";
    }
  
    string string_value = att->as_string(0);
    delete att;
    
    return string_value;
  }
  
  /**
   * @brief Set the field header values
   *
   * @param[out] field_hdr The field header.
   * @param[in] input_proj The projection of the data in the netCDF file.
   * @param[in] field_name The name of the field.
   * @param[in] field_name_long The long field name of the field.
   * @param[in] units The units of the field.
   * @param[in] forecast_time The forecast valid time.
   * @param[in] forecast_secs The forecast lead time in seconds.
   */

  void _setFieldHeader(Mdvx::field_header_t &field_hdr,
		       const MdvxProj &input_proj,
		       const string &field_name,
		       const string &field_name_long,
		       const string &units,
		       const DateTime &forecast_time,
		       const int forecast_secs) const;
  
  /**
   * @brief Set the vlevel header values.
   *
   * @param[out] vlevel_hdr The vlevel header.
   */

  void _setVlevelHeader(Mdvx::vlevel_header_t &vlevel_hdr) const;
  

};


#endif
