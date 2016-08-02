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
 * @file Pressure2Height.cc
 *
 * @class Pressure2Height
 *
 * Pressure2Height is the top level application class.
 *  
 * @date 11/30/2010
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

#include "Pressure2Height.hh"
#include "Params.hh"

using namespace std;

// Global variables

Pressure2Height *Pressure2Height::_instance =
     (Pressure2Height *)NULL;

const double Pressure2Height::MISSING_HEIGHT_VALUE = -999.0;

// ************************************************************************
//  Standard Atmosphere Profile:
// ************************************************************************

//    Pressure (hPa)

static const fl32  _stdPres[] =
{ 1013.25, 954.57, 898.71, 845.50, 794.90,
   746.75, 700.99, 657.53, 616.29, 577.15,
   540.07, 504.90, 471.65, 440.20, 410.46,
   382.35, 355.82, 330.81, 307.24, 285.07,
   264.19, 244.58, 234.53, 226.19, 193.38,
   165.33, 141.35, 120.86, 103.30,  88.34,
    75.53,  64.57,  55.21 };

static const int _numStdPres = 33;

//    Height (meters) 
static const fl32 _stdZ[] =
{     0.,   500.,  1000.,  1500.,  2000.,
   2500.,  3000.,  3500.,  4000.,  4500.,
   5000.,  5500.,  6000.,  6500.,  7000.,
   7500.,  8000.,  8500.,  9000.,  9500.,
  10000., 10500., 10769., 11000., 12000.,
  13000., 14000., 15000., 16000., 17000.,
  18000., 19000., 20000. };


static const double SFC_TEMP = 288.15;

/*********************************************************************
 * Constructor
 */

Pressure2Height::Pressure2Height(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "Pressure2Height::Pressure2Height()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Pressure2Height *)NULL);
  
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
}


/*********************************************************************
 * Destructor
 */

Pressure2Height::~Pressure2Height()
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

Pressure2Height *Pressure2Height::Inst(int argc, char **argv)
{
  if (_instance == (Pressure2Height *)NULL)
    new Pressure2Height(argc, argv);
  
  return(_instance);
}

Pressure2Height *Pressure2Height::Inst()
{
  assert(_instance != (Pressure2Height *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool Pressure2Height::init()
{
  static const string method_name = "Pressure2Height::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void Pressure2Height::run()
{
  static const string method_name = "Pressure2Height::run()";
  
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
 * _convertField()
 */

bool Pressure2Height::_convertField(MdvxField &input_field,
				    const MdvxField &temp_field,
				    const string &output_field_name,
				    const string &output_field_name_long) const
{
  static const string method_name = "Pressure2Height::_convertField()";
  
  // Get the information about the fields.  Note that we ensure on input that
  // the fields use the same projection.

  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  fl32 *input_data = (fl32 *)input_field.getVol();
  
  Mdvx::field_header_t temp_field_hdr = temp_field.getFieldHeader();
  Mdvx::vlevel_header_t temp_vlevel_hdr = temp_field.getVlevelHeader();
  fl32 *temp_data = (fl32 *)temp_field.getVol();
  
  // Each grid point is processed separately

  int plane_size = input_field_hdr.nx * input_field_hdr.ny;
  
  for (int i = 0; i < plane_size; ++i)
  {
    double pressure = input_data[i];
    
    // Check for missing data

    if (pressure == input_field_hdr.bad_data_value ||
	pressure == input_field_hdr.missing_data_value)
    {
      input_data[i] = MISSING_HEIGHT_VALUE;
      continue;
    }
    
    // First, find the pressure levels that surround the pressure value
    // (remember that pressure value decrease with height)

    if (pressure > temp_vlevel_hdr.level[0] ||
	pressure < temp_vlevel_hdr.level[temp_field_hdr.nz])
    {
//      if (_params->verbose)
//      {
//	cerr << "Pressure value outside of vertical limits of temperature data" << endl;
//	cerr << "     pressure = " << pressure << endl;
//      }
      
      input_data[i] = MISSING_HEIGHT_VALUE;
      continue;
    }
    
    double pres_bottom;
    double pres_top;
    
    double temp_bottom;
    double temp_top;
    
    for (int z = 0; z < temp_field_hdr.nz - 1; ++z)
    {
      if (pressure < temp_vlevel_hdr.level[z] &&
	  pressure > temp_vlevel_hdr.level[z+1])
      {
	pres_bottom = temp_vlevel_hdr.level[z];
	pres_top = temp_vlevel_hdr.level[z + 1];

	temp_bottom = temp_data[(z * plane_size) + i];
	temp_top = temp_data[((z+1) * plane_size) + i];
	
	break;
      }
    }
    
    if (temp_bottom == temp_field_hdr.bad_data_value ||
	temp_bottom == temp_field_hdr.missing_data_value ||
	temp_top == temp_field_hdr.bad_data_value ||
	temp_top == temp_field_hdr.missing_data_value)
    {
      if (_params->verbose)
	cerr << "Temperature value missing" << endl;
      
      input_data[i] = MISSING_HEIGHT_VALUE;
      continue;
    }
    
    // Calculate the logarithmic pressure ratio

    double ratio =
      (log(pres_bottom) - log(pressure)) / (log(pres_bottom) - log(pres_top));
    
    // Interpolate the associated temperature values

    double temperature = (ratio * (temp_top - temp_bottom)) + temp_bottom;
    
    // Convert the pressure value to height

    input_data[i] = _pressure2Height(pressure, temperature);
    
  } /* endfor - i */
  
  // Update the input field header to match the new units

  input_field_hdr.missing_data_value = MISSING_HEIGHT_VALUE;
  input_field_hdr.bad_data_value = MISSING_HEIGHT_VALUE;
  input_field_hdr.min_value = 0.0;
  input_field_hdr.max_value = 0.0;
  STRcopy(input_field_hdr.field_name_long, output_field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(input_field_hdr.field_name, output_field_name.c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(input_field_hdr.units, "ft", MDV_UNITS_LEN);
  
  input_field.setFieldHeader(input_field_hdr);
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool Pressure2Height::_initTrigger(void)
{
  static const string method_name = "Pressure2Height::_initTrigger()";
  
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
 * _pressure2Height()
 */

double Pressure2Height::_pressure2Height(const double pres,
					 const double temp) const
{
  static const string method_name = "Pressure2Height::_pressure2Height()";
  
  // Check for an acceptable temperature value

  if (temp < 150.0 || temp > 288.15)
  {
//    if (_params->verbose)
//      cerr << "*** Temperature value outside of acceptable range: " << temp << endl;
    
    return MISSING_HEIGHT_VALUE;
  }
  
  // If the given pressure is greater than the pressure at the lowest
  // standard atmosphere level, we are at the surface

  if (pres > _stdPres[0])
    return 0.0;
  
  // See if the given pressure is lower than the pressure at the highest
  // standard atmosphere level represented in our table.

  if (pres < _stdPres[_numStdPres-1])
  {
    double std_t = 218.15;
    double tmp_term = 14.6429 * log(_stdPres[_numStdPres-1] / pres);
    double ht_m = _stdZ[_numStdPres-1] + tmp_term * (temp + std_t);
    return ht_m * 3.28084;
  }
  
  // If we get here, the pressure is within the limits of our table.  See
  // where the pressure lies and do the appropriate interpolation.

  for (int p = 0; p < _numStdPres - 1; ++p)
  {
    if (pres > _stdPres[p] || pres < _stdPres[p+1])
      continue;
    
    // Check for tropopause

    if (p <= 22)
    {
      // Somewhere in the tropopause

      double std_t = 288.15 - _stdZ[p] * 0.0065;
      double tmp_term = 14.6429 * log(_stdPres[p] / pres);
      double ht_m =
	_stdZ[p] + tmp_term * (temp + std_t) / (1.0 + 0.0065 * tmp_term);
      return ht_m * 3.28084;
    }
    else
    {
      // Isothermal region at or above the tropopause

      double std_t = 288.15;
      double tmp_term = 14.6429 * log(_stdPres[p] / pres);
      double ht_m = _stdZ[p] + tmp_term * (temp + std_t);
      return ht_m * 3.28084;
    }
    
  } /* endfor - p */
  
  if (_params->verbose)
    cerr << "**** Pressure level not found in standard atmosphere table" << endl;
  
  return MISSING_HEIGHT_VALUE;
}


/*********************************************************************
 * _processData()
 */

bool Pressure2Height::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "Pressure2Height::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: "
	 << DateTime::str(trigger_info.getIssueTime()) << endl;
  
  // Read in the input file, containing the single input field

  DsMdvx input_file;
  
  if (!_readInputFile(input_file,
		      trigger_info.getIssueTime(),
		      trigger_info.getForecastTime()))
    return false;
  
  MdvxProj proj(input_file.getField(0)->getFieldHeader());
  
  // Read in the 3D temperature field

  MdvxField *temp_field;
  
  if ((temp_field = _readModelField(_params->temperature_field,
				    trigger_info.getIssueTime(),
				    trigger_info.getForecastTime(),
				    proj)) == 0)
    return false;
  
  // Convert the input fields from pressure to height

  for (int i = 0; i < input_file.getNFields(); ++i)
  {
    MdvxField *field = input_file.getField(i);
    
    if (!_convertField(*field, *temp_field,
		       _params->_input_fields[i].output_field_name,
		       _params->_input_fields[i].output_field_name_long))
    {
      delete temp_field;
      
      return false;
    }
    
  }
  
  delete temp_field;
  
  // Write the output file

  if (!_writeFile(input_file, _params->output_url))
    return false;
  
  return true;
}


/*********************************************************************
 * _readModelField()
 */

MdvxField *Pressure2Height::_readModelField(Params::model_field_info_t &field_info,
					    const DateTime &data_time,
					    const DateTime &fcst_time,
					    const MdvxProj &proj) const
{
  static const string method_name = "Pressure2Height::_readModelField()";
  
  // Create the file object

  DsMdvx mdvx;
  
  // Set up the read request

  mdvx.clearRead();
  
  if (fcst_time == DateTime::NEVER)
    mdvx.setReadTime(Mdvx::READ_CLOSEST,
		     field_info.url,
		     field_info.max_valid_secs,
		     data_time.utime());
  else
    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
		     field_info.url,
		     field_info.max_valid_secs,
		     data_time.utime(),
		     fcst_time.utime() - data_time.utime());

  string field_name = field_info.field_name;
  if (field_name == "")
    mdvx.addReadField(field_info.field_num);
  else
    mdvx.addReadField(field_info.field_name);
  
  mdvx.setReadRemap(proj);
  
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
    
    return 0;
  }
  
  // Extract the read field and return a copy

  return new MdvxField(*mdvx.getField(0));
}


/*********************************************************************
 * _readInputFile()
 */

bool Pressure2Height::_readInputFile(DsMdvx &mdvx,
				const DateTime &data_time,
				const DateTime &fcst_time) const
{
  static const string method_name = "Pressure2Height::_readInputFile()";
  
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

  for (int i = 0; i < _params->input_fields_n; ++i)
  {
    if (_params->use_field_names)
      mdvx.addReadField(_params->_input_fields[i].field_name);
    else
      mdvx.addReadField(_params->_input_fields[i].field_num);
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
  
  // Make sure that all of the fields use the same projection

  MdvxProj proj1(mdvx.getField(0)->getFieldHeader());
  
  for (int i = 1; i < mdvx.getNFields(); ++i)
  {
    MdvxProj proj(mdvx.getField(i)->getFieldHeader());
    
    if (proj != proj1)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "All input field projections must match" << endl;
      cerr << "Projection for <" << mdvx.getField(0)->getFieldName()
	   << "> doesn't match projection for <"
	   << mdvx.getField(i)->getFieldName() << ">" << endl;
      
      return false;
    }
    
  }
  
  return true;
}


/*********************************************************************
 * _updateMasterHeader()
 */

void Pressure2Height::_updateMasterHeader(DsMdvx &mdvx) const
{
  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();

  STRcopy(master_hdr.data_set_info, "Pressure2Height", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Pressure2Height", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->input_url,
	  MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}


/*********************************************************************
 * _writeFile()
 */

bool Pressure2Height::_writeFile(DsMdvx &mdvx,
				 const string &url) const
{
  static const string method_name = "Pressure2Height::_writeFile()";
  
  // Update the master header to reflect the source of this file

  _updateMasterHeader(mdvx);
  
  // Write the file

  mdvx.setIfForecastWriteAsForecast();
  
  if (mdvx.writeToDir(url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}
