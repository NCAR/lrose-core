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
 * @file MdvResample.cc
 *
 * @class MdvResample
 *
 * MdvResample is the top level application class.
 *  
 * @date 8/30/2011
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/TaThreadSimple.hh>

#include "MdvResample.hh"
#include "Params.hh"

#include "MaxStatCalc.hh"
#include "MeanStatCalc.hh"
#include "MedianStatCalc.hh"
#include "MinStatCalc.hh"
#include "ModeStatCalc.hh"
#include "PercentileStatCalc.hh"
#include "ResampleInfo.hh"

using namespace std;

// Global variables

MdvResample *MdvResample::_instance =
     (MdvResample *)NULL;

/********************************************************
 * Threading clone method
 */

TaThread *MdvResample::ThreadAlg::clone(const int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(MdvResample::_compute);
  t->setThreadContext(this);
  return dynamic_cast<TaThread *>(t);
}


/*********************************************************************
 * Constructor
 */

MdvResample::MdvResample(int argc, char **argv) :
  _dataTrigger(0),
  _statCalc(0),
  _resampleTemplate(0)
{
  static const string method_name = "MdvResample::MdvResample()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvResample *)NULL);
  
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
  char *params_path = new char[strlen("unknown")+1];
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

  _thread.init(_params->num_threads, _params->thread_debug);
}


/*********************************************************************
 * Destructor
 */

MdvResample::~MdvResample()
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

MdvResample *MdvResample::Inst(int argc, char **argv)
{
  if (_instance == (MdvResample *)NULL)
    new MdvResample(argc, argv);
  
  return(_instance);
}

MdvResample *MdvResample::Inst()
{
  assert(_instance != (MdvResample *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool MdvResample::init()
{
  static const string method_name = "MdvResample::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the output projection

  if (!_initOutputProj())
    return false;
  
  // Initialize the statistics calculator

  if (!_initStatCalc())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void MdvResample::run()
{
  static const string method_name = "MdvResample::run()";
  
  TriggerInfo trigger_info;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger info" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: "
	   << trigger_info.getIssueTime() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createBlankField()
 */

MdvxField *MdvResample::_createBlankField(const Mdvx::field_header_t input_field_hdr,
					  const Mdvx::vlevel_header_t input_vlevel_hdr) const
{
  static const string method_name = "MdvResample::_createBlankField()";
  
  // Set the field header values.  We need to restore the nz value after
  // syncing with the output projection since the output projection assumes
  // one plane.  We do this because we could have some 2D and some 3D fields
  // in the input file and we want to preserve this in the output.

  Mdvx::field_header_t field_hdr = input_field_hdr;
  _outputProj.syncToFieldHdr(field_hdr);
  field_hdr.nz = input_field_hdr.nz;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  
  // Create and return the new field object.

  return new MdvxField(field_hdr, input_vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _initOutputFile()
 */

bool MdvResample::_initOutputFile(DsMdvx &output_file,
				  const Mdvx::master_header_t input_master_hdr) const
{
  static const string method_name = "MdvResample::_initOutputFile()";
  
  Mdvx::master_header_t master_hdr = input_master_hdr;
  master_hdr.n_fields = 0;
  master_hdr.max_nx = 0;
  master_hdr.max_ny = 0;
  master_hdr.max_nz = 0;
  master_hdr.n_chunks = 0;
  STRcopy(master_hdr.data_set_info, "Resampled data from MdvResample",
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "MdvResample", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->input_url, MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
  
  return true;
}


/*********************************************************************
 * _initOutputProj()
 */

bool MdvResample::_initOutputProj()
{
  static const string method_name = "MdvResample::_initOutputProj()";
  
  switch (_params->resample_proj_info.proj_type)
  {
  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params->resample_proj_info.nx,
			   _params->resample_proj_info.ny,
			   1,
			   _params->resample_proj_info.dx,
			   _params->resample_proj_info.dy,
			   1.0,
			   _params->resample_proj_info.minx,
			   _params->resample_proj_info.miny,
			   0.0);
    break;
  
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params->resample_proj_info.origin_lat,
			 _params->resample_proj_info.origin_lon,
			 _params->resample_proj_info.rotation,
			 _params->resample_proj_info.nx,
			 _params->resample_proj_info.ny,
			 1,
			 _params->resample_proj_info.dx,
			 _params->resample_proj_info.dy,
			 1.0,
			 _params->resample_proj_info.minx,
			 _params->resample_proj_info.miny,
			 0.0);
    break;
  
  case Params::PROJ_LAMBERT :
    _outputProj.initLc2(_params->resample_proj_info.origin_lat,
			_params->resample_proj_info.origin_lon,
			_params->lambert_lat1,
			_params->lambert_lat2,
			 _params->resample_proj_info.nx,
			 _params->resample_proj_info.ny,
			 1,
			 _params->resample_proj_info.dx,
			 _params->resample_proj_info.dy,
			 1.0,
			 _params->resample_proj_info.minx,
			 _params->resample_proj_info.miny,
			 0.0);
    break;
  
  } /* endswitch - _params->resample_proj_info.proj_type */

  return true;
}


/*********************************************************************
 * _initStatCalc()
 */

bool MdvResample::_initStatCalc()
{
  static const string method_name = "MdvResample::_initStatCalc()";
  
  switch (_params->stat_type)
  {
  case Params::STAT_MAX :
    _statCalc = new MaxStatCalc();
    break;
    
  case Params::STAT_MIN :
    _statCalc = new MinStatCalc();
    break;
    
  case Params::STAT_MEAN :
    _statCalc = new MeanStatCalc();
    break;
    
  case Params::STAT_MEDIAN :
    _statCalc = new MedianStatCalc();
    break;
    
  case Params::STAT_MODE_MAX :
    _statCalc = new ModeStatCalc(new MaxStatCalc());
    break;
    
  case Params::STAT_MODE_MIN :
    _statCalc = new ModeStatCalc(new MinStatCalc());
    break;
    
  case Params::STAT_MODE_MEAN :
    _statCalc = new ModeStatCalc(new MeanStatCalc());
    break;
    
  case Params::STAT_MODE_MEDIAN :
    _statCalc = new ModeStatCalc(new MedianStatCalc());
    break;
    
  case Params::STAT_PERCENTILE :
    _statCalc = new PercentileStatCalc(_params->stat_percentile_value);
    break;
    
  } /* endswitch - _params->stat_type */
  
  if (_statCalc == 0)
    return false;
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool MdvResample::_initTrigger()
{
  static const string method_name = "MdvResample::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
    {
      cerr << "Initializing LATEST_DATA trigger: " << endl;
      cerr << "    URL = " << _params->input_url << endl;
    }
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger: " << endl;
      cerr << "    URL = " << _params->input_url << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify start and end times on command line" << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::FCST_TIME_LIST :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify start and end times on command line" << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing FCST_TIME_LIST trigger: " << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
    }
    
    DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FCST_TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool MdvResample::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "MdvResample::_processData()";
  
  if (_params->debug)
  {
    if (trigger_info.getForecastTime() == DateTime::NEVER)
      cerr << endl << "*** Processing data for time: "
	   << DateTime::str(trigger_info.getIssueTime()) << endl;
    else
      cerr << endl << "*** Processing data for gen time: "
	   << DateTime::str(trigger_info.getIssueTime())
	   << ", valid time: " << DateTime::str(trigger_info.getForecastTime())
	   << ", lt: " 
	   << trigger_info.getForecastTime() - trigger_info.getIssueTime() 
	   << endl;
  }
  
  // Read in the input file

  DsMdvx input_file;
  
  if (!_readInputFile(input_file,
		      trigger_info.getIssueTime(),
		      trigger_info.getForecastTime()))
    return false;
  
  // Initialize the output file

  DsMdvx output_file;
  
  if (!_initOutputFile(output_file, input_file.getMasterHeader()))
    return false;
  
  // Resample the fields and add them to the output file

  for (size_t i = 0; i < input_file.getNFields(); ++i)
  {
    MdvxField *output_field;
    
    if ((output_field = _resampleField(*input_file.getField(i))) == 0)
      return false;
    
    // leave as 32 bit float, but compress
    output_field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
    output_file.addField(output_field);

  } /* endfor - i */
  
  // Write the output file

  return _writeFile(output_file);
  
  return true;
}


/*********************************************************************
 * _readInputFile()
 */

bool MdvResample::_readInputFile(DsMdvx &mdvx,
				const DateTime &data_time,
				const DateTime &fcst_time) const
{
  static const string method_name = "MdvResample::_readInputFile()";
  
  // Set up the read request

  mdvx.clearRead();
  
  if (fcst_time == DateTime::NEVER)
    mdvx.setReadTime(Mdvx::READ_CLOSEST,
		     _params->input_url,
		     _params->max_valid_secs,
		     data_time.utime());
  else
    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
		     _params->input_url,
		     _params->max_valid_secs,
		     data_time.utime(),
		     fcst_time.utime() - data_time.utime());

  if (_params->specify_fields)
  {
    for (int i = 0; i < _params->input_fields_n; ++i)
    {
      if (_params->use_field_names)
	mdvx.addReadField(_params->_input_fields[i].field_name);
      else
	mdvx.addReadField(_params->_input_fields[i].field_num);
    }
  }
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->verbose)
    mdvx.printReadRequest(cerr);
  
  // Read the file

  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file:" << endl;
    cerr << "   URL = " << _params->input_url << endl;
    cerr << "   time = " << data_time << endl;
    cerr << "   fcst time = " << fcst_time << endl;
    cerr << "   fcst secs = " << (fcst_time.utime() - data_time.utime()) << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  // Make sure that all of the fields use either a lat/lon or flat projection.

  for (size_t i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxPjg proj(mdvx.getField(i)->getFieldHeader());
    
    if (proj.getProjType() != Mdvx::PROJ_LATLON &&
	proj.getProjType() != Mdvx::PROJ_FLAT &&
	proj.getProjType() != Mdvx::PROJ_LAMBERT_CONF)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Field <" << mdvx.getField(i)->getFieldName()
	   << "> uses " << Mdvx::projType2Str(proj.getProjType())
	   << " projection" << endl;
      cerr << "Resampling can only be done on data using lat/lon or flat projections" << endl;
    
      return false;
    }
  }
  
  return true;
}


/*********************************************************************
 * _resampleField()
 */

MdvxField *MdvResample::_resampleField(const MdvxField &input_field)
{
  static const string method_name = "MdvResample::_resampleField()";
  
  // Set the input projection, which will also set up the template used
  // to run through the data.

  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  _setInputProj(MdvxPjg(input_field_hdr));
  
  // Create the new field, filled with missing data values

  MdvxField *field;
  
  if ((field = _createBlankField(input_field.getFieldHeader(),
				 input_field.getVlevelHeader())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank resampled field" << endl;
    
    return 0;
  }
  
  // Loop through the output grid, resampling the data in the input grid

  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  
  fl32 *data = (fl32 *)field->getVol();
  fl32 *input_data = (fl32 *)input_field.getVol();
  
  size_t plane_size = field_hdr.nx * field_hdr.ny;
  
  for (int z = 0; z < field_hdr.nz; ++z)
  {
    fl32 *plane_ptr = data + (z * plane_size);
    fl32 *input_plane_ptr =
      input_data + (z * input_field_hdr.nx * input_field_hdr.ny);
    
    for (int y = 0; y < field_hdr.ny; ++y)
    {
      PMU_auto_register("Resample y");
      ResampleInfo *info = new ResampleInfo(z, y, plane_ptr, input_plane_ptr,
					    &field_hdr, &input_field_hdr, this);
      _thread.thread(z*input_field_hdr.nx*input_field_hdr.ny +
		     y*input_field_hdr.nx, info);
		       
    } /* endfor - y */
    
  } /* endfor - z */
  
  _thread.waitForThreads();


  return field;
}


/*********************************************************************
 * _updateMasterHeader()
 */

void MdvResample::_updateMasterHeader(DsMdvx &mdvx) const
{
  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();

  STRcopy(master_hdr.data_set_info, "MdvResample", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "MdvResample", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->input_url,
	  MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}


/*********************************************************************
 * _writeFile()
 */

bool MdvResample::_writeFile(DsMdvx &mdvx) const
{
  static const string method_name = "MdvResample::_writeFile()";
  
  // Update the master header to reflect the source of this file

  _updateMasterHeader(mdvx);
  
  // Write the file

  mdvx.setIfForecastWriteAsForecast();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}

/*********************************************************************
 * _compute()
 */

void MdvResample::_compute(void *i)
{
  ResampleInfo *info = (ResampleInfo *)(i);

  info->_alg->_resample(*info);
  delete info;
}

/*********************************************************************
 * _resample()
 */

void MdvResample::_resample(const ResampleInfo &info) const
{
  if (_params->verbose) 
  {
    cerr << "Resampling [y,z]=[" << info._y << "," << info._z << "]" << endl;
  }

  // make a copy of the resample template, as it gets modified here
  EllipticalTemplate *e = (EllipticalTemplate *)(_resampleTemplate);
  EllipticalTemplate resampler(*e);

  for (int x = 0; x < info._field_hdr->nx; ++x)
  {
    _resampleX(&resampler, info, x);
  }
}

/*********************************************************************
 * _resampleX()
 */

void MdvResample::_resampleX(GridTemplate *resampler, const ResampleInfo &info,
			     int x) const
{
  // Find the location of the center of the output grid square in the
  // input grid.  This will be used as the center of the resample template
  // when gathering data values for the statistics.  If the center of the
  // output grid falls outside of the input grid, don't process this
  // point.
	
  double lat, lon;
  int input_x, input_y;
	
  _outputProj.xyIndex2latlon(x, info._y, lat, lon);
  if (_inputProj.latlon2xyIndex(lat, lon, input_x, input_y) != 0)
  {
    return;
  }

  // Gather the input values to use for the resampling

  vector< double > input_values;
	
  for (GridPoint *point = resampler->getFirstInGrid(input_x, input_y,
						    info._input_field_hdr->nx,
						    info._input_field_hdr->ny);
       point != 0;
       point = resampler->getNextInGrid())
  {
    double value =
      info._input_plane_ptr[point->getIndex(info._input_field_hdr->nx,
					    info._input_field_hdr->ny)];
	  
    if (value == info._input_field_hdr->bad_data_value ||
	value == info._input_field_hdr->missing_data_value)
    {
      continue;
    }
	  
    input_values.push_back(value);
    
  } /* endfor - point */
	
  // Calculate the resample statistic and set the output grid value

  double resample_value = _statCalc->calculate(input_values);
	
  if (resample_value == StatCalc::BAD_DATA_VALUE)
    info._plane_ptr[(info._y * info._field_hdr->nx) + x] =
      info._field_hdr->missing_data_value;
  else
    info._plane_ptr[(info._y * info._field_hdr->nx) + x] = resample_value;
}
