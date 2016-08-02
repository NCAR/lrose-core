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
//   $Date: 2016/03/04 02:22:09 $
//   $Id: Mdv2Grib.cc,v 1.36 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.36 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Mdv2Grib: Mdv2Grib program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputPathTrigger.hh>
#include <dsdata/DsSpecificGenLdataTrigger.hh>
#include <dsserver/DsLdataInfo.hh>
#include <grib/GribFile.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "Mdv2Grib.hh"
#include "Params.hh"

#include "MultiplyDataConverter.hh"
#include "NullDataConverter.hh"

using namespace std;


// Global variables

Mdv2Grib *Mdv2Grib::_instance =
     (Mdv2Grib *)NULL;



/*********************************************************************
 * Constructor
 */

Mdv2Grib::Mdv2Grib(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "Mdv2Grib::Mdv2Grib()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (Mdv2Grib *)NULL);
  _inputPath = NULL;
  // Initialize the okay flag.

  okay = true;

  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  // Get TDRP parameters.

  _params = new Params();

  char *params_path;
  
  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;

    okay = false;

    return;
  }

  // If an additional TDRP file was specified, load that
  // over the existing params.
  if (NULL != _args->additional_tdrp_file){

    if (_params->debug){
      cerr << "Attempting to load additional param file " << _args->additional_tdrp_file << endl;
    }
    if (_params->load(_args->additional_tdrp_file, NULL, TRUE, FALSE)){
      cerr << "ERROR: " << method_name << endl;
      cerr << "Problem with TDRP parameters in file: " << _args->additional_tdrp_file << endl;
      okay = false;
      return;
    }
  }

  return;

}


/*********************************************************************
 * Destructor
 */

Mdv2Grib::~Mdv2Grib()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;

  delete _dataTrigger;
  
  if (_inputPath)
    delete _inputPath;

  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Mdv2Grib *Mdv2Grib::Inst(int argc, char **argv)
{
  if (_instance == (Mdv2Grib *)NULL)
    new Mdv2Grib(argc, argv);

  return(_instance);
}

Mdv2Grib *Mdv2Grib::Inst()
{
  assert(_instance != (Mdv2Grib *)NULL);

  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Mdv2Grib::init()
{
  static const string method_name = "Mdv2Grib::init()";

  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  case Params::LATEST_DATA_FCST :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
        _params->latest_data_trigger << endl;

    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->latest_data_trigger,
                      -1,
                      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
        _params->latest_data_trigger << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::DS_INPUT_PATH :
  case Params::LOCAL_FILEPATH_REALTIME :
  {
    if (_params->debug)
      cerr << "Initializing DS_INPUT_PATH trigger using input dir: " <<
        _params->input_url << endl;

    DsInputPathTrigger *trigger = new DsInputPathTrigger();
    if (trigger->init(_params->input_url,
		      18000, PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing DS_INPUT_PATH trigger using directory: "
	   << _params->input_url << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }

  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);

    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
        _params->time_list_trigger.start_time << endl;

      return false;
    }

    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
        _params->time_list_trigger.end_time << endl;

      return false;
    }

    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
                      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
        _params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
        endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getInputFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger." << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::SPECIFIC_GEN_TIME_LDATA :
  {
    vector < int > gen_times;
    gen_times.push_back(_params->specific_gen_time_ldata_trigger);
    
    DsSpecificGenLdataTrigger *trigger = new DsSpecificGenLdataTrigger();

    if (trigger->init(_params->input_url,
		      gen_times, -1, PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing SPECIFIC_GEN_TIME_LDATA trigger." << endl;
      cerr << "    url: " << _params->input_url << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }
  } /* endswitch - _params->trigger_mode */

  // Initialize the GRIB writer object

  _gribWriter.setDebugFlag(_params->debug);

  _gribWriter.setForecastFlag(_params->is_forecast_data);

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
                PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

int Mdv2Grib::run()
{
  static const string method_name = "Mdv2Grib::run()";

  int retVal = 0;

  TriggerInfo trigger_info;
      
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      retVal = -1;
	      
      continue;
    }
	  
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: "
	   << DateTime(trigger_info.getIssueTime()) << endl;
      retVal = -1;
	      
      continue;
    }
	  
  } /* endwhile - !_dataTrigger->endOfData() */

  return retVal;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getFieldType() - Get the GribWriter field type given the field type
 *                   from the parameter file.
 */

GribWriter::field_type_t Mdv2Grib::_getFieldType(const Params::field_type_t field_type) const
{
  switch (field_type)
  {
  case Params::FIELD_TYPE_NORMAL :
    return GribWriter::FIELD_TYPE_NORMAL;
      
  case Params::FIELD_TYPE_ACCUM :
    return GribWriter::FIELD_TYPE_ACCUM;
      
  case Params::FIELD_TYPE_AVG :
    return GribWriter::FIELD_TYPE_AVG;
      
  } /* endswitch - field_type */

  // Should never get here

  return GribWriter::FIELD_TYPE_NORMAL;
}


/*********************************************************************
 * _getOutputPath() - Construct the output file path
 */

void Mdv2Grib::_getOutputPath(const DsMdvx &input_mdv,
			      string &grib_file_path,
			      string &rel_data_path) const
{
  static const string method_name = "Mdv2Grib::_getOutputPath()";

  // Initialize the output paths

  grib_file_path = "";
  rel_data_path = "";
  
  // Construct the file name

  char file_name[MAX_PATH_LEN];

  if (_params->use_iso8601_filename_convention)
  {
    DateTime fileTime;

    DsMdvx::master_header_t inMhdr = input_mdv.getMasterHeader();
    MdvxField *field = input_mdv.getField(0);
    const Mdvx::field_header_t fHdr  =  field->getFieldHeader();
    
    // Get the proper time to assign to filename

    if (_params->is_forecast_data)
      fileTime.set(inMhdr.time_gen);
    else
      fileTime.set(inMhdr.time_centroid);
      
    int year = fileTime.getYear();
    int month = fileTime.getMonth();
    int day =  fileTime.getDay();
    int hour = fileTime.getHour();
    int minute = fileTime.getMin();
    int seconds = fileTime.getSec();
      
    if (_params->is_forecast_data)
    {
      int leadTime = fHdr.forecast_delta;
      int leadTimeHrs = leadTime/3600;
      int leadTimeMins = (leadTime % 3600 )/ 60;
	  
      sprintf(file_name, "%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.PT%.2d:%.2d.grb", 
	      _params->output_file_prefix, 
	      year, month, day, hour, minute, seconds,
	      leadTimeHrs, leadTimeMins);
    }
    else
    {
      sprintf(file_name, "%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.grb", 
	      _params->output_file_prefix,
	      year, month, day, hour, minute,seconds);
    }
  }
  else
  {
    DateTime valid_time;
      
    if (_params->is_forecast_data)
    {
      valid_time = input_mdv.getField(0)->getFieldHeader().forecast_time;
    }
    else
    {
      valid_time = input_mdv.getMasterHeader().time_centroid;
    }
      
    sprintf(file_name, "%s%04d%02d%02d_%02d%02d%02d.grb",
	    _params->output_file_prefix,
	    valid_time.getYear(), valid_time.getMonth(), valid_time.getDay(),
	    valid_time.getHour(), valid_time.getMin(), valid_time.getSec()); 
  }

  // Construct the output path.  Do this after constructing the file name
  // so we can also construct the relative data path here.

  char file_path[MAX_PATH_LEN];
  
  if (_params->use_ral_subdirs)
  {
    if (_params->is_forecast_data)
    {
      DateTime file_time = input_mdv.getMasterHeader().time_gen;
    
      char rel_file_path[MAX_PATH_LEN];
    
      sprintf(rel_file_path, "%04d%02d%02d/g_%02d%02d%02d",
	      file_time.getYear(), file_time.getMonth(), file_time.getDay(),
	      file_time.getHour(), file_time.getMin(), file_time.getSec());

      sprintf(file_path, "%s/%s",
	      _params->output_dir, rel_file_path);

      rel_data_path = string(rel_file_path) + "/" + string(file_name);
    }
    else
    {
      DateTime file_time = input_mdv.getMasterHeader().time_centroid;
    
      char rel_file_path[MAX_PATH_LEN];
    
      sprintf(rel_file_path, "%04d%02d%02d",
	      file_time.getYear(), file_time.getMonth(), file_time.getDay());

      sprintf(file_path, "%s/%s",
	      _params->output_dir, rel_file_path);

      rel_data_path = string(rel_file_path) + "/" + string(file_name);
    }
  }
  else
  {
    sprintf(file_path, "%s", _params->output_dir);

    rel_data_path = string(file_name);
  }
  
  // Make sure that the output directory exists

  if (ta_makedir_recurse(file_path) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: " << file_path << endl;
  }

  // Construct the full GRIB file path which is returned in the
  // parameters

  grib_file_path = string(file_path) + "/" + string(file_name);
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool Mdv2Grib::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "Mdv2Grib::_processData()";

  if (_params->debug)
  {
    if (_dataTrigger->getType() == DsTrigger::TYPE_TIME_TRIGGER)
      cerr << endl
	   << "*** Processing data for time: "
	   << DateTime(trigger_info.getIssueTime()) << endl;
    else
      cerr << endl
	   << "*** Processing file: " << trigger_info.getFilePath() << endl;
  }
  
  // Read in the input file

  DsMdvx input_mdv;

  if (!_readMdvFile(input_mdv, trigger_info))
    return false;

  // Open the output file

  string rel_data_path;
  string grib_file_path;

  _getOutputPath(input_mdv, grib_file_path, rel_data_path);
  
  if (!_gribWriter.openFile(grib_file_path))
    return false;

  // Extract each of the fields in the MDV file and write them to the
  // output GRIB file.

  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();

  for (int field_num = 0; field_num < master_hdr.n_fields; ++field_num)
  {
    
    int local_grib_tables_version = _params->grib_tables_version;
    if (_params->_output_fields[field_num].override_grib_tables_version != -1)
    {
      local_grib_tables_version = 
	_params->_output_fields[field_num].override_grib_tables_version;
    }

    MdvxField *field = input_mdv.getField(field_num);
    if (field == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error extracting field number " << field_num
           << " from MDV file." << endl;
      cerr << "Skipping field....." << endl;

      continue;
    }

    DataConverter *converter;
    switch (_params->_output_fields[field_num].data_convert_type)
    {
    case Params::DATA_CONVERT_NONE :
    {
      converter = new NullDataConverter();
      break;
    }
    case Params::DATA_CONVERT_MULTIPLY :
    {
      converter = new MultiplyDataConverter(_params->_output_fields[field_num].data_convert_parameter);
      break;
    }
    } /* endswitch - convert_type */

    if (!_gribWriter.writeField(master_hdr, *field, *converter,
                                local_grib_tables_version,
                                _params->originating_center,
                                _params->subcenter_id,
                                _params->generating_process_id,
                                _params->grid_id,
                                _params->_output_fields[field_num].grib_code,
                                _params->_output_fields[field_num].precision,
                                _params->_output_fields[field_num].max_bit_length,
                                _params->forecast_interval_type,
                                _params->time_range_id,
                                _params->_output_fields[field_num].override_vert_level,
                                _params->_output_fields[field_num].vert_level_type,
                                _params->_output_fields[field_num].vert_level_bottom,
                                _params->_output_fields[field_num].vert_level_top,
                                _params->_output_fields[field_num].data_addend,
				_getFieldType(_params->_output_fields[field_num].field_type),
				_params->_output_fields[field_num].accum_secs) )
    {
      delete converter;
      return false;
    }

    delete converter;

  } /* endfor - field_num */

  // Close the GRIB file

  _gribWriter.closeFile();

  if (_params->debug)
    cerr << "---> Wrote GRIB data to file: " << grib_file_path << endl;
  
  // Write the ldata info file, if requested

  if (_params->write_ldata_info)
  {
    DsLdataInfo ldata(_params->output_dir);
    
    ldata.setDataType("grib");
    ldata.setDataFileExt("grb");
    ldata.setWriter("Mdv2Grib");
    ldata.setRelDataPath(rel_data_path);
    
    if (_params->is_forecast_data)
    {
      ldata.setLatestTime(input_mdv.getMasterHeader().time_gen);
      ldata.setIsFcast();
      ldata.setLeadTime(input_mdv.getField(0)->getFieldHeader().forecast_delta);
    }
    else
    {
      ldata.setLatestTime(input_mdv.getMasterHeader().time_centroid);
    }
    
    if (ldata.write() != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing ldata info file" << endl;
    }
  }
  
  return true;
}


/*********************************************************************
 * _readMdvFile() - Read the MDV file for the given time.
 */

bool Mdv2Grib::_readMdvFile(DsMdvx &input_mdv,
                            const TriggerInfo &trigger_info) const
{
  static const string method_name = "Mdv2Grib::_readMdvFile()";

  // Set up the read request

  if (_dataTrigger->getType() == DsTrigger::TYPE_FILE_TRIGGER)
  {
    input_mdv.setReadPath(trigger_info.getFilePath());
  }
  else
  {
    if (_params->trigger_mode == Params::LATEST_DATA_FCST)
    {
      input_mdv.setReadTime(Mdvx::Mdvx::READ_SPECIFIED_FORECAST,
                          _params->input_url,
                          0, trigger_info.getIssueTime(), trigger_info.getForecastTime()-trigger_info.getIssueTime());
    }
    else
    {
      input_mdv.setReadTime(Mdvx::READ_CLOSEST,
                          _params->input_url,
                          0, trigger_info.getIssueTime());
    }
  }

  // Set the fields to read in the request.  Note that this method
  // must be called after setting the read path or read time above.

  if (!_setReadFields(input_mdv))
    return false;
  
  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);

  if (_params->remap_output)
  {
    switch (_params->remap_info.proj_type)
    {
    case Params::PROJ_LATLON :
      input_mdv.setReadRemapLatlon(_params->remap_info.nx,
                                   _params->remap_info.ny,
                                   _params->remap_info.minx,
                                   _params->remap_info.miny,
                                   _params->remap_info.dx,
                                   _params->remap_info.dy);
      break;

    case Params::PROJ_LAMBERT_CONF :
      if (_params->use_horiz_limits)
      {
        double minLat = _params->horiz_limits.min_lat;
        double maxLat = _params->horiz_limits.max_lat;
        double minLon = _params->horiz_limits.min_lon;
        double maxLon = _params->horiz_limits.max_lon;

        cerr << "INFO: minLat=" << minLat
             << ", maxLat=" << maxLat
             << ", minLon=" << minLon
             << ", maxLon=" << maxLon << endl;

        input_mdv.setReadHorizLimits(minLat, minLon, maxLat, maxLon);
      }
      else
      {
        input_mdv.setReadRemapLc2(_params->remap_info.nx,
                                  _params->remap_info.ny,
                                  _params->remap_info.minx,
                                  _params->remap_info.miny,
                                  _params->remap_info.dx,
                                  _params->remap_info.dy,
                                  _params->remap_info.origin_lat,
                                  _params->remap_info.origin_lon,
                                  _params->remap_info.lat1,
                                  _params->remap_info.lat2);
	if (_params->offset_latitude != 0.0 ||
	    _params->offset_longitude != 0.0)
	  input_mdv.setReadFalseCoords(_params->offset_latitude,
				       _params->offset_longitude);
      }
      break;

    } /* endswitch - _params->remap_info.proj_type */
  }

  if (_params->debug)
    input_mdv.printReadRequest(cerr);
  
  // Read the MDV file

  if (input_mdv.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input MDV file:" << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   Request time: " << DateTime::str(trigger_info.getIssueTime()) << endl;
    cerr << "   msg: " << input_mdv.getErrStr() << endl;

    return false;
  }
  
  if (_params->debug)
    cerr << "---> Input file path: " << input_mdv.getPathInUse() << endl;
  
  return true;
}


/*********************************************************************
 * _setReadField() - Set the fields to read in the MDV request
 */

bool Mdv2Grib::_setReadFields(DsMdvx &input_mdv) const
{
  static const string method_name = "Mdv2Grib::_setReadFields()";

  if (_params->all_fields_required)
  {
    // Put all of the specified fields in the read request.  If any field
    // is missing, the read will fail.

    for (int i = 0; i < _params->output_fields_n; ++i)
    {
      if (_params->use_mdv_field_name)
	input_mdv.addReadField(_params->_output_fields[i].mdv_field_name);
      else
	input_mdv.addReadField(_params->_output_fields[i].mdv_field_num);
    }
  }
  else
  {
    // Read the headers from the file

    if (input_mdv.readAllHeaders() != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading headers from MDV file" << endl;
      input_mdv.printReadRequest(cerr);
      
      return false;
    }
    
    // If the fields are specified by name, look through the fields and
    // include any named field that matches the request.  Otherwise, include
    // all of the numbers that are in the file.

    if (_params->use_mdv_field_name)
    {
      // Loop through the fields headers, adding fields to the request if they
      // are found in the file.

      for (int i = 0; i < _params->output_fields_n; ++i)
      {
	string field_name = _params->_output_fields[i].mdv_field_name;
	  
	for (int j = 0; j < input_mdv.getMasterHeaderFile().n_fields; ++j)
	{
	  Mdvx::field_header_t field_hdr = input_mdv.getFieldHeaderFile(j);
      
	  string mdv_field_name = field_hdr.field_name;
	  string mdv_field_name_long = field_hdr.field_name_long;
	
	  if (field_name == string(field_hdr.field_name) ||
	      field_name == string(field_hdr.field_name_long))
	  {
	    input_mdv.addReadField(_params->_output_fields[i].mdv_field_name);
	    break;
	  }
	} /* endfor - j */
      
      } /* endfor - i */
    
    }
    else
    {
      // Get the number of fields in the file.  Add any field whose index
      // is less than this number.

      int n_fields = input_mdv.getMasterHeaderFile().n_fields;
      
      for (int i = 0; i < _params->output_fields_n; ++i)
      {
	if (_params->_output_fields[i].mdv_field_num < n_fields)
	  input_mdv.addReadField(_params->_output_fields[i].mdv_field_num);
      } /* endfor - i */
      
    }
    
  }
  
  return true;
}
