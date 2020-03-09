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
//   $Id: Mdv2Ascii.cc,v 1.14 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.14 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Mdv2Ascii: Mdv2Ascii program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Mdv2Ascii.hh"
#include "Params.hh"
using namespace std;


// Global variables

Mdv2Ascii *Mdv2Ascii::_instance =
     (Mdv2Ascii *)NULL;


/*********************************************************************
 * Constructor
 */

Mdv2Ascii::Mdv2Ascii(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "Mdv2Ascii::Mdv2Ascii()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Mdv2Ascii *)NULL);
  
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
  char *params_path = (char *) (char *) "unknown";
  
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

Mdv2Ascii::~Mdv2Ascii()
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

Mdv2Ascii *Mdv2Ascii::Inst(int argc, char **argv)
{
  if (_instance == (Mdv2Ascii *)NULL)
    new Mdv2Ascii(argc, argv);
  
  return(_instance);
}

Mdv2Ascii *Mdv2Ascii::Inst()
{
  assert(_instance != (Mdv2Ascii *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Mdv2Ascii::init()
{
  static const string method_name = "Mdv2Ascii::init()";
  
  // Initialize the data trigger

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
      cerr << "  " << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::FILE_LIST :
  {
    if (_params->debug)
      cerr << "Initializing FILE_LIST trigger" << endl;
    
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->inputFileList) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger using file list: " << endl;
      for (int ii = 0; ii < (int) _args->inputFileList.size(); ii++) {
        cerr << "  " << _args->inputFileList[ii] << endl;
      }
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Mdv2Ascii::run()
{
  static const string method_name = "Mdv2Ascii::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time.utime()))
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
 * _openOutputFile() - Open the proper output file.
 *
 * Returns a pointer to the output file on success, 0 on failure.
 */

FILE *Mdv2Ascii::_openOutputFile(const time_t trigger_time, 
				 const time_t genTime)
{
  static const string method_name = "Mdv2Ascii::_openOutputFile()";
  
  // Construct the output file's path

  Path output_path;
  
  DateTime when(trigger_time);
  output_path.setDirectory(_params->output_dir,
                           when.getYear(),
                           when.getMonth(),
                           when.getDay());

  if (_params->filenames_with_gentime){

    date_time_t T;
    T.unix_time = genTime;
    uconvert_from_utime( &T );

    char extension[1024];
    sprintf(extension,"%d%02d%02d_%02d%02d%02d.txt",
	    T.year, T.month, T.day, T.hour, T.min, T.sec);
    output_path.setFile(DateTime(trigger_time), extension);

  } else {
    output_path.setFile(DateTime(trigger_time), "txt");
  }
  // Make sure the directory exists

  if (output_path.makeDirRecurse() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error making directory path for output file: " << output_path.getPath() << endl;
    
    return 0;
  }
  
  // Open the file

  FILE *output_file;
  
  if ((output_file = ta_fopen_uncompress(output_path.getPath().c_str(),
					 "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file for write: " << output_path.getPath() << endl;
    
    return 0;
  }

  if (_params->debug) {
    cerr << "Opening output file: " << output_path.getPath() << endl;
  }
  
  return output_file;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool Mdv2Ascii::_processData(const time_t trigger_time)
{
  static const string method_name = "Mdv2Ascii::_processData()";
  
  // Read in the input file

  DsMdvx input_file;
  
  if (!_readInputFile(trigger_time, input_file))
    return false;
  
  Mdvx::master_header_t Mhdr = input_file.getMasterHeader();

  // Open the output file

  FILE *output_file;
  
  if ((output_file = _openOutputFile(trigger_time, Mhdr.time_gen)) == 0)
    return false;
    
  // Write out the field data in ASCII format

  MdvxField *field = input_file.getField(0);
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *field_data = (fl32 *)field->getVol();
  
  int column_count = 0;
  
  // Add ESRI-compatible header, if requested

  if (_params->include_ESRI_header)
  {
    if (!_writeEsriHeader(output_file, input_file.getMasterHeader(), field_hdr))
      return false;
  }

  // Add USDA Bird Radar project meta-data, if requested

  if (_params->include_USDA_header)
  {
    if (!_writeUsdaHeader(output_file, *field))
      return false;
  }
  
  // Figure out where to begin and end in the data

  int y_begin = 0;
  int y_end = field_hdr.ny;
  int x_begin = 0;
  int x_end = field_hdr.nx;
  
  if (_params->use_subplane)
  {
    MdvxProj proj(field_hdr);
    Mdvx::coord_t coord = proj.getCoord();
    
    double min_x = MAX(_params->subplane_limits.min_x, coord.minx);
    double max_x = MIN(_params->subplane_limits.max_x,
		       coord.minx + (coord.dx * (double)coord.nx));
    double min_y = MAX(_params->subplane_limits.min_y, coord.miny);
    double max_y = MIN(_params->subplane_limits.max_y,
		       coord.miny + (coord.dy * (double)coord.ny));

    if (proj.xy2xyIndex(min_x, min_y, x_begin, y_begin) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Subplane limits outside of grid" << endl;
      return false;
    }
    
    if (proj.xy2xyIndex(max_x, max_y, x_end, y_end) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Subplane limits outside of grid" << endl;
      return false;
    }
    
    // Increment end values so that we process the last point in the
    // specified subplane.

    if (x_end < field_hdr.nx)
      ++x_end;
    if (y_end < field_hdr.ny)
      ++y_end;
  }
  
  int  y_increment = 1;

  if (_params->start_row == Params::START_TOP)
  {
    int temp = y_begin;
    y_begin = y_end - 1;
    y_end = temp;
    
    y_increment = -1;
  }
  
  // Now write the data to the file

  for (int z = 0; z < field_hdr.nz; ++z)
  {
    for (int y = y_begin; y != y_end; y += y_increment)
    {
      for (int x = x_begin; x < x_end; ++x)
      {
	int i = x + (y * field_hdr.nx) + (z * field_hdr.nx * field_hdr.ny);
	
	if (field_data[i] == field_hdr.bad_data_value ||
	    field_data[i] == field_hdr.missing_data_value)
	{
	  if (_params->specify_bad_data_value)
	    fprintf(output_file, _params->format_string,
		    _params->bad_data_value);
	  else
	    fprintf(output_file, _params->format_string,
		    field_hdr.missing_data_value);
	}
	else if (_params->constrain_min_output_value &&
		 field_data[i] < _params->min_output_value)
	  fprintf(output_file, _params->format_string, _params->min_output_value);
	else if (_params->constrain_max_output_value &&
		 field_data[i] > _params->max_output_value)
	  fprintf(output_file, _params->format_string, _params->max_output_value);
	else
	  fprintf(output_file, _params->format_string, field_data[i]);

	++column_count;
    
	if (column_count >= _params->num_print_columns)
	{
	  fprintf(output_file, "\n");
	  column_count = 0;
	}
      } /* endfor - x */
    } /* endfor - y */
  } /* endfor - z */
    
  // Close the output file

  fclose(output_file);
  
  return true;
}


/*********************************************************************
 * _readInputFile() - Reads the specified input file.
 *
 * Returns true on success, false on failure.
 */

bool Mdv2Ascii::_readInputFile(const time_t trigger_time,
			       DsMdvx &input_file)
{
  static const string method_name = "Mdv2Ascii::_readInputFile()";
  
  // Construct the MDV request based on the given parameters

  const string &path = _dataTrigger->getTriggerInfo().getFilePath();
  if (path.size() > 0) {
    input_file.setReadPath(path);
  } else {
    input_file.setReadTime(Mdvx::READ_CLOSEST,
                           _params->input_url,
                           0, trigger_time);
  }
  
  input_file.clearReadFields();
  
  if (_params->use_field_name)
    input_file.addReadField(_params->input_field.field_name);
  else
    input_file.addReadField(_params->input_field.field_num);

  switch (_params->process_type)
  {
  case Params::PROCESS_VOLUME :
    break;
    
  case Params::PROCESS_PLANE :
    input_file.setReadPlaneNumLimits(_params->plane_num, _params->plane_num);
    break;
    
  case Params::PROCESS_COMPOSITE :
    input_file.setReadComposite();
    break;
  }
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug)
  {
    cerr << endl;
    input_file.printReadRequest(cerr);
  }
  
  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV volume for time: " <<
      DateTime::str(trigger_time) << endl;
    cerr << input_file.getErrStr() << endl;
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeEsriHeader() - Writes the ESRI header to the output file.
 */

bool Mdv2Ascii::_writeEsriHeader(FILE *output_file,
				 const Mdvx::master_header_t &master_hdr,
				 const Mdvx::field_header_t &field_hdr)
{
  static const string method_name = "Mdv2Ascii::_writeEsriHeader()";
  
  MdvxProj projection(master_hdr, field_hdr);
    
  int nx = field_hdr.nx;
  int ny = field_hdr.ny;
  double grid_minx = field_hdr.grid_minx;
  double grid_miny = field_hdr.grid_miny;
  
  if (_params->use_subplane)
  {
    Mdvx::coord_t coord = projection.getCoord();
    
    int x_begin, x_end;
    int y_begin, y_end;
    
    double min_x = MAX(_params->subplane_limits.min_x, coord.minx);
    double max_x = MIN(_params->subplane_limits.max_x,
		       coord.minx + (coord.dx * (double)coord.nx));
    double min_y = MAX(_params->subplane_limits.min_y, coord.miny);
    double max_y = MIN(_params->subplane_limits.max_y,
		       coord.miny + (coord.dy * (double)coord.ny));

    if (projection.xy2xyIndex(min_x, min_y, x_begin, y_begin) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Subplane limits outside of grid" << endl;
      return false;
    }
    
    if (projection.xy2xyIndex(max_x, max_y, x_end, y_end) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Subplane limits outside of grid" << endl;
      return false;
    }
    
    nx = x_end - x_begin;
    ny = y_end - y_begin;
    
    grid_minx = min_x;
    grid_miny = min_y;
  }
  
  // Print out the grid size.

  fprintf(output_file, "nrows %d\n", nx);
  fprintf(output_file, "ncols %d\n", ny);

  // Convert the lower left corner coordinates from the MDV
  // projection units to lat/lon

  double llcorner_x, llcorner_y;
  double llcorner_lon, llcorner_lat;
    
  llcorner_x = grid_minx - (field_hdr.grid_dx / 2.0);
  llcorner_y = grid_miny - (field_hdr.grid_dy / 2.0);
    
  projection.xy2latlon(llcorner_x, llcorner_y,
		       llcorner_lat, llcorner_lon);
    
  fprintf(output_file, "xllcorner %f\n", llcorner_lon);
  fprintf(output_file, "yllcorner %f\n", llcorner_lat);
  
  // For now, let the user provide the cell size

  fprintf(output_file, "cellsize %f\n", _params->cell_size);

  // Print out the missing data value for the file.  Use the same
  // format string that you use for the data so that this value will
  // be rounded in the same way.

  fprintf(output_file, "NODATA_value ");
    
  if (_params->specify_bad_data_value)
    fprintf(output_file, _params->format_string, _params->bad_data_value);
  else
    fprintf(output_file, _params->format_string,
	    field_hdr.missing_data_value);

  fprintf(output_file, "\n");

  return true;
}


/*********************************************************************
 * _writeUsdaHeader() - Writes the USDA header to the output file.
 */

bool Mdv2Ascii::_writeUsdaHeader(FILE *output_file,
				 const MdvxField &field)
{
  static const string method_name = "Mdv2Ascii::_writeUsdaHeader()";
  
  Mdvx::field_header_t field_hdr = field.getFieldHeader();

  int y_begin = 0;
  int y_end = field_hdr.ny;
  int x_begin = 0;
  int x_end = field_hdr.nx;
  double min_x = field_hdr.grid_minx;
  double min_y = field_hdr.grid_miny;
  
  if (_params->use_subplane)
  {
    MdvxProj proj(field_hdr);
    Mdvx::coord_t coord = proj.getCoord();
    
    min_x = MAX(_params->subplane_limits.min_x, coord.minx);
    double max_x = MIN(_params->subplane_limits.max_x,
		       coord.minx + (coord.dx * (double)coord.nx));
    min_y = MAX(_params->subplane_limits.min_y, coord.miny);
    double max_y = MIN(_params->subplane_limits.max_y,
		       coord.miny + (coord.dy * (double)coord.ny));

    if (proj.xy2xyIndex(min_x, min_y, x_begin, y_begin) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Subplane limits outside of grid" << endl;
      return false;
    }
    
    if (proj.xy2xyIndex(max_x, max_y, x_end, y_end) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Subplane limits outside of grid" << endl;
      return false;
    }
    
    if (x_end < field_hdr.nx)
      ++x_end;
    if (y_end < field_hdr.ny)
      ++y_end;
  }
  
  // Get the needed information
  
  int num_good_beams = 0;
  int first_good_beam = -1;
  
  fl32 *data = (fl32 *)field.getVol();
  
  for (int beam = y_begin; beam < y_end; ++beam)
  {
    bool beam_is_good = false;
    
    for (int gate = x_begin; gate < x_end; ++gate)
    {
      int index = (beam * field_hdr.nx) + gate;
      
      if (data[index] != field_hdr.bad_data_value &&
	  data[index] != field_hdr.missing_data_value)
      {
	beam_is_good = true;
	break;
      }
    } /* endfor - gate */
    
    if (beam_is_good)
    {
      ++num_good_beams;
      if (first_good_beam < 0)
	first_good_beam = beam - y_begin;
    }
  } /* endfor - beam */
  
  // Write out the meta-data

  fprintf(output_file, "Original data file name: %s\n",
	  _params->data_file_name);
  fprintf(output_file, "Number of gates: %d\n", x_end - x_begin);
  fprintf(output_file, "Dist to first gate: %f m\n", min_x * 1000.0);
  fprintf(output_file, "Gate spacing: %f m\n", field_hdr.grid_dx * 1000.0);
  fprintf(output_file, "Number of beams: %d\n", y_end - y_begin);
  fprintf(output_file, "First beam az: %f deg\n", min_y);
  fprintf(output_file, "Num good beams: %d\n", num_good_beams);
  fprintf(output_file, "First good beam index: %d\n", first_good_beam);
  fprintf(output_file, "First good beam az: %f\n",
	  (first_good_beam * field_hdr.grid_dy) + min_y);

  return true;
}
