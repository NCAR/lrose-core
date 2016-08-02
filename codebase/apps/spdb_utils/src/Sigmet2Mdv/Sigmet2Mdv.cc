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
 * @file Sigmet2Mdv.cc
 *
 * @class Sigmet2Mdv
 *
 * Sigmet2Mdv program object.
 *  
 * @date 6/7/2010
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <euclid/geometry.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Sigmet2Mdv.hh"
#include "Params.hh"

using namespace std;

// Global variables

Sigmet2Mdv *Sigmet2Mdv::_instance =
     (Sigmet2Mdv *)NULL;


/*********************************************************************
 * Constructors
 */

Sigmet2Mdv::Sigmet2Mdv(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "Sigmet2Mdv::Sigmet2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Sigmet2Mdv *)NULL);
  
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
  char *params_path = new char[strlen("unknown") + 1];
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
}


/*********************************************************************
 * Destructor
 */

Sigmet2Mdv::~Sigmet2Mdv()
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

Sigmet2Mdv *Sigmet2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Sigmet2Mdv *)NULL)
    new Sigmet2Mdv(argc, argv);
  
  return(_instance);
}

Sigmet2Mdv *Sigmet2Mdv::Inst()
{
  assert(_instance != (Sigmet2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool Sigmet2Mdv::init()
{
  static const string method_name = "Sigmet2Mdv::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
 
  // Initialize the output projection

  if (!_initOutputProj())
    return false;
 
  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void Sigmet2Mdv::run()
{
  static const string method_name = "Sigmet2Mdv::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    _processData(trigger_info.getIssueTime());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addSigmetToGrid()
 */

void Sigmet2Mdv::_addSigmetToGrid(const SigAirMet &sigmet,
				  ui08 *grid) const
{
  static const string method_name = "Sigmet2Mdv;:_addSigmetToGrid()";

  // Put the vertices into the format needed for the filling method

  const vector< sigairmet_vertex_t > vertices = sigmet.getVertices();
  
  int num_pts = vertices.size();
  Point_d *pts = new Point_d[num_pts];
  
  for (int i = 0; i < num_pts; ++i)
  {
    double normal_lon = vertices[i].lon;
    
    if (_outputProj.getProjType() == Mdvx::PROJ_LATLON)
    {
      double min_lon = _outputProj.getMinx();
      double max_lon = min_lon + 360.0;
      
      while (normal_lon < min_lon)
	normal_lon += 360.0;
      while (normal_lon >= max_lon)
	normal_lon -= 360.0;
    }
    
    _outputProj.latlon2xy(vertices[i].lat, normal_lon,
			  pts[i].x, pts[i].y);
  } /* endfor - i */
  
  // Call the filling method

  int num_pts_filled = EG_fill_polygon(pts, num_pts,
				       _outputProj.getNx(),
				       _outputProj.getNy(),
				       _outputProj.getMinx(),
				       _outputProj.getMiny(),
				       _outputProj.getDx(),
				       _outputProj.getDy(),
				       grid,
				       1);

  if (_params->verbose)
    cerr << "   Added " << num_pts_filled << " squares to output grid" << endl;
  
  // Reclaim memory

  delete [] pts;
}


/*********************************************************************
 * _createBlankSigmetField()
 */

MdvxField *Sigmet2Mdv::_createBlankSigmetField() const
{
  static const string method_name = "Sigmet2Mdv;:_createBlankSigmetField()";
  
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  _outputProj.syncToFieldHdr(field_hdr);
  
  field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  field_hdr.data_element_nbytes = 1;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = 255;
  field_hdr.missing_data_value = 255;
  STRcopy(field_hdr.field_name_long, "SIGMET flag", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "sigmets", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "none", MDV_UNITS_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  // Create the field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0);
}


/*********************************************************************
 * _createSigmetField()
 */

MdvxField *Sigmet2Mdv::_createSigmetField(const vector< SigAirMet > &sigmets) const
{
  static const string method_name = "Sigmet2Mdv;:_createSigmetField()";
  
  // Create the blank SIGMET field

  MdvxField *sigmet_field;
  
  if ((sigmet_field = _createBlankSigmetField()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank MDV field" << endl;
    
    return 0;
  }
  
  // Initialize the grid values at 0

  Mdvx::field_header_t field_hdr = sigmet_field->getFieldHeader();
  ui08 *grid = (ui08 *)sigmet_field->getVol();
  memset(grid, 0, field_hdr.nx * field_hdr.ny * sizeof(ui08));
  
  // Add each SIGMET to the gric

  vector< SigAirMet >::const_iterator sigmet;
  
  for (sigmet = sigmets.begin(); sigmet != sigmets.end(); ++sigmet)
  {
    if (_params->verbose)
      sigmet->print(cerr);
    
    if (_params->debug)
      cerr << "Adding SIGMET to grid..." << endl;
    
    if (sigmet->getNVertices() == 0)
    {
      if (_params->debug)
	cerr << "   Skipping SIGMET with no vertices" << endl;
      
      continue;
    }
    
    _addSigmetToGrid(*sigmet, grid);
    
  } /* endfor - sigmet */
  
  return sigmet_field;
}


/*********************************************************************
 * _initOutputProj()
 */

bool Sigmet2Mdv::_initOutputProj()
{
  static const string method_name = "Sigmet2Mdv;:_initOutputProj()";
  
  switch (_params->output_proj.proj_type)
  {
  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params->output_proj.nx,
			   _params->output_proj.ny,
			   1,
			   _params->output_proj.dx,
			   _params->output_proj.dy,
			   1.0,
			   _params->output_proj.minx,
			   _params->output_proj.miny,
			   0.0);
    break;
    
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params->output_proj.origin_lat,
			 _params->output_proj.origin_lon,
			 _params->output_proj.rotation,
			 _params->output_proj.nx,
			 _params->output_proj.ny,
			 1,
			 _params->output_proj.dx,
			 _params->output_proj.dy,
			 1.0,
			 _params->output_proj.minx,
			 _params->output_proj.miny,
			 0.0);
    break;
  }
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool Sigmet2Mdv::_initTrigger()
{
  static const string method_name = "Sigmet2Mdv;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   max valid secs: "
	   << _params->max_valid_secs << endl;
      
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
      cerr << "In TIME_LIST mode, start and end times must be included on the command line." << endl;
      cerr << "Update command line and try again." << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
      
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

bool Sigmet2Mdv::_processData(const DateTime &trigger_time)
{
  static const string method_name = "Sigmet2Mdv::_processData()";
  
  PMU_auto_register("Processing data...");

  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the SIGMETs from the database

  vector< SigAirMet > sigmets;
  
  if (!_readSigmets(trigger_time, sigmets))
    return false;
  
  // Create the gridded field

  MdvxField *sigmet_field;

  if ((sigmet_field = _createSigmetField(sigmets)) == 0)
    return false;
  
  // Write the output file

  if (!_writeOutputFile(sigmet_field, trigger_time))
    return false;
  
  return true;
}


/*********************************************************************
 * _readSigmets()
 */

bool Sigmet2Mdv::_readSigmets(const DateTime &data_time,
			      vector< SigAirMet > & sigmets) const
{
  static const string method_name = "Sigmet2Mdv::_readSigmets()";
  
  // Read the data from the database

  DsSpdb spdb;
  
  if (spdb.getExact(_params->input_url,
		    data_time.utime()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading chunks from SPDB database:" << endl;
    cerr << "   url: " << _params->input_url << endl;
    cerr << "   request time: " << data_time << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "---> Read " << spdb.getNChunks()
	 << " chunks from database" << endl;
  
  // Convert the database chunks into SIGMET objects

  const vector< Spdb::chunk_t > chunks = spdb.getChunks();
  vector< Spdb::chunk_t >::const_iterator chunk;
  
  for (chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
  {
    SigAirMet sigmet;
    
    if (sigmet.disassemble(chunk->data, chunk->len) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error disassembling chunk into SigAirMet object" << endl;
      cerr << "--- Skipping chunk ---" << endl;
      
      continue;
    }
    
    // Add the sigmet to the return lise

    sigmets.push_back(sigmet);
    
  } /* endfor - chunk */
  
  return true;
}


/*********************************************************************
 * _writeOutputFile()
 */

bool Sigmet2Mdv::_writeOutputFile(MdvxField *sigmet_field,
				  const DateTime &data_time) const
{
  static const string method_name = "Sigmet2Mdv::_writeOutputFile()";
  
  // Create the output file

  DsMdvx output_file;
  
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time.utime();
  master_hdr.time_end = data_time.utime();
  master_hdr.time_centroid = data_time.utime();
  master_hdr.time_expire = data_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, "Gridded SIGMETs from Sigmet2Mdv",
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Sigmet2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->input_url, MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
  
  // Compress the field and add it to the file

  sigmet_field->compress(Mdvx::COMPRESSION_RLE);
  
  output_file.addField(sigmet_field);
  
  // Write the output file

  if (_params->debug)
    cerr << "---> Writing output file to URL: " << _params->output_url << endl;
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing MDV file to url: " << _params->output_url << endl;
    cerr << output_file.getErrStr();
    
    return false;
  }
			    
  return true;
}
