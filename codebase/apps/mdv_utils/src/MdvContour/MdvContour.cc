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
//   $Id: MdvContour.cc,v 1.7 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvContour: MdvContour program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <contour/BinarySmoother.hh>
#include <contour/Contour.hh>
#include <contour/DouglasPeuckerSmoother.hh>
#include <contour/SimpleBoundaryContourAlg.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapformats/GenPoly.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "MdvContour.hh"
#include "Params.hh"

using namespace std;


// Global variables

MdvContour *MdvContour::_instance =
     (MdvContour *)NULL;


/*********************************************************************
 * Constructor
 */

MdvContour::MdvContour(int argc, char **argv) :
  _dataTrigger(0),
  _contourAlg(0),
  _smoother(0)
{
  static const string method_name = "MdvContour::MdvContour()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvContour *)NULL);
  
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

MdvContour::~MdvContour()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _contourAlg;
  delete _smoother;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvContour *MdvContour::Inst(int argc, char **argv)
{
  if (_instance == (MdvContour *)NULL)
    new MdvContour(argc, argv);
  
  return(_instance);
}

MdvContour *MdvContour::Inst()
{
  assert(_instance != (MdvContour *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvContour::init()
{
  static const string method_name = "MdvContour::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
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
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
      
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
  } /* endswitch - _params->trigger_mode */

  // Create the contour algorithm object

  switch (_params->contour_alg_type)
  {
  case Params::SIMPLE_BOUNDARY_CONTOUR_ALG :
    _contourAlg =
      new SimpleBoundaryContourAlg(_params->simple_bdry_alg_params.min_overlap,
				   _params->simple_bdry_alg_params.min_num_poly_pts,
				   SimpleBoundaryContourAlg::CENTER_REF,
				   _params->debug);
    break;

  }
  
  // Create the contour smoother algorithm object

  if (_params->smooth_contours)
  {
    switch (_params->smoother_type)
    {
    case Params::DOUGLAS_PEUCKER_SMOOTHER :
      _smoother =
	new DouglasPeuckerSmoother(_params->douglas_peucker_params.epsilon,
				   _params->debug);
      break;

    case Params::BINARY_SMOOTHER :
      _smoother = new BinarySmoother(_params->binary_params.max_num_pts,
				     _params->debug);
      break;
    } /* endswitch - _params->smoother_type */
  }

  // Save the specified contour level.  We put it into a vector with a
  // single level since the contouring algorithms take a vector

  _contourLevels.push_back(_params->contour_level);
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvContour::run()
{
  static const string method_name = "MdvContour::run()";
  
  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " 
	   << DateTime(trigger_info.getIssueTime()) << endl;
      
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

bool MdvContour::_processData(TriggerInfo &trigger_info)
{
  static const string method_name = "MdvContour::_processData()";
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " 
	 << DateTime(trigger_info.getIssueTime()) << endl;
  
  // Read in the input file

  DsMdvx input_mdv;
  
  if (!_readMdvFile(input_mdv, trigger_info))
    return false;
  
  // Calculate the contours

  MdvxField *mdv_field = input_mdv.getField(0);
  
  if (mdv_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting field from MDV file" << endl;
    
    return false;
  }
  
  Mdvx::field_header_t field_hdr = mdv_field->getFieldHeader();
  
  Contour *contour =
    _contourAlg->generateContour(field_hdr.nx, field_hdr.ny,
				 field_hdr.grid_dx, field_hdr.grid_dy,
				 field_hdr.grid_minx, field_hdr.grid_miny,
				 _contourLevels,
				 (fl32 *)mdv_field->getVol());
  
  if (contour == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating contour" << endl;
    
    return false;
  }

  // Smooth the contours, if requested

  if (_smoother != 0 && !_smoother->smoothContour(*contour))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error smoothing contour" << endl;
    
    delete contour;
      
    return false;
  }

  // Write the contour to the output database

  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();
  
  if (!_writeContour(*contour,
		     master_hdr.time_centroid, master_hdr.time_end,
		     MdvxPjg(master_hdr, field_hdr),
		     field_hdr.units))
    return false;
  
  delete contour;
  
  return true;
}


/*********************************************************************
 * _readMdvFile() - Read the MDV file for the given time.
 */

bool MdvContour::_readMdvFile(DsMdvx &input_mdv,
			    TriggerInfo &trigger_info) const
{
  static const string method_name = "MdvContour::_readMdvFile()";
  
  // Set up the read request

  if(_params->trigger_mode == Params::FILE_LIST) 
  {
    input_mdv.setReadPath(trigger_info.getFilePath());
  }
  else 
  {
    input_mdv.setReadTime(Mdvx::READ_CLOSEST,
			  _params->input_url,
			  0, trigger_info.getIssueTime());
  }
    

  if (_params->input_field.mdv_field_name[0] == '\0')
    input_mdv.addReadField(_params->input_field.mdv_field_num);
  else
    input_mdv.addReadField(_params->input_field.mdv_field_name);
  
  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);
  
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
  
  return true;
}


/*********************************************************************
 * _writeContour() - Write the given contour to the output database.
 */

bool MdvContour::_writeContour(const Contour &contour,
			       const time_t valid_time,
			       const time_t expire_time,
			       const Pjg &projection,
			       const string &units) const
{
  static const string method_name = "MdvContour::_writeContour()";
  
  // Initialize the database stuff

  DsSpdb database;
  database.clearPutChunks();
  database.setPutMode(Spdb::putModeAdd);
  
  // Convert each polygon in the contour to the output database format

  for (const ContourLevel *contour_level = contour.getFirstLevel();
       contour_level != 0; contour_level = contour.getNextLevel())
  {
    for (const ContourPolyline *polyline = contour_level->getFirstPolyline();
	 polyline != 0; polyline = contour_level->getNextPolyline())
    {
      GenPoly polygon;
      
      const vector< ContourPoint > points = polyline->getPoints();
      vector< ContourPoint >::const_iterator contour_point;
      
      for (contour_point = points.begin(); contour_point != points.end();
	   ++contour_point)
      {
	GenPoly::vertex_t vertex;
	
	projection.xy2latlon(contour_point->getX(), contour_point->getY(),
			     vertex.lat, vertex.lon);
	
	polygon.addVertex(vertex);
	
      } /* endfor - contour_point */
      
      polygon.setTime(valid_time);
      polygon.setExpireTime(expire_time);
    
      polygon.addFieldInfo("contour level", units);
      polygon.addVal(_params->contour_level);
      
      polygon.assemble();
    
      database.addPutChunk(0,
			   valid_time,
			   expire_time,
			   polygon.getBufLen(),
			   polygon.getBufPtr());
      
    } /* endfor - polyline */
    
  } /* endfor - contour_level */
  
  // Write the contours to the output database

  if (_params->debug)
    cerr << "Putting " << database.nPutChunks()
	 << " polygons to database for time "
	 << DateTime::str(valid_time) << endl;
  
  if (database.put(_params->output_url,
		   SPDB_GENERIC_POLYLINE_ID,
		   SPDB_GENERIC_POLYLINE_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error putting polygons to output database: "
	 << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
