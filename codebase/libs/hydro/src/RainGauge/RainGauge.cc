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
///////////////////////////////////////////////////////////////
// RainGauge.cc
//
// Class representing a RainGauge.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include <euclid/WorldPoint2D.hh>
#include <hydro/RainGauge.hh>
#include <shapelib/shapefil.h>
#include <toolsa/str.h>
using namespace std;


// Define global constants

const string RainGauge::LAT_FIELD_NAME = "LAT";
const string RainGauge::LON_FIELD_NAME = "LNG";
const string RainGauge::ID_FIELD_NAME = "ID";
const string RainGauge::AFOS_ID_FIELD_NAME = "AFOSID";
const string RainGauge::NAME_FIELD_NAME = "NAME";
const string RainGauge::ELEV_FIELD_NAME = "ELEV";
const string RainGauge::INST_FIELD_NAME = "INSTRUMENT";


/*********************************************************************
 * Constructors
 */

RainGauge::RainGauge(const bool debug_flag) :
  _debugFlag(debug_flag),
  _dataLoaded(false)
{
  // Do nothing
}


RainGauge::RainGauge(const WorldPoint2D &location,
		     const int id,
		     const string &afos_id,
		     const string &name,
		     const int elevation,
		     const int instrument,
		     const bool debug_flag) :
  _debugFlag(debug_flag),
  _location(location),
  _id(id),
  _afosId(afos_id),
  _name(name),
  _elevation(elevation),
  _instrument(instrument),
  _dataLoaded(true)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

RainGauge::~RainGauge()
{
  // Do nothing
}


/*********************************************************************
 * loadShapeInfo() - Load the gauge information from the given shape file.
 *
 * Returns true if the gauge information was successfully loaded, false
 * otherwise.
 */

bool RainGauge::loadShapeInfo(const string shape_file_base,
			      const int record_number)
{
  const string method_name = "loadShapeInfo()";
  
  // Open the database file

  string dbf_file = shape_file_base + ".dbf";
  DBFHandle dbf_handle = DBFOpen(dbf_file.c_str(), "rb");
  
  if (dbf_handle == 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "Error opening database file: " << dbf_file << endl;
    
    return false;
  }
  
  // Load the shape information

  bool return_code = loadShapeInfo(dbf_handle,
				   record_number);
  
  // Close the shape files since we don't need them anymore

  DBFClose(dbf_handle);
  
  return return_code;
}


bool RainGauge::loadShapeInfo(const DBFHandle dbf_handle,
			      const int record_number)
{
  const string method_name = "loadShapeInfo()";
  
  // Make sure all of the needed fields exist in the database

  int lat_field_num = -1;
  int lon_field_num = -1;
  int id_field_num = -1;
  int afos_id_field_num = -1;
  int name_field_num = -1;
  int elev_field_num = -1;
  int inst_field_num = -1;
  
  int num_fields = DBFGetFieldCount(dbf_handle);
  
  bool error_flag = false;
  
  for (int i = 0; i < num_fields; ++i)
  {
    char field_name[BUFSIZ];
    int width;
    int decimals;
    
    DBFFieldType field_type = DBFGetFieldInfo(dbf_handle, i,
					      field_name, &width, &decimals);
    
    // Check for a latitude field

    if (STRequal_exact(field_name, LAT_FIELD_NAME.c_str()))
    {
      if (field_type != FTDouble)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not a double" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      lat_field_num = i;
      
      continue;
    }
    
    // Check for a longitude field

    if (STRequal_exact(field_name, LON_FIELD_NAME.c_str()))
    {
      if (field_type != FTDouble)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not a double" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      lon_field_num = i;
      
      continue;
    }
    
    // Check for an id field

    if (STRequal_exact(field_name, ID_FIELD_NAME.c_str()))
    {
      if (field_type != FTInteger)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not an integer" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      id_field_num = i;
      
      continue;
    }
    
    // Check for an AFOS id field

    if (STRequal_exact(field_name, AFOS_ID_FIELD_NAME.c_str()))
    {
      if (field_type != FTString)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not a string" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      afos_id_field_num = i;
      
      continue;
    }
    
    // Check for a name field

    if (STRequal_exact(field_name, NAME_FIELD_NAME.c_str()))
    {
      if (field_type != FTString)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not a string" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      name_field_num = i;
      
      continue;
    }
    
    // Check for an elevation field

    if (STRequal_exact(field_name, ELEV_FIELD_NAME.c_str()))
    {
      if (field_type != FTInteger)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not an integer" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      elev_field_num = i;
      
      continue;
    }
    
    // Check for an instrument field

    if (STRequal_exact(field_name, INST_FIELD_NAME.c_str()))
    {
      if (field_type != FTInteger)
      {
	cerr << "ERROR: " << _className() << "::" << method_name << endl;
	cerr << "Error reading data for gauge " << record_number <<
	  " from database" << endl;
	cerr << field_name << " field is not an integer" << endl;
	cerr << "*** Skipping gauge ***" << endl;
	
	error_flag = true;
	
	break;
      }
      
      inst_field_num = i;
      
      continue;
    }
    
  } /* endfor - i */
  
  // Check for errors in reading information from the database

  if (lat_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No latitude (" << LAT_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (lon_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No longitude (" << LON_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (id_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No id (" << ID_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (afos_id_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No AFOS id (" << AFOS_ID_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (name_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No name (" << NAME_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (elev_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No elevation (" << ELEV_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (inst_field_num < 0)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "No instrument (" << INST_FIELD_NAME <<
      ") field found in database" << endl;
    
    error_flag = true;
  }
  
  if (error_flag)
    return false;
  
  // Read in the database information for the gauge.

  _location.setPoint(DBFReadDoubleAttribute(dbf_handle, record_number,
					    lat_field_num),
		     DBFReadDoubleAttribute(dbf_handle, record_number,
					    lon_field_num));
  _id = DBFReadIntegerAttribute(dbf_handle, record_number, id_field_num);
  _afosId = DBFReadStringAttribute(dbf_handle, record_number,
				   afos_id_field_num);
  _name = DBFReadStringAttribute(dbf_handle, record_number, name_field_num);
  _elevation = DBFReadIntegerAttribute(dbf_handle, record_number,
				       elev_field_num);
  _instrument = DBFReadIntegerAttribute(dbf_handle, record_number,
					inst_field_num);
  
  // Return, indicating that the information was successfully loaded.

  _dataLoaded = true;
  
  return true;
}


/*********************************************************************
 * print() - Print the current RainGauge information to the given stream
 *           for debugging purposes.
 */

void RainGauge::print(ostream &stream) const
{
  stream << "RainGauge information:" << endl;
  stream << "======================" << endl;
  stream << "debug flag = " << _debugFlag << endl;
  stream << "latitude = " << _location.lat << endl;
  stream << "longitude = " << _location.lon << endl;
  stream << "id = " << _id << endl;
  stream << "AFOS id = " << _afosId << endl;
  stream << "name = " << _name << endl;
  stream << "elevation = " << _elevation << endl;
  stream << "instrument = " << _instrument << endl;
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/
