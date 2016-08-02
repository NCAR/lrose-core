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
// TopsFilter.cc
//
// TopsFilter object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1998
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>

#include <toolsa/os_config.h>
#include <euclid/CircularTemplate.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxInput.hh>
#include <toolsa/str.h>
#include <toolsa/TaArray.hh>

#include "TopsFilter.hh"
#include "Args.hh"
#include "Params.hh"


// Global variables

TopsFilter *TopsFilter::_instance = (TopsFilter *)NULL;

// Global constants

const int FOREVER = true;


/**************************************************************
 * Constructor
 */

TopsFilter::TopsFilter(int argc, char **argv) :
  _outputField(0)
{
  const string routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TopsFilter *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _programName = STRdup(progname_parts.base);
  
  // Get command line args

  _args = new Args(argc, argv, _programName);

  if (!_args->okay)
  {
    cerr << "ERROR: " << _programName << endl;
    cerr << "Problem with command line args" << endl;

    okay = false;

    return;
  }

  _archiveStartTime = _args->startTime.utime();
  _archiveEndTime = _args->endTime.utime();
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

  // Initialize process registration
  
  PMU_auto_init(_programName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // Initialize data handles

  if (_params->mode == Params::REALTIME)
    _radarRetriever.setRealtime(_params->radar_data_url,
				_params->max_valid_age,
				PMU_auto_register,
				_params->sleep_msecs);
  else
    _radarRetriever.setArchive(_params->radar_data_url,
			       _archiveStartTime,
			       _archiveEndTime);
  
  // Create the circular template used for filtering the data

  _template = new CircularTemplate(_params->radius_of_influence);
  
}


/**************************************************************
 * Destructor
 */

TopsFilter::~TopsFilter()
{
  // Unregister process

  PMU_auto_unregister();

  // Free strings

  STRfree(_programName);
  
  // Call destructors.  Note that we don't delete the _outputField
  // pointer because it is always added to the output file object,
  // which takes control of the pointer at that point.

  delete _params;
  delete _args;
  delete _template;
  
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TopsFilter *TopsFilter::Inst(int argc, char **argv)
{
  if (_instance == (TopsFilter *)NULL)
    new TopsFilter(argc, argv);
  
  return(_instance);
}

TopsFilter *TopsFilter::Inst()
{
  assert(_instance != (TopsFilter *)NULL);
  
  return(_instance);
}


/**************************************************************
 * run()
 */

void TopsFilter::run()
{
  // Process new input files forever.
  
  while (FOREVER)
  {
    PMU_auto_register("Waiting for data");
      
    if (_params->debug_level >= Params::DEBUG_EXTRA)
      cerr << endl;
    
    // Get the radar data
     
    time_t radar_time;
      
    if ((radar_time = _getRadarData()) < 0)
    {
      if (_params->mode == Params::ARCHIVE)
	return;
      
      continue;
    }
    
    // Get the matching tops data

    if (_getTopsData(radar_time))
    {
      // We have all of the data so we can go ahead and filter the
      // grid.

      PMU_auto_register("Filtering data");
    
      if (!_filterTops())
	continue;
    }
    
    if (_params->debug_level >= Params::DEBUG_NORM)
      cerr << "Writing output file" << endl;
    
    _writeOutputFile();

    if (_params->debug_level >= Params::DEBUG_NORM)
      cerr << "---> Done processing files" << endl;
    
  } /* endwhile - Forever */
    
  return;
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/**************************************************************
 * _getRadarData() - Wait for new radar data and fill an internal
 *                   structure when it arrives.
 *
 * Returns the data time of the radar data in UNIX format if
 * radar data was successfully retrieved, -1 otherwise.
 *
 * Upon successful return, _radarMasterHdr, _radarFieldHdr
 * and _radarData are correctly set.
 */

time_t TopsFilter::_getRadarData(void)
{
  const string routine_name = "_getRadarData()";
  
  // Construct the read request

  _radarFile.clearRead();
  
  _radarFile.addReadField(_params->radar_field_num);
  _radarFile.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _radarFile.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _radarFile.setReadScalingType(Mdvx::SCALING_NONE);
  _radarFile.setReadFieldFileHeaders();
  
  // Wait for new radar data

  if (_radarRetriever.readVolumeNext(_radarFile) != 0)
  {
    if (_params->mode == Params::REALTIME)
    {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "Error reading in next radar file" << endl;
      cerr << _radarRetriever.getErrStr() << endl;
    }
    
    return -1;
  }
  
  // Register with the process mapper

  char pmu_message[BUFSIZ];
  
  sprintf(pmu_message, "Processing file <%s>",
	  _radarFile.getPathInUse().c_str());
  PMU_force_register(pmu_message);
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << pmu_message << endl;
  
  // Create the output field which initially contains a copy of the
  // radar data.

  _outputField = new MdvxField(*(_radarFile.getFieldByNum(0)));
  
  return _radarFile.getMasterHeader().time_centroid;

}


/**************************************************************
 * _getTopsData() - Get the tops data matching the time of the
 *                  radar data.
 *
 * Returns true if tops data was successfully retrieved, false
 * otherwise.
 *
 * Upon successful return, _topsData is correctly set.
 */

bool TopsFilter::_getTopsData(time_t radar_data_time)
{
  static char *routine_name = "_getTopsData()";
  
  PMU_auto_register("Reading tops data");

  // Construct the read request

  _topsFile.clearRead();
  
  _topsFile.setReadTime(Mdvx::READ_CLOSEST,
			_params->tops_data_url,
			_params->time_offset_max,
			radar_data_time);
  _topsFile.addReadField(_params->tops_field_num);
  _topsFile.setReadPlaneNumLimits(_params->tops_level_num,
				  _params->tops_level_num);
  _topsFile.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _topsFile.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _topsFile.setReadScalingType(Mdvx::SCALING_NONE);
  
  // We always remap the tops data to match the radar data.
  // We won't reach here unless we've successfully read some
  // radar data.

  Mdvx::field_header_t output_field_hdr = _outputField->getFieldHeader();
  
  switch (output_field_hdr.proj_type)
  {
  case Mdvx::PROJ_LATLON :
    _topsFile.setReadRemapLatlon(output_field_hdr.nx,
				 output_field_hdr.ny,
				 output_field_hdr.grid_minx,
				 output_field_hdr.grid_miny,
				 output_field_hdr.grid_dx,
				 output_field_hdr.grid_dy);
    break;
    
  case Mdvx::PROJ_FLAT :
    _topsFile.setReadRemapFlat(output_field_hdr.nx,
			       output_field_hdr.ny,
			       output_field_hdr.grid_minx,
			       output_field_hdr.grid_miny,
			       output_field_hdr.grid_dx,
			       output_field_hdr.grid_dy,
			       output_field_hdr.proj_origin_lat,
			       output_field_hdr.proj_origin_lon,
			       output_field_hdr.proj_rotation);
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    _topsFile.setReadRemapLc2(output_field_hdr.nx,
			      output_field_hdr.ny,
			      output_field_hdr.grid_minx,
			      output_field_hdr.grid_miny,
			      output_field_hdr.grid_dx,
			      output_field_hdr.grid_dy,
			      output_field_hdr.proj_origin_lat,
			      output_field_hdr.proj_origin_lon,
			      output_field_hdr.proj_param[0],
			      output_field_hdr.proj_param[1]);
    break;
    
  default:
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Cannot remap tops data to match radar data projection" << endl;
    cerr << "Radar data uses " <<
      Mdvx::projType2Str(output_field_hdr.proj_type) << " projection" << endl;
    
    return false;

  } /* endswitch - output_field_hdr.proj_type */
  
  // Read the data

  if (_topsFile.readVolume() != 0)
  {
    // Couldn't find tops data.  In realtime mode, wait to see if some
    // data appears.

    if (_params->mode == Params::REALTIME)
    {
      while (true)
      {
	PMU_auto_register("Waiting for tops data...");

	if (_topsFile.readVolume() == 0)
	  break;
      
	time_t current_time = time(0);
      
	if (current_time > radar_data_time + _params->time_offset_max)
	{
	  cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	  cerr << "No realtime tops data found for radar data at time: " <<
	    utimstr(radar_data_time) << endl;
	
	  return false;
	}
      
	sleep(_params->sleep_msecs / 1000);
      
      } /* endwhile - true */
    }
    else
    {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "No archive tops data found for radar data at time: " <<
	utimstr(radar_data_time) << endl;
	
      return false;
    }
    
  } /* endif - _topsFile.readVolume() != 0 */
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "Filtering using tops file <" << _topsFile.getPathInUse() <<
      ">" << endl;
  
  return true;
}


/**************************************************************
 * _getTopsValue() - Retrieve the tops value for the given point.
 *
 * Returns the tops value for the point on success, -1.0 if there
 * is no valid data at the point.
 */

double TopsFilter::_getTopsValue(int x, int y)
{
  PMU_auto_register("Filtering data");
  
  // Create the array for storing the tops values

  TaArray<double> value_array_;
  double *value_array = value_array_.alloc(_template->size());
  
  // Loop through the template to find the maximum tops value

  GridPoint *point;
  int value_points = 0;
  
  MdvxField *tops_field = _topsFile.getFieldByNum(0);
  Mdvx::field_header_t tops_field_hdr = tops_field->getFieldHeader();
  fl32 *tops_data = (fl32 *)tops_field->getVol();
  
  for (point = _template->getFirstInGrid(x, y,
					 tops_field_hdr.nx, tops_field_hdr.ny);
       point != (GridPoint *)NULL;
       point = _template->getNextInGrid())
  {
    int index = point->getIndex(tops_field_hdr.nx, tops_field_hdr.ny);
    
    if (tops_data[index] != (fl32)tops_field_hdr.missing_data_value &&
	tops_data[index] != (fl32)tops_field_hdr.bad_data_value)
      value_array[value_points++] = tops_data[index];
    
  } /* endfor - point */
  
  // Make sure there was some non-missing data

  if (value_points <= _params->num_ignored_top_values)
    return -1.0;
  
  // Sort the array and return the "num_ignored_top_values"nth point

  qsort(value_array, value_points, sizeof(double), _qsortValueArray);
  
  return value_array[_params->num_ignored_top_values];
}


/**************************************************************
 * _qsortValueArray() - Set the values in the output MDV headers.
 */

int TopsFilter::_qsortValueArray(const void *value1, const void *value2)
{
  double double_value1 = *(double *)value1;
  double double_value2 = *(double *)value2;
  
  if (double_value1 > double_value2)
    return -1;
  
  if (double_value1 < double_value2)
    return 1;
  
  return 0;
}


/**************************************************************
 * _filterTops() - Filter the radar data using the tops data.
 *
 * Returns true upon success, false upon error.
 *
 * Upon return, _radarData has been filtered.
 */

bool TopsFilter::_filterTops(void)
{
  Mdvx::field_header_t output_field_hdr = _outputField->getFieldHeader();
  fl32 *output_data = (fl32 *)_outputField->getVol();
  
  // Go through each data point

  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "Filtering data..." << endl;
  
  for (int y = 0; y < output_field_hdr.ny; ++y)
  {
    PMU_auto_register("Filtering data...");

    for (int x = 0; x < output_field_hdr.nx; ++x)
    {
      double tops_value = _getTopsValue(x, y);
      int grid_index = (output_field_hdr.nx * y) + x;
      
      if (tops_value < 0.0)
      {
	if (!_params->leave_missing_data_alone)
	  output_data[grid_index] = output_field_hdr.missing_data_value;

	continue;
      }
    
      // Convert the tops value to feet so it can be compared with the
      // parameter file min value.

      switch (_params->tops_units)
      {
      case Params::UNITS_FEET :
	break;
      
      case Params::UNITS_KFEET :
	tops_value *= 1000.0;
	break;
      }
    
      if (tops_value < _params->min_valid_height)
	output_data[grid_index] = output_field_hdr.missing_data_value;

    } /* endfor - x */
  } /* endfor - y */
  
  return(TRUE);
}


/**************************************************************
 * _writeOutputFile() - Writes the filtered data to the appropriate
 *                      output file.
 */

void TopsFilter::_writeOutputFile(void)
{
  static char *routine_name = "_writeOutputFile()";
  
  // Create the output file

  DsMdvx output_file;
  
  // Set the master header values based on the radar master header values

  Mdvx::master_header_t radar_master_hdr = _radarFile.getMasterHeader();
  Mdvx::master_header_t output_master_hdr;
  
  Mdvx::field_header_t output_field_hdr = _outputField->getFieldHeader();
  
  memset(&output_master_hdr, 0, sizeof(output_master_hdr));
  
  output_master_hdr.time_gen = time(0);
  output_master_hdr.user_time = radar_master_hdr.user_time;
  output_master_hdr.time_begin = radar_master_hdr.time_begin;
  output_master_hdr.time_end = radar_master_hdr.time_end;
  output_master_hdr.time_centroid = radar_master_hdr.time_centroid;
  output_master_hdr.time_expire = radar_master_hdr.time_expire;
  output_master_hdr.data_dimension = 2;
  output_master_hdr.data_collection_type =
    radar_master_hdr.data_collection_type;
  output_master_hdr.user_data = radar_master_hdr.user_data;
  output_master_hdr.native_vlevel_type = radar_master_hdr.native_vlevel_type;
  output_master_hdr.vlevel_type = radar_master_hdr.vlevel_type;
  output_master_hdr.vlevel_included = 1;
  output_master_hdr.grid_orientation = radar_master_hdr.grid_orientation;
  output_master_hdr.data_ordering = radar_master_hdr.data_ordering;
  output_master_hdr.n_fields = 0;
  output_master_hdr.max_nx = output_field_hdr.nx;
  output_master_hdr.max_ny = output_field_hdr.ny;
  output_master_hdr.max_nz = 1;
  output_master_hdr.n_chunks = 0;
  output_master_hdr.field_grids_differ = 0;
  for (int i = 0; i < 8; ++i)
    output_master_hdr.user_data_si32[i] = radar_master_hdr.user_data_si32[i];
  
  for (int i = 0; i < 6; ++i)
    output_master_hdr.user_data_fl32[i] = radar_master_hdr.user_data_fl32[i];
  output_master_hdr.sensor_lon = radar_master_hdr.sensor_lon;
  output_master_hdr.sensor_lat = radar_master_hdr.sensor_lat;
  output_master_hdr.sensor_alt = radar_master_hdr.sensor_alt;
  
  STRcopy(output_master_hdr.data_set_info, "tops_filter output",
	  MDV_INFO_LEN);
  STRcopy(output_master_hdr.data_set_name, radar_master_hdr.data_set_name,
	  MDV_NAME_LEN);
  STRcopy(output_master_hdr.data_set_source,
	  _radarFile.getPathInUse().c_str(), MDV_NAME_LEN);
  
  output_file.setMasterHeader(output_master_hdr);
  
  // Add the output field to the output file

  _outputField->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_RLE,
			    Mdvx::SCALING_SPECIFIED,
			    _outputField->getFieldHeaderFile()->scale,
			    _outputField->getFieldHeaderFile()->bias);
  
  output_file.addField(_outputField);
  
  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error writing data to output URL" << endl;
    cerr << "Data time: " <<
      utimstr(output_file.getMasterHeader().time_centroid) << endl;
    
    return;
  }
  
  return;
}
