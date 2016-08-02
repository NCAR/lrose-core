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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Terascan2Mdv.cc: Terascan2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

/*************************************************************************

  Terascan Functions called:
	cmdform		filename	fmtout		gpclone
	gpclonedim	gpclose		gpdefvar	gpdim
	gpgetvar	gphistprms	gpopen		gpputvar
	gpvar		msgout		prmreal		prmint
	
*************************************************************************/

#include <new>

#include <cassert>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <sys/wait.h>

// Include the needed Terascan include files.  Note that they were
// written assuming that people would only write in C, not in C++.

#ifdef __cplusplus
extern "C" {
#endif

#include <terrno.h>
#include <gp.h>
#include <uif.h>
#include <hist.h>
#include <etx.h>
#include <cdfnames.h>
#include <etnames.h>

#ifdef __cplusplus
}
#endif

#include <toolsa/os_config.h>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/InputDir.hh>
#include <toolsa/pjg.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>


#include "Terascan2Mdv.hh"
#include "Params.hh"

#include "FlatLookupTable.hh"

#include "DataProcessor.hh"
#include "ByteDataProcessor.hh"
#include "ShortDataProcessor.hh"
#include "FloatDataProcessor.hh"

#include "FieldProcessor.hh"
#include "FlatFieldProcessor.hh"
#include "LatlonFieldProcessor.hh"

using namespace std;


// Global variables

Terascan2Mdv *Terascan2Mdv::_instance =
     (Terascan2Mdv *)NULL;



/*********************************************************************
 * Constructor
 */

Terascan2Mdv::Terascan2Mdv(int argc, char **argv)
{
  const string method_name = "Terascan2Mdv::Terascan2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Terascan2Mdv *)NULL);
  
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

  if(!(_args = new(nothrow) Args(argc, argv, _progName))) {
    cerr << "new failed" << endl;
    okay = false;
    return;
  }

  // Get TDRP parameters.

  if(!(_params = new(nothrow) Params())) {
    cerr << "new failed" << endl;
    okay = false;
    return;
  }

  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

}


/*********************************************************************
 * Destructor
 */

Terascan2Mdv::~Terascan2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Terascan2Mdv *Terascan2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Terascan2Mdv *)NULL) {
    if(!(new(nothrow) Terascan2Mdv(argc, argv))) {
      return 0;
    }
  }

  return _instance;
}

Terascan2Mdv *Terascan2Mdv::Inst()
{
  assert(_instance != (Terascan2Mdv *)NULL);
  
  return _instance;
}


/*********************************************************************
 * run() - run the program.
 */

void Terascan2Mdv::run()
{
  string input_file_path = _args->getInputFilePath();
  string output_file_path = _args->getOutputFilePath();


  //
  
  if (input_file_path == "")
  {
    switch (_params->mode)
    {
    case Params::ARCHIVE :
      _runArchiveMode();
      break;
      
    case Params::REALTIME :
      _runRealtimeMode();
      break;
    } /* endswitch - _params->mode */
    
  }
  else
  {
    _processDataset(input_file_path, output_file_path);
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getDataTimeFromFile() - Get the data time from the satellite file.
 */

DateTime Terascan2Mdv::_getDataTimeFromDataset(SETP dataset)
{
  const string method_name = "Terascan2Mdv::_getDataTimeFromDataset()";
  
  DateTime data_time;
  
  /* Use the image time within the file */

  int date;
  double time;
    
  int year, month, day, hour, min, sec;
  int ts;
    
  if (gpgetatt(dataset, C_PASSDATE, (char *) &date) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting date from dataset" << endl;
    msgout(terrno);
    
    return DateTime::NEVER;
  }

  if (gpgetatt(dataset, C_STARTTIME, (char *) &time) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting time from dataset" << endl;
    msgout(terrno);
    
    return DateTime::NEVER;
  }

  stdymd( date, &year, &month, &day);

  ts = (int)time;

  hour = ts / 3600;
  min = (ts - hour*3600) / 60;
  sec = (ts - hour*3600 - min*60);

  data_time.set(year, month, day, hour, min, sec);

  return data_time;
}


/*********************************************************************
 * _getDataTimeFromFilename() - Get the data time from the input filename.
 */

DateTime Terascan2Mdv::_getDataTimeFromFilename(const char *input_filename) const
{
  const string method_name = "Terascan2Mdv::_getDataTimeFromFilename()";
  
  DateTime data_time;
  
  switch (_params->input_style)
  {
  case Params::FLAT_DIRECTORY :
  {
    int year, jday, hour, min;
  
    if (sscanf(input_filename,
	       "%*2c.%02d%03d.%02d%02d",
	       &year, &jday, &hour, &min) == 4)
    {
      if (year > 50)
	year += 1900;
      else
	year += 2000;
    }
    else if (sscanf(input_filename,
		    "%*3c.%02d%03d.%02d%02d",
		    &year, &jday, &hour, &min) == 4)
    {
      if (year > 50)
	year += 1900;
      else
	year += 2000;
    }
    else if (sscanf(input_filename,
		    "%*2c.%03d%03d.%02d%02d",
		    &year, &jday, &hour, &min) == 4)
    {
      year += 2000;
    }
    else if (sscanf(input_filename,
		    "%*3c.%03d%03d.%02d%02d",
		    &year, &jday, &hour, &min) == 4)
    {
      year += 2000;
    }
    else if (sscanf(input_filename,
		    "%*2c.%04d%03d.%02d%02d",
		    &year, &jday, &hour, &min) == 4)
    {
      // Do nothing -- the year is fully specified
    }
    else if (sscanf(input_filename,
		    "%*3c.%04d%03d.%02d%02d",
		    &year, &jday, &hour, &min) == 4)
    {
      // Do nothing -- the year is fully specified
    }
    else
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Unable to get data time from file name: " << input_filename << endl;
      cerr << "Skipping file" << endl;
      
      return DateTime::NEVER;
    }
  
    data_time.setByDayOfYear(year, jday, hour, min);
  }
  break;
  
  case Params::RAP_DIRECTORY :
  {
    time_t file_time;
    
    if (DsInputPath::getDataTime(input_filename, file_time) != 0)
      return DateTime::NEVER;
    
    data_time.set(file_time);
  }
  break;
  } /* endswitch - _params->input_style */
  
  return data_time;
}


/*********************************************************************
 * _processDataset() - Process the given dataset.
 */

bool Terascan2Mdv::_processDataset(const string &input_filename,
				   const string &output_filename)
{
  const string method_name = "Terascan2Mdv::_processDataset()";
  
  string pmu_message;
  
  int childStatus; // keep track of the child's exit status

  if (_params->debug)
  {
    cerr << "*** Processing dataset: " << input_filename << endl;
    if (output_filename != "")
      cerr << "    Output filename: " << output_filename << endl;
  }
  
  pmu_message = "Processing file: " + input_filename;
  PMU_auto_register(pmu_message.c_str());
  
  // fork child to do the work

  _pid = fork();

  // check for parent or child
  if (_pid == 0)
  {
    // child processing
    bool status = _childsPlay(input_filename, output_filename);

    if (status)
    {   
      _exit(0); // normal exit point for child
    }
    else
    {
      _exit(1); // normal exit point for child      
    }

  }
  else
  {
    // parent processing

    if (waitpid(_pid, &childStatus, (int) WUNTRACED))
    {
      if (_params->debug)
        cerr << "  pid done: " << _pid << endl;
    }
  } // endif -- _pid == 0
 
  int exitStatus = 1;

  if (WIFEXITED(childStatus) != 0) 
  {
    exitStatus = WEXITSTATUS(childStatus);
  }
  else 
  {
#ifdef WCOREDUMP 
    if (WCOREDUMP(childStatus) != 0)
    {
      if (_params->debug)
        cerr << "*** The child dumped core ***" << endl;
    }
#endif // WCOREDUMP   

  }

  if (exitStatus == 0) 
  {
    return true;
  }
  else
  {
    return false;
  }
  
}

/*********************************************************************
 * _childsPlay() - Process the given dataset.
 */

bool Terascan2Mdv::_childsPlay(const string &input_filename,
			       const string &output_filename)
{
  const string method_name = "Terascan2Mdv::_childsPlay()";

  string pmu_message;
  
  SETP input_dataset;              /* input dataset pointers */
  
  char *namestring;
  //  ETXFORM mxfm;
  //  double lat,lon, lat_0, lon_0, lat_c, lon_c, x0, y0;
  //  double delta_lat, delta_lon, dx, dy, delta_x, delta_y, xgrid, ygrid;

  //  int iret;

  DsMdvx sat_mdv;

  /* Open input dataset (gpopen(3)). */
  if ((input_dataset = gpopen((char *)input_filename.c_str(), GP_READ))
      == NULL) 
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening dataset: " << input_filename << endl;
      msgout(terrno);
    
      return false;
    }

  if (_params->debug)
    cerr << "Dataset successfully opened" << endl;

  // Create and initialize the field processor object

  FieldProcessor *field_processor;
  
  switch (_params->projection)
    {
    case Params::FLAT :
      if(!(field_processor = new(nothrow) FlatFieldProcessor(input_dataset,
							     _params->lookup_file_name))) {
	cerr << "new failed" << endl;
      }

      break;
    
    case Params::LATLON :
      if(!(field_processor = new(nothrow) LatlonFieldProcessor(input_dataset))) {
	cerr << "new failed" << endl;
      }

      break;
    } /* endswitch - _params->projection */
  
  // Calculate the data time

  DateTime data_time;
  
  if (_params->time_flag == TRUE )
    {
      switch (_params->input_style)
	{
	case Params::FLAT_DIRECTORY :
	  {
	    if ( (namestring = strrchr(input_filename.c_str(), '/') )!= NULL )
	      data_time = _getDataTimeFromFilename(namestring+1);
	    else
	      data_time = _getDataTimeFromFilename(input_filename.c_str());
	  }
	break;
    
	case Params::RAP_DIRECTORY :
	  {
	    time_t data_utime;
      
	    if (DsInputPath::getDataTime(input_filename, data_utime) == 0)
	      data_time.set(data_utime);
	    else
	      data_time = DateTime::NEVER;
	  }
	break;
	} /* endswitch - _params->input_style */
    
      if (data_time == DateTime::NEVER)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error converting filename to a data time: "
	       << input_filename << endl;
	  cerr << "Skipping file..." << endl;
      
	  /* close the dataset, then return */
	  
	  gpclose(input_dataset);
	  return false;
	}
    
    }
  else
    {
      data_time = _getDataTimeFromDataset(input_dataset);
    }

  /* close the dataset */

  
  if (_params->debug)
    cerr << "Dataset time: " << data_time << endl;
  
  /* Fill in the MDV master header */

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.max_nx = field_processor->getNumSamples();
  master_hdr.max_ny = field_processor->getNumLines();
  master_hdr.max_nz = 1;
  /* This is the center of the grid */
  master_hdr.sensor_lat = field_processor->getCenterLat();
  master_hdr.sensor_lon = field_processor->getCenterLon();
  master_hdr.sensor_alt = 0.0;

  master_hdr.time_gen = data_time.utime();
  master_hdr.user_time= master_hdr.time_gen;
  master_hdr.time_begin = master_hdr.time_gen;
  master_hdr.time_end = master_hdr.time_gen;
  master_hdr.time_centroid = master_hdr.time_gen;
  master_hdr.time_expire = master_hdr.time_gen + _params->valid_duration;
  master_hdr.num_data_times = 1;

  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;

  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  master_hdr.n_fields = 0;
  master_hdr.field_grids_differ = FALSE;
  master_hdr.index_number = 0;
  master_hdr.n_chunks = 0;

  strncpy(master_hdr.data_set_source,
	  input_filename.c_str(), strlen(input_filename.c_str()));

  sat_mdv.setMasterHeader(master_hdr);
  
  // Get the requested field from the file

  for (int var = 0; var < _params->grids_n; ++var)
    {
      pmu_message = "Processing field: " + input_filename + "::" +
	_params->_grids[var].name;
      PMU_auto_register(pmu_message.c_str());
    
      if (_params->debug)
	cerr << pmu_message << endl;
    
      MdvxField *output_field =
	field_processor->createField(_params->_grids[var].name,
				     _params->_grids[var].field_name,
				     _params->_grids[var].field_units,
				     _params->_grids[var].field_code,
				     _params->scaling_type,
				     _params->_grids[var].scale,
				     _params->_grids[var].offset);

      if (output_field == 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error processing field: " << _params->_grids[var].name << endl;
	  cerr << "Skipping field..." << endl;
	}
      else
	// output_field will be freed by the Mdvx object
	sat_mdv.addField(output_field);

    } /* endfor - var */

  /* Now write the data to the MDV file */
  sat_mdv.setWriteLdataInfo();
  
  gpclose(input_dataset);

  if (output_filename == "")
    {
      if (_params->debug)
	cerr << "*** Writing MDV file to URL: " << _params->output_url << endl;
      
      if (sat_mdv.writeToDir(_params->output_url) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error writing to output directory: " << _params->output_url
	       << endl;
    
	  return false;
	}
    }
  else
    {
      if (_params->debug)
	cerr << "*** Writing MDV file to path: " << output_filename << endl;
      
      if (sat_mdv.writeToPath(output_filename.c_str()) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error writing to output path: " << output_filename << endl;
      
	  return false;
	}
    }
  
  // Reclaim memory

  delete field_processor;

  return true;

}

/*********************************************************************
 * _runArchiveMode() - Run in archive mode.
 */

void Terascan2Mdv::_runArchiveMode()
{
  const string method_name = "Terascan2Mdv::_runArchiveMode()";
  
  switch (_params->input_style)
  {
  case Params::FLAT_DIRECTORY :
  {
    InputDir input_dir(_params->input_dir,
		       "",
		       1);

    char *input_filename;
      
    while ((input_filename = input_dir.getNextFilename(0)) != 0)
    {
      _processDataset(input_filename);

      delete input_filename;
    }
    
  }
  break;
  
  case Params::RAP_DIRECTORY :
  {
    time_t start_time = _args->getStartTime();
    time_t end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify -start and -end on command line to run with input from RAP directories in archive mode" << endl;
      
      return;
    }
    
    DsInputPath input_path(_progName,
			   _params->debug,
			   _params->input_dir,
			   start_time, end_time);

    char *input_filename;
    
    while ((input_filename = input_path.next()) != 0)
      _processDataset(input_filename);
  }
  break;
  } /* endswitch - _params->input_style */
  
}


/*********************************************************************
 * _runRealtimeMode() - Run in realtime mode.
 */

void Terascan2Mdv::_runRealtimeMode()
{
  switch (_params->input_style)
  {
  case Params::FLAT_DIRECTORY :
  {
    InputDir input_dir(_params->input_dir,
		       "",
		       0);
      
    char *input_filename;
      
    while (true)
    {
      if ((input_filename = input_dir.getNextFilename(1)) != 0)
      {
	_waitForStableFile(input_filename);
	PMU_auto_register("Starting process delay.");
	for (time_t wait_interval = _params->process_delay; wait_interval <= 0; wait_interval--) 
	{
	  sleep(1);
	  PMU_auto_register("In process delay.");
	}
	_processDataset(input_filename);
      }

      PMU_auto_register("Waiting for data");
      
      sleep(5);
    }
  }
  break;
  
  case Params::RAP_DIRECTORY :
  {
    DsInputPath input_path(_progName,
			   _params->debug,
			   _params->input_dir,
			   -1,
			   PMU_auto_register);
    
    char *input_filename;
    
    while ((input_filename = input_path.next()) != 0)
      _processDataset(input_filename);
  }
  break;
  } /* endswitch - _params->input_style */
  
}


/*********************************************************************
 * _waitForStableFile() - Wait for the given file to become stable
 *                        (i.e. for the file writing to finish).
 */

void Terascan2Mdv::_waitForStableFile(const string &filepath)
{
  const string method_name = "Terascan2Mdv::_waitForStableFile()";
  
  struct stat prev_file_stat;
  struct stat curr_file_stat;
  
  if (stat(filepath.c_str(), &prev_file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error stating file: " << filepath << endl;
    
    return;
  }
  
  while (true)
  {
    sleep(1);
    
    if (stat(filepath.c_str(), &curr_file_stat) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error stating file: " << filepath << endl;
      
      return;
    }
    
    if (curr_file_stat.st_size == prev_file_stat.st_size)
      break;
    
    prev_file_stat = curr_file_stat;
    
  } /* endwhile -- true */
  
}
