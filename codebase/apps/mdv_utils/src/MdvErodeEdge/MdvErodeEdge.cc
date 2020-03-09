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

/*********************************************************************
 * MdvErodeEdge: MdvErodeEdge program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2007
 *
 * Dan Megenhardt
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

#include "MdvErodeEdge.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvErodeEdge *MdvErodeEdge::_instance =
     (MdvErodeEdge *)NULL;



/*********************************************************************
 * Constructor
 */

MdvErodeEdge::MdvErodeEdge(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvErodeEdge::MdvErodeEdge()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvErodeEdge *)NULL);
  
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

MdvErodeEdge::~MdvErodeEdge()
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

MdvErodeEdge *MdvErodeEdge::Inst(int argc, char **argv)
{
  if (_instance == (MdvErodeEdge *)NULL)
    new MdvErodeEdge(argc, argv);
  
  return(_instance);
}

MdvErodeEdge *MdvErodeEdge::Inst()
{
  assert(_instance != (MdvErodeEdge *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvErodeEdge::init()
{
  static const string method_name = "MdvErodeEdge::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvErodeEdge::run()
{
  static const string method_name = "MdvErodeEdge::run()";
  
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
 * _doTheWork() - Fill grid points with missing data values when 
 *                the fill_percent is below threshold. This is
 *                done for the given field. Input int8_t data.
 *
 * Returns true on success, false on failure.
 */

void MdvErodeEdge::_doTheWork(int8_t *data, Mdvx::field_header_t &field_hdr) const
{
    int plane_size = field_hdr.nx * field_hdr.ny;
    int volume_size = plane_size * field_hdr.nz;
  
    // Copy the original data so that we aren't using values
    // we have already altered

    int8_t *orig_data = new int8_t[volume_size];
    memcpy(orig_data, data, volume_size * sizeof(int8_t));

    // Update the template to match the current data projection

    MdvxPjg proj(field_hdr);
  
    _updateTemplate(proj);
  
    // Loop through the data, setting values to missing if
    // if the percent_fill is not met.

    for (int z = 0; z < field_hdr.nz; ++z)
    {
	for (int y = 0; y < field_hdr.ny; ++y)
	{
	    for (int x = 0; x < field_hdr.nx; ++x)
	    {
		int index = proj.xyIndex2arrayIndex(x, y, z);
      
	        // IF values are already missing no need to 
	        // run filter

		if (data[index] == field_hdr.missing_data_value &&
		    data[index] == field_hdr.bad_data_value)
		    continue;
      
		GridPoint *point;
	
		int npts_filled = 0;
		int npts_in_template = 0;

		for (point = _template.getFirstInGrid(x, y,
						      field_hdr.nx, field_hdr.ny);
		     point != 0;
		     point = _template.getNextInGrid())
		{
		    ++npts_in_template;
	  
		    int template_index =
			point->getIndex(field_hdr.nx, field_hdr.ny) + (z * plane_size);
	  

		    // Don't count point if it's bad or missing

		    if (orig_data[template_index] == field_hdr.bad_data_value ||
			orig_data[template_index] == field_hdr.missing_data_value)
			continue;
	  
		    ++npts_filled;
		} /* endfor - point */
	
		if (npts_in_template == 0 || npts_filled == 0)
		    continue;
	
		double percent_filled = (double)npts_filled / (double)npts_in_template;

	        // Keep the value if at or above the fill_percent

		if (percent_filled >= _params->fill_percent)
		    continue;
		
		data[index] = (int8_t)field_hdr.missing_data_value;
	
	    } /* endfor - x */
      
	} /* endfor - y */
    
    } /* endfor - z */
  
// Reclaim memory

    delete [] orig_data;
}
/*********************************************************************
 * _doTheWork() - Fill grid points with missing data values when 
 *                the fill_percent is below threshold. This is
 *                done for the given field. Input int16_t data.
 *
 * Returns true on success, false on failure.
 */

void MdvErodeEdge::_doTheWork(int16_t *data, Mdvx::field_header_t &field_hdr) const
{
    int plane_size = field_hdr.nx * field_hdr.ny;
    int volume_size = plane_size * field_hdr.nz;
  
    // Copy the original data so that we aren't using values
    // we have already altered

    int16_t *orig_data = new int16_t[volume_size];
    memcpy(orig_data, data, volume_size * sizeof(int16_t));

    // Update the template to match the current data projection

    MdvxPjg proj(field_hdr);
  
    _updateTemplate(proj);
  
    // Loop through the data, setting values to missing if
    // if the percent_fill is not met.

    for (int z = 0; z < field_hdr.nz; ++z)
    {
	for (int y = 0; y < field_hdr.ny; ++y)
	{
	    for (int x = 0; x < field_hdr.nx; ++x)
	    {
		int index = proj.xyIndex2arrayIndex(x, y, z);
      
	        // IF values are already missing no need to 
	        // run filter

		if (data[index] == field_hdr.missing_data_value &&
		    data[index] == field_hdr.bad_data_value)
		    continue;
      
		GridPoint *point;
	
		int npts_filled = 0;
		int npts_in_template = 0;

		for (point = _template.getFirstInGrid(x, y,
						      field_hdr.nx, field_hdr.ny);
		     point != 0;
		     point = _template.getNextInGrid())
		{
		    ++npts_in_template;
	  
		    int template_index =
			point->getIndex(field_hdr.nx, field_hdr.ny) + (z * plane_size);
	  

		    // Don't count point if it's bad or missing

		    if (orig_data[template_index] == field_hdr.bad_data_value ||
			orig_data[template_index] == field_hdr.missing_data_value)
			continue;
	  
		    ++npts_filled;
		} /* endfor - point */
	
		if (npts_in_template == 0 || npts_filled == 0)
		    continue;
	
		double percent_filled = (double)npts_filled / (double)npts_in_template;

	        // Keep the value if at or above the fill_percent

		if (percent_filled >= _params->fill_percent)
		    continue;
		
		data[index] = (int16_t)field_hdr.missing_data_value;
	
	    } /* endfor - x */
      
	} /* endfor - y */
    
    } /* endfor - z */
  
// Reclaim memory

    delete [] orig_data;
}
/*********************************************************************
 * _doTheWork() - Fill grid points with missing data values when 
 *                the fill_percent is below threshold. This is
 *                done for the given field. Input fl32 data.
 *
 * Returns true on success, false on failure.
 */

void MdvErodeEdge::_doTheWork(fl32 *data, Mdvx::field_header_t &field_hdr) const
{
    int plane_size = field_hdr.nx * field_hdr.ny;
    int volume_size = plane_size * field_hdr.nz;
  
    // Copy the original data so that we aren't using values
    // we have already altered

    fl32 *orig_data = new fl32[volume_size];
    memcpy(orig_data, data, volume_size * sizeof(fl32));

    // Update the template to match the current data projection

    MdvxPjg proj(field_hdr);
  
    _updateTemplate(proj);
  
    // Loop through the data, setting values to missing if
    // if the percent_fill is not met.

    for (int z = 0; z < field_hdr.nz; ++z)
    {
	for (int y = 0; y < field_hdr.ny; ++y)
	{
	    for (int x = 0; x < field_hdr.nx; ++x)
	    {
		int index = proj.xyIndex2arrayIndex(x, y, z);
      
	        // IF values are already missing no need to 
	        // run filter

		if (data[index] == field_hdr.missing_data_value &&
		    data[index] == field_hdr.bad_data_value)
		    continue;
      
		GridPoint *point;
	
		int npts_filled = 0;
		int npts_in_template = 0;

		for (point = _template.getFirstInGrid(x, y,
						      field_hdr.nx, field_hdr.ny);
		     point != 0;
		     point = _template.getNextInGrid())
		{
		    ++npts_in_template;
	  
		    int template_index =
			point->getIndex(field_hdr.nx, field_hdr.ny) + (z * plane_size);
	  

		    // Don't count point if it's bad or missing

		    if (orig_data[template_index] == field_hdr.bad_data_value ||
			orig_data[template_index] == field_hdr.missing_data_value)
			continue;
	  
		    ++npts_filled;
		} /* endfor - point */
	
		if (npts_in_template == 0 || npts_filled == 0)
		    continue;
	
		double percent_filled = (double)npts_filled / (double)npts_in_template;

	        // Keep the value if at or above the fill_percent

		if (percent_filled >= _params->fill_percent)
		    continue;
		
		data[index] = field_hdr.missing_data_value;
	
	    } /* endfor - x */
      
	} /* endfor - y */
    
    } /* endfor - z */
  
// Reclaim memory

    delete [] orig_data;
}


/*********************************************************************
 * _fillMissingFields() - Fill grid points with missing data values when 
 *                        the fill_percent is below threshold. This is
 *                        done for the given field.
 *
 * Returns true on success, false on failure.
 */

void MdvErodeEdge::_fillMissingField(MdvxField &field) const
{
  static const string method_name = "MdvErodeEdge::_fillMissingField()";
  
  Mdvx::field_header_t field_hdr = field.getFieldHeader();

  switch (field_hdr.encoding_type)
  {
    case Mdvx::ENCODING_INT8 :
    {
	
	int8_t *data = (int8_t *)field.getVol();
	_doTheWork(data, field_hdr);
	
	break;
    }
    case Mdvx::ENCODING_INT16 :
    {
	
	int16_t *data = (int16_t *)field.getVol();
	_doTheWork(data, field_hdr);

	break;
    }
    case Mdvx::PLANE_RLE8 :
    {
	
	int8_t *data = (int8_t *)field.getVol();
	_doTheWork(data, field_hdr);

	break;
    }
    case Mdvx::ENCODING_FLOAT32 :
    {
	
	fl32 *data = (fl32 *)field.getVol();
	_doTheWork(data, field_hdr);
    }
  
  }
}


/*********************************************************************
 * _fillMissingFields() - Fill grid points with missing data values when 
 *                        the fill_percent is below threshold. This is
 *                        done for all fields in the file. 
 *
 * Returns true on success, false on failure.
 */

void MdvErodeEdge::_fillMissingFields(Mdvx &mdvx) const
{
  for (size_t i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    
    _fillMissingField(*field);
    
  } /* endfor - i */
  
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvErodeEdge::_initTrigger(void)
{
  static const string method_name = "MdvErodeEdge::_initTrigger()";
  
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

bool MdvErodeEdge::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvErodeEdge::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the new input data

  DsMdvx mdvx;
  
  if (!_readInputFile(mdvx, trigger_time))
    return false;
  
  _fillMissingFields(mdvx);
  
  // Write the output file
  
  for (size_t i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    
//    Mdvx::field_header_t field_hdr = (*field).getFieldHeader();
//    cerr << "field_hdr.encding_type = " << field_hdr.encoding_type << endl;
//    cerr << "field_hdr.scale = " << field_hdr.scale << endl;
//    cerr << "field_hdr.bias = " << field_hdr.bias << endl;

    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  } /* endfor - i */
  
  mdvx.setWriteLdataInfo();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing accumulation file to URL: "
	 << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile() - Read the indicated input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvErodeEdge::_readInputFile(Mdvx &input_file,
				   const DateTime &trigger_time) const
{
  static const string method_name = "MdvErodeEdge::_readInputFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->input_url,
			 0, trigger_time.utime());
  
//  input_file.setReadEncodingType(Mdvx::ENCODING_FL32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);

  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateTemplate() - Update the radius of influence template based on
 *                     the current grid projection.
 */

void MdvErodeEdge::_updateTemplate(const MdvxPjg &proj) const
{
  double radius_x = proj.km2xGrid(_params->radius_of_influence);
  double radius_y = proj.km2yGrid(_params->radius_of_influence);
  
  if (radius_x > radius_y)
    _template.setEllipse(90.0, radius_x, radius_y);
  else
    _template.setEllipse(0.0, radius_y, radius_x);
}
