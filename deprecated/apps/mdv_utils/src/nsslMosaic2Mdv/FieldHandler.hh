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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:15 $
 *   $Id: FieldHandler.hh,v 1.6 2016/03/04 02:22:15 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FieldHandler: Base class for classes that handle fields in the NSSL
 *               mosaic file.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FieldHandler_HH
#define FieldHandler_HH

#include <string>
#include <vector>

#include <Mdv/MdvxField.hh>

using namespace std;


class FieldHandler
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  FieldHandler(const string field_name,
	       const bool debug_flag = false,
	       const bool old_format = true);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~FieldHandler(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * extractField() - Extract the field from the given netCDF file.
   */

  virtual MdvxField *extractField(const int nc_file_id) = 0;
  virtual MdvxField *extractFieldWDSS2(const int nc_file_id) = 0;
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * getFieldName() - Get the name of the field.
   */

  string getFieldName()
  {
    return _fieldName;
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  bool _oldFormat;

  string _fieldName;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**********************************************************************
   * _extractDimensionValue() - Extract the value for the given dimension
   *                            from the given netCDF file.
   *
   * Returns the dimension value on success, -1 on failure.
   */

  static int _extractDimensionValue(const int nc_file_id,
				    const string &dim_name,
				    const bool print_error = true);


  /**********************************************************************
   * _extractFloatGlobalAtt() - Extract the given float global attribute
   *                            from the given netCDF file.
   *
   * Returns true on success, false on failure.
   */

  static bool _extractFloatGlobalAtt(const int nc_file_id,
				     const string &att_name,
				     float &att_value);
  

  /**********************************************************************
   * _extractFloatVariableArray() - Extract the data for the given float
   *                                variable that is an array.  Returns
   *                                the value of the given missing data
   *                                attribute and scales the data using 
   *                                the given scale attribute, if specified.
   *
   * Returns a pointer to the extracted data on successs, 0 on failure.
   */

  float *_extractFloatVariableArray(const int nc_file_id,
				    const string &var_name,
				    const int array_size,
				    const bool print_error = true) const;
  
  float *_extractFloatVariableArray(const int nc_file_id,
				    const string &var_name,
				    const int array_size,
				    const string &missing_value_att_name,
				    float &missing_value,
				    const string scale_att_name,
				    const bool print_error = true) const;
  

  /**********************************************************************
   * _extractIntVariableArray() - Extract the data for the given integer
   *                              variable that is an array.  Returns
   *                              the value of the given missing data
   *                              attribute and scales the data using 
   *                              the given scale attribute, if specified.
   *
   * Returns a pointer to the extracted data on successs, 0 on failure.
   */

  static int *_extractIntVariableArray(const int nc_file_id,
				       const string &var_name,
				       const int array_size);
  
  static int *_extractIntVariableArray(const int nc_file_id,
				       const string &var_name,
				       const int array_size,
				       const string &missing_value_att_name,
				       int &missing_value);
  

  /**********************************************************************
   * _extractStringVariableArray() - Extract the data for the given string
   *                                 variable that is an array.
   *
   * Returns a vector containing all of the strings in the variable.  If
   * there was an error, the returned vector will be empty.
   */

  static vector< string> _extractStringVariableArray(const int nc_file_id,
						     const string &var_name,
						     const string &string_size_dim_name,
						     const int num_strings);
  

  /**********************************************************************
   * _extractStringVariableAtt() - Extract the given string attribute of
   *                               the given variable.
   *
   * Returns the extracted string on success, "" on failure.
   */

  static string _extractStringVariableAtt(const int nc_file_id,
					  const string &var_name,
					  const string &att_name);
  

};


#endif
