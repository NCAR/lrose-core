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
//   $Date: 2016/03/04 02:22:10 $
//   $Id: MdvAccumulate.cc,v 1.8 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvAccumulate: MdvAccumulate program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvAccumulate.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvAccumulate *MdvAccumulate::_instance =
     (MdvAccumulate *)NULL;


const string MdvAccumulate::DATA_TIME_FIELD_NAME = "AccumDataTime";
const ui32 MdvAccumulate::DATA_TIME_MISSING_VALUE = 0;


/*********************************************************************
 * Constructor
 */

MdvAccumulate::MdvAccumulate(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvAccumulate::MdvAccumulate()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvAccumulate *)NULL);
  
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

MdvAccumulate::~MdvAccumulate()
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
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvAccumulate *MdvAccumulate::Inst(int argc, char **argv)
{
  if (_instance == (MdvAccumulate *)NULL)
    new MdvAccumulate(argc, argv);
  
  return(_instance);
}

MdvAccumulate *MdvAccumulate::Inst()
{
  assert(_instance != (MdvAccumulate *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvAccumulate::init()
{
  static const string method_name = "MdvAccumulate::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the input projection

  if (!_initInputProj())
    return false;
  
  // Initialize the background fields

  if (!_initBackgroundFields())
    return false;
  
  // Initialize the quality control objects

  if (!_initQualityControl())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvAccumulate::run()
{
  static const string method_name = "MdvAccumulate::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createAccumTimeField() - Create the initial accumulation data time
 *                           field.  This field will have the given time
 *                           where the given field has a data value, and
 *                           will have a missing data value where the
 *                           given field data is missing.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *MdvAccumulate::_createAccumTimeField(const MdvxField &input_field,
						const DateTime &data_time,
						const DateTime &bkgnd_time) const
{
  static const string method_name = "MdvAccumulate::_createAccumTimeField()";
  
  // Create the field header.  It will be a copy of the input field header
  // with a few fields changed.  We know that the input field uses
  // ENCODE_FLOAT32, which is what we also want for the time field since
  // MDV doesn't have an ENCODE_INT32 type.

  Mdvx::field_header_t time_field_hdr;
  memset(&time_field_hdr, 0, sizeof(time_field_hdr));
  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  
  time_field_hdr = input_field_hdr;
  
  time_field_hdr.bad_data_value = DATA_TIME_MISSING_VALUE;
  time_field_hdr.missing_data_value = DATA_TIME_MISSING_VALUE;
  time_field_hdr.min_value = 0;
  time_field_hdr.max_value = 0;
  time_field_hdr.min_value_orig_vol = 0;
  time_field_hdr.max_value_orig_vol = 0;
  STRcopy(time_field_hdr.field_name_long, DATA_TIME_FIELD_NAME.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(time_field_hdr.field_name, DATA_TIME_FIELD_NAME.c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(time_field_hdr.units, "sec", MDV_UNITS_LEN);
  time_field_hdr.transform[0] = '\0';
  
  // Create the data time field.  It will have the same vlevel header as
  // the input field.  Initialize it with missing data values so we can
  // overwrite the data values where necessary below.

  MdvxField *time_field = new MdvxField(time_field_hdr,
					input_field.getVlevelHeader(),
					(void *)0, true);
  
  if (time_field == 0)
    return 0;
  
  // Fill in the time values

  int volume_size = time_field_hdr.nx * time_field_hdr.ny * time_field_hdr.nz;
  
  fl32 *input_data = (fl32 *)input_field.getVol();
  ui32 *time_data = (ui32 *)time_field->getVol();
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (input_data[i] == input_field_hdr.missing_data_value ||
	input_data[i] == input_field_hdr.bad_data_value)
    {
      if (bkgnd_time == DateTime::NEVER)
	time_data[i] = DATA_TIME_MISSING_VALUE;
      else
	time_data[i] = bkgnd_time.utime();
    }
    else
    {
      time_data[i] = data_time.utime();
    }
  } /* endfor - i */
  
  return time_field;
}


/*********************************************************************
 * _createAccumulation() - Create a new accumulation file based on the
 *                         given input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_createAccumulation(const Mdvx &input_file,
					Mdvx &accum_file) const
{
  static const string method_name = "MdvAccumulate::_createAccumulation()";
  
  // Clear out any existing data in the accumulation file.  There
  // shouldn't be any, but might as well be careful.

  accum_file.clearFields();
  accum_file.clearChunks();
  
  // Update the master header

  _updateAccumMasterHeader(accum_file, input_file.getMasterHeader(),
			   input_file.getPathInUse());
  
  // Add all of the fields.  Use the background field for any that
  // have one.

  DateTime bkgnd_time = DateTime::NEVER;
  
  for (size_t field_num = 0; field_num < input_file.getNFields(); ++field_num)
  {
    // See if we have a background field available

    MdvxField *input_field = input_file.getField(field_num);
    Mdvx::field_header_t input_field_hdr = input_field->getFieldHeader();
    
    map< string, BackgroundField >::const_iterator bkgnd_field_info;
    bool bkgnd_field_info_found = true;
    
    if ((bkgnd_field_info =
	 _backgroundFieldList.find(input_field_hdr.field_name))
	== _backgroundFieldList.end() &&
	(bkgnd_field_info =
	 _backgroundFieldList.find(input_field_hdr.field_name_long))
	== _backgroundFieldList.end())
      bkgnd_field_info_found = false;
    
    MdvxField *bkgnd_field = 0;
    
    if (bkgnd_field_info_found)
      bkgnd_field = _readBackgroundField(input_file.getMasterHeader().time_centroid,
					 _params->max_accum_secs,
					 (*bkgnd_field_info).second);
    
    // Create the new accumulation field

    MdvxField *accum_field;
    
    if (bkgnd_field == 0)
    {
      // We don't have a background field so the accumulation field is just
      // the same as the new input field

      accum_field = new MdvxField(*input_field);
    }
    else
    {
      // We have a background field so initialize the accumulation field 
      // using both the background field and the input field.

      accum_field = new MdvxField(*input_field);

      Mdvx::field_header_t accum_field_hdr = accum_field->getFieldHeader();
      Mdvx::field_header_t bkgnd_field_hdr = bkgnd_field->getFieldHeader();
      
      // Find the quality control object, if there is one, for this field

      map< string, QualityControl* >::const_iterator qc_iter;
      QualityControl *qc = 0;
      
      if ((qc_iter = _qualityControllers.find(accum_field_hdr.field_name)) !=
	   _qualityControllers.end() ||
	  (qc_iter = _qualityControllers.find(accum_field_hdr.field_name_long)) !=
	   _qualityControllers.end())
	qc = (*qc_iter).second;
      
      // Update the accumulation data

      int volume_size =
	accum_field_hdr.nx * accum_field_hdr.ny * accum_field_hdr.nz;
      
      fl32 *accum_data = (fl32 *)accum_field->getVol();
      fl32 *bkgnd_data = (fl32 *)bkgnd_field->getVol();
      
      for (int i = 0; i < volume_size; ++i)
      {
	// If we have an accumulation value (which means we got a value
	// in the new input file) use the new value.  Quality control the
	// new value if specified by the user.

	if (accum_data[i] != accum_field_hdr.missing_data_value &&
	    accum_data[i] != accum_field_hdr.bad_data_value)
	{
	  if (qc != 0)
	  {
	    if (bkgnd_data[i] == bkgnd_field_hdr.missing_data_value ||
		bkgnd_data[i] == bkgnd_field_hdr.bad_data_value)
	      accum_data[i] = qc->qcValue(accum_data[i],
					  accum_field_hdr.missing_data_value);
	    else
	      accum_data[i] = qc->qcValue(accum_data[i], bkgnd_data[i],
					  accum_field_hdr.missing_data_value);
	  }
	  
	  continue;
	}
	
	// If we get here, the new data value was missing.  If the background
	// value is missing, too, then we can't do anything

	if (bkgnd_data[i] == bkgnd_field_hdr.missing_data_value ||
	    bkgnd_data[i] == bkgnd_field_hdr.bad_data_value)
	  continue;
	
	// If we get here, the new input value was missing, but we have a
	// background value.  So, update the accumulation with the background
	// value.

	accum_data[i] = bkgnd_data[i];
      }
      
      if (bkgnd_time == DateTime::NEVER ||
	  bkgnd_time < bkgnd_field_hdr.forecast_time)
	bkgnd_time = bkgnd_field_hdr.forecast_time;
    }
    
    // Compress the accumulated field

    const Mdvx::field_header_t *file_field_hdr =
      input_field->getFieldHeaderFile();
    
    if (file_field_hdr != 0 &&
	file_field_hdr->scaling_type == Mdvx::SCALING_SPECIFIED)
      accum_field->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_BZIP,
			       Mdvx::SCALING_SPECIFIED,
			       file_field_hdr->scale, file_field_hdr->bias);
    else
      accum_field->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_BZIP,
			       Mdvx::SCALING_DYNAMIC);
    
    accum_file.addField(accum_field);
  } /* endfor - field_num */
  
  // Create a new data time field and add it to the accum file.

  MdvxField *accum_time_field =
    _createAccumTimeField(*(input_file.getField(0)),
			  input_file.getMasterHeader().time_centroid,
			  bkgnd_time);
  
  accum_file.addField(accum_time_field);
  
  return true;
}


/*********************************************************************
 * _initBackgroundFields() - Initialize the background fields.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_initBackgroundFields(void)
{
  static const string method_name = "MdvAccumulate::_initBackgroundFields()";
  
  for (int i = 0; i < _params->background_field_info_n; ++i)
    _backgroundFieldList[_params->_background_field_info[i].input_field_name] =
      BackgroundField(_params->_background_field_info[i].background_field_url,
		      _params->_background_field_info[i].background_field_name,
		      _params->_background_field_info[i].background_field_level_num);
  
  return true;
}


/*********************************************************************
 * _initInputProj() - Initialize the input projection.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_initInputProj(void)
{
  static const string method_name = "MdvAccumulate::_initInputProj()";
  
  switch (_params->remap_info.remap_type)
  {
  case Params::REMAP_LATLON :
    _inputProj.initLatlon(_params->remap_info.nx,
			  _params->remap_info.ny,
			  1,
			  _params->remap_info.dx,
			  _params->remap_info.dy,
			  1.0,
			  _params->remap_info.minx,
			  _params->remap_info.miny,
			  0.0);
    break;
  
  case Params::REMAP_FLAT :
    _inputProj.initFlat(_params->remap_info.origin_lat,
			_params->remap_info.origin_lon,
			_params->remap_info.rotation,
			_params->remap_info.nx,
			_params->remap_info.ny,
			1,
			_params->remap_info.dx,
			_params->remap_info.dy,
			1.0,
			_params->remap_info.minx,
			_params->remap_info.miny,
			0.0);
    break;
  
  case Params::REMAP_LAMBERT_CONFORMAL2 :
    _inputProj.initLc2(_params->remap_info.origin_lat,
		       _params->remap_info.origin_lon,
		       _params->remap_info.lat1,
		       _params->remap_info.lat2,
		       _params->remap_info.nx,
		       _params->remap_info.ny,
		       1,
		       _params->remap_info.dx,
		       _params->remap_info.dy,
		       1.0,
		       _params->remap_info.minx,
		       _params->remap_info.miny,
		       0.0);
    break;
    
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _initQualityControl() - Initialize the quality control objects
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_initQualityControl(void)
{
  static const string method_name = "MdvAccumulate::_initQualityControl()";
  
  for (int i = 0; i < _params->field_qcs_n; ++i)
  {
    QualityControl *qc =
      new QualityControl(_params->_field_qcs[i].max_value_diff);
    
    _qualityControllers[_params->_field_qcs[i].field_name] = qc;
  }
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_initTrigger(void)
{
  static const string method_name = "MdvAccumulate::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
      cerr << trigger->getErrStr() << endl;
      
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
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
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
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvAccumulate::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the new input data

  DsMdvx input_file;
  
  if (!_readInputFile(input_file, trigger_time))
    return false;
  
  // Read in the most recent accumulation file.  If one is found, update the
  // accumulation.  Otherwise, start a new accumulation.

  DsMdvx accum_file;

  if (_readAccumFile(accum_file, trigger_time))
  {
    if (_params->debug)
      cerr << "Got accumulation file -- updating" << endl;
    
    if (!_updateAccumulation(input_file, accum_file))
      return false;
  }
  else
  {
    if (_params->debug)
      cerr << "Accumulation file not found -- creating new one" << endl;

    if (!_createAccumulation(input_file, accum_file))
      return false;
  }
  
  // Write the accumulation file

  accum_file.setWriteLdataInfo();
  
  if (accum_file.writeToDir(_params->accum_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing accumulation file to URL: "
	 << _params->accum_url << endl;
    cerr << accum_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readAccumFile() - Read the indicated accumulation file.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_readAccumFile(Mdvx &accum_file,
				   const DateTime &trigger_time) const
{
  static const string method_name = "MdvAccumulate::_readAccumFile()";
  
  // Set up the read request

  accum_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->accum_url,
			 _params->max_accum_secs,
			 trigger_time.utime());
  
  accum_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  accum_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  accum_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  accum_file.setReadRemap(_inputProj);
  
  // Read the data.  Note that it's not an error if the data can't be
  // read.

  if (accum_file.readVolume() != 0)
    return false;
  
  return true;
}


/*********************************************************************
 * _readBackgroundField() - Read the indicated background field.
 *
 * Returns a pointer to the read field on success, 0 on failure.
 */

MdvxField *MdvAccumulate::_readBackgroundField(const DateTime &data_end_time,
					       const DateTime &data_start_time,
					       const BackgroundField &background_field_info) const
{
  static const string method_name = "MdvAccumulate::_readBackgroundField()";
  
  // Determine the search time for requesting the background field

  int search_margin = data_end_time - data_start_time;
  
  // Read the field

  return _readBackgroundField(data_end_time, search_margin,
			      background_field_info);
}


MdvxField *MdvAccumulate::_readBackgroundField(const DateTime &data_end_time,
					       const int search_margin,
					       const BackgroundField &background_field_info) const
{
  static const string method_name = "MdvAccumulate::_readBackgroundField()";
  
  // Set up the read request

  DsMdvx bkgnd_file;
  
  bkgnd_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 background_field_info.getUrl().c_str(),
			 search_margin, data_end_time.utime());
  
  bkgnd_file.addReadField(background_field_info.getFieldName());

  int level_num = background_field_info.getLevelNum();

  if (level_num < 0)
    bkgnd_file.setReadComposite();
  else
    bkgnd_file.setReadPlaneNumLimits(level_num, level_num);
  
  bkgnd_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  bkgnd_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  bkgnd_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  bkgnd_file.setReadRemap(_inputProj);
  
  // Read the file.  If the read fails, it just means that there isn't a
  // background field to process.  This is okay, we just send back a 0
  // to indicate this.

  if (bkgnd_file.readVolume() != 0)
    return 0;
  
  // If we get here, the read was successful.  So, now we pull the background
  // field out of the file and return a copy of it.

  return new MdvxField(*(bkgnd_file.getField(0)));
}


/*********************************************************************
 * _readInputFile() - Read the indicated input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_readInputFile(Mdvx &input_file,
				   const DateTime &trigger_time) const
{
  static const string method_name = "MdvAccumulate::_readInputFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->input_url,
			 0, trigger_time.utime());
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);

  input_file.setReadFieldFileHeaders();
  
  if (_params->remap_data)
    input_file.setReadRemap(_inputProj);
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  // Set the input projection based on the projection read.  We are assuming
  // that all of the input fields use the same projection.

  if (!_params->remap_data)
    _inputProj.init(input_file);
  
  return true;
}


/*********************************************************************
 * _updateAccumBackground() - Update the given accumulation file with
 *                            any new background data.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_updateAccumBackground(const DateTime &data_time,
					   Mdvx &accum_file) const
{
  static const string method_name = "MdvAccumulate::_updateAccumBackground()";
  
  // Loop through the accumulation field and update any that have new
  // background information

  DateTime bkgnd_time = DateTime::NEVER;
  
  for (size_t field_num = 0; field_num < accum_file.getNFields(); ++field_num)
  {
    MdvxField *accum_field = accum_file.getField(field_num);
    Mdvx::field_header_t accum_field_hdr = accum_field->getFieldHeader();
    
    // See if the accum field (which has the same name as the input field)
    // is in the background field list.  If it's not, we can move on to the
    // next field.

    map< string, BackgroundField >::const_iterator bkgnd_field;
    
    if ((bkgnd_field =
	 _backgroundFieldList.find(accum_field_hdr.field_name))
	== _backgroundFieldList.end() &&
	(bkgnd_field =
	 _backgroundFieldList.find(accum_field_hdr.field_name_long))
	== _backgroundFieldList.end())
      continue;
    
    if (_params->debug)
    {
      cerr << "Background field found for input field: " << (*bkgnd_field).first << endl;
      (*bkgnd_field).second.print(cerr);
    }
    
    DateTime new_bkgnd_time;
    
    if (!_updateAccumFieldBackground(data_time,
				     accum_file.getMasterHeader().time_centroid,
				     *accum_field, (*bkgnd_field).second,
				     new_bkgnd_time))
      return false;
    
    if (bkgnd_time == DateTime::NEVER ||
	bkgnd_time < new_bkgnd_time)
      bkgnd_time = new_bkgnd_time;
    
  } /* endfor - field_num */
  
  // Update the accum time field if we got a background field

  if (bkgnd_time != DateTime::NEVER)
  {
    // Retrieve the time field

    MdvxField *time_field;
    if ((time_field = accum_file.getField(DATA_TIME_FIELD_NAME.c_str())) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error retrieving time field (" << DATA_TIME_FIELD_NAME
	   << ") from accumulation file" << endl;
      cerr << "Cannot continue accumulation" << endl;
    
      return false;
    }

    if (!_updateAccumTimeField(*time_field, bkgnd_time))
      return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateAccumFieldBackground() - Update the given accumulation field
 *                                 with the specified background field.
 *                                 If the background field is updated,
 *                                 the background field time is sent back
 *                                 in the bkgnd_time parameter.  If it is
 *                                 not updated, DateTime::NEVER is returned.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_updateAccumFieldBackground(const DateTime &data_time,
						const DateTime &accum_time,
						MdvxField &accum_field,
						const BackgroundField &bkgnd_field_info,
						DateTime &bkgnd_time) const
{
  static const string method_name = "MdvAccumulate::_updateAccumFieldBackground()";
  
  // Initialize returned values

  bkgnd_time = DateTime::NEVER;
  
  // Read in the background field.  If we don't get a background field,
  // then everything is okay and we can use the accumulation field as is.

  MdvxField *bkgnd_field;
  
  if ((bkgnd_field = _readBackgroundField(data_time, accum_time,
					  bkgnd_field_info)) == 0)
    return true;
  
  // If we get here, we have a background field that is more recent than the
  // latest accumulated field so we need to replace the accumulated field
  // with the background information.

  Mdvx::field_header_t accum_field_hdr = accum_field.getFieldHeader();
  Mdvx::field_header_t bkgnd_field_hdr = bkgnd_field->getFieldHeader();
  
  fl32 *accum_data = (fl32 *)accum_field.getVol();
  fl32 *bkgnd_data = (fl32 *)bkgnd_field->getVol();
  
  int volume_size =
    accum_field_hdr.nx * accum_field_hdr.ny * accum_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (bkgnd_data[i] == bkgnd_field_hdr.bad_data_value)
      accum_data[i] = accum_field_hdr.bad_data_value;
    else if (bkgnd_data[i] == bkgnd_field_hdr.missing_data_value)
      accum_data[i] = accum_field_hdr.missing_data_value;
    else
      accum_data[i] = bkgnd_data[i];
  } /* endfor - i */
  
  // Update the background time that is returned to the caller.

  bkgnd_time = bkgnd_field_hdr.forecast_time;
  
  // Now reclaim the memory for the background field

  delete bkgnd_field;
  
  return true;
}


/*********************************************************************
 * _updateAccumTimeField() - Update the accumulation time field with
 *                           the new input time.  The time values will
 *                           only be updated where the input field has
 *                           valid data.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_updateAccumTimeField(MdvxField &time_field,
					  const MdvxField &input_field,
					  const DateTime &data_time) const
{
  static const string method_name = "MdvAccumulate::_updateAccumTimeField()";
  
  // Fill in the time values

  Mdvx::field_header_t time_field_hdr = time_field.getFieldHeader();
  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  
  int volume_size = time_field_hdr.nx * time_field_hdr.ny * time_field_hdr.nz;
  
  fl32 *input_data = (fl32 *)input_field.getVol();
  ui32 *time_data = (ui32 *)time_field.getVol();
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (input_data[i] == input_field_hdr.missing_data_value ||
	input_data[i] == input_field_hdr.bad_data_value)
      continue;
    
    time_data[i] = data_time.utime();
  }
  
  return true;
}


bool MdvAccumulate::_updateAccumTimeField(MdvxField &time_field,
					  const DateTime &data_time) const
{
  static const string method_name = "MdvAccumulate::_updateAccumTimeField()";
  
  // Fill in the time values

  Mdvx::field_header_t time_field_hdr = time_field.getFieldHeader();
  
  int volume_size = time_field_hdr.nx * time_field_hdr.ny * time_field_hdr.nz;
  
  ui32 *time_data = (ui32 *)time_field.getVol();
  
  for (int i = 0; i < volume_size; ++i)
    time_data[i] = data_time.utime();
  
  return true;
}


/*********************************************************************
 * _updateAccumulation() - Update the given accumulation file based on the
 *                         given input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvAccumulate::_updateAccumulation(const Mdvx &input_file,
					Mdvx &accum_file) const
{
  static const string method_name = "MdvAccumulate::_updateAccumulation()";
  
  // Update the previous accumulation field with the background data
  // so it is ready for adding the new AIRS data.

  if (!_updateAccumBackground(input_file.getMasterHeader().time_centroid,
			      accum_file))
    return false;
    
  // Update the master header

  _updateAccumMasterHeader(accum_file, input_file.getMasterHeader(),
			   input_file.getPathInUse());
  
  // Time out any old data in the old accumulation fields.  We do this
  // before updating the data since it's easier even though it requires
  // multiple passes through the data.

  MdvxField *time_field;
  if ((time_field = accum_file.getField(DATA_TIME_FIELD_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving time field (" << DATA_TIME_FIELD_NAME
	 << ") from accumulation file" << endl;
    cerr << "Cannot continue accumulation" << endl;
    
    return false;
  }
  
  Mdvx::field_header_t time_field_hdr = time_field->getFieldHeader();
  int volume_size = time_field_hdr.nx * time_field_hdr.ny * time_field_hdr.nz;
  ui32 *time_data = (ui32 *)time_field->getVol();
  ui32 expire_time =
    input_file.getMasterHeader().time_centroid - _params->max_accum_secs;
  
  if (_params->debug)
  {
    cerr << "---> expiring data" << endl;
    cerr << "     data time: "
	 << DateTime::str(input_file.getMasterHeader().time_centroid) << endl;
    cerr << "     expire time: " << DateTime::str(expire_time) << endl;
  }
  
  for (size_t field_num = 0; field_num < accum_file.getNFields(); ++field_num)
  {
    MdvxField *field = accum_file.getField(field_num);
    
    if (string(field->getFieldHeader().field_name) == DATA_TIME_FIELD_NAME)
      continue;
    
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    fl32 *data = (fl32 *)field->getVol();
    
    for (int j = 0; j < volume_size; ++j)
    {
      if (time_data[j] != DATA_TIME_MISSING_VALUE &&
	  time_data[j] < expire_time)
	data[j] = field_hdr.missing_data_value;
    } /* endfor - j */
    
  } /* endfor - field_num */
  
  // Update each of the accumulation fields with the new data

  for (size_t field_num = 0; field_num < input_file.getNFields(); ++field_num)
  {
    MdvxField *input_field = input_file.getField(field_num);
    MdvxField *accum_field =
      accum_file.getField(input_field->getFieldNameLong());
    
    if (accum_field == 0)
    {
      MdvxField *new_field = new MdvxField(*input_field);

      const Mdvx::field_header_t *file_field_hdr =
	input_field->getFieldHeaderFile();
      
      if (file_field_hdr != 0 &&
	  file_field_hdr->scaling_type == Mdvx::SCALING_SPECIFIED)
	new_field->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_BZIP,
			       Mdvx::SCALING_SPECIFIED,
			       file_field_hdr->scale, file_field_hdr->bias);
      else
	new_field->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_BZIP,
			       Mdvx::SCALING_DYNAMIC);
      
      accum_file.addField(new_field);
    }
    else
    {
      // Make sure the data is uncompressed just in case we are processing
      // this accum field multiple times

      accum_field->convertType(Mdvx::ENCODING_FLOAT32,
			       Mdvx::COMPRESSION_NONE,
			       Mdvx::SCALING_NONE);
      
      Mdvx::field_header_t input_field_hdr = input_field->getFieldHeader();
      Mdvx::field_header_t accum_field_hdr = accum_field->getFieldHeader();
      
      // Find the quality control object, if there is one, for this field

      map< string, QualityControl* >::const_iterator qc_iter;
      QualityControl *qc = 0;
      
      if ((qc_iter = _qualityControllers.find(accum_field_hdr.field_name)) !=
	   _qualityControllers.end() ||
	  (qc_iter = _qualityControllers.find(accum_field_hdr.field_name_long)) !=
	   _qualityControllers.end())
	qc = (*qc_iter).second;
      
      // Update the accumulation data

      int volume_size =
	input_field_hdr.nx * input_field_hdr.ny * input_field_hdr.nz;
      
      fl32 *input_data = (fl32 *)input_field->getVol();
      fl32 *accum_data = (fl32 *)accum_field->getVol();
      
      for (int j = 0; j < volume_size; ++j)
      {
	// If the input data value is missing, don't do anything

	if (input_data[j] == input_field_hdr.missing_data_value ||
	    input_data[j] == input_field_hdr.bad_data_value)
	  continue;
	
	// If we aren't quality controlling this field, update the accumulation
	// with the new input value

	if (qc == 0)
	{
	  accum_data[j] = input_data[j];
	  continue;
	}
	
	// Update the accumulation value with the new quality controlled
	// value

	if (accum_data[j] == accum_field_hdr.missing_data_value ||
	    accum_data[j] == accum_field_hdr.bad_data_value)
	  accum_data[j] = qc->qcValue(input_data[j],
				      input_field_hdr.missing_data_value);
	else
	  accum_data[j] = qc->qcValue(input_data[j], accum_data[j],
				      input_field_hdr.missing_data_value);
	
      } /* endfor - j */

      // Update the accum field header because the data values changed.

      accum_field_hdr.min_value = 0;
      accum_field_hdr.max_value = 0;
      accum_field_hdr.min_value_orig_vol = 0;
      accum_field_hdr.max_value_orig_vol = 0;
      
      accum_field->setFieldHeader(accum_field_hdr);

      const Mdvx::field_header_t *file_field_hdr =
	input_field->getFieldHeaderFile();
      
      if (file_field_hdr != 0 &&
	  file_field_hdr->scaling_type == Mdvx::SCALING_SPECIFIED)
	accum_field->convertType(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_BZIP,
				 Mdvx::SCALING_SPECIFIED,
				 file_field_hdr->scale, file_field_hdr->bias);
      else
	accum_field->convertType(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_BZIP,
				 Mdvx::SCALING_DYNAMIC);
    }

  } /* endfor - field_num */
  
  // Now update the accumulation data time file

  if (!_updateAccumTimeField(*time_field,
			     *(input_file.getField(0)),
			     input_file.getMasterHeader().time_centroid))
    return false;
  
  return true;
}


/*********************************************************************
 * __updateAccumMasterHeader() - Update the accumulation file master
 *                               header based on the information in
 *                               the given master header.
 */

void MdvAccumulate::_updateAccumMasterHeader(Mdvx &accum_file,
					     const Mdvx::master_header_t &master_hdr,
					     const string &input_file_path) const
{
  Mdvx::master_header_t accum_master_hdr = accum_file.getMasterHeader();
  
  accum_master_hdr.time_gen = master_hdr.time_gen;
  accum_master_hdr.time_begin =
    master_hdr.time_centroid - _params->max_accum_secs;
  accum_master_hdr.time_end = master_hdr.time_centroid;
  accum_master_hdr.time_centroid = master_hdr.time_centroid;
  accum_master_hdr.time_expire = master_hdr.time_expire;
  accum_master_hdr.data_dimension = master_hdr.data_dimension;
  accum_master_hdr.data_collection_type = master_hdr.data_collection_type;
  accum_master_hdr.native_vlevel_type = master_hdr.native_vlevel_type;
  accum_master_hdr.vlevel_type = master_hdr.vlevel_type;
  accum_master_hdr.vlevel_included = master_hdr.vlevel_included;
  accum_master_hdr.grid_orientation = master_hdr.grid_orientation;
  accum_master_hdr.data_ordering = master_hdr.data_ordering;
  accum_master_hdr.sensor_lon = master_hdr.sensor_lon;
  accum_master_hdr.sensor_lat = master_hdr.sensor_lat;
  accum_master_hdr.sensor_alt = master_hdr.sensor_alt;
  STRcopy(accum_master_hdr.data_set_info,
	  "Output from MdvAccumulate", MDV_INFO_LEN);
  STRcopy(accum_master_hdr.data_set_name, "MdvAccumulate", MDV_NAME_LEN);
  STRcopy(accum_master_hdr.data_set_source, input_file_path.c_str(),
	  MDV_NAME_LEN);
  
  accum_file.setMasterHeader(accum_master_hdr);
}
