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
 * @file FeatureNcFile.cc
 *
 * @class FeatureNcFile
 *
 * Class controlling access to a Feature netCDF file.
 *  
 * @date 8/12/2014
 *
 */

#include <iostream>
#include <stdio.h>

#include <os_config.h>

#include "FeatureNcFile.hh"

using namespace std;


// Global constants

//const int FeatureNcFile::MAX_FIELD_NAME_LEN = 64;


/*********************************************************************
 * Constructors
 */

FeatureNcFile::FeatureNcFile(const string &file_format_string,
			     const bool debug_flag) :
  NetcdfClassic(),
  _debug(debug_flag),
  _fileFormat(file_format_string),
  _varListInitialized(false)
{
}


/*********************************************************************
 * Destructor
 */

FeatureNcFile::~FeatureNcFile()
{
}


/*********************************************************************
 * openWrite()
 */

bool FeatureNcFile::openWrite(const string &output_dir,
			      const DateTime &interval_start,
			      const DateTime &interval_end)
{
  static const string method_name = "FeatureNcFile::openWrite()";
  
  // Construct the file name

  _filePath = _constructFilePath(output_dir, interval_start, interval_end);
  
  // Make sure the output directory exists

  if (_filePath.makeDirRecurse() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: " << _filePath.getDirectory() << endl;
    
    return false;
  }
  
  // Open the file.  Note that we have to use "nc4" format.  The other formats seem
  // to cause a "NcInDefineMode" exception when we try to add data.  I think that's
  // a bug in this netCDF interface, but I haven't tracked it down.

  if (NetcdfClassic::openWrite(_filePath.getPath(), NcFile::nc4) != 0)
  {
    cerr << getErrStr() << endl;
    return false;
  }
  
  // Add the global attributes

  if (addGlobAttr("CFfeatureType", "point") != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error adding feature type global attribute" << endl;
    
    return false;
  }
  
  if (addGlobAttr("Conventions", "CF-1.5") != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error adding conventions global attribute" << endl;
    
    return false;
  }
  
  if (addGlobAttr("file_format", _fileFormat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error adding conventions global attribute" << endl;
    
    return false;
  }
  
  // Add the unlimited record dimension

  if (addDim(_recordDim, "record", 0) != 0)
  {
    cerr << getErrStr() << endl;
    return false;
  }
  
  return true;
}


/*********************************************************************
 * appendValue()
 */

bool FeatureNcFile::appendValue(const string &name, const double value,
				const size_t record_num)
{
  static const string method_name = "FeatureNcFile::appendValue()";
  
  // Find the file variable

  map< string, NcVar* >::iterator var_iter;
  
  if ((var_iter = _varList.find(name)) == _varList.end())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find variable with name <" << name << ">" << endl;
    
    return false;
  }
  
  NcVar *var = var_iter->second;
  
  // Create the index vector.

  vector< size_t > index;
  index.push_back(record_num);
  
  // Add the value to the variable

  var->putVar(index, value);
  
  return true;
}

bool FeatureNcFile::appendValue(const string &name, const int value,
				const size_t record_num)
{
  static const string method_name = "FeatureNcFile::appendValue()";
  
  // Find the file variable

  map< string, NcVar* >::iterator var_iter;
  
  if ((var_iter = _varList.find(name)) == _varList.end())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find variable with name <" << name << ">" << endl;
    
    return false;
  }
  
  NcVar *var = var_iter->second;
  
  // Create the index vector.

  vector< size_t > index;
  index.push_back(record_num);
  
  // Add the value to the variable

  var->putVar(index, value);
  
  return true;
}

bool FeatureNcFile::appendValue(const string &name, const bool value,
				const size_t record_num)
{
  static const string method_name = "FeatureNcFile::appendValue()";
  
  // Find the file variable

  map< string, NcVar* >::iterator var_iter;
  
  if ((var_iter = _varList.find(name)) == _varList.end())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find variable with name <" << name << ">" << endl;
    
    return false;
  }
  
  NcVar *var = var_iter->second;
  
  // Create the index vector.

  vector< size_t > index;
  index.push_back(record_num);
  
  // Convert the boolean value to a byte

  short byte_value;
  
  if (value)
    byte_value = 1;
  else
    byte_value = 0;
  
  // Add the value to the variable

  var->putVar(index, byte_value);
  
  return true;
}

bool FeatureNcFile::appendValue(const string &name, const string &value,
				const size_t record_num)
{
  static const string method_name = "FeatureNcFile::appendValue()";
  
  // Find the file variable

  map< string, NcVar* >::iterator var_iter;
  
  if ((var_iter = _varList.find(name)) == _varList.end())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find variable with name <" << name << ">" << endl;
    
    return false;
  }
  
  NcVar *var = var_iter->second;
  
  // Create the index vector.

  vector< size_t > index;
  index.push_back(record_num);
  
  // Add the value to the variable

  var->putVar(index, value);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _constructFilePath()
 */

Path FeatureNcFile::_constructFilePath(const string &output_dir,
				       const DateTime &interval_start,
				       const DateTime &interval_end) const
{
  // Construct the file name

  char file_name[MAX_PATH_LEN];

  sprintf(file_name, "%s.%04d%02d%02d_%02d%02d%02d_to_%04d%02d%02d_%02d%02d%02d.nc",
	  _fileFormat.c_str(),
	  interval_start.getYear(), interval_start.getMonth(), interval_start.getDay(),
	  interval_start.getHour(), interval_start.getMin(), interval_start.getSec(),
	  interval_end.getYear(), interval_end.getMonth(), interval_end.getDay(),
	  interval_end.getHour(), interval_end.getMin(), interval_end.getSec());
  
  if (_debug)
    cerr << "    file_name = <" << file_name << ">" << endl;
  
  return Path(output_dir, file_name);
}
