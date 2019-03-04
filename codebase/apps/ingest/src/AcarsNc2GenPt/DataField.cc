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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2019/03/04 00:13:37 $
//   $Id: DataField.cc,v 1.3 2019/03/04 00:13:37 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DataField: Class for storing the data field information.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "DataField.hh"

using namespace std;


const float DataField::FLOAT_MISSING_DATA_VALUE = -9999.0;


/*********************************************************************
 * Constructors
 */

DataField::DataField(const string &nc_field_name,
		     const string &genpt_field_name,
		     const string &genpt_units,
		     Converter *converter,
		     const bool debug_flag) :
  _debug(debug_flag),
  _ncFieldName(nc_field_name),
  _genptFieldName(genpt_field_name),
  _genptUnits(genpt_units),
  _converter(converter),
  _fieldValues(0),
  _dataRead(false)
{
}

  
/*********************************************************************
 * Destructor
 */

DataField::~DataField()
{
  delete _converter;
  delete _fieldValues;
}


/*********************************************************************
 * getData() - Retrieve the data for this field from the given netCDF
 *             file.
 */

void DataField::getData(Nc3File &nc_file,
			const string &missing_data_value_att_name)
{
  static const string method_name = "DataField::getData()";

  _dataRead = false;
  
  // Get the variable object from the netCDF file

  Nc3Var *field = 0;

  if ((field = nc_file.get_var(_ncFieldName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << _ncFieldName 
	 << " variable from ACARS file" << endl;

    return;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _ncFieldName << " var from ACARS file is invalid" << endl;

    return;
  }

  if (field->type() != nc3Float)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable " << _ncFieldName << " not a float variable, as expected" << endl;
    
    return;
  }
  
  // Get the actual variable values from the file

  delete _fieldValues;
  
  if ((_fieldValues = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << _ncFieldName
	 << " field from ACARS file" << endl;
    
    return;
  }
  
  // Get the missing data value for this variable

  _missingDataValue = _getVarFloatAtt(*field, missing_data_value_att_name);

  _dataRead = true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getVarFloatAtt() - Get the specified attribute from the given
 *                     netCDF variable as a float.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global FLOAT_MISSING_DATA_VALUE on failure.
 */

float DataField::_getVarFloatAtt(const Nc3Var &variable,
				  const string &att_name) const
{
  static const string method_name = "AcarsFile::_getVarFloatAtt()";
  
  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute for variable "
	 << variable.name() << " from ACARS file" << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute value for variable "
	 << variable.name() << " from ACARS file" << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  float att_value = att_values->as_float(0);
  
  delete attribute;
  delete att_values;
  
  return att_value;
}
