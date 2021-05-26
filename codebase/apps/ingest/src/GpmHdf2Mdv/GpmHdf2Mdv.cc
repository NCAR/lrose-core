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
 * @file GpmHdf2Mdv.cc
 *
 * @class GpmHdf2Mdv
 *
 * GpmHdf2Mdv program object.
 *  
 * @date 10/30/2008
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "HdfFile.hh"
#include "GpmHdf2Mdv.hh"

#include "FullResSdsField.hh"
#include "LowResSdsField.hh"

using namespace std;

// Global variables

GpmHdf2Mdv *GpmHdf2Mdv::_instance =
     (GpmHdf2Mdv *)NULL;


/*********************************************************************
 * Constructors
 */

GpmHdf2Mdv::GpmHdf2Mdv(int argc, char **argv) :
  _dataTrigger(0),
  _loadSolarCalibData(false),
  _loadBtTable(false)
{
  static const string method_name = "GpmHdf2Mdv::GpmHdf2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (GpmHdf2Mdv *)NULL);
  
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
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
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

GpmHdf2Mdv::~GpmHdf2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

GpmHdf2Mdv *GpmHdf2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (GpmHdf2Mdv *)NULL)
    new GpmHdf2Mdv(argc, argv);
  
  return(_instance);
}

GpmHdf2Mdv *GpmHdf2Mdv::Inst()
{
  assert(_instance != (GpmHdf2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool GpmHdf2Mdv::init()
{
  static const string method_name = "GpmHdf2Mdv::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the output projection

  if (!_initOutputProjection())
    return false;
  
  // Initialize the data handlers

  if (!_initDataHandlers())
    return false;
  
  // Initialize the radiance conversion object.  This must be done after
  // _initDataHandlers() so that we know whether to load the brightness
  // temperature table.

  if (!_radConvert.init(_params->debug, _params->verbose))
    return false;
  
  if (_loadBtTable)
  {
    if (!_radConvert.loadTable(_params->brightness_temp_table))
      return false;
  }
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void GpmHdf2Mdv::run()
{
  static const string method_name = "GpmHdf2Mdv::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    _processFile(trigger_info.getFilePath());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/


/*********************************************************************
 * _processFile()
 */

bool GpmHdf2Mdv::_processFile(const string &input_file_name)
{
  static const string method_name = "GpmHdf2Mdv::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << "*** Processing file: " << input_file_name << endl;
  
  // Create the HDF file object

  if (_params->verbose)
    cerr << "Creating HDF file object" << endl;
  
  HdfFile hdf_file(input_file_name, _params->debug, _params->verbose);

  if (_loadSolarCalibData)
    hdf_file.setReadMagData();
  
  if (_params->verbose)
    cerr << "Initializing HDF file" << endl;
  
  if (!hdf_file.init())
    return false;
  
  // Retrieve the data headers (numScans, numPixels, geolocations)

  if (_params->debug)
    cerr << "Retrieving geolocation information from HDF file" << endl;

  if (!hdf_file.readDataHeaders())
    return false;
  
  // Retrieve the scan time information

  if (_params->verbose)
    cerr << "Retrieving scan time information" << endl;
  
  DateTime scan_begin_time;
  DateTime scan_end_time;
  
  if (!hdf_file.getScanTimeRange(scan_begin_time, scan_end_time))
    return false;
  
  // Create the output file

  DsMdvx mdvx;
  _setMasterHeader(mdvx, scan_begin_time, scan_end_time, input_file_name);
  
  // Add the geolocation fields to the file

  if (!_addMdvGeolocationFields(mdvx, hdf_file))
    return false;
  
  // Process the requested fields

  vector< SdsDataField* >::iterator data_handler_iter;
  
  for (data_handler_iter = _dataHandlers.begin();
       data_handler_iter != _dataHandlers.end(); ++data_handler_iter)
  {
    SdsDataField *sds_field = *data_handler_iter;
    
    // Create the MDV fields for this SDS array

    vector< MdvxField* > mdv_fields;
    
    if (!sds_field->createMdvFields(mdv_fields, _outputProj, hdf_file))
      return false;
    
    // Add the fields to the output file.  The MDV object takes control of
    // the field pointers at this point so the field pointers should not be
    // deleted after this call.

    vector< MdvxField* >::iterator field;
    
    for (field = mdv_fields.begin(); field != mdv_fields.end(); ++field)
    {
      //      (*field)->convertType(Mdvx::ENCODING_INT8,
      //	    Mdvx::COMPRESSION_BZIP,
      //		    Mdvx::SCALING_DYNAMIC);
      
      mdvx.addField(*field);
    } /* endfor - field */
    
  } /* endfor - data_handler_iter */
  
  // Write the output file

  mdvx.setWriteLdataInfo();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << endl << endl;
  
  return true;
}

/*********************************************************************
 * _addMdvGeolocationFields()
 */

bool GpmHdf2Mdv::_addMdvGeolocationFields(DsMdvx &mdvx,
					   const HdfFile &hdf_file) const
{
  static const string method_name = "GpmHdf2Mdv::_addMdvGeolocationFields()";
  
  // Add the scan time field

  if (_params->debug)
    cerr << "Adding scan time field to MDV file" << endl;
  
  if (!_addMdvScanTimeField(mdvx, hdf_file))
    return false;
  
  // Add the delta time field

  if (_params->debug)
    cerr << "Adding delta scan time field to MDV file" << endl;
  
  if (!_addMdvDeltaTimeField(mdvx, hdf_file))
    return false;
  
  if (_params->debug)
    cerr << "Time fields successfully added to MDV file" << endl;
  
  if (_loadSolarCalibData)
  {
    // Add the solar magnitude field

    if (_params->debug)
      cerr << "Adding solar magnitude field to MDV file" << endl;
  
    if (!_addMdvSolarMagField(mdvx, hdf_file))
      return false;
    
    // Add the solar zenith field

    if (_params->debug)
      cerr << "Adding solar zenith field to MDV file" << endl;
  
    if (!_addMdvSolarZenithField(mdvx, hdf_file))
      return false;
    
  }
  
  return true;
}


/*********************************************************************
 * _addMdvDeltaTimeField()
 */

bool GpmHdf2Mdv::_addMdvDeltaTimeField(DsMdvx &mdvx,
					const HdfFile &hdf_file) const
{
  static const string method_name = "GpmHdf2Mdv::_addMdvDeltaTimeField()";
  
  // Get the centroid time from the master header.  The delta time is the
  // scan time difference from the MDV centroid time.

  time_t time_centroid = mdvx.getMasterHeader().time_centroid;
  
  // Create the blank field
  
  MdvxField *field = _createBlankMdvTimeField("scan_delta_time", "secs");
    
  // Fill in the field data

  int num_elements =
    hdf_file.getNumScans() * hdf_file.getNumPixels();
    
  si32 *mdv_data = (si32 *)field->getVol();
    
  for (int i = 0; i < num_elements; ++i)
  {
    // Get the lat/lon for the raw data point and normalize the
    // lon to the output grid

    double raw_data_lat = hdf_file.getLat(i);
    double raw_data_lon = hdf_file.getLon(i);
	
    while (raw_data_lon < _outputProj.getMinx())
      raw_data_lon += 360.0;
    while (raw_data_lon >= _outputProj.getMinx() + 360.0)
      raw_data_lon -= 360.0;
      
    // Get the index into the MDV grid

    int mdv_data_index;

    if (_outputProj.latlon2arrayIndex(raw_data_lat, raw_data_lon,
				      mdv_data_index) < 0)
    {
      if (_params->verbose)
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Data point outside of output grid." << endl;
	cerr << "lat = " << hdf_file.getLat(i)
	     << ", lon = " << hdf_file.getLon(i) << endl;
      }
	
      continue;
    }
      
    // Update the MDV data value
	
    mdv_data[mdv_data_index] =
      hdf_file.getScanTime(i) - time_centroid;
	
  } /* endfor - i */

  // Add the new field to the MDV file.  Note that we don't want to
  // scale the field before adding it to the file because we need full
  // precision for time data.  We are not compressing the data here, either,
  // because the bzip compression never seemed to return for some reason.

  mdvx.addField(field);
    
  return true;
}


/*********************************************************************
 * _addMdvScanTimeField()
 */

bool GpmHdf2Mdv::_addMdvScanTimeField(DsMdvx &mdvx,
				       const HdfFile &hdf_file) const
{
  static const string method_name = "GpmHdf2Mdv::_addMdvScanTimeField()";
  
  // Create the blank field
  
  MdvxField *field = _createBlankMdvTimeField("scan_time", "secs");
    
  // Fill in the field data

  int num_elements =
    hdf_file.getNumScans() * hdf_file.getNumPixels();
    
  si32 *mdv_data = (si32 *)field->getVol();
    
  for (int i = 0; i < num_elements; ++i)
  {
    // Get the lat/lon for the raw data point and normalize the
    // lon to the output grid

    double raw_data_lat = hdf_file.getLat(i);
    double raw_data_lon = hdf_file.getLon(i);
	
    while (raw_data_lon < _outputProj.getMinx())
      raw_data_lon += 360.0;
    while (raw_data_lon >= _outputProj.getMinx() + 360.0)
      raw_data_lon -= 360.0;
      
    // Get the index into the MDV grid

    int mdv_data_index;

    if (_outputProj.latlon2arrayIndex(raw_data_lat, raw_data_lon,
				      mdv_data_index) < 0)
    {
      if (_params->verbose)
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Data point outside of output grid." << endl;
	cerr << "lat = " << hdf_file.getLat(i)
	     << ", lon = " << hdf_file.getLon(i) << endl;
      }
	
      continue;
    }
      
    // Update the MDV data value
	
    mdv_data[mdv_data_index] = hdf_file.getScanTime(i);
	
  } /* endfor - i */

  // Add the new field to the MDV file.  Note that we don't want to
  // scale the field before adding it to the file because we need full
  // precision for time data.  We are not compressing the data here, either,
  // because the bzip compression never seemed to return for some reason.

  mdvx.addField(field);
    
  return true;
}


/*********************************************************************
 * _addMdvSolarMagField()
 */

bool GpmHdf2Mdv::_addMdvSolarMagField(DsMdvx &mdvx,
				       const HdfFile &hdf_file) const
{
  static const string method_name = "GpmHdf2Mdv::_addMdvSolarMagField()";
  
  // Create the blank field
  
  MdvxField *field = _createBlankMdvField("solar_mag", "");
    
  // Fill in the field data

  int num_elements =
    hdf_file.getNumScans() * hdf_file.getNumPixels();
    
  fl32 *mdv_data = (fl32 *)field->getVol();
    
  for (int i = 0; i < num_elements; ++i)
  {
    // Get the lat/lon for the raw data point and normalize the
    // lon to the output grid

    double raw_data_lat = hdf_file.getLat(i);
    double raw_data_lon = hdf_file.getLon(i);
	
    while (raw_data_lon < _outputProj.getMinx())
      raw_data_lon += 360.0;
    while (raw_data_lon >= _outputProj.getMinx() + 360.0)
      raw_data_lon -= 360.0;
      
    // Get the index into the MDV grid

    int mdv_data_index;

    if (_outputProj.latlon2arrayIndex(raw_data_lat, raw_data_lon,
				      mdv_data_index) < 0)
    {
      if (_params->verbose)
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Data point outside of output grid." << endl;
	cerr << "lat = " << hdf_file.getLat(i)
	     << ", lon = " << hdf_file.getLon(i) << endl;
      }
	
      continue;
    }
      
    // Update the MDV data value
	
    mdv_data[mdv_data_index] = hdf_file.getSunMag(i);
	
  } /* endfor - i */

  // Add the new field to the MDV file.

  field->convertType(Mdvx::ENCODING_INT8,
		     Mdvx::COMPRESSION_BZIP,
		     Mdvx::SCALING_DYNAMIC);
  
  mdvx.addField(field);
    
  return true;
}


/*********************************************************************
 * _addMdvSolarZenithField()
 */

bool GpmHdf2Mdv::_addMdvSolarZenithField(DsMdvx &mdvx,
					  const HdfFile &hdf_file) const
{
  static const string method_name = "GpmHdf2Mdv::_addMdvSolarZenithField()";
  
  // Create the blank field
  
  MdvxField *field = _createBlankMdvField("solar_zenith", "deg");
    
  // Fill in the field data

  int num_elements =
    hdf_file.getNumScans() * hdf_file.getNumPixels();
    
  fl32 *mdv_data = (fl32 *)field->getVol();
    
  for (int i = 0; i < num_elements; ++i)
  {
    // Get the lat/lon for the raw data point and normalize the
    // lon to the output grid

    double raw_data_lat = hdf_file.getLat(i);
    double raw_data_lon = hdf_file.getLon(i);
	
    while (raw_data_lon < _outputProj.getMinx())
      raw_data_lon += 360.0;
    while (raw_data_lon >= _outputProj.getMinx() + 360.0)
      raw_data_lon -= 360.0;
      
    // Get the index into the MDV grid

    int mdv_data_index;

    if (_outputProj.latlon2arrayIndex(raw_data_lat, raw_data_lon,
				      mdv_data_index) < 0)
    {
      if (_params->verbose)
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Data point outside of output grid." << endl;
	cerr << "lat = " << hdf_file.getLat(i)
	     << ", lon = " << hdf_file.getLon(i) << endl;
      }
	
      continue;
    }
      
    // Update the MDV data value
	
    mdv_data[mdv_data_index] = hdf_file.getSolarZenith(i);
	
  } /* endfor - i */

  // Add the new field to the MDV file.

  field->convertType(Mdvx::ENCODING_INT8,
		     Mdvx::COMPRESSION_BZIP,
		     Mdvx::SCALING_DYNAMIC);
  
  mdvx.addField(field);
    
  return true;
}


/*********************************************************************
 * _createBlankMdvField()
 */

MdvxField *GpmHdf2Mdv::_createBlankMdvField(const string &field_name,
					     const string &field_units) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  _outputProj.syncXyToFieldHdr(field_hdr);
  
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.grid_dz = 0.0;
  field_hdr.grid_minz = 0.0;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = HdfFile::MISSING_VALUE;
  field_hdr.missing_data_value = HdfFile::MISSING_VALUE;
  strncpy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  strncpy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  strncpy(field_hdr.units, field_units.c_str(), MDV_UNITS_LEN);
  
  if (_params->verbose)
    Mdvx::printFieldHeader(field_hdr, cerr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  if (_params->verbose)
    Mdvx::printVlevelHeader(vlevel_hdr, 1,
			    field_hdr.field_name, cerr);
  
  // Create the field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _createBlankMdvTimeField()
 */

MdvxField *GpmHdf2Mdv::_createBlankMdvTimeField(const string &field_name,
						 const string &field_units) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  _outputProj.syncXyToFieldHdr(field_hdr);
  
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.grid_dz = 0.0;
  field_hdr.grid_minz = 0.0;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = DateTime::NEVER;
  field_hdr.missing_data_value = DateTime::NEVER;
  strncpy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  strncpy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  strncpy(field_hdr.units, field_units.c_str(), MDV_UNITS_LEN);
  
  if (_params->verbose)
    Mdvx::printFieldHeader(field_hdr, cerr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  if (_params->verbose)
    Mdvx::printVlevelHeader(vlevel_hdr, 1,
			    field_hdr.field_name, cerr);
  
  // Create the field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _createFieldVector()
 */

bool GpmHdf2Mdv::_createFieldVector(Params::output_field_t output_field,
				     vector< FieldInfo > &field_info)
{
  static const string method_name = "GpmHdf2Mdv::_createFieldVector()";
  
  // Parse the field names string.

  vector< string > field_names;
  
  if (!_parseStringList(output_field.mdv_field_names, field_names))
    return false;
  
  if (field_names.size() != (size_t)output_field.num_fields)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing field names string from param file" << endl;
    cerr << "Expected " << output_field.num_fields << " names" << endl;
    cerr << "Found " << field_names.size() << " names" << endl;
    cerr << "String: <" << output_field.mdv_field_names << ">" << endl;
      
    return false;
  }
    
  // Parse the field units string

  vector< string > field_units;
  
  if (!_parseStringList(output_field.mdv_field_units, field_units))
    return false;
  
  if (field_units.size() != (size_t)output_field.num_fields)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing field units string from param file" << endl;
    cerr << "Expected " << output_field.num_fields << " units" << endl;
    cerr << "Found " << field_units.size() << " units" << endl;
    cerr << "String: <" << output_field.mdv_field_units << ">" << endl;
      
    return false;
  }
    
  // Parse the scales string.  If the scales string is empty, the data in
  // the TRMM file isn't scaled.

  vector< double > scales;
  
  if (output_field.scales[0] != '\0')
  {
    if (!_parseDoubleList(output_field.scales, scales))
      return false;
  
    if (scales.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing scales string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " scales" << endl;
      cerr << "Found " << scales.size() << " scales" << endl;
      cerr << "String: <" << output_field.scales << ">" << endl;
    
      return false;
    }
  }
  
  // Parse the biases string.  If the biases string is empty, the data in
  // the TRMM file doesn't use a bias.

  vector< double > biases;
  
  if (output_field.biases[0] != '\0')
  {
    if (!_parseDoubleList(output_field.biases, biases))
      return false;
  
    if (biases.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing biases string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " biases" << endl;
      cerr << "Found " << biases.size() << " biases" << endl;
      cerr << "String: <" << output_field.biases << ">" << endl;
    
      return false;
    }
  }
  
  // Parse the missing values string.  If the missing values string is empty,
  // the data in the TRMM file doesn't use a missing data value.

  vector< double > missing_values1;
  
  if (output_field.missing_values1[0] != '\0')
  {
    if (!_parseDoubleList(output_field.missing_values1, missing_values1))
      return false;
  
    if (missing_values1.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing missing values 1 string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " missing values" << endl;
      cerr << "Found " << missing_values1.size() << " missing values" << endl;
      cerr << "String: <" << output_field.missing_values1 << ">" << endl;
    
      return false;
    }
  }
  
  vector< double > missing_values2;
  
  if (output_field.missing_values2[0] != '\0')
  {
    if (!_parseDoubleList(output_field.missing_values2, missing_values2))
      return false;
  
    if (missing_values2.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing missing values 2 string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " missing values" << endl;
      cerr << "Found " << missing_values2.size() << " missing values" << endl;
      cerr << "String: <" << output_field.missing_values2 << ">" << endl;
    
      return false;
    }
  }
  
  vector< double > missing_values3;
  
  if (output_field.missing_values3[0] != '\0')
  {
    if (!_parseDoubleList(output_field.missing_values3, missing_values3))
      return false;
  
    if (missing_values3.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing missing values 3 string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " missing values" << endl;
      cerr << "Found " << missing_values3.size() << " missing values" << endl;
      cerr << "String: <" << output_field.missing_values3 << ">" << endl;
    
      return false;
    }
  }
  
  // Parse the bad values string.  If the bad values string is empty,
  // the data in the TRMM file doesn't use a bad data value.

  vector< double > bad_values1;
  
  if (output_field.bad_values1[0] != '\0')
  {
    if (!_parseDoubleList(output_field.bad_values1, bad_values1))
      return false;
  
    if (bad_values1.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing bad values 1 string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " bad values" << endl;
      cerr << "Found " << bad_values1.size() << " bad values" << endl;
      cerr << "String: <" << output_field.bad_values1 << ">" << endl;
    
      return false;
    }
  }
  
  vector< double > bad_values2;
  
  if (output_field.bad_values2[0] != '\0')
  {
    if (!_parseDoubleList(output_field.bad_values2, bad_values2))
      return false;
  
    if (bad_values2.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing bad values 2 string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " bad values" << endl;
      cerr << "Found " << bad_values2.size() << " bad values" << endl;
      cerr << "String: <" << output_field.bad_values2 << ">" << endl;
    
      return false;
    }
  }
  
  vector< double > bad_values3;
  
  if (output_field.bad_values3[0] != '\0')
  {
    if (!_parseDoubleList(output_field.bad_values3, bad_values3))
      return false;
  
    if (bad_values3.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing bad values 3 string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " bad values" << endl;
      cerr << "Found " << bad_values3.size() << " bad values" << endl;
      cerr << "String: <" << output_field.bad_values3 << ">" << endl;
    
      return false;
    }
  }
  
  vector< string > convert_rad_strings;
  vector< RadConvert::convert_type_t > convert_rad_values;
  
  if (output_field.radiance_convert[0] == '\0')
  {
    for (int i = 0; i < output_field.num_fields; ++i)
      convert_rad_values.push_back(RadConvert::CONVERT_RAD_NONE);
  }
  else
  {
    if (!_parseStringList(output_field.radiance_convert, convert_rad_strings))
      return false;
  
    if (convert_rad_strings.size() != (size_t)output_field.num_fields)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing radiance conversion string from param file" << endl;
      cerr << "Expected " << output_field.num_fields << " radiance convert values" << endl;
      cerr << "Found " << convert_rad_strings.size() << " radiance convert values" << endl;
      cerr << "String: <" << output_field.radiance_convert << ">" << endl;
    
      return false;
    }

    for (int i = 0; i < output_field.num_fields; ++i)
    {
      if (convert_rad_strings[i] == "CONVERT_RAD_NONE")
      {
	convert_rad_values.push_back(RadConvert::CONVERT_RAD_NONE);
      }
      else if (convert_rad_strings[i] == "CONVERT_RAD_VIS")
      {
	convert_rad_values.push_back(RadConvert::CONVERT_RAD_VIS);
	_loadSolarCalibData = true;
      }
      else if (convert_rad_strings[i] == "CONVERT_RAD_CH3")
      {
	convert_rad_values.push_back(RadConvert::CONVERT_RAD_CH3);
	_loadBtTable = true;
      }
      else if (convert_rad_strings[i] == "CONVERT_RAD_CH4")
      {
	convert_rad_values.push_back(RadConvert::CONVERT_RAD_CH4);
	_loadBtTable = true;
      }
      else if (convert_rad_strings[i] == "CONVERT_RAD_CH5")
      {
	convert_rad_values.push_back(RadConvert::CONVERT_RAD_CH5);
	_loadBtTable = true;
      }
      else
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Invalid radiance conversion string found: <"
	     << convert_rad_strings[i] << ">" << endl;
	cerr << "Radiance conversion string must be one of the following:" << endl;
	cerr << "   CONVERT_RAD_NONE" << endl;
	cerr << "   CONVERT_RAD_VIS" << endl;
	cerr << "   CONVERT_RAD_CH3" << endl;
	cerr << "   CONVERT_RAD_CH4" << endl;
	cerr << "   CONVERT_RAD_CH5" << endl;
	
	return false;
      }
      
    } /* endfor - i */
    
  }
  
  // Create the field information vector

  for (int i = 0; i < output_field.num_fields; ++i)
  {
    FieldInfo field;
    
    field.setMdvFieldName(field_names[i]);
    field.setMdvFieldUnits(field_units[i]);

    if (scales.size() > 0)
      field.setScale(scales[i]);
    if (biases.size() > 0)
      field.setBias(biases[i]);

    if (missing_values1.size() > 0)
      field.addMissingValue(missing_values1[i]);
    if (missing_values2.size() > 0)
      field.addMissingValue(missing_values2[i]);
    if (missing_values3.size() > 0)
      field.addMissingValue(missing_values3[i]);

    if (bad_values1.size() > 0)
      field.addBadValue(bad_values1[i]);
    if (bad_values2.size() > 0)
      field.addBadValue(bad_values2[i]);
    if (bad_values3.size() > 0)
      field.addBadValue(bad_values3[i]);

    field.setRadConvertType(convert_rad_values[i]);
    
    field_info.push_back(field);
  }
  
  return true;
}


/*********************************************************************
 * _createVertLevelsVector()
 */

bool GpmHdf2Mdv::_createVertLevelsVector(Params::output_field_t output_field,
					  vector< double > &vert_levels,
					  bool &dz_constant) const
{
  static const string method_name = "GpmHdf2Mdv::_createVertLevelsVector()";
  
  switch (output_field.level_spec_type)
  {
  case Params::LEVELS_LISTED :
  {
    if (!_parseDoubleList(output_field.vert_levels, vert_levels))
      return false;
    
    if (vert_levels.size() != (size_t)output_field.num_vert_levels)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing vertical levels string from param file" << endl;
      cerr << "Expected " << output_field.num_vert_levels << " levels" << endl;
      cerr << "Found " << vert_levels.size() << " levels" << endl;
      cerr << "String: <" << output_field.vert_levels << ">" << endl;
      
      return false;
    }
    
    dz_constant = false;
    
    return true;
  } /* endcase - LEVELS_LISTED */
  
  case Params::LEVELS_CONST_DZ :
  {
    for (int i = 0; i < output_field.num_vert_levels; ++i)
      vert_levels.push_back(output_field.min_vert_level +
			    (i * output_field.delta_vert_level));
    
    dz_constant = true;
    
    return true;
  } /* endcase - LEVELS_CONST_DZ */

  } /* endswitch - output_field.level_spec_type */
  
  return true;
}


/*********************************************************************
 * _initDataHandlers()
 */

bool GpmHdf2Mdv::_initDataHandlers()
{
  static const string method_name = "GpmHdf2Mdv::_initDataHandlers()";
  
  for (int i = 0; i < _params->output_fields_n; ++i)
  {
    // Check the dimensions of the field.  The MDV files store the
    // volume size in an si32 variable, so if we calculate the volume
    // size here and it is negative, then we have an overflow condition
    // and the user needs to reduce the size of this output field.  We
    // are storing the data in fl32 volumes, so each element contains
    // 4 bytes.

    si32 volume_size = _params->output_proj.nx * _params->output_proj.ny *
      _params->_output_fields[i].num_vert_levels * 4;
    
    if (volume_size < 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "SDS field " << _params->_output_fields[i].sds_field_name
	   << " too large for MDV file" << endl;
      cerr << "Reduce size of output projection and try again." << endl;
      
      return false;
    }
    
    // Convert the parameter file vertical level type to the MDV value

    Mdvx::vlevel_type_t mdv_vert_type = Mdvx::VERT_TYPE_Z;
    
    switch (_params->_output_fields[i].vert_level_type)
    {
    case Params::VERT_TYPE_SATELLITE_IMAGE :
      mdv_vert_type = Mdvx::VERT_SATELLITE_IMAGE;
      break;
    case Params::VERT_TYPE_Z :
      mdv_vert_type = Mdvx::VERT_TYPE_Z;
      break;
    }
    
    // Create the vectors of field names and units for this SDS field

    vector< FieldInfo > field_info;
    
    if (!_createFieldVector(_params->_output_fields[i], field_info))
      return false;
    
    // Create the vector of vertical levels for this field

    vector< double > vert_levels;
    bool dz_constant;
    
    if (!_createVertLevelsVector(_params->_output_fields[i],
				 vert_levels, dz_constant))
      return false;
    
    // Now create the handler object
    
    switch (_params->_output_fields[i].data_type)
    {
    case Params::FULL_RES_DATA :
    {
      if (_params->debug)
	cerr << "Initializing full resolution data handler for SDS field: "
	     << _params->_output_fields[i].sds_field_name << endl;
      
      FullResSdsField *full_res_sds_field =
	new FullResSdsField(_params->_output_fields[i].sds_field_name,
			    field_info,
			    _params->_output_fields[i].num_vert_levels,
			    vert_levels,
			    mdv_vert_type,
			    dz_constant,
			    _params->_output_fields[i].invert_vert_levels,
			    _radConvert,
			    _params->debug,
			    _params->verbose);

      _dataHandlers.push_back(full_res_sds_field);
    }
    break;
      
    case Params::LOW_RES_DATA :
    {
      if (_params->debug)
	cerr << "Initializing low resolution data handler for SDS field: "
	     << _params->_output_fields[i].sds_field_name << endl;
      
      LowResSdsField *low_res_sds_field =
	new LowResSdsField(_params->_output_fields[i].sds_field_name,
			   field_info,
			   _params->_output_fields[i].num_vert_levels,
			   vert_levels,
			   mdv_vert_type,
			   dz_constant,
                           _params->_output_fields[i].invert_vert_levels,
			   _radConvert,
			   _params->debug,
			   _params->verbose);

      _dataHandlers.push_back(low_res_sds_field);
    }
    break;
      
    }
    
  } /* endfor - i */

  return true;
}


/*********************************************************************
 * _initOutputProjection()
 */

bool GpmHdf2Mdv::_initOutputProjection()
{
  switch (_params->output_proj.proj_type)
  {
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params->output_proj.origin_lat,
			 _params->output_proj.origin_lon,
			 _params->output_proj.rotation,
			 _params->output_proj.nx,
			 _params->output_proj.ny,
			 1,
			 _params->output_proj.dx,
			 _params->output_proj.dy,
			 1.0,
			 _params->output_proj.minx,
			 _params->output_proj.miny,
			 0.0);
    break;
    
  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params->output_proj.nx,
			   _params->output_proj.ny,
			   1,
			   _params->output_proj.dx,
			   _params->output_proj.dy,
			   1.0,
			   _params->output_proj.minx,
			   _params->output_proj.miny,
			   0.0);
    break;
    
  case Params::PROJ_LC2 :
    _outputProj.initLc2(_params->output_proj.origin_lat,
			_params->output_proj.origin_lon,
			_params->output_proj.lat1,
			_params->output_proj.lat2,
			_params->output_proj.nx,
			_params->output_proj.ny,
			1,
			_params->output_proj.dx,
			_params->output_proj.dy,
			1.0,
			_params->output_proj.minx,
			_params->output_proj.miny,
			0.0);
    break;
  }
  
  return true;
  
}


/*********************************************************************
 * _initTrigger()
 */

bool GpmHdf2Mdv::_initTrigger()
{
  static const string method_name = "GpmHdf2Mdv;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, false,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR_RECURSE :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, true,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR_RECURSE trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

return true;
}

    
/*********************************************************************
 * _parseDoubleList()
 */

bool GpmHdf2Mdv::_parseDoubleList(const char *double_list_string,
				   vector< double > &double_list)
{
  static const string method_name = "GpmHdf2Mdv::_parseDoubleList()";
  
  // Make a copy of the string since strtok corrupts the string

  char *tmp_str = STRdup(double_list_string);
  
  char *token = strtok(tmp_str, ",");
  double_list.push_back(atof(token));
  
  while ((token = strtok(0, ",")) != 0)
    double_list.push_back(atof(token));
  
  STRfree(tmp_str);
  
  return true;
}


/*********************************************************************
 * _parseStringList()
 */

bool GpmHdf2Mdv::_parseStringList(const char *string_list_string,
				   vector< string > &string_list)
{
  static const string method_name = "GpmHdf2Mdv::_parseStringList()";
  
  // Make a copy of the string since strtok corrupts the string

  char *tmp_str = STRdup(string_list_string);
  
  char *token = strtok(tmp_str, ",");
  string_list.push_back(token);
  
  while ((token = strtok(0, ",")) != 0)
    string_list.push_back(token);
  
  STRfree(tmp_str);
  
  return true;
}


/*********************************************************************
 * _setMasterHeader()
 */

void GpmHdf2Mdv::_setMasterHeader(DsMdvx &mdvx,
				   const DateTime &begin_time,
				   const DateTime &end_time,
				   const string &input_path) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));

  master_hdr.time_gen = time(0);
  master_hdr.time_begin = begin_time.utime();
  master_hdr.time_end = end_time.utime();
  master_hdr.time_centroid =
    (master_hdr.time_begin / 2) + (master_hdr.time_end / 2);
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.vlevel_included = 1;
  strncpy(master_hdr.data_set_info,
	  "TRMM satellite data ingested by GpmHdf2Mdv", MDV_INFO_LEN);
  strncpy(master_hdr.data_set_name, "GpmHdf2Mdv", MDV_NAME_LEN);
  strncpy(master_hdr.data_set_source, input_path.c_str(), MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}
