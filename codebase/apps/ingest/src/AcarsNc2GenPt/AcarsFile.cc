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
//   $Id: AcarsFile.cc,v 1.4 2019/03/04 00:13:37 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AcarsFile: Class for controlling access to the netCDF ACARS files.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cerrno>
#include <cfloat>
#include <cstdio>
#include <math.h>
#include <string.h>

#include <toolsa/os_config.h>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>

#include "AcarsFile.hh"

using namespace std;

const double AcarsFile::FLOAT_MISSING_DATA_VALUE = -9999.0;
const double AcarsFile::DOUBLE_MISSING_DATA_VALUE = -9999.0;


/*********************************************************************
 * Constructors
 */

AcarsFile::AcarsFile() :
  _debug(false),
  _filePath(""),
  _numRecsDimName(""),
  _tailLenDimName(""),
  _missingDataValueAttName(""),
  _latitudeVarName(""),
  _longitudeVarName(""),
  _altitudeVarName(""),
  _dataSourceVarName(""),
  _tailNumberVarName(""),
  _dataTimesVarName(""),
  _objectInitialized(false),
  _fileInitialized(false)
{
  _acarsFile = NULL;
  _ncfError = new Nc3Error(Nc3Error::silent_nonfatal);
}

  
/*********************************************************************
 * Destructor
 */

AcarsFile::~AcarsFile()
{
  if (_acarsFile) {
    _acarsFile->close();
    delete _acarsFile;
  }

  vector< DataField* >::iterator field;
  for (field = _dataFields.begin(); field != _dataFields.end(); ++field)
    delete *field;
}


/*********************************************************************
 * initialize() - Initialize the AcarsFile object.  This method MUST
 *                be called before any other methods are called.
 *
 * Returns true on success, false on failure
 */

bool AcarsFile::initialize(const string &num_recs_dim_name,
			   const string &tail_len_dim_name,
			   const string &missing_data_value_att_name,
			   const string &latitude_var_name,
			   const string &longitude_var_name,
			   const string &altitude_var_name,
			   const string &tail_number_var_name,
			   const string &data_source_var_name,
			   const string &data_times_var_name,
			   const bool debug_flag)
{
  static const string method_name = "AcarsFile::initialize()";
  
  PMU_auto_register("Initializing ACARS file");
  
  _objectInitialized = false;
  
  _debug = debug_flag;
  
  _numRecsDimName = num_recs_dim_name;
  _tailLenDimName = tail_len_dim_name;
  
  _missingDataValueAttName = missing_data_value_att_name;
  
  _latitudeVarName = latitude_var_name;
  _longitudeVarName = longitude_var_name;
  _altitudeVarName = altitude_var_name;
  _dataSourceVarName = data_source_var_name;
  _tailNumberVarName = tail_number_var_name;
  _dataTimesVarName = data_times_var_name;
  
  _objectInitialized = true;
  
  return true;
}


/*********************************************************************
 * initializeFile() - Initialize the ACARS file information.  This 
 *                    method MUST be called at the beginning of processing
 *                    any file.
 *
 * Returns true on success, false on failure
 */

bool AcarsFile::initializeFile(const string &acars_file_path)
{
  static const string method_name = "AcarsFile::initialize()";
  
  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "PROGRAMMING ERROR" << endl;
    cerr << "Must call initialize() before calling any other AcarsFile method" << endl;
    
    return false;
  }
  
  _fileInitialized = false;
  
  _filePath = acars_file_path;
  
  _acarsFile = new Nc3File(_filePath.c_str());
  
  if (!_acarsFile->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Acars file isn't valid: " << _filePath << endl;

    _acarsFile->close();
    return false;
  }

  _fileInitialized = true;
  
  return true;
}


/*********************************************************************
 * writeAsSpdb() - Write out the acars file in SPDB format.
 *
 * Returns true on success, false on failure
 */

bool AcarsFile::writeAsSpdb(const string &spdb_url,
			    const bool save_only_specified_source,
			    const int data_source)
{
  static const string method_name = "AcarsFile::writeAsSpdb()";
  
  PMU_auto_register("Writing ACARS data as SPDB");
  
  // Make sure the object was initialized

  if (!_objectInitialized || !_fileInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Object not initialized" << endl;
    
    return false;
  }

  // Retrieve the ACARS data from the file

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeAddUnique);
	
  if (!_retrieveData(spdb, save_only_specified_source, data_source))
    return false;
  
  spdb.put(spdb_url,
	   SPDB_GENERIC_POINT_ID,
	   SPDB_GENERIC_POINT_LABEL);
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getByteFieldVar() - Get the specified byte values from the
 *                      netCDF file.
 *
 * Returns a pointer to the byte values on success, 0 on failure.
 */

Nc3Values *AcarsFile::_getByteFieldVar(const string &field_name) const
{
  static const string method_name = "AcarsFile::_getByteFieldVar()";

  Nc3Var *field = 0;

  if ((field = _acarsFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != nc3Byte)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a float variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  Nc3Values *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from acars file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getCharFieldVar() - Get the specified character values from the
 *                      netCDF file.
 *
 * Returns a pointer to the character values on success, 0 on failure.
 */

Nc3Values *AcarsFile::_getCharFieldVar(const string &field_name) const
{
  static const string method_name = "AcarsFile::_getCharFieldVar()";

  Nc3Var *field = 0;

  if ((field = _acarsFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != nc3Char)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a character variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  Nc3Values *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from acars file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getDoubleFieldVar() - Get the specified double values from the
 *                        netCDF file.
 *
 * Returns a pointer to the double values on success, 0 on failure.
 */

Nc3Values *AcarsFile::_getDoubleFieldVar(const string &field_name,
					double &missing_data_value) const
{
  static const string method_name = "AcarsFile::_getDoubleFieldVar()";

  Nc3Var *field = 0;

  if ((field = _acarsFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != nc3Double)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a double variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  Nc3Values *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from acars file: " << _filePath << endl;
    
    return 0;
  }
  
  // Get the missing data value for this variable

  missing_data_value = _getVarDoubleAtt(*field, _missingDataValueAttName);
  
  return field_values;
}


/*********************************************************************
 * _getFloatFieldVar() - Get the specified float values from the
 *                       netCDF file.
 *
 * Returns a pointer to the float values on success, 0 on failure.
 */

Nc3Values *AcarsFile::_getFloatFieldVar(const string &field_name,
				       float &missing_data_value) const
{
  static const string method_name = "AcarsFile::_getFloatFieldVar()";

  // Get the variable object from the netCDF file

  Nc3Var *field = 0;

  if ((field = _acarsFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != nc3Float)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not a float variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  // Get the actual variable values from the file

  Nc3Values *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from acars file: " << _filePath << endl;
    
    return 0;
  }
  
  // Get the missing data value for this variable

  missing_data_value = _getVarFloatAtt(*field, _missingDataValueAttName);
  
  return field_values;
}


/*********************************************************************
 * _getIntFieldVar() - Get the specified integer values from the
 *                     netCDF file.
 *
 * Returns a pointer to the integer values on success, 0 on failure.
 */

Nc3Values *AcarsFile::_getIntFieldVar(const string &field_name) const
{
  static const string method_name = "AcarsFile::_getIntFieldVar()";

  Nc3Var *field = 0;

  if ((field = _acarsFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  if (field->type() != nc3Int)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Variable not an integer variable, as expected" << endl;
    cerr << "Variable name: " << field_name << endl;
    cerr << "Input file: " << _filePath << endl;
    
    return 0;
  }
  
  Nc3Values *field_values;
  if ((field_values = field->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from acars file: " << _filePath << endl;
    
    return 0;
  }
  
  return field_values;
}


/*********************************************************************
 * _getVarDoubleAtt() - Get the specified attribute from the given
 *                      netCDF variable as a double.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global DOUBLE_MISSING_DATA_VALUE on failure.
 */

double AcarsFile::_getVarDoubleAtt(const Nc3Var &variable,
				   const string &att_name) const
{
  static const string method_name = "AcarsFile::_getVarDoubleAtt()";
  
  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute for variable "
	 << variable.name() << endl;
    cerr << "Acars file: " << _filePath << endl;
    
    return DOUBLE_MISSING_DATA_VALUE;
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute value for variable "
	 << variable.name() << endl;
    cerr << "Acars file: " << _filePath << endl;
    
    return DOUBLE_MISSING_DATA_VALUE;
  }
  
  double att_value = att_values->as_double(0);
  
  delete attribute;
  delete att_values;
  
  return att_value;
}


/*********************************************************************
 * _getVarFloatAtt() - Get the specified attribute from the given
 *                     netCDF variable as a float.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global FLOAT_MISSING_DATA_VALUE on failure.
 */

float AcarsFile::_getVarFloatAtt(const Nc3Var &variable,
				  const string &att_name) const
{
  static const string method_name = "AcarsFile::_getVarFloatAtt()";
  
  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute for variable "
	 << variable.name() << endl;
    cerr << "Acars file: " << _filePath << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute value for variable "
	 << variable.name() << endl;
    cerr << "Acars file: " << _filePath << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  float att_value = att_values->as_float(0);
  
  delete attribute;
  delete att_values;
  
  return att_value;
}


/*********************************************************************
 * _retrieveData() - Retrieve the data from the netCDF file and put it
 *                   into the given SPDB object.
 *
 * Returns true on success, false on failure.
 */

bool AcarsFile::_retrieveData(Spdb &spdb,
			      const bool save_only_specified_source,
			      const int data_source)
{
  static const string method_name = "AcarsFile::_retrieveData()";
  
  // First, get the number of records in the file.

  Nc3Dim *num_recs_dim;
  if ((num_recs_dim = _acarsFile->get_dim(_numRecsDimName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << _numRecsDimName
	 << " dimension from input file: " << _filePath << endl;

    return false;
  }

  int num_recs = num_recs_dim->size();

  if (_debug)
    cerr << "---> File has " << num_recs << " records" << endl;
  
  // Now get the number of characters stored for the tail number

  Nc3Dim *tail_len_dim;
  if ((tail_len_dim = _acarsFile->get_dim(_tailLenDimName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << _tailLenDimName
	 << " dimension from input file: " << _filePath << endl;

    return false;
  }

  int tail_len = tail_len_dim->size();

  if (_debug)
    cerr << "     Tail len = " << tail_len << " chars" << endl;
  
  // Retrieve the variables that we always need

  float latitude_missing_data_value;
  float longitude_missing_data_value;
  float altitude_missing_data_value;
  double data_times_missing_data_value;
  
  Nc3Values *latitudes = _getFloatFieldVar(_latitudeVarName,
					  latitude_missing_data_value);
  Nc3Values *longitudes = _getFloatFieldVar(_longitudeVarName,
					   longitude_missing_data_value);
  Nc3Values *altitudes = _getFloatFieldVar(_altitudeVarName,
					  altitude_missing_data_value);
  Nc3Values *tail_numbers = _getCharFieldVar(_tailNumberVarName);
  Nc3Values *data_sources = _getByteFieldVar(_dataSourceVarName);
  Nc3Values *data_times = _getDoubleFieldVar(_dataTimesVarName,
					    data_times_missing_data_value);
  
  if (latitudes == 0 ||
      longitudes == 0 ||
      altitudes == 0 ||
      data_sources == 0 ||
      data_times == 0)
  {
    delete latitudes;
    delete longitudes;
    delete altitudes;

    delete tail_numbers;
    delete data_sources;
    delete data_times;

    return false;
  }
  
  // Retrieve the other requested variables

  vector< DataField* >::iterator field;
  
  for (field = _dataFields.begin(); field != _dataFields.end(); ++field)
    (*field)->getData(*_acarsFile,
		      _missingDataValueAttName);
  
  // Loop through the data in the file, processing each data point

  for (int i = 0; i < num_recs; ++i)
  {
    // See if this data point is from the correct source
    
    if (save_only_specified_source &&
	data_sources->as_ncbyte(i) != data_source)
      continue;
    
    // If we don't have a data time value, we shouldn't save the point

    if (data_times->as_double(i) == data_times_missing_data_value)
      continue;
    
    // Compile the point information

    GenPt point;
  
    char *tail_number = tail_numbers->as_string(i * tail_len);
    point.setName(tail_number);
    delete [] tail_number;
    
    point.setTime(data_times->as_double(i));
    point.setLat(latitudes->as_float(i));
    point.setLon(longitudes->as_float(i));
    
    point.addVal(altitudes->as_float(i));
    point.addFieldInfo("altitude", "m");
    
    vector< DataField* >::const_iterator field_iter;
    
    for (field_iter = _dataFields.begin(); field_iter != _dataFields.end();
	 ++field_iter)
    {
      DataField *field = *field_iter;
      double data_value;
      
      if (!field->getDataValue(i, data_value))
	continue;
      
      point.addVal(data_value);
      point.addFieldInfo(field->getGenptName(),
			 field->getGenptUnits());
    } /* endfor - field_iter */
    
    if (_debug)
      point.print(cerr);

    // Write out the data

    point.assemble();
	
    spdb.addPutChunk(0,
		     point.getTime(),
		     point.getTime(),  // add some offset
		     point.getBufLen(),
		     point.getBufPtr());
      
  } /* endfor - i */
  
  // Reclaim space

  delete latitudes;
  delete longitudes;
  delete altitudes;

  delete tail_numbers;
  delete data_sources;
  delete data_times;

  return true;
}
