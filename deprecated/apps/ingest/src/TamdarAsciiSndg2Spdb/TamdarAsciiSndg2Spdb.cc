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
//   $Date: 2016/03/07 01:23:06 $
//   $Id: TamdarAsciiSndg2Spdb.cc,v 1.4 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TamdarAsciiSndg2Spdb: TamdarAsciiSndg2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <rapformats/Sndg.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "TamdarAsciiSndg2Spdb.hh"
#include "Params.hh"

#include "AncAsciiReader.hh"
#include "RegularAsciiReader.hh"

#include "PlusSpdbWriter.hh"
#include "SndgSpdbWriter.hh"

using namespace std;


// Global variables

TamdarAsciiSndg2Spdb *TamdarAsciiSndg2Spdb::_instance =
     (TamdarAsciiSndg2Spdb *)NULL;



/*********************************************************************
 * Constructor
 */

TamdarAsciiSndg2Spdb::TamdarAsciiSndg2Spdb(int argc, char **argv) :
  _dataTrigger(0),
  _reader(0),
  _writer(0)
{
  static const string method_name = "TamdarAsciiSndg2Spdb::TamdarAsciiSndg2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TamdarAsciiSndg2Spdb *)NULL);
  
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

TamdarAsciiSndg2Spdb::~TamdarAsciiSndg2Spdb()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _reader;
  delete _writer;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TamdarAsciiSndg2Spdb *TamdarAsciiSndg2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (TamdarAsciiSndg2Spdb *)NULL)
    new TamdarAsciiSndg2Spdb(argc, argv);
  
  return(_instance);
}

TamdarAsciiSndg2Spdb *TamdarAsciiSndg2Spdb::Inst()
{
  assert(_instance != (TamdarAsciiSndg2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TamdarAsciiSndg2Spdb::init()
{
  static const string method_name = "TamdarAsciiSndg2Spdb::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::REALTIME :
  {
    if (_params->debug)
      cerr << "Initializing REALTIME trigger using directory: " <<
	_params->input_dir << endl;
    
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      _params->process_old_files,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing REALTIME trigger using directory: " <<
	_params->input_dir << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    if (_params->debug)
      cerr << "Successfully initialized REALTIME trigger." << endl;
    
    break;
  }
  
  case Params::ARCHIVE :
  {
    const vector< string > file_list = _args->getInputFileList();

    if (_params->debug)
    {
      cerr << "Initializing ARCHIVE trigger with files: " << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "    " << *file << endl;
    }
    
    DsFileListTrigger *trigger = new DsFileListTrigger();
    
    if (trigger->init(file_list) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing ARCHIVE trigger. " << endl;
      cerr << "File list:" << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "    " << *file << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    if (_params->debug)
      cerr << "Successfully initialized ARCHIVE trigger." << endl;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the ASCII reader object.

  switch (_params->input_format)
  {
  case Params::REGULAR_FORMAT :
    _reader = new RegularAsciiReader(_params->debug);
    break;
    
  case Params::ANC_FORMAT :
    _reader = new AncAsciiReader(_params->debug);
    break;
  } /* endswitch - _params->output_format */
  
  // Initialize the SPDB writer object

  switch (_params->output_format)
  {
  case Params::SNDG_FORMAT :
    _writer = new SndgSpdbWriter(_params->output_url,
				 _params->output_expire_secs,
				 _params->debug);
    break;

  case Params::SNDG_PLUS_FORMAT :
    _writer = new PlusSpdbWriter(_params->output_url,
				 _params->output_expire_secs,
				 _params->debug);
    break;
  } /* endswitch - _params->output_format */
  
  // initialize process registration

  if (_params->trigger_mode == Params::REALTIME)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void TamdarAsciiSndg2Spdb::run()
{
  static const string method_name = "TamdarAsciiSndg2Spdb::run()";
  
  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getFilePath()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file: " << trigger_info.getFilePath() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool TamdarAsciiSndg2Spdb::_processData(const string &input_file_path)
{
  static const string method_name = "TamdarAsciiSndg2Spdb::_processData()";
  
  PMU_auto_register("Processing file...");
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing file: " << input_file_path << endl;

  // Open the input file

  if (!_reader->openFile(input_file_path))
    return false;

  // Process all of the soundings from the input file

  Sndg sounding;
  
  while (_reader->getNextSndg(sounding))
  {
    if (_params->debug)
    {
      cerr << "Read sounding:" << endl;
      sounding.print(cerr, "   ");
    }
    
    // Make sure the sounding has enough points

    if (sounding.getNumPoints() < _params->min_sounding_layers)
    {
      if (_params->debug)
	cerr << "Skipping sounding with only " << sounding.getNumPoints()
	     << " layers" << endl;
	
      continue;
    }
    
    // Set the sounding location

    _setSoundingLocation(sounding);
    
    // Sort the sounding points, if requested

    if (_params->sort_points_on_output)
    {
      _sortPoints(sounding);
      
      if (_params->debug)
      {
	cerr << "Returned sorted points:" << endl;
	
	vector< Sndg::point_t > points = sounding.getPoints();
	vector< Sndg::point_t >::const_iterator point;
	
	for (point = points.begin(); point != points.end(); ++point)
	  cerr << "    " << point->pressure << endl;
      }
    }
    
    _writer->writeSndg(sounding);

    sounding.clearPoints();
  }
  
  return true;
}


/*********************************************************************
 * _setSoundingLocation() - Set the overall sounding location using the
 *                          method indicated by the user.
 */

void TamdarAsciiSndg2Spdb::_setSoundingLocation(Sndg &sounding) const
{
  Sndg::header_t sndg_hdr = sounding.getHeader();
  
  switch (_params->sounding_location)
  {
  case Params::FIRST_POINT_IN_TIME :
  {
    vector< Sndg::point_t > points = sounding.getPoints();
    
    sndg_hdr.lat = points.begin()->latitude;
    sndg_hdr.lon = points.begin()->longitude;
    
    break;
  }
  
  case Params::LAST_POINT_IN_TIME :
  {
    vector< Sndg::point_t > points = sounding.getPoints();
    vector< Sndg::point_t >::const_iterator point = points.end();
    point--;
    
    sndg_hdr.lat = point->latitude;
    sndg_hdr.lon = point->longitude;
    
    break;
  }
  
  case Params::AVERAGE :
  {
    double lat_total = 0.0;
    double lon_total = 0.0;
    double num_pts = 0.0;
    
    vector< Sndg::point_t > points = sounding.getPoints();
    vector< Sndg::point_t >::const_iterator point;
    
    for (point = points.begin(); point != points.end(); ++point)
    {
      lat_total += point->latitude;
      lon_total += point->longitude;
      num_pts += 1.0;
    } /* endfor - point */
    
    if (num_pts == 0.0)
    {
      sndg_hdr.lat = 0.0;
      sndg_hdr.lon = 0.0;
    }
    else
    {
      sndg_hdr.lat = lat_total / num_pts;
      sndg_hdr.lon = lon_total / num_pts;
    }
    
    break;
  }
  
  case Params::MIN_PRESSURE_POINT :
  {
    vector< Sndg::point_t > points = sounding.getPoints();
    vector< Sndg::point_t >::const_iterator point;
    
    double min_pressure = -1.0;
    
    for (point = points.begin(); point != points.end(); ++point)
    {
      if (min_pressure < 0.0 ||
	  point->pressure < min_pressure)
      {
	min_pressure = point->pressure;
	sndg_hdr.lat = point->latitude;
	sndg_hdr.lon = point->longitude;
      }
      
    } /* endfor - point */
    
    break;
  }
  
  case Params::MAX_PRESSURE_POINT :
  {
    vector< Sndg::point_t > points = sounding.getPoints();
    vector< Sndg::point_t >::const_iterator point;
    
    double max_pressure = -1.0;
    
    for (point = points.begin(); point != points.end(); ++point)
    {
      if (max_pressure < 0.0 ||
	  point->pressure > max_pressure)
      {
	max_pressure = point->pressure;
	sndg_hdr.lat = point->latitude;
	sndg_hdr.lon = point->longitude;
      }
      
    } /* endfor - point */
    
    break;
  }
  
  } /* endswitch - _params->sounding_location */
  
  sounding.setHeader(sndg_hdr);
}


/*********************************************************************
 * _sortPoints() - Sort the points in the sounding by pressure.
 */

void TamdarAsciiSndg2Spdb::_sortPoints(Sndg &sounding) const
{
  vector< Sndg::point_t > points = sounding.getPoints();
  vector< Sndg::point_t > sorted_points;
  
  vector< Sndg::point_t >::const_iterator point;
  
  if (_params->debug)
    cerr << "Unsorted points:" << endl;
  
  for (point = points.begin(); point != points.end(); ++point)
  {
    if (_params->debug)
      cerr << "    " << point->pressure << endl;
    
    vector< Sndg::point_t >::iterator sorted_point;
    
    for (sorted_point = sorted_points.begin();
	 sorted_point != sorted_points.end(); ++ sorted_point)
    {
      if (sorted_point->pressure <= point->pressure)
	break;
    } /* endfor - sorted_point */
    
    sorted_points.insert(sorted_point, *point);
    
  } /* endfor - point */

  if (_params->debug)
  {
    cerr << "Sorted points:" << endl;
    vector< Sndg::point_t >::const_iterator sorted_point;
    for (sorted_point = sorted_points.begin();
	 sorted_point != sorted_points.end(); ++sorted_point)
      cerr << "    " << sorted_point->pressure << endl;
  }
  
  sounding.setPoints(sorted_points);
}
