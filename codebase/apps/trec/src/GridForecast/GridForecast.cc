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
// GridForecast.cc
//
// GridForecast object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////
//
// GridForecast produces a forecast image based on (a) motion data
// provided in the form of (u,v) components on a grid and
// (b) image data on a grid.
//
///////////////////////////////////////////////////////////////

#include <assert.h>

#include <advect/GridAdvect.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>

#include <dsdata/DsTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsMultipleTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsSpecificFcstLdataTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>

#include "GridForecast.hh"

// Constructor

GridForecast::GridForecast(int argc, char **argv)
{
  const string routine_name = "GridForecast::constructor";
  
  OK = TRUE;
  Done = FALSE;

  // set programe name

  _progName = STRdup("GridForecast");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK)
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }
  if (_args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << routine_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    OK = FALSE;
    
    return;
  }

  // input file object

  switch (_params->mode)
  {
    case Params::ARCHIVE :
    {
	if (_args->nFiles > 0)
	{
	    vector< string > file_list;
	    
	    for (int i = 0; i < _args->nFiles; ++i)
		file_list.push_back(_args->filePaths[i]);
	    
	    DsFileListTrigger *data_trigger = new DsFileListTrigger();
	    data_trigger->init(file_list);
	    
	    _dataTrigger = data_trigger;
	}
	else if (_args->startTime.utime() != 0 && _args->endTime.utime() != 0)
	{
	    DsTimeListTrigger *data_trigger = new DsTimeListTrigger();
	    cerr << _args->startTime.utime() << endl;
	    cerr << _args->endTime.utime() << endl;
	    
	    if (data_trigger->init(_params->motion_grid_url,
			      _args->startTime.utime(),
			      _args->endTime.utime()) != 0)
	    {
		cerr << "ERROR: " << _progName << endl;
		cerr << "Error initializing ARCHIVE for url: " <<
		    _params->motion_grid_url << endl;
		cerr << "    Start time: " << _args->startTime <<
		    endl;
		cerr << "    End time: " << _args->endTime << endl;
	    }
	    
	    
	    if (_params->debug)
	    {
		cerr << "DsTimeListTrigger: url: " << _params->motion_grid_url << endl;
		cerr << "                   startTime: " << _args->startTime << endl;
		cerr << "                   endTime: " << _args->endTime << endl;
	    }
	    _dataTrigger = data_trigger;
	}
	else
	{
	    cerr << "ERROR: " << _progName << endl;
	    cerr << "In ARCHIVE mode you must either specify a file list using" <<
		endl;
	    cerr << "-f or the start and end times using -start and -end" << endl;
	    OK = FALSE;
	}

	break;
    }

    case Params::REALTIME :
    {
	DsLdataTrigger *data_trigger = new DsLdataTrigger();

	data_trigger->init(_params->motion_grid_url,
			   _params->max_realtime_valid_age,
			   PMU_auto_register);

	_dataTrigger = data_trigger;

	break;
    
    }

    case Params::SPEC_FCAST_REALTIME :
    {
      DsSpecificFcstLdataTrigger *trigger =
	new DsSpecificFcstLdataTrigger();
      vector< int > fcast_lead_times;
      fcast_lead_times.push_back(_params->fcast_lead_time.lead_time_secs);
    
      if (trigger->init(_params->image_grid_url,
			fcast_lead_times,
			_params->fcast_lead_time.use_gen_time,
			600, PMU_auto_register) != 0)
      {
	cerr << trigger->getErrStr() << endl;
	OK = FALSE;
	return;
      }
    
      _dataTrigger = trigger;
    
      break;
    }

    case Params::LATEST_DATA :
    {
	if (_params->debug)
	    cerr << "Initializing LATEST_DATA trigger using url: " <<
		_params->latest_data_trigger << endl;
    
	DsLdataTrigger *trigger = new DsLdataTrigger();
	if (trigger->init(_params->latest_data_trigger,
			  -1,
			  PMU_auto_register) != 0)
	{
	    cerr << "ERROR: " << _progName << endl;
	    cerr << "Error initializing LATEST_DATA trigger using url: " <<
		_params->latest_data_trigger << endl;
      
	    OK = FALSE;
	    return;
	}

	_dataTrigger = trigger;
    
	break;
    }
  
    case Params::TIME_LIST :
    {

	if (_args->startTime.utime() > 0 && _args->endTime.utime() > 0)
	{
	    if (_params->debug)
	    {
		cerr << "Initializing TIME_LIST trigger: " << endl;
		cerr << "   url: " << _params->time_list_trigger.url << endl;
		cerr << "   start time: " << _args->startTime << endl;
		cerr << "   end time: " << _args->endTime << endl;
	    }
    
	    DsTimeListTrigger *trigger = new DsTimeListTrigger();
	    if (trigger->init(_params->time_list_trigger.url,
			      _args->startTime.utime(), 
			      _args->endTime.utime()) != 0)
	    {
		cerr << "ERROR: " << _progName << endl;
		cerr << "Error initializing TIME_LIST trigger for url: " <<
		    _params->time_list_trigger.url << endl;
		cerr << "    Start time: " << _args->startTime <<
		    endl;
		cerr << "    End time: " << _args->endTime << endl;
      
		OK = FALSE;
		return;
	    }
	    _dataTrigger = trigger;
    
	    break;
	}
	else 
	{
	    time_t start_time =
	    DateTime::parseDateTime(_params->time_list_trigger.start_time);
	    time_t end_time
		= DateTime::parseDateTime(_params->time_list_trigger.end_time);
	    if (start_time == DateTime::NEVER)
	    {
		cerr << "ERROR: " << _progName << endl;
		cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
		    _params->time_list_trigger.start_time << endl;
      
		OK = FALSE;
		return;
	    }
    
	    if (end_time == DateTime::NEVER)
	    {
		cerr << "ERROR: " << _progName << endl;
		cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
		    _params->time_list_trigger.end_time << endl;
      
		OK = FALSE;
		return;
	    }
    
	    if (_params->debug)
	    {
		cerr << "Initializing TIME_LIST trigger: " << endl;
		cerr << "   url: " << _params->time_list_trigger.url << endl;
		cerr << "   start time: " << _params->time_list_trigger.start_time << endl;
		cerr << "   end time: " << _params->time_list_trigger.end_time << endl;
	    }	
    
	    DsTimeListTrigger *trigger = new DsTimeListTrigger();
	    if (trigger->init(_params->time_list_trigger.url,
			      start_time, end_time) != 0)
	    {
		cerr << "ERROR: " << _progName << endl;
		cerr << "Error initializing TIME_LIST trigger for url: " <<
		    _params->time_list_trigger.url << endl;
		cerr << "    Start time: " << _params->time_list_trigger.start_time <<
		    endl;
		cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
		OK = FALSE;
		return;
	    }

	    _dataTrigger = trigger;
    
	    break;
	}
    }
  
    case Params::MULTIPLE_URL :
    {
	if (_params->debug)
	{
	    cerr << "Initializing MULTIPLE_URL trigger using urls: " << endl;
	    for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
		cerr << "    " << _params->_multiple_url_trigger[i] << endl;
	}
    
	DsMultipleTrigger *trigger = new DsMultipleTrigger();

	if (!trigger->initRealtime(-1,
				   PMU_auto_register))
	{
	    cerr << "ERROR: " << _progName << endl;
	    cerr << "Error initializing MULTIPLE_URL trigger using urls: " << endl;
	    for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
		cerr << "    " << _params->_multiple_url_trigger[i] << endl;
      
	    OK = FALSE;
	    return;
	}

	for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
	    trigger->add(_params->_multiple_url_trigger[i]);
	trigger->set_debug(_params->debug);
    
	_dataTrigger = trigger;
    
	break;
    }
  }
  
  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);


  return;

}

// destructor

GridForecast::~GridForecast()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_params);
  delete(_args);
  delete _dataTrigger;
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

bool GridForecast::Run ()
{
  static const string routine_name = "GridForecast::Run()";

  PMU_auto_register("GridForecast::Run");

  _input.reset();
  
  // Initialize the advection objects

  VectorsAdvector vv(_params->vector_spacing,
		     _params->smoothing_radius,
		     _params->debug >= Params::DEBUG_VERBOSE);

  GridAdvect forecast(_params->image_val_min,
		      _params->image_val_max,
		      _params->debug >= Params::DEBUG_NORM);
  
  // Process the data

  DateTime trigger_time;

  while (!_dataTrigger->endOfData()) {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)  {
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }

    PMU_auto_register("Waiting for data...");

    // Read in the motion file

    if (!_readVectors(trigger_time.utime()))
    {
      if (_params->mode == Params::ARCHIVE)
      {
	break;
      }
      else
      {
	cerr << "ERROR: " << routine_name << endl;
	cerr << "Error reading next vectors file" << endl;

	continue;
      }
    }
    
    // read in image

    if (!_readImage(trigger_time.utime()))
    {
      cerr << "ERROR: " << routine_name << endl;
      cerr << "Error reading image file for time: "
	   << DateTime::str(_motionFile.getMasterHeader().time_centroid) << endl;

      continue;
    }

    MdvxPjg image_projection(_imageFile);
    
    if (_params->debug >= Params::DEBUG_NORM)
    {
      cerr << "Motion file time: " <<
	DateTime(_motionFile.getMasterHeader().time_centroid) << endl;
      cerr << "Image file time: " <<
	DateTime(_imageFile.getMasterHeader().time_centroid) << endl;
    }

    // load up the vectors from the motion image

    MdvxPjg motion_proj(_motionUField->getFieldHeader());
    Mdvx::field_header_t u_field_hdr = _motionUField->getFieldHeader();
    Mdvx::field_header_t v_field_hdr = _motionVField->getFieldHeader();
    
    int u_plane_num = _motionUField->computePlaneNum(_params->u_plane_ht);
    int v_plane_num = _motionVField->computePlaneNum(_params->v_plane_ht);
    
    if (_params->debug >= Params::DEBUG_NORM)
    {
      cerr << "u_plane_num = " << u_plane_num << endl;
      cerr << "v_plane_num = " << v_plane_num << endl;
    }

    vv.loadVectors(motion_proj,
		   (fl32 *)_motionUField->getPlane(u_plane_num),
		   u_field_hdr.missing_data_value,
		   (fl32 *)_motionVField->getPlane(v_plane_num),
		   v_field_hdr.missing_data_value);
      
    for (int i = 0; i < _params->forecast_output_n; i++)
    {
      vv.precompute(image_projection, _params->_forecast_output[i].lead_time);
      
      if (forecast.compute(vv,
			   image_projection,
			   (fl32 *)_imageField->getVol(),
			   _imageField->getFieldHeader().missing_data_value))
      {
	_writeForecast(_params->image_grid_url,
		       _params->_forecast_output[i].url,
		       _params->_forecast_output[i].lead_time,
		       _imageFile.getMasterHeader(),
		       forecast.getForecastData());
	if (_params->write_motion_files)
	{
	  _writeMotion(vv,
		       _params->image_grid_url,
		       _params->output_motion_url,
		       _params->_forecast_output[i].lead_time,
		       _imageFile.getMasterHeader(),
		       *_imageField);
	}
      }
      else
      {
	cerr << "WARNING: GridAdvect::compute failed for " <<
	  _motionFile.getPathInUse() << endl;
      }
      
    } // i

  } // while

  return true;

}


////////////////
// _readImage()
//
// Read in image from MDV file
//
// Returns true on success, false on failure.
//

bool GridForecast::_readImage(const time_t trigger_time)
{
  const string method_name = "GridForecast:GridForecast::_readImage()";
  
  // read in image file
  
  _imageFile.clearRead();
  
  _imageFile.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->image_grid_url,
			 _params->image_time_margin,
			 trigger_time);
  
  _imageFile.setReadFieldFileHeaders();

  if(_params->use_image_field_name)
    _imageFile.addReadField(_params->image_field_name);
  else
    _imageFile.addReadField(_params->image_field_num);
  
  if (_params->image_plane_ht < 0)
    _imageFile.setReadComposite();
  else
    _imageFile.setReadVlevelLimits(_params->image_plane_ht,
				   _params->image_plane_ht);
  
  _imageFile.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _imageFile.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _imageFile.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_imageFile.readVolume() != 0)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << "Error reading image file for time " <<
      DateTime(trigger_time) << endl;
    return false;
  }
  
  // Save a pointer to the field and field header

  _imageField = _imageFile.getField(0);
  return true;

}


////////////////////////////
// _readVectors()
//
// Read the vectors to be used for advecting
//
// Returns true on success, false on failure

bool GridForecast::_readVectors(const time_t trigger_time)
{
  const string method_name = "GridForecast:GridForecast::_readVectors()";
  
  // Read in the motion file

  _motionFile.clearRead();

  _motionFile.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->motion_grid_url,
			  _params->vector_time_margin,
			  trigger_time);
  
  _motionFile.addReadField(_params->u_field_num);
  _motionFile.addReadField(_params->v_field_num);
    
  _motionFile.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _motionFile.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _motionFile.setReadScalingType(Mdvx::SCALING_NONE);
    
  if (_motionFile.readVolume() != 0)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << "Error reading motion file for time " <<
      DateTime(trigger_time) << endl;
    return false;
  }
    
  // Save pointers to the motion fields

  _motionUField = _motionFile.getFieldByNum(0);
  _motionVField = _motionFile.getFieldByNum(1);
  
  if (_motionUField == 0 || _motionVField == 0)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << "Could not get motion field from motion file" << endl;
    return false;
  }
  
  // Make sure the projections of the two fields match

  Mdvx::field_header_t u_field_hdr = _motionUField->getFieldHeader();
  Mdvx::field_header_t v_field_hdr = _motionVField->getFieldHeader();

  if (u_field_hdr.proj_type != v_field_hdr.proj_type ||
      u_field_hdr.nx != v_field_hdr.nx ||
      u_field_hdr.ny != v_field_hdr.ny ||
      u_field_hdr.nz != v_field_hdr.nz ||
      u_field_hdr.grid_dx != v_field_hdr.grid_dx ||
      u_field_hdr.grid_dy != v_field_hdr.grid_dy ||
      u_field_hdr.grid_dz != v_field_hdr.grid_dz ||
      u_field_hdr.grid_minx != v_field_hdr.grid_minx ||
      u_field_hdr.grid_miny != v_field_hdr.grid_miny ||
      u_field_hdr.grid_minz != v_field_hdr.grid_minz)
  {
    cerr <<  "ERROR - " << method_name << endl;
    cerr << "Projections for U and V field must match" << endl;
    return false;
  }
  
  // Set the plane pointers for the fields

  _motionUField->setPlanePtrs();
  _motionVField->setPlanePtrs();
  
  return true;
}


////////////////////////////
// _writeForecast()
//
// Write the forecast grid to file.
//
// Returns true on success, false on failure

bool GridForecast::_writeForecast(const string &image_file_url,
				  const string &output_url,
				  int lead_time_secs,
				  const Mdvx::master_header_t &image_master_hdr,
				  const fl32 *forecast_data)
{
  // Calculate the forecast time in hours for text fields

  Mdvx::field_header_t field_hdr = _imageField->getFieldHeader();
  const Mdvx::field_header_t *orig_field_hdr =
    _imageField->getFieldHeaderFile();

  assert(orig_field_hdr != 0);

  lead_time_secs = lead_time_secs + field_hdr.forecast_delta;
  double lead_time_hrs = (double)lead_time_secs / 3600.0;
  
  // create the output file

  DsMdvx output_file;
  
  // copy over the master header

  Mdvx::master_header_t master_hdr = image_master_hdr;
  
  // modify the master header - the forecast data 
  // will be copied into the first plane of the first field

  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr.n_fields = 0;
  master_hdr.max_nz = 0;
  master_hdr.n_chunks = 0;
  master_hdr.field_grids_differ = FALSE;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;

  master_hdr.time_gen =   master_hdr.time_centroid;
  master_hdr.time_begin = master_hdr.time_centroid;
  master_hdr.time_end =   master_hdr.time_centroid;
  master_hdr.time_expire = master_hdr.time_centroid;
  
  output_file.setMasterHeader(master_hdr);
  
  // append note to mdv_data_set info

  char text[1024];
  sprintf(text,
	  "Grid forecast:\n"
	  "  forecast lead time %d secs\n"
	  "  vector_spacing %g km\n",
	  lead_time_secs,
	  _params->vector_spacing);

  output_file.setDataSetInfo(text);
  
  // set the field name and source name

  sprintf(text, "%s: %g hour forecast",
	  master_hdr.data_set_name,
	  lead_time_hrs);

  output_file.setDataSetName(text);
  output_file.setDataSetSource(image_file_url.c_str());
  output_file.setForecastLeadSecs(lead_time_secs);
  
  // Create the forecast field

  field_hdr.forecast_delta = lead_time_secs;
  field_hdr.forecast_time = image_master_hdr.time_centroid + lead_time_secs;
  
  sprintf(text, "%s: %g hr fcast", field_hdr.field_name_long, lead_time_hrs);
  STRncopy(field_hdr.field_name_long, text, MDV_LONG_FIELD_LEN);

  MdvxField *forecast_field = new MdvxField(field_hdr,
					    _imageField->getVlevelHeader(),
					    (void *)forecast_data);
  
  // Compress the output field
  
  forecast_field->compress(Mdvx::COMPRESSION_GZIP);

  // add to vol

  output_file.addField(forecast_field);
  
  // write to directory

  output_file.clearWrite();
  output_file.setWriteLdataInfo();

  if(_params->write_to_forecast_directory)
  {
    output_file.setWriteAsForecast();
  }
  
  if (output_file.writeToDir(output_url) == 0)
    return true;
  else
    return false;
}


////////////////////////////
// _writeMotion()
//
// Write the motion grid data to file for debugging.
//
// Returns true on success, false on failure

bool GridForecast::_writeMotion(const VectorsAdvector &vectors,
				const string &image_file_url,
				const string &output_motion_url,
				const int lead_time_secs,
				const Mdvx::master_header_t &image_master_hdr,
				const MdvxField &image_field)
{
  // Calculate the forecast time in hours for text fields

  double lead_time_hrs = (double)lead_time_secs / 3600.0;
  
  // create a new file object

  DsMdvx motion_file;
  
  // Create the new master header based on the image file header

  Mdvx::master_header_t master_hdr = image_master_hdr;
  
  // modify the master header - the forecast data 
  // will be copied into the first plane of the first field

  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr.n_fields = 0;
  master_hdr.max_nz = 0;
  master_hdr.n_chunks = 0;
  master_hdr.field_grids_differ = FALSE;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;

  // append note to mdv_data_set info

  char text[1024];
  sprintf(text,
	  "Motion data:\n"
	  "  forecast lead time %d secs\n"
	  "  vector_spacing %g km\n",
	  (int)lead_time_secs,
	  _params->vector_spacing);
  STRcopy(master_hdr.data_set_info, text, MDV_INFO_LEN);

  sprintf(text, "%s: %g hour forecast",
	  master_hdr.data_set_name, lead_time_hrs);
  STRcopy(master_hdr.data_set_name, text, MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, image_file_url.c_str(), MDV_NAME_LEN);

  motion_file.setMasterHeader(master_hdr);
  
  // allocate and load up u and v arrays

  Mdvx::field_header_t u_field_hdr = image_field.getFieldHeader();
  
  u_field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  u_field_hdr.data_element_nbytes = 4;
  u_field_hdr.volume_size = u_field_hdr.nx * u_field_hdr.ny *
    u_field_hdr.data_element_nbytes;
  u_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  u_field_hdr.scaling_type = Mdvx::SCALING_NONE;
  u_field_hdr.scale = 0.0;
  u_field_hdr.bias = 0.0;
  u_field_hdr.min_value = 0.0;
  u_field_hdr.max_value = 0.0;
  sprintf(text, "U for %g hr fcast", lead_time_hrs);
  STRcopy(u_field_hdr.field_name_long, text, MDV_LONG_FIELD_LEN);
  STRcopy(u_field_hdr.field_name, "U", MDV_SHORT_FIELD_LEN);
  STRcopy(u_field_hdr.units, "m/s", MDV_UNITS_LEN);
  STRcopy(u_field_hdr.transform, "none", MDV_TRANSFORM_LEN);
  
  MdvxField *u_field = new MdvxField(u_field_hdr,
				     image_field.getVlevelHeader(),
				     (void *)vectors.getUData());
  
  Mdvx::field_header_t v_field_hdr = u_field_hdr;
  
  sprintf(text, "V for %g hr fcast", lead_time_hrs);
  STRcopy(v_field_hdr.field_name_long, text, MDV_LONG_FIELD_LEN);
  STRcopy(v_field_hdr.field_name, "V", MDV_SHORT_FIELD_LEN);
  
  MdvxField *v_field = new MdvxField(v_field_hdr,
				     image_field.getVlevelHeader(),
				     (void *)vectors.getVData());
  
  // Compress the fields

  u_field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_GZIP,
		       Mdvx::SCALING_DYNAMIC);
  
  motion_file.addField(u_field);
  
  v_field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_GZIP,
		       Mdvx::SCALING_DYNAMIC);
  
  motion_file.addField(v_field);
  
  // write to directory

  motion_file.clearWrite();
  motion_file.setWriteLdataInfo();
  
  if (motion_file.writeToDir(output_motion_url.c_str()) == 0)
    return true;
  else
    return false;
}
