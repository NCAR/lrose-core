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

/* RCS info
 *   $Author: jcraig $
 *   $Date: 2019/01/11 21:04:07 $
 *   $Revision: 1.32 $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvtoGrib2: MdvtoGrib2 program object.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Sept 2006
 *
 *********************************************************************/

#include <iostream>
#include <fstream>
#include <string>

#include <didss/LdataInfo.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <grib2/GDS.hh>
#include <grib2/LatLonProj.hh>
#include <grib2/PolarStereoProj.hh>
#include <grib2/LambertConfProj.hh>
#include <grib2/Template4.0.hh>
#include <grib2/Template4.1.hh>
#include <grib2/Template4.2.hh>
#include <grib2/Template4.5.hh>
#include <grib2/Template4.6.hh>
#include <grib2/Template4.7.hh>
#include <grib2/Template4.8.hh>
#include <grib2/Template4.9.hh>
#include <grib2/Template4.10.hh>
#include <grib2/Template4.11.hh>
#include <grib2/Template4.12.hh>
#include <grib2/Template5.0.hh>
#include <grib2/Template5.41.hh>
#include <grib2/Template5.4000.hh>
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgLc2Calc.hh>
#include <euclid/PjgPolarStereoCalc.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "MdvtoGrib2.hh"
#include "Params.hh"

using namespace std;


// Global variables

MdvtoGrib2 *MdvtoGrib2::_instance =
     (MdvtoGrib2 *)NULL;



/*********************************************************************
 * Constructor
 */

MdvtoGrib2::MdvtoGrib2(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvtoGrib2::MdvtoGrib2()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvtoGrib2 *)NULL);

  // Initialize the okay flag.

  okay = true;

  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  if(progname_parts.dir != NULL)
    free(progname_parts.dir);
  if(progname_parts.name != NULL)
    free(progname_parts.name);
  if(progname_parts.base != NULL)
    free(progname_parts.base);
  if(progname_parts.ext != NULL)
    free(progname_parts.ext);

  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  // Get TDRP parameters.

  _params = new Params();
  char *params_path;

  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
                            &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;

    okay = false;

    return;
  }

  // If an additional TDRP file was specified, load that
  // over the existing params.
  if (NULL != _args->additional_tdrp_file){

    if (_params->debug){
      cerr << "Attempting to load additional param file " << _args->additional_tdrp_file << endl;
    }
    if (_params->load(_args->additional_tdrp_file, NULL, TRUE, FALSE)){
      cerr << "ERROR: " << method_name << endl;
      cerr << "Problem with TDRP parameters in file: " << _args->additional_tdrp_file
           << endl;
      okay = false;
      return;
    }
  }

  return;

}


/*********************************************************************
 * Destructor
 */

MdvtoGrib2::~MdvtoGrib2()
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

MdvtoGrib2 *MdvtoGrib2::Inst(int argc, char **argv)
{
  if (_instance == (MdvtoGrib2 *)NULL)
    new MdvtoGrib2(argc, argv);

  return(_instance);
}

MdvtoGrib2 *MdvtoGrib2::Inst()
{
  assert(_instance != (MdvtoGrib2 *)NULL);

  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvtoGrib2::init()
{
  static const string method_name = "MdvtoGrib2::init()";

  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA : 
  case Params:: LATEST_DATA_FCST :
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
    time_t start_time = _args->start_time.utime();
    time_t end_time = _args->end_time.utime();

    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger." << endl;
      return false;
    }

    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger." << endl;
      return false;
    }

    if (_params->debug)
      cerr << "Initializing TIME_LIST trigger using url: " <<
        _params->input_url << ", start_time: " << start_time <<
        ", end_time: " << end_time << endl;


    DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
    if (trigger->init(_params->input_url,
                      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
        _params->input_url << endl;
      cerr << "    Start time: " << start_time <<
        endl;
      cerr << "    End time: " << end_time << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::FILE_LIST :
  {
    if (_params->debug) {
      cerr << "Initializing archive FILELIST mode. Files:" << endl;
      vector<string> files = _args->getInputFileList();
      for(vector<string>::iterator file = files.begin(); file != files.end(); file++)
        cerr << *file << endl;
    }

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

  // Make sure that the output directory exists
  if (ta_makedir_recurse(_params->output_dir) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: " << _params->output_dir << endl;

    return false;
  }

  // initialize process registration
  PMU_auto_init(_progName, _params->instance,
                PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

int MdvtoGrib2::run()
{
  static const string method_name = "MdvtoGrib2::run()";

  TriggerInfo trigger_info;

  int retVal = 0;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      retVal = -1;

      continue;
    }

    if (!_processData(trigger_info))
    {
      if(_params->trigger_mode == Params::FILE_LIST)
        cerr << "Error processing file: " << trigger_info.getFilePath() << endl;
      else
        cerr << "Error processing data for time: "
             << DateTime(trigger_info.getIssueTime()) << endl;
      retVal = -1;

      continue;
    }

  } /* endwhile - !_dataTrigger->endOfData() */

  return retVal;

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool MdvtoGrib2::_processData(TriggerInfo &trigger_info)
{
  static const string method_name = "MdvtoGrib2::_processData()";

  if (_params->debug) {
    if(_params->trigger_mode == Params::FILE_LIST) {
      cerr << "*** Processing file: " << trigger_info.getFilePath() << endl;
    } else {
      cerr << endl << "*** Processing data for time: "
           << DateTime(trigger_info.getIssueTime()) << endl;
    }
  }

  //
  // Read in the input file
  //
  DsMdvx input_mdv;

  if (!_readMdvFile(input_mdv, trigger_info))
    return false;

  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();

  //
  // Determine Output filenaming convention based on input file
  //
  char date_dir[20];
  char grib_file_base[MAX_PATH_LEN];
  string mdv_file_path = input_mdv.getPathInUse();
  grib_file_base[0] = char(0);

  if(mdv_file_path.size() >= 32 && mdv_file_path[mdv_file_path.size() - 14] == 'f') {
    int year, month, day, hour, minute, second, forecast_seconds;
    if (7 == sscanf(mdv_file_path.c_str() + (mdv_file_path.size() - 32),
                    "%4d%2d%2d/g_%2d%2d%2d/f_%8d.mdv",
                    &year, &month, &day, &hour, &minute, &second,
                    &forecast_seconds)) {
      sprintf(date_dir, "/%04d%02d%02d/", year, month, day);
      if (_params->use_iso8601_filename_convention) {
	int lead_time_hrs = forecast_seconds/3600;
	int lead_time_mins = (forecast_seconds % 3600 )/ 60;
	sprintf(grib_file_base, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.PT%.2d:%.2d.grb2",
		year, month, day, hour, minute, second, lead_time_hrs, lead_time_mins);
      }
      else {
	sprintf(grib_file_base, "%.4d%.2d%.2d_i%.2d%.2d%.2d_f%.8d.grb2",
		year, month, day, hour, minute, second, forecast_seconds);
      }
    }
  } else if(mdv_file_path.size() >= 19 && mdv_file_path[mdv_file_path.size() - 11] == '/') {
    int year, month, day, hour, minute, second;
    if (6 == sscanf(mdv_file_path.c_str() + (mdv_file_path.size() - 19),
                    "%4d%2d%2d/%2d%2d%2d.mdv",
                    &year, &month, &day, &hour, &minute, &second)) {
      sprintf(date_dir, "/%04d%02d%02d/", year, month, day);
      if (_params->use_iso8601_filename_convention) {
	sprintf(grib_file_base, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.grb2",
		year, month, day, hour, minute, second);
      }
      else {
	sprintf(grib_file_base, "%.4d%.2d%.2d_%.2d%.2d%.2d.grb2",
		year, month, day, hour, minute, second);
      }
    }
  }
  if(grib_file_base[0] == char(0)) {
    DateTime trigger_time;
    if(master_hdr.forecast_delta == 0)
      trigger_time.set(master_hdr.time_centroid);
    else
      trigger_time.set(master_hdr.time_gen);

    // maybe need to add the forecast case in here instead of doing non-forecast ?
    // can use master_hdr.forecast_delta as lead time??

    sprintf(date_dir, "/%04d%02d%02d/", trigger_time.getYear(), trigger_time.getMonth(),
            trigger_time.getDay());

    if (_params->use_iso8601_filename_convention) {
      sprintf(grib_file_base, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.grb2",
	      trigger_time.getYear(), trigger_time.getMonth(), trigger_time.getDay(),
	      trigger_time.getHour(), trigger_time.getMin(), trigger_time.getSec());
    }
    else {
      sprintf(grib_file_base, "%04d%02d%02d_%02d%02d%02d.grb2",
	      trigger_time.getYear(), trigger_time.getMonth(), trigger_time.getDay(),
	      trigger_time.getHour(), trigger_time.getMin(), trigger_time.getSec());
    }
  }
  Grib2::Grib2File grib2file;

  //
  // Run through the output field list and:
  // - Determine if any Local Tables are being used
  // - Determine a file wide data_type
  int localTables = 0;
  int file_data_type = -1;

  if(master_hdr.forecast_delta > 0)
    file_data_type = 1;                // Forecast products
  if(master_hdr.forecast_delta == 0)
    file_data_type = 0;                // Analysis products

  for (int field_num = 0; field_num < _params->output_fields_n; field_num++)
  {
    if((_params->_output_fields[field_num].param_category >= 192 &&
        _params->_output_fields[field_num].param_category < 255) ||
       (_params->_output_fields[field_num].param_number >= 192 &&
        _params->_output_fields[field_num].param_number <255))
      localTables = 1;


    if((_params->_output_fields[field_num].override_surface_type &&
        _params->_output_fields[field_num].first_surface_type >= 192 &&
	_params->_output_fields[field_num].first_surface_type < 255) ||
       (_params->_output_fields[field_num].second_surface_type >= 192 &&
        _params->_output_fields[field_num].second_surface_type <255))
      localTables = 1;

    if(_params->_output_fields[field_num].process_type == 8) {
      file_data_type = 192;              // Other observation data
      if(_params->discipline_number == 0 && _params->_output_fields[field_num].param_category == 15)
        file_data_type = 7;              // Radar observation data
      if(_params->discipline_number == 3 && _params->_output_fields[field_num].param_category == 0)
        file_data_type = 6;              // Satellite observation data
    }
    if((_params->_output_fields[field_num].data_type == 1 || _params->_output_fields[field_num].data_type == 11)
       && _params->_output_fields[field_num].user_data_value == 0) {
      if(file_data_type == 4 || file_data_type == 5)
        file_data_type = 5;              // Control and Perturbed Forecast products
      else
        file_data_type = 3;              // Control Forecast products
    }
    if((_params->_output_fields[field_num].data_type == 1 || _params->_output_fields[field_num].data_type == 11)
       && _params->_output_fields[field_num].user_data_value > 0) {
      if(file_data_type == 3 || file_data_type == 5)
        file_data_type = 4;              // Control and Perturbed Forecast products
     else
       file_data_type = 4;               // Perturbed Forecast products
    }
  }
  if(file_data_type == 192)
    localTables = 1;


  //
  // Figure out the grib file time and set the significance flag based on what
  // type of data we are processing.
  int time_type = 1;  // Default is Forecast Time
  time_t grib_file_time = master_hdr.time_gen;
  if(file_data_type == 0) {
    time_type = 0;    // Analysis Time
    grib_file_time = master_hdr.time_centroid;
  } else if(file_data_type == 6 || file_data_type == 7 || file_data_type == 192) {
    time_type = 3;    // Observation Time
    grib_file_time = master_hdr.time_centroid;
  }

  // Initialize the Grib2 file
  grib2file.create(_params->discipline_number, grib_file_time, time_type,
                   file_data_type, _params->sub_centre_id, _params->centre_id,
                   _params->production_status, localTables);

  //
  // Extract each of the fields listed in the params and add them to the
  // output GRIB2 file.
  //
  int lastGridDefNum = -1;
  Grib2::GribProj *lastGribProj = NULL;
  for (int field_num = 0; field_num < _params->output_fields_n; field_num++)
  {
    int field_index;
    if(_params->_output_fields[field_num].mdv_field_num > -1)
      field_index = _params->_output_fields[field_num].mdv_field_num;
    else {
      field_index = input_mdv.getFieldNum(_params->_output_fields[field_num].mdv_field_name);

      if (field_index == -1) {
        cerr << "ERROR: " << method_name << endl;
        cerr << "Cannot find field name " << _params->_output_fields[field_num].mdv_field_name
             << " in MDV file." << endl;
        cerr << "Skipping field....." << endl;
        continue;
      }
    }

    MdvxField *field = input_mdv.getField(field_index);

    if (field == NULL) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error extracting field number " << field_index
           << " from MDV file." << endl;
      cerr << "Skipping field....." << endl;
      continue;
    }

    if(field_num > 0 && (_params->record_mode == Params::RECORD_BY_FIELD ||
			 _params->record_mode == Params::RECORD_BY_PLANE) ) {
      // Re-Initialize the Grib2 library, forcing start of a new grib2 record
      lastGridDefNum = -1;
      lastGribProj = NULL;
      grib2file.create(_params->discipline_number, grib_file_time, time_type,
		       file_data_type, _params->sub_centre_id, _params->centre_id,
		       _params->production_status, localTables);
    }

    //
    // Process the field
    //
    Mdvx::field_header_t field_hdr = (*field).getFieldHeader();
    // Mdvx::vlevel_header_t vlevel_hdr = (*field).getVlevelHeader();

    if (_params->debug) {
      cerr << "---> Processing field: " << field_hdr.field_name_long << endl;
    }

    //
    // Create the Projection Template
    // gribProj is returned NULL if it is equal to lastGridProj
    Grib2::GribProj *gribProj;
    int gridDefNum = _createGDSTemplate(field, lastGridDefNum, lastGribProj, &gribProj);
    if(gridDefNum == -1) {
      return false;
    }

    //
    // Only add a Grid Difinition Section if it is different than
    // the previous GDS.
    //
    if(gribProj != NULL) {
      grib2file.addGrid(field_hdr.nx * field_hdr.ny, gridDefNum, gribProj);
      lastGridDefNum = gridDefNum;
      lastGribProj = gribProj;
    }

    //
    // Each level in the field becames a different GRIB record
    //
    (*field).setPlanePtrs();

    //
    // Parse the data multiplier and andend
    //
    double addend = _params->_output_fields[field_num].data_addend;
    double multiplier = 1;
    if(_params->_output_fields[field_num].data_convert_type == Params::DATA_CONVERT_MULTIPLY)
    {
      multiplier = _params->_output_fields[field_num].data_convert_parameter;
    }

    for (int z = 0; z < field_hdr.nz; ++z)
    {

      if(z > 0 && _params->record_mode == Params::RECORD_BY_PLANE) {
	// Re-Initialize the Grib2 library, forcing start of a new grib2 record
	lastGridDefNum = -1;
	lastGribProj = NULL;
	int gridDefNum = _createGDSTemplate(field, lastGridDefNum, lastGribProj, &gribProj);
	if(gridDefNum == -1) {
	  return false;
	}
	grib2file.create(_params->discipline_number, grib_file_time, time_type,
			 file_data_type, _params->sub_centre_id, _params->centre_id,
			 _params->production_status, localTables);
	grib2file.addGrid(field_hdr.nx * field_hdr.ny, gridDefNum, gribProj);
	lastGridDefNum = gridDefNum;
	lastGribProj = gribProj;
      }

      //
      // Create the Product Definition Section Template (section 4)
      //
      Grib2::ProdDefTemp *prodDefTemplate = NULL;
      int prodDefNum =_createPDSTemplate(field, field_num, z, grib_file_time,
                                         file_data_type, &prodDefTemplate);
      if(prodDefNum == -1 || prodDefTemplate == NULL) {
        return false;
      }

      prodDefTemplate->setParamNumbers(_params->_output_fields[field_num].param_category,
                                       _params->_output_fields[field_num].param_number);


      //
      // Create the Data Representation Section Template (section 5)
      //
      int dataRepNum;
      Grib2::DataRepTemp *dataRepTemplate = NULL;
      if(_params->_output_fields[field_num].compress_method == 0) {
        dataRepNum = 0;
        Grib2::Template5_pt_0 *template5_0 = new
          Grib2::Template5_pt_0(_params->_output_fields[field_num].floating_point_precision);

        dataRepTemplate = (Grib2::DataRepTemp *) template5_0;

      } else if(_params->_output_fields[field_num].compress_method == 40 ||
                _params->_output_fields[field_num].compress_method == 4000 ) {

        dataRepNum = 40;
        Grib2::Template5_pt_4000 *template5_4000 = new
          Grib2::Template5_pt_4000(_params->_output_fields[field_num].floating_point_precision);

        dataRepTemplate = (Grib2::DataRepTemp *) template5_4000;

      } else if(_params->_output_fields[field_num].compress_method == 41 ) {

        dataRepNum = 41;
        Grib2::Template5_pt_41 *template5_41 = new
          Grib2::Template5_pt_41(_params->_output_fields[field_num].floating_point_precision);

        dataRepTemplate = (Grib2::DataRepTemp *) template5_41;

      } else {
        cerr << "ERROR: " << method_name << endl;
        cerr << "Grib2 Compression method " << _params->_output_fields[field_num].compress_method <<
          " is not yet implemented in Grib2 library." << endl;
        return false;
      }

      //
      // Create the data array and the bit map
      //
      int plane_size = field_hdr.nx * field_hdr.ny;
      fl32 *grib_data = new fl32[plane_size];
      fl32 *mdv_data = (fl32 *)(*field).getPlane(z);
      si32 *bitmap = new si32[plane_size];
      int bitMapType = 255;

      memset(bitmap, 0, plane_size * sizeof(ui08));

      for (int i = 0; i < plane_size; ++i)
      {
        if (mdv_data[i] == field_hdr.missing_data_value ||
            mdv_data[i] == field_hdr.bad_data_value)
        {
          bitMapType = 0;
          bitmap[i] = 0;
        }
        else
        {
          bitmap[i] = 1;

          // Convert the data value, using our multiplier and addend
          mdv_data[i] = (mdv_data[i] * multiplier) + addend;
        }
      } /* endfor - i */

      //
      // Add the Field to the Grib2 File
      //
      if(grib2file.addField(prodDefNum, prodDefTemplate, dataRepNum, dataRepTemplate,
			    mdv_data, bitMapType, bitmap) == Grib2::GRIB_FAILURE) {
	delete [] bitmap;
	delete [] grib_data;
	return false;
      }

      delete [] bitmap;
      delete [] grib_data;

    } /* endfor - z */


  } /* endfor - field_num */

  //
  // Write out the GRIB2 record
  string grib_file_path(_params->output_dir);
  grib_file_path += date_dir;
  mkdir(grib_file_path.c_str(),S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);

  string grib_file_name;
  if (_params->use_iso8601_filename_convention) {
    grib_file_name = _params->basename;
    grib_file_name += grib_file_base;
  }
  else {
    grib_file_name = grib_file_base;
  }

  grib_file_path += grib_file_name;
  if (_params->debug) {
    cout << " Writing out file " << grib_file_path << endl;
  }
  grib2file.write(grib_file_path.c_str());

  //
  // Create LDataInfo file
  if (_params->create_ldatainfo) {
    LdataInfo ldata;
    ldata.setDir(_params->output_dir);
    ldata.setDebug(_params->debug);
    ldata.setDataFileExt("grb2");
    ldata.setDataType("grb2");
    ldata.setWriter(_progName);
    string relPath = string(date_dir) + grib_file_name;
    ldata.setRelDataPath(relPath);
    if(master_hdr.forecast_delta > 0) {
      ldata.setIsFcast();
      ldata.setLeadTime(master_hdr.forecast_delta);
    }
    if (ldata.write(grib_file_time)) {
      cerr << "Error writing _latest_data_info file" <<
        "   for output file: " <<  grib_file_path << endl;
    }
  }

  return true;
}


/*********************************************************************
 * _readMdvFile() - Read the MDV file for the given time.
 */

bool MdvtoGrib2::_readMdvFile(DsMdvx &input_mdv,
                            TriggerInfo &trigger_info) const
{
  static const string method_name = "MdvtoGrib2::_readMdvFile()";

  // Set up the read request

  if(_params->trigger_mode == Params::FILE_LIST)
  {
    input_mdv.setReadPath(trigger_info.getFilePath());
  }
  else if (_params->trigger_mode == Params::LATEST_DATA)
  {
    input_mdv.setReadTime(Mdvx::READ_CLOSEST,
                          _params->input_url,
                          _params->tolerance_seconds, trigger_info.getIssueTime());
  }
  else if (_params->trigger_mode == Params::LATEST_DATA_FCST)
  {
    input_mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
			  _params->input_url,
			  _params->tolerance_seconds, trigger_info.getIssueTime(), 
			  trigger_info.getForecastTime() - trigger_info.getIssueTime() );
  }  else // _params->trigger_mode == Params::TIME_LIST
  {
    input_mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
			  _params->input_url,
			  _params->tolerance_seconds, trigger_info.getIssueTime(), 
			  trigger_info.getForecastTime() - trigger_info.getIssueTime() );
  }


  for (int i = 0; i < _params->output_fields_n; ++i)
  {
    if (_params->_output_fields[i].mdv_field_num == -1)
      input_mdv.addReadField(_params->_output_fields[i].mdv_field_name);
    else
      input_mdv.addReadField(_params->_output_fields[i].mdv_field_num);
  }

  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);

  //
  // Apply Remaping on read, if requested
  if (_params->auto_remap_to_latlon ){
    input_mdv.setReadAutoRemap2LatLon();
  }
  else if (_params->remap_output)
  {
    switch (_params->remap_info.proj_type)
    {
    case Params::PROJ_LATLON :
      input_mdv.setReadRemapLatlon(_params->remap_info.nx,
                                   _params->remap_info.ny,
                                   _params->remap_info.minx,
                                   _params->remap_info.miny,
                                   _params->remap_info.dx,
                                   _params->remap_info.dy);
      break;

    case Params::PROJ_LAMBERT_CONF :
      if (_params->use_horiz_limits)
      {
        double minLat = _params->horiz_limits.min_lat;
        double maxLat = _params->horiz_limits.max_lat;
        double minLon = _params->horiz_limits.min_lon;
        double maxLon = _params->horiz_limits.max_lon;

        cerr << "INFO: minLat=" << minLat
             << ", maxLat=" << maxLat
             << ", minLon=" << minLon
             << ", maxLon=" << maxLon << endl;

        input_mdv.setReadHorizLimits(minLat, minLon, maxLat, maxLon);
      }
      else
      {
        input_mdv.setReadRemapLc2(_params->remap_info.nx,
                                  _params->remap_info.ny,
                                  _params->remap_info.minx,
                                  _params->remap_info.miny,
                                  _params->remap_info.dx,
                                  _params->remap_info.dy,
                                  _params->remap_info.origin_lat,
                                  _params->remap_info.origin_lon,
                                  _params->remap_info.lat1,
                                  _params->remap_info.lat2);
      }
      break;

    } /* endswitch - _params->remap_info.proj_type */
  }

  if (_params->debug)
    input_mdv.printReadRequest(cerr);

  // Read the MDV file
  if (input_mdv.readVolume() != 0)
  {
    cerr << input_mdv.getErrStr() << endl;
    return false;
  }

  return true;
}


/*********************************************************************
 * _convertMdvtoGrib2LevelType() - Convert the given MDV vertical level info
 *                                 to the equivalent GRIB level info.
 */

bool MdvtoGrib2::_convertMdvtoGrib2LevelType(const int mdv_level_type,
                                             const double mdv_level_value,
                                             int &firstSurfaceType,
                                             int &secondSurfaceType,
                                             double &firstValue,
                                             double &secondValue)
{
  static const string method_name = "MdvtoGrib2::_convertMdvtoGrib2LevelType()";

  switch (mdv_level_type)
  {
  case Mdvx::VERT_TYPE_SURFACE :
    firstSurfaceType = 1;
    secondSurfaceType = 255;
    firstValue = 0;
    secondValue = 0;
    break;

  case Mdvx::VERT_TYPE_PRESSURE :
    // Mdv uses hPa for pressure, Grib2 uses Pa
    firstSurfaceType = 100;
    secondSurfaceType = 255;
    firstValue = mdv_level_value * 100;
    secondValue = 0;
    break;

  case Mdvx::VERT_TYPE_Z :
    firstSurfaceType = 102;
    secondSurfaceType = 255;
    firstValue = (mdv_level_value * 1000.0);
    secondValue = 0;
    break;

  case Mdvx::VERT_TYPE_ZAGL_FT :
    firstSurfaceType = 102;
    secondSurfaceType = 255;
    firstValue = (mdv_level_value * 0.3048);
    secondValue = 0;
    break;

  case Mdvx::VERT_FLIGHT_LEVEL :
    // note flight levels are defined as 100's of feet above MSL
    // we want meters abve MSL
    firstSurfaceType = 102;
    secondSurfaceType = 255;
    firstValue = (mdv_level_value * 100.0 * 0.3048);
    secondValue = 0;
    break;

  case Mdvx::VERT_SOIL :
    firstSurfaceType = 106;
    secondSurfaceType = 255;
    firstValue = (mdv_level_value * 1000.0);
    secondValue = 0;
    break;

  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Vertical level type " << Mdvx::vertType2Str(mdv_level_type)
         << " not yet implemented" << endl;
    return false;
  }

  return true;
}


void MdvtoGrib2::_scaleFactorValue(double &value, int &scaleFactor)
{
  if(value > 0.0) {
    while(fmod(value, 1.0) != 0.0 &&
          value < 214748364.7) {
      value *= 10.0;
      scaleFactor ++;
    }
  } else if(value < 0.0) {
    while(fmod(value, 1.0) != 0.0 && value - (si32)value < -.000001 &&
          value >  -214748364.7) {
      value *= 10.0;
      scaleFactor ++;
    }
  }
}


//
// Create the Grid Definition Section Template (section 3)
//
int MdvtoGrib2::_createGDSTemplate(MdvxField *field, int lastGridDefNum, Grib2::GribProj *lastGribProj,
                                   Grib2::GribProj **gribProj)
{
  Mdvx::field_header_t field_hdr = (*field).getFieldHeader();
  // Mdvx::vlevel_header_t vlevel_hdr = (*field).getVlevelHeader();

  // int plane_size = field_hdr.nx * field_hdr.ny;
  int gridDefNum = -1;

  if(field_hdr.proj_type == Mdvx::PROJ_LATLON)
    {
      gridDefNum = Grib2::GDS::EQUIDISTANT_CYL_PROJ_ID;
      Grib2::LatLonProj *latlonProj = new Grib2::LatLonProj();

      latlonProj->_earthShape = 6;
      latlonProj->_radiusScaleFactor = 0;
      latlonProj->_radiusScaleValue = 0;
      latlonProj->_majorAxisScaleFactor = 0;
      latlonProj->_majorAxisScaleValue = 0;
      latlonProj->_minorAxisScaleFactor = 0;
      latlonProj->_minorAxisScaleValue = 0;
      latlonProj->_basicAngleProdDomain = 0;
      latlonProj->_basicAngleSubdivisions = 0;
      latlonProj->_resolutionFlag = 56;
      latlonProj->_scanModeFlag = 64;

      latlonProj->_ni  = field_hdr.nx;
      latlonProj->_nj  = field_hdr.ny;

      double minx;
      if( field_hdr.grid_minx < 0)
	minx = 360.0 + field_hdr.grid_minx;
      else
	minx = field_hdr.grid_minx;
      
      latlonProj->_lo1 = minx;
      latlonProj->_la1 = field_hdr.grid_miny;
      latlonProj->_di  = field_hdr.grid_dx;
      latlonProj->_dj  = field_hdr.grid_dy;
      latlonProj->_la2 = field_hdr.grid_miny + (field_hdr.grid_dy * (field_hdr.ny - 1));
      latlonProj->_lo2 = minx + (field_hdr.grid_dx * (field_hdr.nx - 1));

      (*gribProj) = (Grib2::GribProj *) latlonProj;
      if(lastGridDefNum == gridDefNum) {
        Grib2::LatLonProj *lastProj = (Grib2::LatLonProj *) lastGribProj;
        if(lastProj->_ni == latlonProj->_ni &&
           lastProj->_nj == latlonProj->_nj &&
           lastProj->_lo1 == latlonProj->_lo1 &&
           lastProj->_la1 == latlonProj->_la1 &&
           lastProj->_di == latlonProj->_di &&
           lastProj->_dj == latlonProj->_dj &&
           lastProj->_la2 == latlonProj->_la2 &&
           lastProj->_lo2 == latlonProj->_lo2) {
          delete latlonProj;
          (*gribProj) = NULL;
        }
      }
    } else if(field_hdr.proj_type == Mdvx::PROJ_POLAR_STEREO)
    {
      gridDefNum = Grib2::GDS::POLAR_STEREOGRAPHIC_PROJ_ID;
      Grib2::PolarStereoProj *polarProj = new Grib2::PolarStereoProj();

      polarProj->_earthShape = 6;
      polarProj->_radiusScaleFactor = 0;
      polarProj->_radiusScaleValue = 0;
      polarProj->_majorAxisScaleFactor = 0;
      polarProj->_majorAxisScaleValue = 0;
      polarProj->_minorAxisScaleFactor = 0;
      polarProj->_minorAxisScaleValue = 0;
      polarProj->_resolutionFlag = 56;
      polarProj->_scanModeFlag = 64;
      polarProj->_lad = 90;

      polarProj->_nx = field_hdr.nx;
      polarProj->_ny = field_hdr.ny;
      polarProj->_dx = field_hdr.grid_dx;
      polarProj->_dy = field_hdr.grid_dy;
      polarProj->_lov = field_hdr.proj_param[0];
      polarProj->_projCtrFlag = (unsigned char) field_hdr.proj_param[1];
      polarProj->_la1 = field_hdr.proj_origin_lat;
      polarProj->_lo1 = field_hdr.proj_origin_lon;

      (*gribProj) = (Grib2::GribProj *) polarProj;
      if(lastGridDefNum == gridDefNum) {
        Grib2::PolarStereoProj *lastProj = (Grib2::PolarStereoProj *) lastGribProj;
        if(lastProj->_nx == polarProj->_nx &&
           lastProj->_ny == polarProj->_ny &&
           lastProj->_lo1 == polarProj->_lo1 &&
           lastProj->_la1 == polarProj->_la1 &&
           lastProj->_dx == polarProj->_dx &&
           lastProj->_dy == polarProj->_dy &&
           lastProj->_lad == polarProj->_lad &&
           lastProj->_lov == polarProj->_lov) {
          delete polarProj;
          (*gribProj) = NULL;
        }
      }

    } else if(field_hdr.proj_type == Mdvx::PROJ_LAMBERT_CONF)
    {
      gridDefNum = Grib2::GDS::LAMBERT_CONFORMAL_PROJ_ID;
      Grib2::LambertConfProj *lambertProj = new Grib2::LambertConfProj();

      lambertProj->_earthShape = 6;
      lambertProj->_radiusScaleFactor = 0;
      lambertProj->_radiusScaleValue = 0;
      lambertProj->_majorAxisScaleFactor = 0;
      lambertProj->_majorAxisScaleValue = 0;
      lambertProj->_minorAxisScaleFactor = 0;
      lambertProj->_minorAxisScaleValue = 0;
      lambertProj->_resolutionFlag = 56;
      lambertProj->_scanModeFlag = 64;
      lambertProj->_projCtrFlag = 0;
      lambertProj->_las = 0;
      lambertProj->_los = 0;

      lambertProj->_nx = field_hdr.nx;
      lambertProj->_ny = field_hdr.ny;
      lambertProj->_lov = field_hdr.proj_origin_lon;
      lambertProj->_dx = field_hdr.grid_dx;
      lambertProj->_dy = field_hdr.grid_dy;
      lambertProj->_latin1 = field_hdr.proj_param[0];
      lambertProj->_latin2 = field_hdr.proj_param[1];
      lambertProj->_lad =  field_hdr.proj_param[0];

      double lat1, lon1;
      PjgCalc *calculator;
      if (lambertProj->_latin1 == lambertProj->_latin2)
        calculator = new PjgLc1Calc(lambertProj->_latin1, lambertProj->_lov, lambertProj->_latin1);
      else
        calculator = new PjgLc2Calc(lambertProj->_latin1, lambertProj->_lov,
                                    lambertProj->_latin1, lambertProj->_latin2);

      calculator->xy2latlon(field_hdr.grid_minx, field_hdr.grid_miny, lat1, lon1);
      delete(calculator);

      if(lambertProj->_lov < 0.0)
        lambertProj->_lov += 360.0;

      if(lon1 < 0.0)
        lon1 += 360.0;

      lambertProj->_la1 = lat1;
      lambertProj->_lo1 = lon1;

      (*gribProj) = (Grib2::GribProj *) lambertProj;
      if(lastGridDefNum == gridDefNum) {
        Grib2::LambertConfProj *lastProj = (Grib2::LambertConfProj *) lastGribProj;
        if(lastProj->_nx == lambertProj->_nx &&
           lastProj->_ny == lambertProj->_ny &&
           lastProj->_lo1 == lambertProj->_lo1 &&
           lastProj->_la1 == lambertProj->_la1 &&
           lastProj->_las == lambertProj->_las &&
           lastProj->_los == lambertProj->_los &&
           lastProj->_latin1 == lambertProj->_latin1 &&
           lastProj->_latin2 == lambertProj->_latin2 &&
           lastProj->_dx == lambertProj->_dx &&
           lastProj->_dy == lambertProj->_dy &&
           lastProj->_lad == lambertProj->_lad &&
           lastProj->_lov == lambertProj->_lov) {
          delete lambertProj;
          (*gribProj) = NULL;
        }
      }
    } else {
      cerr << "ERROR: MDV Projection " << field_hdr.proj_type << " is not yet implemented in MdvtoGrib2." << endl;
      gridDefNum = -1;
    }
  
  return gridDefNum;
}

//
// Create the Product Definition Section Template (section 4)
//
int MdvtoGrib2::_createPDSTemplate(MdvxField *field, int field_num, int z, time_t reference_time,
                                   int file_data_type, Grib2::ProdDefTemp **prodDefTemplate)
{
  Mdvx::field_header_t field_hdr = (*field).getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr = (*field).getVlevelHeader();

  int prodDefNum = -1;

  int firstSurfaceType, secondSurfaceType;
  int scaleFactorFirst = 0, scaleFactorSecond = 0;
  double firstValue, secondValue;
  if(_params->_output_fields[field_num].override_surface_type) {
    firstSurfaceType = _params->_output_fields[field_num].first_surface_type;
    firstValue = vlevel_hdr.level[z];
    secondSurfaceType = _params->_output_fields[field_num].second_surface_type;
    secondValue = vlevel_hdr.level[z];
  } else
    if (!_convertMdvtoGrib2LevelType(vlevel_hdr.type[z], vlevel_hdr.level[z],
                                     firstSurfaceType, secondSurfaceType,
                                     firstValue, secondValue))
      return -1;

  _scaleFactorValue(firstValue, scaleFactorFirst);
  _scaleFactorValue(secondValue, scaleFactorSecond);

  //
  // Determine forecast time and units
  int forecastDelta = field_hdr.forecast_delta;
  int timeUnit = 13;
  int forecastTime = forecastDelta;

  //
  // If we have a time_interval type subtract the interval from the forecast time
  if(_params->_output_fields[field_num].data_type >= 8 &&
     _params->_output_fields[field_num].data_type <= 12)
    forecastTime -= _params->_output_fields[field_num].time_interval;

  if(forecastTime % 3600 == 0) {
    timeUnit = 1;
    forecastTime /= 3600;
  } else if(forecastTime % 60 == 0) {
    timeUnit = 0;
    forecastTime /= 60;
  }

  //
  // Determine time_interval time and units
  int timeRangeUnit = 13;
  int timeRangeLen = _params->_output_fields[field_num].time_interval;
  if(timeRangeLen % 3600 == 0) {
    timeRangeUnit = 1;
    timeRangeLen /= 3600;
  } else if(timeRangeLen % 60 == 0) {
    timeRangeUnit = 0;
    timeRangeLen /= 60;
  }

  int process_type = _params->_output_fields[field_num].process_type;
  int data_type = _params->_output_fields[field_num].data_type;
  int lastProcessType = -1;
  if(field_num > 0)
    lastProcessType = _params->_output_fields[field_num-1].process_type;
  //
  // Checks that process_type and data_type for each field make sense
  if(process_type == 8 && !(data_type == 0 || data_type == 8) ) {
    cerr << "ERROR: Field " << field_num << " process_type is Observation but data_type is a forecast." << endl;
    return -1;
  }

  if((process_type == 0 || process_type == 7) && !(data_type == 0 || data_type == 7 || data_type == 8) ) {
    cerr << "ERROR: Field " << field_num << " process_type is Analysis but data_type is a forecast." << endl;
    return -1;
  }

  if(process_type == 1 && !(data_type == 0 || data_type == 7 || data_type == 8) ) {
    cerr << "ERROR: Field " << field_num << " process_type is Initialization but data_type is a forecast." << endl;
    return -1;
  }

  if(file_data_type == 0 && forecastDelta == 0 && process_type == 2) {
    if(_params->debug)
      cerr << "WARNING: Field " << field_num << " is of type forecast but has no mdv " <<
        "forecast time. " << endl << "Changing process_type to Analysis." << endl;
    process_type = 0;
    _params->_output_fields[field_num].process_type = 0;
  }
  if(forecastDelta == 0 && (process_type == 2 || process_type == 3 ||
                            process_type == 4 || process_type == 5 ||
                            process_type == 6))
    {
      cerr << "ERROR: Field " << field_num << " is of type forecast but has no mdv " <<
        "forecast time. " << endl;
      return -1;
    }

  if(file_data_type == 1 && forecastDelta > 0 && process_type == 0) {
    if(_params->debug)
      cerr << "ERROR: Field " << field_num << " is of type non-forecast but has a mdv " <<
        "forecast time. " << endl << "Changing process_type to Forecast." << endl;
    process_type = 2;
    _params->_output_fields[field_num].process_type = 2;
  }
  if(forecastDelta > 0 && (process_type == 0 || process_type == 1 || process_type == 8))
    {
      cerr << "ERROR: Field " << field_num << " is of type non-forecast but has a mdv " <<
        "forecast time. " << endl;
      return -1;
    }

  //
  // Additional checks that certain fields types are not mixed within a grib2 file
  if(field_num > 0 && (process_type == 0 || process_type == 1 || process_type == 8) &&
     (lastProcessType == 2 || lastProcessType == 3 || lastProcessType == 4 ||
      lastProcessType == 5 || lastProcessType == 6))
    {
      cerr << "WARNING: Mixing forecast and non-forecast data into one grib2 file is " <<
        "not recommended." << endl;
    }

  if(field_num > 0 && (lastProcessType == 0 || lastProcessType == 1 || lastProcessType == 8) &&
     (process_type == 2 || process_type == 3 || process_type == 4 ||
      process_type == 5 || process_type == 6))
    {
      cerr << "WARNING: Mixing forecast and non-forecast data into one grib2 file is " <<
        "not recommended." << endl;
    }

  if(field_num > 0 && ( (lastProcessType == 8 && process_type != 8) ||
                        (process_type == 8 && lastProcessType != 8) ) )
    {
      cerr << "WARNING: Mixing observation data with any other process_type into one grib2 file is " <<
        "not recommended." << endl;
    }


  if(_params->_output_fields[field_num].data_type == 0) {
    prodDefNum = 0;
    Grib2::Template4_pt_0 *template_4_pt_0 = new Grib2::Template4_pt_0();

    template_4_pt_0->_processType = process_type;
    template_4_pt_0->_backgrdProcessId = _params->background_process_id;
    template_4_pt_0->_hoursObsDataCutoff = 0;
    template_4_pt_0->_minutesObsDataCutoff = 0;
    template_4_pt_0->_timeRangeUnit = timeUnit;
    template_4_pt_0->_forecastTime = forecastTime;
    template_4_pt_0->_firstSurfaceType = firstSurfaceType;
    template_4_pt_0->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_0->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_0->_secondSurfaceType = secondSurfaceType;
    template_4_pt_0->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_0->_scaleValSecondSurface = (si32)(secondValue + .5);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_0;

  } else if(_params->_output_fields[field_num].data_type == 1) {
    prodDefNum = 1;
    Grib2::Template4_pt_1 *template_4_pt_1 = new Grib2::Template4_pt_1();

    template_4_pt_1->_processType = process_type;
    template_4_pt_1->_backgrdProcessId = _params->background_process_id;
    template_4_pt_1->_hoursObsDataCutoff = 0;
    template_4_pt_1->_minutesObsDataCutoff = 0;
    template_4_pt_1->_timeRangeUnit = timeUnit;
    template_4_pt_1->_forecastTime = forecastTime;
    template_4_pt_1->_firstSurfaceType = firstSurfaceType;
    template_4_pt_1->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_1->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_1->_secondSurfaceType = secondSurfaceType;
    template_4_pt_1->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_1->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_1->_ensembleType = _params->_output_fields[field_num].prod_type;
    template_4_pt_1->_perturbationNum = _params->_output_fields[field_num].user_data_value;
    template_4_pt_1->_numForecasts = _params->_output_fields[field_num].num_forecasts;

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_1;

  } else if(_params->_output_fields[field_num].data_type == 2) {
    prodDefNum = 2;
    Grib2::Template4_pt_2 *template_4_pt_2 = new Grib2::Template4_pt_2();

    template_4_pt_2->_processType = process_type;
    template_4_pt_2->_backgrdProcessId = _params->background_process_id;
    template_4_pt_2->_hoursObsDataCutoff = 0;
    template_4_pt_2->_minutesObsDataCutoff = 0;
    template_4_pt_2->_timeRangeUnit = timeUnit;
    template_4_pt_2->_forecastTime = forecastTime;
    template_4_pt_2->_firstSurfaceType = firstSurfaceType;
    template_4_pt_2->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_2->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_2->_secondSurfaceType = secondSurfaceType;
    template_4_pt_2->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_2->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_2->_derivedForecastType = _params->_output_fields[field_num].prod_type;
    template_4_pt_2->_numForecasts = _params->_output_fields[field_num].num_forecasts;

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_2;

  } else if(_params->_output_fields[field_num].data_type == 5) {
    prodDefNum = 5;
    Grib2::Template4_pt_5 *template_4_pt_5 = new Grib2::Template4_pt_5();

    template_4_pt_5->_processType = process_type;
    template_4_pt_5->_backgrdProcessId = _params->background_process_id;
    template_4_pt_5->_hoursObsDataCutoff = 0;
    template_4_pt_5->_minutesObsDataCutoff = 0;
    template_4_pt_5->_timeRangeUnit = timeUnit;
    template_4_pt_5->_forecastTime = forecastTime;
    template_4_pt_5->_firstSurfaceType = firstSurfaceType;
    template_4_pt_5->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_5->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_5->_secondSurfaceType = secondSurfaceType;
    template_4_pt_5->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_5->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_5->_forecastProbability = _params->_output_fields[field_num].user_data_value;
    template_4_pt_5->_numberOfForcastProbs = _params->_output_fields[field_num].num_forecasts;
    template_4_pt_5->_proababilityType = _params->_output_fields[field_num].prod_type;

    double lowerLimit = _params->_output_fields[field_num].lower_limit;
    double upperLimit = _params->_output_fields[field_num].upper_limit;
    int scaleFactorLower = 0, scaleFactorUpper = 0;
    _scaleFactorValue(lowerLimit, scaleFactorLower);
    _scaleFactorValue(upperLimit, scaleFactorUpper);

    template_4_pt_5->_scaleFactorLowerLimit = scaleFactorLower;
    template_4_pt_5->_scaleValLowerLimit = (si32)(lowerLimit + .5);
    template_4_pt_5->_scaleFactorUpperLimit = scaleFactorUpper;
    template_4_pt_5->_scaleValUpperLimit = (si32)(upperLimit + .5) ;

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_5;

  } else if(_params->_output_fields[field_num].data_type == 6) {
    prodDefNum = 6;
    Grib2::Template4_pt_6 *template_4_pt_6 = new Grib2::Template4_pt_6();

    template_4_pt_6->_processType = process_type;
    template_4_pt_6->_backgrdProcessId = _params->background_process_id;
    template_4_pt_6->_hoursObsDataCutoff = 0;
    template_4_pt_6->_minutesObsDataCutoff = 0;
    template_4_pt_6->_timeRangeUnit = timeUnit;
    template_4_pt_6->_forecastTime = forecastTime;
    template_4_pt_6->_firstSurfaceType = firstSurfaceType;
    template_4_pt_6->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_6->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_6->_secondSurfaceType = secondSurfaceType;
    template_4_pt_6->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_6->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_6->_percentileValue = _params->_output_fields[field_num].user_data_value;

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_6;

  } else if(_params->_output_fields[field_num].data_type == 7) {
    prodDefNum = 7;
    Grib2::Template4_pt_7 *template_4_pt_7 = new Grib2::Template4_pt_7();

    template_4_pt_7->_processType = process_type;
    template_4_pt_7->_backgrdProcessId = _params->background_process_id;
    template_4_pt_7->_hoursObsDataCutoff = 0;
    template_4_pt_7->_minutesObsDataCutoff = 0;
    template_4_pt_7->_timeRangeUnit = timeUnit;
    template_4_pt_7->_forecastTime = forecastTime;
    template_4_pt_7->_firstSurfaceType = firstSurfaceType;
    template_4_pt_7->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_7->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_7->_secondSurfaceType = secondSurfaceType;
    template_4_pt_7->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_7->_scaleValSecondSurface = (si32)(secondValue + .5);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_7;

  } else if(_params->_output_fields[field_num].data_type == 8) {
    prodDefNum = 8;
    Grib2::Template4_pt_8 *template_4_pt_8 = new Grib2::Template4_pt_8();

    template_4_pt_8->_processType = process_type;
    template_4_pt_8->_backgrdProcessId = _params->background_process_id;
    template_4_pt_8->_hoursObsDataCutoff = 0;
    template_4_pt_8->_minutesObsDataCutoff = 0;
    template_4_pt_8->_timeRangeUnit = timeUnit;
    template_4_pt_8->_forecastTime = forecastTime;
    template_4_pt_8->_firstSurfaceType = firstSurfaceType;
    template_4_pt_8->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_8->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_8->_secondSurfaceType = secondSurfaceType;
    template_4_pt_8->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_8->_scaleValSecondSurface = (si32)(secondValue + .5);

    date_time_t *tmpTimePtr = udate_time(reference_time + forecastDelta);
    template_4_pt_8->_year = tmpTimePtr->year;
    template_4_pt_8->_month = tmpTimePtr->month;
    template_4_pt_8->_day = tmpTimePtr->day;
    template_4_pt_8->_hour = tmpTimePtr->hour;
    template_4_pt_8->_minute = tmpTimePtr->min;
    template_4_pt_8->_second = tmpTimePtr->sec;

    template_4_pt_8->_numTimeIntervals = 1;
    template_4_pt_8->_numMissingVals = 0;

    Grib2::ProdDefTemp::interval_t intrv;
    intrv._processId = _params->_output_fields[field_num].time_interval_type;
    intrv._timeIncrementType = 2;
    intrv._timeRangeUnit = timeRangeUnit;
    intrv._timeRangeLen = timeRangeLen;
    intrv._timeIncrUnit = 255;
    intrv._timeIncrement = 0;
    template_4_pt_8->_interval.push_back(intrv);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_8;

  } else if(_params->_output_fields[field_num].data_type == 9) {
    prodDefNum = 9;
    Grib2::Template4_pt_9 *template_4_pt_9 = new Grib2::Template4_pt_9();

    template_4_pt_9->_processType = process_type;
    template_4_pt_9->_backgrdProcessId = _params->background_process_id;
    template_4_pt_9->_hoursObsDataCutoff = 0;
    template_4_pt_9->_minutesObsDataCutoff = 0;
    template_4_pt_9->_timeRangeUnit = timeUnit;
    template_4_pt_9->_forecastTime = forecastTime;
    template_4_pt_9->_firstSurfaceType = firstSurfaceType;
    template_4_pt_9->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_9->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_9->_secondSurfaceType = secondSurfaceType;
    template_4_pt_9->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_9->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_9->_forecastProbability = _params->_output_fields[field_num].user_data_value;
    template_4_pt_9->_numberOfForcastProbs = _params->_output_fields[field_num].num_forecasts;
    template_4_pt_9->_proababilityType = _params->_output_fields[field_num].prod_type;

    double lowerLimit = _params->_output_fields[field_num].lower_limit;
    double upperLimit = _params->_output_fields[field_num].upper_limit;
    int scaleFactorLower = 0, scaleFactorUpper = 0;
    _scaleFactorValue(lowerLimit, scaleFactorLower);
    _scaleFactorValue(upperLimit, scaleFactorUpper);

    template_4_pt_9->_scaleFactorLowerLimit = scaleFactorLower;
    template_4_pt_9->_scaleValLowerLimit = (si32)(lowerLimit + .5);
    template_4_pt_9->_scaleFactorUpperLimit = scaleFactorUpper;
    template_4_pt_9->_scaleValUpperLimit = (si32)(upperLimit + .5) ;

    date_time_t *tmpTimePtr = udate_time(reference_time + forecastDelta);
    template_4_pt_9->_year = tmpTimePtr->year;
    template_4_pt_9->_month = tmpTimePtr->month;
    template_4_pt_9->_day = tmpTimePtr->day;
    template_4_pt_9->_hour = tmpTimePtr->hour;
    template_4_pt_9->_minute = tmpTimePtr->min;
    template_4_pt_9->_second = tmpTimePtr->sec;

    template_4_pt_9->_numTimeIntervals = 1;
    template_4_pt_9->_numMissingVals = 0;

    Grib2::ProdDefTemp::interval_t intrv;
    intrv._processId = _params->_output_fields[field_num].time_interval_type;
    intrv._timeIncrementType = 2;
    intrv._timeRangeUnit = timeRangeUnit;
    intrv._timeRangeLen = timeRangeLen;
    intrv._timeIncrUnit = 255;
    intrv._timeIncrement = 0;
    template_4_pt_9->_interval.push_back(intrv);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_9;

  } else if(_params->_output_fields[field_num].data_type == 10) {
    prodDefNum = 10;
    Grib2::Template4_pt_10 *template_4_pt_10 = new Grib2::Template4_pt_10();

    template_4_pt_10->_processType = process_type;
    template_4_pt_10->_backgrdProcessId = _params->background_process_id;
    template_4_pt_10->_hoursObsDataCutoff = 0;
    template_4_pt_10->_minutesObsDataCutoff = 0;
    template_4_pt_10->_timeRangeUnit = timeUnit;
    template_4_pt_10->_forecastTime = forecastTime;
    template_4_pt_10->_firstSurfaceType = firstSurfaceType;
    template_4_pt_10->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_10->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_10->_secondSurfaceType = secondSurfaceType;
    template_4_pt_10->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_10->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_10->_percentileValue = _params->_output_fields[field_num].user_data_value;

    date_time_t *tmpTimePtr = udate_time(reference_time + forecastDelta);
    template_4_pt_10->_year = tmpTimePtr->year;
    template_4_pt_10->_month = tmpTimePtr->month;
    template_4_pt_10->_day = tmpTimePtr->day;
    template_4_pt_10->_hour = tmpTimePtr->hour;
    template_4_pt_10->_minute = tmpTimePtr->min;
    template_4_pt_10->_second = tmpTimePtr->sec;

    template_4_pt_10->_numTimeIntervals = 1;
    template_4_pt_10->_numMissingVals = 0;

    Grib2::ProdDefTemp::interval_t intrv;
    intrv._processId = _params->_output_fields[field_num].time_interval_type;
    intrv._timeIncrementType = 2;
    intrv._timeRangeUnit = timeRangeUnit;
    intrv._timeRangeLen = timeRangeLen;
    intrv._timeIncrUnit = 255;
    intrv._timeIncrement = 0;
    template_4_pt_10->_interval.push_back(intrv);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_10;

  } else if(_params->_output_fields[field_num].data_type == 11) {
    prodDefNum = 11;
    Grib2::Template4_pt_11 *template_4_pt_11 = new Grib2::Template4_pt_11();

    template_4_pt_11->_processType = process_type;
    template_4_pt_11->_backgrdProcessId = _params->background_process_id;
    template_4_pt_11->_hoursObsDataCutoff = 0;
    template_4_pt_11->_minutesObsDataCutoff = 0;
    template_4_pt_11->_timeRangeUnit = timeUnit;
    template_4_pt_11->_forecastTime = forecastTime;
    template_4_pt_11->_firstSurfaceType = firstSurfaceType;
    template_4_pt_11->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_11->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_11->_secondSurfaceType = secondSurfaceType;
    template_4_pt_11->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_11->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_11->_ensembleType = _params->_output_fields[field_num].prod_type;
    template_4_pt_11->_perturbationNum = _params->_output_fields[field_num].user_data_value;
    template_4_pt_11->_numForecasts = _params->_output_fields[field_num].num_forecasts;

    date_time_t *tmpTimePtr = udate_time(reference_time + forecastDelta);
    template_4_pt_11->_year = tmpTimePtr->year;
    template_4_pt_11->_month = tmpTimePtr->month;
    template_4_pt_11->_day = tmpTimePtr->day;
    template_4_pt_11->_hour = tmpTimePtr->hour;
    template_4_pt_11->_minute = tmpTimePtr->min;
    template_4_pt_11->_second = tmpTimePtr->sec;

    template_4_pt_11->_numTimeIntervals = 1;
    template_4_pt_11->_numMissingVals = 0;

    Grib2::ProdDefTemp::interval_t intrv;
    intrv._processId = _params->_output_fields[field_num].time_interval_type;
    intrv._timeIncrementType = 2;
    intrv._timeRangeUnit = timeRangeUnit;
    intrv._timeRangeLen = timeRangeLen;
    intrv._timeIncrUnit = 255;
    intrv._timeIncrement = 0;
    template_4_pt_11->_interval.push_back(intrv);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_11;

  } else if(_params->_output_fields[field_num].data_type == 12) {
    prodDefNum = 12;
    Grib2::Template4_pt_12 *template_4_pt_12 = new Grib2::Template4_pt_12();

    template_4_pt_12->_processType = process_type;
    template_4_pt_12->_backgrdProcessId = _params->background_process_id;
    template_4_pt_12->_hoursObsDataCutoff = 0;
    template_4_pt_12->_minutesObsDataCutoff = 0;
    template_4_pt_12->_timeRangeUnit = timeUnit;
    template_4_pt_12->_forecastTime = forecastTime;
    template_4_pt_12->_firstSurfaceType = firstSurfaceType;
    template_4_pt_12->_scaleFactorFirstSurface = scaleFactorFirst;
    template_4_pt_12->_scaleValFirstSurface = (si32)(firstValue + .5);
    template_4_pt_12->_secondSurfaceType = secondSurfaceType;
    template_4_pt_12->_scaleFactorSecondSurface = scaleFactorSecond;
    template_4_pt_12->_scaleValSecondSurface = (si32)(secondValue + .5);
    template_4_pt_12->_derivedForecastType = _params->_output_fields[field_num].prod_type;
    template_4_pt_12->_numForecasts = _params->_output_fields[field_num].num_forecasts;

    date_time_t *tmpTimePtr = udate_time(reference_time + forecastDelta);
    template_4_pt_12->_year = tmpTimePtr->year;
    template_4_pt_12->_month = tmpTimePtr->month;
    template_4_pt_12->_day = tmpTimePtr->day;
    template_4_pt_12->_hour = tmpTimePtr->hour;
    template_4_pt_12->_minute = tmpTimePtr->min;
    template_4_pt_12->_second = tmpTimePtr->sec;

    template_4_pt_12->_numTimeIntervals = 1;
    template_4_pt_12->_numMissingVals = 0;

    Grib2::ProdDefTemp::interval_t intrv;
    intrv._processId = _params->_output_fields[field_num].time_interval_type;
    intrv._timeIncrementType = 2;
    intrv._timeRangeUnit = timeRangeUnit;
    intrv._timeRangeLen = timeRangeLen;
    intrv._timeIncrUnit = 255;
    intrv._timeIncrement = 0;
    template_4_pt_12->_interval.push_back(intrv);

    (*prodDefTemplate) = (Grib2::ProdDefTemp *) template_4_pt_12;

  } else {
    cerr << "Variable data_type " << _params->_output_fields[field_num].data_type;
    cerr << " is unimplemented." << endl;
    prodDefNum = -1;
  }

  (*prodDefTemplate)->setProcessID(_params->forecast_process_id);
  return prodDefNum;
}
