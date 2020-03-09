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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:39:55 $
//   $Id: GenPolyStats2Ascii.cc,v 1.6 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GenPolyStats2Ascii: GenPolyStats2Ascii program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2009
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <string>

#include <rapformats/ds_radar.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "GenPolyStats2Ascii.hh"
#include "Params.hh"

using namespace std;


// Global variables

GenPolyStats2Ascii *GenPolyStats2Ascii::_instance =
     (GenPolyStats2Ascii *)NULL;


/*********************************************************************
 * Constructor
 */

GenPolyStats2Ascii::GenPolyStats2Ascii(int argc, char **argv)
{
  static const string method_name = "GenPolyStats2Ascii::GenPolyStats2Ascii()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (GenPolyStats2Ascii *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

GenPolyStats2Ascii::~GenPolyStats2Ascii()
{
  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

GenPolyStats2Ascii *GenPolyStats2Ascii::Inst(int argc, char **argv)
{
  if (_instance == (GenPolyStats2Ascii *)NULL)
    new GenPolyStats2Ascii(argc, argv);
  
  return(_instance);
}

GenPolyStats2Ascii *GenPolyStats2Ascii::Inst()
{
  assert(_instance != (GenPolyStats2Ascii *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool GenPolyStats2Ascii::init()
{
  static const string method_name = "GenPolyStats2Ascii::init()";
  
  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void GenPolyStats2Ascii::run()
{
  static const string method_name = "GenPolyStats2Ascii::run()";
  
  time_t start_time = DateTime::parseDateTime(_params->start_time);
  time_t end_time = DateTime::parseDateTime(_params->end_time);
  
  if (start_time == DateTime::NEVER)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing start_time string: " << _params->start_time << endl;
    
    return;
  }
    
  if (end_time == DateTime::NEVER)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing end_time string: " << _params->end_time << endl;
      
    return;
  }
    
  _processData(start_time, end_time);
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool GenPolyStats2Ascii::_processData(const DateTime &start_time,
				      const DateTime &end_time)
{
  static const string method_name = "GenPolyStats2Ascii::_processData()";
  
  if (_params->debug)
  {
    cerr << endl;
    cerr << "*** Processing data: " << endl;
    cerr << "    start time = " << start_time << endl;
    cerr << "    end time = " << end_time << endl;
  }
  
  // Retrieve the chunks from the input SPDB database

  DsSpdb input_spdb;
  
  if (input_spdb.getInterval(_params->input_url,
			     start_time.utime(),
			     end_time.utime()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving polygons from SPDB database: " << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   start time: " << start_time << endl;
    cerr << "   end time: " << end_time << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "    Successfully retrieved " << input_spdb.getNChunks()
	 << " chunks from input database" << endl;
  
  // Make sure that the database contains the right type of products

  if (input_spdb.getProdId() != SPDB_GENERIC_POLYLINE_ID)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "SPDB database contains the wrong type of data." << endl;
    cerr << "Database must contain GenPoly data for this application" << endl;
    
    return false;
  }
  
  // Process each of the chunks

  const vector< Spdb::chunk_t > chunks = input_spdb.getChunks();
  vector< Spdb::chunk_t >::const_iterator chunk;
  int chunk_num;
  
  for (chunk = chunks.begin(), chunk_num = 0; chunk != chunks.end();
       ++chunk, ++chunk_num)
  {
    GenPolyStats polygon;
    
    if (!polygon.disassemble(chunk->data, chunk->len))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting SPDB data to GenPoly format for chunk "
	   << chunk_num << endl;
      cerr << "Skipping chunk" << endl;
      
      continue;
    }

    // See if this is a storm we want to process

    if (_params->storm_substring[0] != '\0' &&
	polygon.getName().find(_params->storm_substring) == string::npos)
    {
      if (_params->debug)
	cerr << "Skipping storm: <" << polygon.getName() << ">" << endl;
      
      continue;
    }
    
    // If we get here, we process this storm

    if (_params->debug)
      cerr << endl << "Processing storm: " << polygon.getName() << endl;
    
    _writePolygon(polygon);
    
  } /* endfor - chunk, chunk_num */
  
  return true;
}


/*********************************************************************
 * _writeFieldListValues() - Write all of the fields whose field names
 *                           begin with the given string to cout.
 */

void GenPolyStats2Ascii::_writeFieldListValues(const GenPolyStats &polygon,
					       const string &field_prefix) const
{
  static const string method_name = "GenPolyStats2Ascii::_writeFieldListValues()";
  
  vector< int > field_list = polygon.getFieldListPrefix(field_prefix);
  vector< int >::const_iterator field_num;
  
  for (field_num = field_list.begin(); field_num != field_list.end();
       ++field_num)
  {
    string name = polygon.getFieldName(*field_num);
    double value = polygon.get1DVal(*field_num);
    string units = polygon.getFieldUnits(*field_num);
  
    if (units == "" || units == "none")
      cout << name << " = " << value << endl;
    else
      cout << name << " = " << value << " " << units << endl;
  }
  
}


/*********************************************************************
 * _writeFieldValue() - Write the given field value to cout
 */

void GenPolyStats2Ascii::_writeFieldValue(const GenPolyStats &polygon,
					  const string &field_name,
					  const string &field_label) const
{
  static const string method_name = "GenPolyStats2Ascii::_writeFieldValue()";
  
  string label;
  
  if (field_label == "")
    label = field_name;
  else
    label = field_label;
  
  int field_num;
  if ((field_num = polygon.getFieldNum(field_name)) < 0)
  {
    if (_params->debug)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << field_name << " field not found in polygon" << endl;
    }
    
    return;
  }

  double value = polygon.get1DVal(field_num);
  string units = polygon.getFieldUnits(field_num);
  
  if (units == "" || units == "none")
    cout << label << " = " << value << endl;
  else
    cout << label << " = " << value << " " << units << endl;
}


/*********************************************************************
 * _writePolygon() - Write the information for this polygon to cout.
 */

void GenPolyStats2Ascii::_writePolygon(const GenPolyStats &polygon) const
{
  static const string method_name = "GenPolyStats2Ascii::_writePolygon()";
  
  char formatted_string[1024];
  
  cout << endl;
  cout << "Storm id = " << polygon.getName() << endl;
  cout << "Polygon # = " << polygon.getId() << endl;
  
  if (_params->include_polygon_time)
  {
    DateTime polygon_time(polygon.getTime());
    sprintf(formatted_string, "%02d%02d%02d_%02d%02d%02d",
	    polygon_time.getDay(), polygon_time.getMonth(),
	    polygon_time.getYear() % 100,
	    polygon_time.getHour(), polygon_time.getMin(),
	    polygon_time.getSec());
    cout << _params->polygon_time_label << " = " << formatted_string << endl;
  }
  
  if (_params->include_scan_time)
  {
    double scan_time_offset = polygon.getScanTimeOffset();
    
    if (scan_time_offset == GenPoly::missingVal)
    {
      if (_params->debug)
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "scan time offset field not found in polygon" << endl;
      }
    }
    else
    {
      DateTime scan_time(polygon.getTime() +
			 (int)(scan_time_offset + 0.5));
      sprintf(formatted_string, "%02d%02d%02d_%02d%02d%02d",
	      scan_time.getDay(), scan_time.getMonth(),
	      scan_time.getYear() % 100,
	      scan_time.getHour(), scan_time.getMin(), scan_time.getSec());
      cout << _params->scan_time_label << " = " << formatted_string << endl;
    }
  }
  
  _writeScanType(polygon);
  _writeFieldValue(polygon, GenPolyStats::ELEV_ANGLE_FIELD_NAME);
  _writeFieldValue(polygon, GenPolyStats::DATA_HEIGHT_FIELD_NAME);
  _writeFieldValue(polygon, GenPolyStats::DATA_AREA_FIELD_NAME);
  _writeFieldListValues(polygon, GenPolyStats::THRESHOLD_FIELD_PREFIX);
  _writeFieldListValues(polygon, GenPolyStats::DROPSIZE_THRESH_FIELD_PREFIX);
  
  for (int i = 0; i < _params->output_fields_n; ++i)
  {
    _writeFieldValue(polygon,
		     _params->_output_fields[i].field_name,
		     _params->_output_fields[i].field_label);
  }
  
}


/*********************************************************************
 * _writeScanType() - Write the scan type from the given polygon
 */

void GenPolyStats2Ascii::_writeScanType(const GenPolyStats &polygon) const
{
  static const string method_name = "GenPolyStats2Ascii::_writeScanType()";
  
  int scan_mode = (int)polygon.getScanMode();
  
  cout << "Scan type = ";
  
  switch (scan_mode)
  {
  case DS_RADAR_CALIBRATION_MODE :
    cout << "calibration";
    break;
    
  case DS_RADAR_SECTOR_MODE :
    cout << "sector";
    break;
    
  case DS_RADAR_COPLANE_MODE :
    cout << "coplane";
    break;
    
  case DS_RADAR_RHI_MODE :
    cout << "RHI";
    break;
    
  case DS_RADAR_VERTICAL_POINTING_MODE :
    cout << "vertical pointing";
    break;
    
  case DS_RADAR_TARGET_MODE :
    cout << "target";
    break;
    
  case DS_RADAR_MANUAL_MODE :
    cout << "manual";
    break;
    
  case DS_RADAR_IDLE_MODE :
    cout << "idle";
    break;
    
  case DS_RADAR_SURVEILLANCE_MODE :
    cout << "surveillance";
    break;
    
  case DS_RADAR_AIRBORNE_MODE :
    cout << "airborne";
    break;
    
  case DS_RADAR_HORIZONTAL_MODE :
    cout << "horizontal";
    break;
    
  case DS_RADAR_SUNSCAN_MODE :
    cout << "sunscan";
    break;
    
  case DS_RADAR_POINTING_MODE :
    cout << "pointing";
    break;
    
  case DS_RADAR_FOLLOW_VEHICLE_MODE :
    cout << "follow vehicle";
    break;
    
  case DS_RADAR_UNKNOWN_MODE :
  default:
    cout << "unknown";
    break;
  } /* endswitch - scan_mode */
  
  cout << endl;
}
