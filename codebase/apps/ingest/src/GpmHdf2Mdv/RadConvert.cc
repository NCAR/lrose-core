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
 * @file RadConvert.cc
 *
 * @class RadConvert
 *
 * Class for converting radiance to brightness temperature.
 *  
 * @date 4/1/2009
 *
 */

#include <iostream>

#include <rapmath/math_macros.h>

#include "HdfFile.hh"
#include "RadConvert.hh"

using namespace std;


// Globals

const double RadConvert::BT_INVALID = -9999.0;

const double RadConvert::CH1_SOLAR_IRRADIANCE = 168.16;
const double RadConvert::EARTH_SUN_MEAN_DISTANCE = 149597870.0;


/**********************************************************************
 * Constructor
 */

RadConvert::RadConvert (const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _tableInitialized(false)
{
}


/**********************************************************************
 * Destructor
 */

RadConvert::~RadConvert(void)
{
}
  

/**********************************************************************
 * getBT()
 */

double RadConvert::getBT(const double radiance,
			 const convert_type_t convert_rad,
			 const double sun_mag,
			 const double solar_zenith,
			 const double cos_solar_zenith) const
{
  static const string method_name = "RadConvert::getBT()";
  
  switch (convert_rad)
  {
  case CONVERT_RAD_NONE :
    return radiance;
    break;
    
  case CONVERT_RAD_VIS :
    return _getVisBT(radiance, sun_mag, solar_zenith, cos_solar_zenith);
    break;
    
  case CONVERT_RAD_CH3 :
    return _getBTFromTable(radiance, _rad3Table);
    break;
    
  case CONVERT_RAD_CH4 :
    return _getBTFromTable(radiance, _rad4Table);
    break;
    
  case CONVERT_RAD_CH5 :
    return _getBTFromTable(radiance, _rad5Table);
    break;
  }
  
  return BT_INVALID;
}


/**********************************************************************
 * init()
 */

bool RadConvert::init(const bool debug_flag, const bool verbose_flag)
{
  static const string method_name = "RadConvert::init()";
  
  // Save the debug flags

  _debug = debug_flag;
  _verbose = verbose_flag;
  
  return true;
}
  

/**********************************************************************
 * loadTable()
 */

bool RadConvert::loadTable(const string &table_file_path)
{
  static const string method_name = "RadConvert::init()";
  
  // Flag that the object hasn't been initialized, just in case there's a
  // problem reading the table

  _tableInitialized = false;
  
  // Open the file

  FILE *bt_file;
  
  if ((bt_file = fopen(table_file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening brightness temperature table file: "
	 << table_file_path << endl;
    
    return false;
  }
  
  // Read the table entries

  char *line = 0;
  size_t line_len = 0;
  ssize_t chars_read;
  int line_num = 1;
  
  while ((chars_read = getline(&line, &line_len, bt_file)) != -1)
  {
    double temp;
    double rad3;
    double rad4;
    double rad5;
    
    if (sscanf(line, "%lf %lf %lf %lf",
	       &temp, &rad3, &rad4, &rad5) != 4)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading line " << line_num
	   << " from bt table file" << endl;
      cerr << "Line read: <" << line << ">" << endl;
      
      ++line_num;
      
      continue;
    }
    
    _tempTable.push_back(temp);
    _rad3Table.push_back(rad3);
    _rad4Table.push_back(rad4);
    _rad5Table.push_back(rad5);
    
    ++line_num;
  }
  
  // Close the file

  fclose(bt_file);
  
  if (_verbose)
    _printTable(cerr);
  
  // Do some sanity checks which shouldn't be necessary

  if (!_checkTable())
    return false;
  
  // If we get here, everything has initialized successfully

  _tableInitialized = true;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _checkTable()
 */

bool RadConvert::_checkTable() const
{
  static const string method_name = "RadConvert::_checkTable()";
  
  // First check the table sizes

  size_t table_size = _tempTable.size();
  
  if (_rad3Table.size() != table_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "*** Internal error ***" << endl;
    cerr << "Channel 3 radiance table has " << _rad3Table.size()
	 << " elements" << endl;
    cerr << "Brightness temperature table has " << table_size
	 << " elements" << endl;
    cerr << "All tables should be the same size" << endl;
    
    return false;
  }
  
  if (_rad4Table.size() != table_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "*** Internal error ***" << endl;
    cerr << "Channel 4 radiance table has " << _rad4Table.size()
	 << " elements" << endl;
    cerr << "Brightness temperature table has " << table_size
	 << " elements" << endl;
    cerr << "All tables should be the same size" << endl;
    
    return false;
  }
  
  if (_rad5Table.size() != table_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "*** Internal error ***" << endl;
    cerr << "Channel 5 radiance table has " << _rad5Table.size()
	 << " elements" << endl;
    cerr << "Brightness temperature table has " << table_size
	 << " elements" << endl;
    cerr << "All tables should be the same size" << endl;
    
    return false;
  }
  
  // Now check the table values.  The tables should have all decreasing
  // values.

  for (size_t i = 1; i < table_size; ++i)
  {
    if (_tempTable[i] > _tempTable[i-1])
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in brightness temperature table" << endl;
      cerr << "Table should contain all decreasing values" << endl;
      cerr << "Value[" << i << "] = " << _tempTable[i]
	   << ", value[" << (i-1) << " = " << _tempTable[i-1] << endl;
      
      return false;
    }
  }
  
  for (size_t i = 1; i < table_size; ++i)
  {
    if (_rad3Table[i] > _rad3Table[i-1])
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in channel 3 radiance table" << endl;
      cerr << "Table should contain all decreasing values" << endl;
      cerr << "Value[" << i << "] = " << _rad3Table[i]
	   << ", value[" << (i-1) << " = " << _rad3Table[i-1] << endl;
      
      return false;
    }
  }
  
  for (size_t i = 1; i < table_size; ++i)
  {
    if (_rad4Table[i] > _rad4Table[i-1])
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in channel 4 radiance table" << endl;
      cerr << "Table should contain all decreasing values" << endl;
      cerr << "Value[" << i << "] = " << _rad4Table[i]
	   << ", value[" << (i-1) << " = " << _rad4Table[i-1] << endl;
      
      return false;
    }
  }
  
  for (size_t i = 1; i < table_size; ++i)
  {
    if (_rad5Table[i] > _rad5Table[i-1])
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in channel 5 radiance table" << endl;
      cerr << "Table should contain all decreasing values" << endl;
      cerr << "Value[" << i << "] = " << _rad5Table[i]
	   << ", value[" << (i-1) << " = " << _rad5Table[i-1] << endl;
      
      return false;
    }
  }
  
  return true;
}


/**********************************************************************
 * _getBTFromTable()
 */

double RadConvert::_getBTFromTable(const double radiance,
				   const vector< double > &rad_table) const
{
  static const string method_name = "RadConvert::_getBTFromTable()";
  
  if (!_tableInitialized)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Brightness temperature table not loaded!!!" << endl;
    
    return BT_INVALID;
  }
  
  // Find the table indices for this value.  All tables contain decreasing
  // values.  We need the values that surround this one in the table.

  if (radiance > rad_table[0] || radiance < rad_table[rad_table.size()-1])
  {
    if (_verbose)
      cerr << "Radiance value " << radiance << " outside of table limits" << endl;
    
    return BT_INVALID;
  }
  
  int first_index = 0;
  int last_index = rad_table.size();
  int prev_index;
  int next_index;
  
  while (true)
  {
    prev_index = first_index + ((last_index - first_index) / 2);
    next_index = prev_index + 1;

    if (radiance <= rad_table[prev_index] &&
	radiance >= rad_table[next_index])
    {
      break;
    }

    if (radiance > rad_table[prev_index])
    {
      last_index = prev_index;
    }
    else if (radiance < rad_table[next_index])
    {
      first_index = next_index;
    }
  }
  
  if (_verbose)
  {
    cerr << "Got BT from table:" << endl;
    cerr << "rad = " << radiance << ", prev = " << rad_table[prev_index]
	 << ", next = " << rad_table[next_index] << endl;
  }
  
  if (rad_table[prev_index] == radiance)
    return _tempTable[prev_index];
  
  if (rad_table[next_index] == radiance)
    return _tempTable[next_index];
  
  return _tempTable[next_index] +
    ((radiance - rad_table[next_index]) *
     ((_tempTable[prev_index] - _tempTable[next_index]) /
      (rad_table[prev_index] - rad_table[next_index])));
     
  return BT_INVALID;
}


/**********************************************************************
 * _getVisBT()
 */

double RadConvert::_getVisBT(const double radiance,
			     const double sun_mag,
			     const double solar_zenith,
			     const double cos_solar_zenith) const
{
  static const string method_name = "RadConvert::_getVisBT()";
  
  // Check for invalid values

  if (sun_mag == HdfFile::MISSING_VALUE ||
      cos_solar_zenith == HdfFile::MISSING_VALUE)
    return BT_INVALID;
  
  // Check for nighttime

  if (solar_zenith > 90.0)
    return 0.0;
  
  // Calculate corrected solar irradiance

  double earth_sun_distance = sun_mag / 1000.0;
  double ratio = EARTH_SUN_MEAN_DISTANCE / earth_sun_distance;
  double ch1_irradiance = CH1_SOLAR_IRRADIANCE * ratio * ratio;

  double albedo = (radiance * PI / ch1_irradiance / cos_solar_zenith) * 100;

  if (albedo > 100.0)
  {
    if (_verbose)
    {
      cerr << "radiance = " << radiance << endl;
      cerr << "ratio = " << ratio << endl;
      cerr << "ch1_irradiance = " << ch1_irradiance << endl;
      cerr << "albedo = " << albedo << endl;
      cerr << endl;
    }
    
    albedo = 100.0;
  }
  
  return albedo;
}


/**********************************************************************
 * _printTable()
 */

void RadConvert::_printTable(ostream &out) const
{
  size_t table_size = _tempTable.size();
  
  for (size_t i = 0; i < table_size; ++i)
  {
    out << "  " << _tempTable[i] << "   " << _rad3Table[i]
	<< "   " << _rad4Table[i] << "   " << _rad5Table[i] << endl;
  } /* endfor - i */
  
}
