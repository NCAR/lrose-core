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
//   $Id: Affirm.cc,v 1.24 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.24 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Affirm.cc: Affirm program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2000
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <vector>

#include <cassert>
#include <signal.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <rapmath/math_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <dsdata/DsLdataTrigger.hh>

#include "Params.hh"
#include "Affirm.hh"
#include "calcNewField.hh"
using namespace std;

// Initialize global variables

Affirm *Affirm::_instance = (Affirm *)NULL;

const double Affirm::MISSING_DATA_VALUE = -999.0;

/*********************************************************************
 * Constructor
 */

Affirm::Affirm(int argc, char **argv) :
  _lastRealtimeFileTime(0)
{
  static char *routine_name = (char*)"Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Affirm *)NULL);
  
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
  char *params_path = (char*)"unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  if (_params->mode == Params::ARCHIVE &&
      (_args->getArchiveStartTime() == 0 ||
       _args->getArchiveEndTime() == 0))
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error in command line" << endl;
    cerr << "-start and -end must be specified on the command line in ARCHIVE mode" << endl;
    
    exit(-1);
  }
  
  _gridDeltaTol = _params->grid_delta_tol;


  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  // set up Ldata trigger object if needed
  if(_params->use_trigger_url)
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if(trigger->init(_params->trigger_url,
                     _params->max_realtime_valid_age,
                     PMU_auto_register,
                     _params->sleep_time) != 0)
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Error initializing REALTIME ldata trigger" << endl;
      cerr << "url = " << _params->trigger_url << endl;
      cerr << "max valid age = " << _params->max_realtime_valid_age << endl;
      exit(-2);
    }
    _trigger = trigger;
  }
}


/*********************************************************************
 * Destructor
 */

Affirm::~Affirm()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;

  if(_params->use_trigger_url)
  {
    delete _trigger;
  }
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Affirm *Affirm::Inst(int argc, char **argv)
{
  if (_instance == (Affirm *)NULL)
    new Affirm(argc, argv);
  
  return(_instance);
}

Affirm *Affirm::Inst()
{
  assert(_instance != (Affirm *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run() - Run the algorithm.
 */

void Affirm::run(void)
{
  switch (_params->mode)
  {
  case Params::ARCHIVE :
    _runArchive();
    break;
    
  case Params::REALTIME :
    _runRealtime();
    break;
  }
  
}

/*********************************************************************
 * _createAffirmField() - Create the affirm field object based on
 *                      the given field object.
 *
 * Returns a pointer to the created field.
 */

MdvxField *Affirm::_createAffirmField(const MdvxField& field) const
{
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  Mdvx::field_header_t new_output_field_hdr;
  
  memset(&new_output_field_hdr, 0, sizeof(new_output_field_hdr));
  
  new_output_field_hdr.forecast_delta = field_hdr.forecast_delta;
  new_output_field_hdr.forecast_time = field_hdr.forecast_time;
  new_output_field_hdr.nx = field_hdr.nx;
  new_output_field_hdr.ny = field_hdr.ny;
  new_output_field_hdr.nz = field_hdr.nz;
  new_output_field_hdr.proj_type = field_hdr.proj_type;
  new_output_field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  new_output_field_hdr.data_element_nbytes = 4;
  new_output_field_hdr.volume_size = new_output_field_hdr.nx * new_output_field_hdr.ny *
      new_output_field_hdr.nz * new_output_field_hdr.data_element_nbytes;
  new_output_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  new_output_field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  new_output_field_hdr.scaling_type = Mdvx::SCALING_NONE;
  new_output_field_hdr.native_vlevel_type = field_hdr.vlevel_type;
  new_output_field_hdr.vlevel_type = field_hdr.vlevel_type;
  new_output_field_hdr.dz_constant = field_hdr.dz_constant;
  
  new_output_field_hdr.proj_origin_lat = field_hdr.proj_origin_lat;
  new_output_field_hdr.proj_origin_lon = field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    new_output_field_hdr.proj_param[i] = field_hdr.proj_param[i];
  new_output_field_hdr.vert_reference = field_hdr.vert_reference;
  new_output_field_hdr.grid_dx = field_hdr.grid_dx;
  new_output_field_hdr.grid_dy = field_hdr.grid_dy;
  new_output_field_hdr.grid_dz = field_hdr.grid_dz;
  new_output_field_hdr.grid_minx = field_hdr.grid_minx;
  new_output_field_hdr.grid_miny = field_hdr.grid_miny;
//  new_output_field_hdr.grid_minz = field_hdr.grid_minz;
  new_output_field_hdr.grid_minz = 1.0;
  new_output_field_hdr.scale = 1.0;
  new_output_field_hdr.bias = 0.0;
  new_output_field_hdr.bad_data_value = MISSING_DATA_VALUE;
  new_output_field_hdr.missing_data_value = MISSING_DATA_VALUE;
  new_output_field_hdr.proj_rotation = field_hdr.proj_rotation;
  
  STRcopy(new_output_field_hdr.field_name_long,
	  _params->long_field_name, MDV_LONG_FIELD_LEN);
  STRcopy(new_output_field_hdr.field_name,
	  _params->short_field_name, MDV_SHORT_FIELD_LEN);
  STRcopy(new_output_field_hdr.units, _params->units, MDV_UNITS_LEN);
  STRcopy(new_output_field_hdr.transform, "Affirm", MDV_TRANSFORM_LEN);
  
  return new MdvxField(new_output_field_hdr,
		       field.getVlevelHeader());
}


/*********************************************************************
 * _processFile() - Process the given MDV file.  Note that because of
 *                  the way the data is read in, we know that the first
 *                  components are found in field 0 and the second components
 *                  are found in field 1.
 */

void Affirm::_processFile(const DsMdvx& input_file) const
{
  const string routine_name = "_processFile()";

  if(_params->debug) {
    cerr << "Processing file: " << input_file.getPathInUse()  << endl;
  }


// Get pointer to the field for convenience.

  MdvxField *field = input_file.getFieldByNum(0);
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  
  fl32 *field_data = (fl32 *)field->getVol();

// Create the output file.
      
  DsMdvx output_file;
      
// Set up the master header
      
  Mdvx::master_header_t master_hdr_in = input_file.getMasterHeader();
  Mdvx::master_header_t master_hdr_out;
      
  memset(&master_hdr_out, 0, sizeof(master_hdr_out));
      
//  master_hdr_out.time_gen = time((time_t *)NULL);
  master_hdr_out.time_gen = master_hdr_in.time_gen;
  master_hdr_out.time_begin = master_hdr_in.time_begin;
  master_hdr_out.time_end = master_hdr_in.time_end;
  if (field_hdr.forecast_time != 0) {
    master_hdr_out.time_centroid = field_hdr.forecast_time;
  } else {
    master_hdr_out.time_centroid = master_hdr_in.time_centroid;
  }
  master_hdr_out.time_expire = master_hdr_in.time_expire;
  master_hdr_out.data_dimension = master_hdr_in.data_dimension;
  master_hdr_out.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr_out.native_vlevel_type = master_hdr_in.vlevel_type;
  master_hdr_out.vlevel_type = master_hdr_in.vlevel_type;
  master_hdr_out.vlevel_included = master_hdr_in.vlevel_included;
  master_hdr_out.grid_orientation = master_hdr_in.grid_orientation;
  master_hdr_out.data_ordering = master_hdr_in.data_ordering;
  master_hdr_out.max_nx = field_hdr.nx;
  master_hdr_out.max_ny = field_hdr.ny;
  master_hdr_out.max_nz = field_hdr.nz;
  master_hdr_out.n_chunks = 0;
  master_hdr_out.field_grids_differ = 0;
      
  master_hdr_out.sensor_lon = master_hdr_in.sensor_lon;
  master_hdr_out.sensor_lat = master_hdr_in.sensor_lat;
  master_hdr_out.sensor_alt = master_hdr_in.sensor_alt;
  
  STRcopy(master_hdr_out.data_set_info,
	  "Produced by Affirm", MDV_INFO_LEN);
  STRcopy(master_hdr_out.data_set_name,
	  "Affirm", MDV_NAME_LEN);
  STRcopy(master_hdr_out.data_set_source,
	  input_file.getPathInUse().c_str(), MDV_NAME_LEN);
      
  output_file.setMasterHeader(master_hdr_out);

// Create the output field.

  MdvxField *new_output_field = _createAffirmField(*field);

  fl32 *affirm = (fl32 *)new_output_field->getVol();

  int plane_size = field_hdr.nx * field_hdr.ny;
  
// Add the input fields to the output file.
      
  MdvxField *field_out = new MdvxField(*field);
      
  field_out->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_RLE);
      
  output_file.addField(field_out);
      
  calcNewField* doCalculation;

  int numb_grids = (int)_params->apply_constant;
  int max_holder = 0;
  
  fl32 average = 0;

//    cout << field_hdr.grid_minx << endl;
//    cout << field_hdr.grid_miny << endl;
//    cout << field_hdr.grid_minz << endl;
//    cout << field_hdr.scale << endl;
//    cout << field_hdr.bias << endl;
//    cout << field_hdr.bad_data_value << endl;
//    cout << field_hdr.missing_data_value << endl;
//    cout << field_hdr.proj_rotation << endl;
//    cout << field_hdr.min_value << endl;
//    cout << field_hdr.max_value << endl;
    
  if(_params->processing_type == Params::LINE_DET_WE)
  {
      for (int z = 0; z < field_hdr.nz; z++)
      {
      	  for (int y = 0; y < field_hdr.ny; y++)
	  {
	      for (int x = 0; x < field_hdr.nx; x++)
	      {
		  int index = x + (field_hdr.nx * y) + (plane_size * z);

		  if(x >= 1 || x < field_hdr.nx - 1 || 
		     y >= 1 || y < field_hdr.ny - 1)
		  {
		      if(field_data[index] > field_data[max_holder] )
		      {
			  max_holder = index;
		      }
		  }
	      }
	      for (int x = 0; x < field_hdr.nx; x++)
	      {
		  int index = x + (field_hdr.nx * y) + (plane_size * z);

		  if(x < 1 || x >= field_hdr.nx - 1 || 
		     y < 1 || y >= field_hdr.ny - 1 ||
		     index != max_holder)
		  {
		      affirm[index] = MISSING_DATA_VALUE;
		      continue;
		  }
		  else 	      
		      affirm[index] = 1.0;
	      }
	      max_holder = 0;
	  }
      }
  }
  else if(_params->processing_type == Params::LINE_DET_SN)
  {
      for (int z = 0; z < field_hdr.nz; z++)
      {
      	  for (int x = 0; x < field_hdr.nx; x++)
	  {
	      for (int y = 0; y < field_hdr.ny; y++)
	      {
		  int index = x + (field_hdr.nx * y) + (plane_size * z);

		  if(x >= 1 || x < field_hdr.nx - 1 || 
		     y >= 1 || y < field_hdr.ny - 1)
		  {
		      if(field_data[index] > field_data[max_holder] )
		      {
			  max_holder = index;
		      }
		  }
	      }
	      for (int y = 0; y < field_hdr.ny; y++)
	      {
		  int index = x + (field_hdr.nx * y) + (plane_size * z);

		  if(x < 1 || x >= field_hdr.nx - 1 || 
		     y < 1 || y >= field_hdr.ny - 1 ||
		     index != max_holder)
		  {
		      affirm[index] = MISSING_DATA_VALUE;
		      continue;
		  }
		  else 	      
		      affirm[index] = 1.0;
	      }
	      max_holder = 0;
	  }
      }
  }
  else  
  {
      
      for (int z = 0; z < field_hdr.nz; z++)
      {
	  for (int y = 0; y < field_hdr.ny; y++)
	  {
	      for (int x = 0; x < field_hdr.nx; x++)
	      {

	      //Make sure not to apply funtions that need two input fields
	
		  if(_params->processing_type == Params::VECTOR_MAG ||
		     _params->processing_type == Params::VECTOR_DIR ||
		     _params->processing_type == Params::DELTA_WDIR ||
		     _params->processing_type == Params::MAX        ||
		     _params->processing_type == Params::MIN        ||
		     _params->processing_type == Params::AVG)
		  {
		      cerr << "Need two fields to apply this function, file not processed\n" << endl;
		      return;
		  }

		  int index = x + (field_hdr.nx * y) + (plane_size * z);
	      
		  if(_params->processing_type == Params::GRADIENT || 
		     _params->processing_type == Params::GRAD_WDIR)
		  {
		      int index1, index2, index3, index4;
		  
		  // Make sure we remain inside the grid for the calculations

		      if(x < numb_grids || x >= field_hdr.nx - numb_grids || 
			 y < numb_grids || y >= field_hdr.ny - numb_grids)
		      {
			  affirm[index] = MISSING_DATA_VALUE;
			  continue;
		      }
		      else 
		      {
			  index1 = (x + numb_grids) + 
			      (field_hdr.nx * y) + 
			      (field_hdr.nx * 
			       field_hdr.ny * z);

			  index2 = (x - numb_grids) + 
			      (field_hdr.nx * y) + 
			      (field_hdr.nx * 
			       field_hdr.ny * z);

			  index3 = x + 
			      (field_hdr.nx * 
			       (y +  numb_grids)) + 
			      (field_hdr.nx * 
			       field_hdr.ny * 
			       z);
		  		      
			  index4 = x + 
			      (field_hdr.nx * 
			       (y - numb_grids)) + 
			      (field_hdr.nx * 
			       field_hdr.ny * 
			       z);
		      }
		      affirm[index] = doCalculation->processData(field_data[index1],	
								 field_data[index2],
								 field_data[index3],
								 field_data[index4],
								 _params,
								 field_hdr.missing_data_value,
								 field_hdr.bad_data_value,
								 (int)MISSING_DATA_VALUE);
		  }
		  else if(_params->processing_type == Params::SCALE_DATA)
		  {
		      if (x <= 0 || x >= field_hdr.nx - 1 ||
			  y <= 0 || y >= field_hdr.ny - 1)
			  affirm[index] = MISSING_DATA_VALUE;
		      else
			  {
			  average = average + field_data[index];
			  affirm[index] = doCalculation->processData(field_data[index],
								     _params,
								     field_hdr.missing_data_value,
								     field_hdr.bad_data_value,
								     field_hdr.min_value,
								     field_hdr.max_value,
								     (int)MISSING_DATA_VALUE);
			  }
		  }
		  else
		  {

		  // Make sure we remain inside the grid for the calculations

		      if (x <= 0 || x >= field_hdr.nx - 1 ||
			  y <= 0 || y >= field_hdr.ny - 1)
			  affirm[index] = MISSING_DATA_VALUE;
		      else
		      {
			  average = average + field_data[index];
			  affirm[index] = doCalculation->processData(field_data[index],
								     _params,
								     field_hdr.missing_data_value,
								     field_hdr.bad_data_value,
								     (int)MISSING_DATA_VALUE);
			  
		      }
		  } /* end if GRADIENT */
		  
	      }  /* endfor -- x */
	  
	  }  /* endfor -- y */
	  
      }  /* endfor - z */
  }
  
  cout << "Average value of grid = " << average <<endl;

// Add the new output field to the output file
      
  new_output_field->convertType(Mdvx::ENCODING_INT8,
				Mdvx::COMPRESSION_RLE);
      
  output_file.addField(new_output_field);
      
  // Write the output file.
  if ( _params->writeAsForecast)
    output_file.setWriteAsForecast();
  
  output_file.setWriteLdataInfo();
      
  if (output_file.writeToDir(_params->output_url) != 0)
  {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "Error writing data for time " <<
	  utimstr(input_file.getMasterHeader().time_centroid) <<
	  " to URL " << _params->output_url << endl;
      cerr << "*** Skipping ***" << endl << endl;
	  
      return;
  }

  if (_params->debug) {
    cerr << "Wrote file: " << output_file.getPathInUse() << endl;
  }

}

/*********************************************************************
 * _processFile() - Process the given MDV file.  Note that because of
 *                  the way the data is read in, we know that the first
 *                  components are found in field 0 and the second components
 *                  are found in field 1.
 */

void Affirm::_processFile(const DsMdvx& first_input_file,const DsMdvx& sec_input_file) const
{
    const string routine_name = "_processFile()";

// Get pointers to the fields for convenience.

    MdvxField *first_field = first_input_file.getFieldByNum(0);
    Mdvx::field_header_t first_field_hdr = first_field->getFieldHeader();
    fl32 *first_field_data = (fl32 *)first_field->getVol();

    MdvxField *sec_field = sec_input_file.getFieldByNum(0);
    Mdvx::field_header_t sec_field_hdr = sec_field->getFieldHeader();
    fl32 *sec_field_data = (fl32 *)sec_field->getVol();
  
// Create the output file.
      
    DsMdvx output_file;
      
// Set up the master header
      
    Mdvx::master_header_t master_hdr_in;
    if(_params->use_second_field_header)
    {
      master_hdr_in = sec_input_file.getMasterHeader();
    }
    else
    {
      master_hdr_in = first_input_file.getMasterHeader();
    }
    Mdvx::master_header_t master_hdr_out;
      
    memset(&master_hdr_out, 0, sizeof(master_hdr_out));
      
  //    master_hdr_out.time_gen = time((time_t *)NULL);
    master_hdr_out.time_gen = master_hdr_in.time_gen;
    master_hdr_out.time_begin = master_hdr_in.time_begin;
    master_hdr_out.time_end = master_hdr_in.time_end;
    if(_params->use_second_field_header)
    {
      if (sec_field_hdr.forecast_time != 0) {
        master_hdr_out.time_centroid = sec_field_hdr.forecast_time;
      } else {
        master_hdr_out.time_centroid = master_hdr_in.time_centroid;
      }
    }
    else
    {
      if (first_field_hdr.forecast_time != 0) {
        master_hdr_out.time_centroid = first_field_hdr.forecast_time;
      } else {
        master_hdr_out.time_centroid = master_hdr_in.time_centroid;
      }
    }
    master_hdr_out.time_expire = master_hdr_in.time_expire;
    master_hdr_out.data_dimension = master_hdr_in.data_dimension;
    master_hdr_out.data_collection_type = Mdvx::DATA_FORECAST;
    master_hdr_out.native_vlevel_type = master_hdr_in.vlevel_type;
    master_hdr_out.vlevel_type = master_hdr_in.vlevel_type;
    master_hdr_out.vlevel_included = master_hdr_in.vlevel_included;
    master_hdr_out.grid_orientation = master_hdr_in.grid_orientation;
    master_hdr_out.data_ordering = master_hdr_in.data_ordering;
    if(_params->use_second_field_header)
    {
      master_hdr_out.max_nx = sec_field_hdr.nx;
      master_hdr_out.max_ny = sec_field_hdr.ny;
      master_hdr_out.max_nz = sec_field_hdr.nz;
    }
    else
    {
      master_hdr_out.max_nx = first_field_hdr.nx;
      master_hdr_out.max_ny = first_field_hdr.ny;
      master_hdr_out.max_nz = first_field_hdr.nz;
    }
    master_hdr_out.n_chunks = 0;
    master_hdr_out.field_grids_differ = 0;
      
    master_hdr_out.sensor_lon = master_hdr_in.sensor_lon;
    master_hdr_out.sensor_lat = master_hdr_in.sensor_lat;
    master_hdr_out.sensor_alt = master_hdr_in.sensor_alt;
  
    STRcopy(master_hdr_out.data_set_info,
	    "Produced by Affirm", MDV_INFO_LEN);
    STRcopy(master_hdr_out.data_set_name,
	    "Affirm", MDV_NAME_LEN);
    STRcopy(master_hdr_out.data_set_source,
	    first_input_file.getPathInUse().c_str(), MDV_NAME_LEN);
      
    output_file.setMasterHeader(master_hdr_out);

// Create the output field.

    MdvxField *new_output_field = _createAffirmField(*first_field);

    fl32 *affirm = (fl32 *)new_output_field->getVol();

    int plane_size = first_field_hdr.nx * first_field_hdr.ny;
  
// Make sure the grids match.  If they don't, we don't continue.

    if (fabs(first_field_hdr.nx - sec_field_hdr.nx) > _gridDeltaTol ||
	fabs(first_field_hdr.ny - sec_field_hdr.ny) > _gridDeltaTol ||
        fabs(first_field_hdr.nz - sec_field_hdr.nz) > _gridDeltaTol)
    {
	cerr << "WARNING: " << _className() << "::" << routine_name << endl;
	cerr << "Cannot process file for time " <<
	    utimstr(first_input_file.getMasterHeader().time_centroid) << endl;
	cerr << "component field grids do not match" << endl;
	cerr << "The field dimensions are different" << endl;
	cerr << "*** Skipping ***" << endl << endl;
    
	return;
    }
  
// if the grid is 2D don't worry about checking minz or dz
    if(first_field_hdr.nz == 1 && sec_field_hdr.nz == 1) 
    {
      cout << "diff: " << fabs(first_field_hdr.grid_minx - sec_field_hdr.grid_minx) << endl;
      if (fabs(first_field_hdr.grid_minx - sec_field_hdr.grid_minx) > _gridDeltaTol ||
          fabs(first_field_hdr.grid_miny - sec_field_hdr.grid_miny) > _gridDeltaTol)
	{
	    cerr << "WARNING: " << _className() << "::" << routine_name << endl;
	    cerr << "Cannot process file for time " <<
	      utimstr(first_input_file.getMasterHeader().time_centroid) << endl;
	    cerr << "component field grids do not match" << endl;
	    cerr << "The field minimum grid values differ" << endl;
	    cerr << "*** Skipping ***" << endl << endl;
	  
	    return;
	}
  
	if (fabs(first_field_hdr.grid_dx - sec_field_hdr.grid_dx) > _gridDeltaTol ||
	    fabs(first_field_hdr.grid_dy - sec_field_hdr.grid_dy) > _gridDeltaTol)
	{
	    cerr << "WARNING: " << _className() << "::" << routine_name << endl;
	    cerr << "Cannot process file for time " <<
		utimstr(first_input_file.getMasterHeader().time_centroid) << endl;
	    cerr << "component field grids do not match" << endl;
	    cerr << "The field deltas differ" << endl;
	    cerr << "*** Skipping ***" << endl << endl;
    
	    return;
	}
    } else 
    {
      if (fabs(first_field_hdr.grid_minx - sec_field_hdr.grid_minx) > _gridDeltaTol ||
          fabs(first_field_hdr.grid_miny - sec_field_hdr.grid_miny) > _gridDeltaTol ||
          fabs(first_field_hdr.grid_minz - sec_field_hdr.grid_minz) > _gridDeltaTol)
	{
	    cerr << "WARNING: " << _className() << "::" << routine_name << endl;
	    cerr << "Cannot process file for time " <<
	      utimstr(first_input_file.getMasterHeader().time_centroid) << endl;
	    cerr << "component field grids do not match" << endl;
	    cerr << "The field minimum grid values differ" << endl;
	    cerr << "*** Skipping ***" << endl << endl;
	  
	    return;
	}
  
	if (fabs(first_field_hdr.grid_dx - sec_field_hdr.grid_dx) > _gridDeltaTol ||
	    fabs(first_field_hdr.grid_dy - sec_field_hdr.grid_dy) > _gridDeltaTol ||
	    fabs(first_field_hdr.grid_dz - sec_field_hdr.grid_dz) > _gridDeltaTol)
	{
	    cerr << "WARNING: " << _className() << "::" << routine_name << endl;
	    cerr << "Cannot process file for time " <<
		utimstr(first_input_file.getMasterHeader().time_centroid) << endl;
	    cerr << "component field grids do not match" << endl;
	    cerr << "The field deltas differ" << endl;
	    cerr << "*** Skipping ***" << endl << endl;
    
	    return;
	}
    }
    
// Add the input fields to the output file.
      
    MdvxField *first_field_out = new MdvxField(*first_field);
      
    first_field_out->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_RLE);
      
    output_file.addField(first_field_out);
      
    MdvxField *sec_field_out = new MdvxField(*sec_field);
      
    sec_field_out->convertType(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_RLE);
  
    output_file.addField(sec_field_out);

    calcNewField* doCalculation;

    for (int z = 0; z < first_field_hdr.nz; z++)
    {
	for (int y = 0; y < first_field_hdr.ny; y++)
	{
	    for (int x = 0; x < first_field_hdr.nx; x++)
	    {
		int index = x + (first_field_hdr.nx * y) + (plane_size * z);
	
	    // Functions that don't apply to two fields
		if(_params->processing_type == Params::GRADIENT || 
		   _params->processing_type == Params::GRAD_WDIR)
		{
		    cerr << "Only one input field used for this function, file not processed\n" << endl;
		    return;
		}
    
	    // Make sure we remain inside the grid for the calculations
	      
		if (x <= 0 || x >= first_field_hdr.nx - 1 ||
		    y <= 0 || y >= first_field_hdr.ny - 1)
		    affirm[index] = MISSING_DATA_VALUE;
		else 
		{
		// Make sure we have data at the given points.
		    
		    affirm[index] = doCalculation->processData(first_field_data[index],
							       sec_field_data[index],
							       _params,
							       first_field_hdr.missing_data_value,
							       first_field_hdr.bad_data_value,
							       sec_field_hdr.missing_data_value,
							       sec_field_hdr.bad_data_value,
							       (int)MISSING_DATA_VALUE);
		}
	      
	    }  /* endfor -- x */
	  
	}  /* endfor -- y */
	  
    }  /* endfor - z */
      

// Add the new output field to the output file
      
    new_output_field->convertType(Mdvx::ENCODING_INT8,
				  Mdvx::COMPRESSION_RLE);
      
    output_file.addField(new_output_field);
      
// Write the output file.
  if ( _params->writeAsForecast)
    output_file.setWriteAsForecast();
      
    output_file.setWriteLdataInfo();
      
    if (output_file.writeToDir(_params->output_url) != 0)
    {
	cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	cerr << "Error writing data for time " <<
	    utimstr(first_input_file.getMasterHeader().time_centroid) <<
	    " to URL " << _params->output_url << endl;
	cerr << "*** Skipping ***" << endl << endl;
	  
	return;
    }

  if (_params->debug) {
    cerr << "Wrote file: " << output_file.getPathInUse() << endl;
  }

}


/*********************************************************************
 * _readFile() - Read the indicated MDV file.
 *
 * Returns a pointer to the read Mdv file on success, 0 on failure.
 */

DsMdvx *Affirm::_readFile(const time_t search_time,
			  const char * input_url,
			  const int field_num,
			  const char * field_name,
			  const int level_num,
			  const int search_margin) const
{
   const string routine_name = "_readFile()";

   if (_params->debug)
     cerr << "Reading file for time " << utimstr(search_time) << endl;

   // Set up the read request.

   DsMdvx *mdv_file = new DsMdvx();
   
   mdv_file->clearRead();
   mdv_file->setReadTime(Mdvx::READ_FIRST_BEFORE,
			 input_url,
			 _params->time_offset_max, search_time);

   mdv_file->clearReadFields();
   string fieldStr = field_name;
   if(fieldStr.length() > 0) {
     mdv_file->addReadField(fieldStr);
   }
   else {
     mdv_file->addReadField(field_num);
   }
   if(level_num > -1) {
     mdv_file->setReadVlevelLimits(level_num, level_num);
   }
   mdv_file->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
   mdv_file->setReadCompressionType(Mdvx::COMPRESSION_NONE);
   mdv_file->setReadScalingType(Mdvx::SCALING_NONE);
   
   if (_params->debug)
     mdv_file->printReadRequest(cout);
   
   // Read in the data

   if (mdv_file->readVolume() != 0)
   {
     delete mdv_file;
     
     return 0;
   }

   if(_params->debug)
   {
     cerr << "Reading from " << mdv_file->getPathInUse() << endl;
   }
   
   return mdv_file;
}


/*********************************************************************
 * _runArchive() - Run the algorithm in ARCHIVE mode.
 */

void Affirm::_runArchive(void)
{
    const string routine_name = "_runArchive()";

// Compile the list of file times to process.

    DsMdvx time_compiler;
    if(_params->use_trigger_url)
    {
      time_compiler.setTimeListModeValid(_params->trigger_url,
  				       _args->getArchiveStartTime(),
				       _args->getArchiveEndTime());
    }
    else
    {
      time_compiler.setTimeListModeValid(_params->_input_info[0].input_url,
  				       _args->getArchiveStartTime(),
				       _args->getArchiveEndTime());
    }
  
    if (time_compiler.compileTimeList() != 0)
    {
	cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	cerr << "Error compiling file time list" << endl;
    
	exit(-1);
    }

    vector< time_t > time_list = time_compiler.getTimeList();
  
// Process all of the files in the list

    while (!time_list.empty())
    {
    // Get the next file time from the front of the list, deleting the
    // time from the list so we don't process it again.

	time_t file_time = time_list.front();
	time_list.erase(time_list.begin());
    
	if (_params->debug)
	    cerr << "*** Processing first file for time " << utimstr(file_time) << endl;
    
    // Read in the files and process
	int nInput = _params->input_info_n;
	if (nInput > 2)
	{
	    cerr << "\nMax number of input url's is 2\n" << endl;
	    continue;
	}
	
	if(nInput == 1)
	{
	    
	    DsMdvx *mdv_file;
	    
	    if ((mdv_file = _readFile(file_time,
				      _params->_input_info[0].input_url,
				      _params->_input_info[0].field_num,
				      _params->_input_info[0].field_name,
				      (int)_params->_input_info[0].level_num)) == 0)
	    {
		cerr << "ERROR: " << _className() << "::" << routine_name << endl;
		cerr << "Error reading MDV file for time " << utimstr(file_time) <<
		    endl;
		cerr << "*** skipping ***" << endl << endl;
	    
		continue;
	    }
	    _processFile(*mdv_file);

	    delete mdv_file;
	}
	else 
	{
	    DsMdvx *mdv_file0;
	    DsMdvx *mdv_file1;
	    
	    if ((mdv_file0 = _readFile(file_time,
				       _params->_input_info[0].input_url,
				       _params->_input_info[0].field_num,
                                       _params->_input_info[0].field_name,	
				       (int)_params->_input_info[0].level_num)) == 0)
	    {
		cerr << "ERROR: " << _className() << "::" << routine_name << endl;
		cerr << "Error reading MDV file for time " << utimstr(file_time) <<
		    endl;
		cerr << "*** skipping ***" << endl << endl;
	    
		continue;
	    }

	    time_t search_time;
	    if(_params->look_forward) 
	    {
		search_time = file_time + _params->look_back_time;
	    }
	    else 
	    {
		search_time = file_time - _params->look_back_time;
	    }
	    
	    
	    if ((mdv_file1 = _readFile(search_time,
				       _params->_input_info[1].input_url,
				       _params->_input_info[1].field_num,
				       _params->_input_info[1].field_name,
				       (int)_params->_input_info[1].level_num)) == 0)
	    {
		cerr << "ERROR: " << _className() << "::" << routine_name << endl;
		cerr << "Error reading MDV file for time " << utimstr(search_time) <<
		    endl;
		cerr << "*** skipping ***" << endl << endl;
	    
		continue;
	    }
	    _processFile(*mdv_file0, *mdv_file1);

	    delete mdv_file0;
	    delete mdv_file1;
	}
	    
    }
}


/*********************************************************************
 * _runRealtime() - Run the algorithm in REALTIME mode.
 */

void Affirm::_runRealtime(void)
{
   const string routine_name = "_runRealtime()";
  // Process new files forever

  while (true)
  {
    if(!_params->use_trigger_url)
    {
      DsMdvx *mdv_file;
      // Read the next file when it becomes available.

      bool file_found = false;
    
      while (!file_found)
      {
        time_t current_time = time((time_t *)NULL);
        int search_margin = current_time - _lastRealtimeFileTime + 1;
        char* input_url = _params->_input_info[0].input_url;
        long field_num = _params->_input_info[0].field_num;
        char* field_name = _params->_input_info[0].field_name;
        float level_num = _params->_input_info[0].level_num;
      
        if ((mdv_file = _readFile(current_time, 
                    		input_url, 
				field_num,
				field_name,
				(int)level_num,
				search_margin)) != 0)
          file_found = true;
      }
      
      // Process the file

      _processFile(*mdv_file);
    
      // Reclaim the space used by the MDV file

      delete mdv_file;
    }
    else
    {
      // wait for trigger to process files
      DateTime triggerTime;

      while(_trigger->nextIssueTime(triggerTime) == 0)
      {
        if (_params->debug)
        {
          cerr << "  Trigger time: " << triggerTime << endl;
        }
	
        DsMdvx *mdv_file0;
        DsMdvx *mdv_file1;
	    
            if ((mdv_file0 = _readFile(triggerTime.utime(),
				       _params->_input_info[0].input_url,
				       _params->_input_info[0].field_num,	
				       _params->_input_info[0].field_name,	
				       (int)_params->_input_info[0].level_num)) == 0)
	    {
		cerr << "ERROR: " << _className() << "::" << routine_name << endl;
		cerr << "Error reading MDV file for time " << utimstr(triggerTime.utime()) <<
		    endl;
		cerr << "*** skipping ***" << endl << endl;
	    
		continue;
	    }

	    time_t search_time;
	    if(_params->look_forward) 
	    {
              search_time = triggerTime.utime() + _params->look_back_time;
	    }
	    else 
	    {
	      search_time = triggerTime.utime() - _params->look_back_time;
	    }
	    
	    
	    if ((mdv_file1 = _readFile(search_time,
				       _params->_input_info[1].input_url,
				       _params->_input_info[1].field_num,
				       _params->_input_info[1].field_name,	
				       (int)_params->_input_info[1].level_num)) == 0)
	    {
		cerr << "ERROR: " << _className() << "::" << routine_name << endl;
		cerr << "Error reading MDV file for time " << utimstr(search_time) <<
		    endl;
		cerr << "*** skipping ***" << endl << endl;
	    
		continue;
	    }
	    _processFile(*mdv_file0, *mdv_file1);

	    delete mdv_file0;
	    delete mdv_file1;


      }
    }
  } // while true
  
}
