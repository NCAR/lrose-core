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
/*
 *  $Id: AccessA2Mdv.cc,v 1.17 2016/03/07 01:22:59 dixon Exp $
 */

# ifndef    lint
static char RCSid[] = "$Id: AccessA2Mdv.cc,v 1.17 2016/03/07 01:22:59 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	AccessA2Mdv
//
// Author:	George McCabe
//
// Date:	Mon Oct 17 21:00:36 2011
//
// Description: 
//
//

// C++ include files
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <netcdf.hh>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <rapmath/math_macros.h>

// Local include files
#include "Args.hh"
#include "Params.hh"
#include "AccessA2Mdv.hh"
#include "InputStrategy.hh"
#include "FilelistInputStrategy.hh"
#include "LdataInputStrategy.hh"
#include "RealtimeDirInputStrategy.hh"
#include "Navigation.hh"


using namespace std;

// the singleton itself
AccessA2Mdv *AccessA2Mdv::_instance = 0;

// define any constants
const string AccessA2Mdv::_className = "AccessA2Mdv";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

AccessA2Mdv::AccessA2Mdv(int argc, char **argv) :
  _isOK(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0),
  _inputStrategy(0),
  _navigation(0)
{
  const string methodName = _className + string("::Constructor");
  _errStr = string("ERROR: ") + methodName;

 // Make sure the singleton wasn't already created.
  assert(_instance == 0);

  // Set the singleton instance pointer
  _instance = this;

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  // display ucopyright message and RCSid
  ucopyright(const_cast<char*>(_progName.c_str()));

  // get command line args
  _args = new Args(argc, const_cast<char**>(argv), _progName);
  if (!_args->isOK) {
    _errStr += "\tProblem with command line arguments.\n";
    _isOK = false;
  }

  // get TDRP params
  _params = new Params();
  char *paramsPath = const_cast< char *>(string("unknown").c_str());
  if (_params->loadFromArgs(argc, argv, _args->override.list, 
			    &paramsPath)) {
    _errStr += "\tProblem with TDRP parameters.\n";
    _isOK = false;
    return;
  }

  if(!_setupInput()) {
    cerr << "Problem setting up input processing." << endl;
    _isOK = false;
  }
    
  _navigation = new Navigation(_params);

  // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(), _params->instance,
		PROCMAP_REGISTER_INTERVAL);
  cerr << "started procmap with progname: " << _progName << " and instance: " 
       << _params->instance << endl;
  PMU_force_register( "STARTING UP." );

  return;
  
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
AccessA2Mdv::~AccessA2Mdv()
{
  delete _args;
  delete _params;
  delete _inputStrategy;
  delete _navigation;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::instance
//
// Description:	Retrieves the singleton instance.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

AccessA2Mdv* 
AccessA2Mdv::instance( int argc, char **argv )
{
  if ( _instance == 0 ) {
    _instance = new AccessA2Mdv(argc, argv);
  }
  
  return(_instance);
}

AccessA2Mdv* 
AccessA2Mdv::instance()
{
  assert(_instance != 0 );
  return( _instance );
}



//////////////////////////////////////////////////
// run - Run the algorithm.

int 
AccessA2Mdv::run()
{
  const string methodName = _className + string( "::run" );
  
  string filePath;

  //
  // Gather files
  //
  while(true) {
    
    if((filePath = _inputStrategy->next()) == "") {
      if(_params->mode == Params::REALTIME) {
	sleep(10);
	continue;
      }
      else {
	break;
      } // endif -- _params->mode == Params::REALTIME
    } // endif -- (filePath = _inputStrategy ...

    if(_args->printContents()) {

      _printContents(filePath);

    }
    else {
 
      //
      // Process the file
      // 
      string msg = "File " + filePath + " will be processed.";
      PMU_auto_register(msg.c_str());
      if(_params->debug)
	cout << msg << endl;
      
      if(!_processFile(filePath)) {
	msg = "WARNING: Failed to process " + filePath; 
	PMU_auto_register(msg.c_str());
	cerr << msg << endl;
      }

    } // endif -- _args->printContents()

  } // endwhile -- true
   
  return 0;
}
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_setupInput
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool
AccessA2Mdv::_setupInput()
{
  const string methodName = _className + string( "::_setupInput" );

  switch(_params->mode) {
  case Params::REALTIME:
     _inputStrategy =
       new LdataInputStrategy(_params->inputDir,
			      _params->maxInputDataAge,
			      (LdataInputStrategy::heartbeat_t)PMU_auto_register,
			      _params->debug);
    break;
  case Params::REALTIME_DIR:
     _inputStrategy =
       new RealtimeDirInputStrategy(_params->inputDir,
				    _params->inputSubstring,
				    _params->maxInputDataAge,
				    (RealtimeDirInputStrategy::heartbeat_t)PMU_auto_register,
				    _params->debug);
    break;
  case Params::FILELIST:
    _inputStrategy = new FilelistInputStrategy(_args->inputFileList,
					       _params->debug);
    break;
  default:
    cerr << "ERROR: " << methodName << endl;
    cerr << "Invalid mode given for realtime processing" << endl;
    return false;
  } // endswitch - _params->mode


  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_processFile
//
// Description:	processes the file
//
// Returns:	
//
// Notes:
//
//

bool 
AccessA2Mdv::_processFile(const string& file_path)
{
  const string methodName = _className + string( "::_processFile" );

  PMU_auto_register("Processing file");

  NcFile netcdfFile(file_path.c_str());

  if(!netcdfFile.is_valid()) {
    cerr << "ERROR - " << methodName << endl;
    cerr << file_path << " is not a valid netcdf file." << endl;
    return false;
  }

  // declare an error object

  NcError err(NcError::silent_nonfatal);


  // check that this is a valid file

  if(_checkFile(netcdfFile) == false) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  Not a valid ACCESS file" << endl;
    cerr << "  File: " << file_path << endl;
    return false;
  }

  _outMdvx.setDebug(1);

  // Set up the master header
  _setMasterHeader(file_path, netcdfFile);

  if (_navigation->initialize(netcdfFile) == false) {
    cerr << "ERROR - " << methodName << endl;
    return false;
  }

  vector< MdvxField* > outMdvxFields;
  for(int i = 0; i < _params->output_fields_n; i++) {
   
    NcVar *outNcVar = netcdfFile.get_var(_params->_output_fields[i].nc_var_name);
    if(outNcVar == 0) {
      cerr << "ERROR - " << methodName << endl;
      cerr << "  variable missing" << _params->_output_fields[i].nc_var_name << endl;
    } 
    else {
      cout << "Processing: " << _params->_output_fields[i].nc_var_name << endl;
    }

    MdvxField *outMdvxField = 0;

    if(_navigation->processNcVar(_params->_output_fields[i].mdv_short_name, 
                                   _params->_output_fields[i].mdv_long_name, 
				   _params->_output_fields[i].units, 
	         		   _dateTime,
				   outNcVar,
				   &outMdvxField) == false) {
        cerr << "ERROR - " << methodName << endl;      
        cerr << "  Failed to convert NcVar to MdvxField" << endl;
      }
      //    }

    outMdvxFields.push_back(outMdvxField);
  }
  
  // perform any conversion and prepare for output
  for(size_t i = 0; i < outMdvxFields.size(); i++) {
    outMdvxFields[i]->convertType(static_cast<Mdvx::encoding_type_t>(_params->output_encoding_type),
		     static_cast<Mdvx::compression_type_t>(_params->output_compression_type));
    _outMdvx.addField(outMdvxFields[i]);
  }

  _navigation->cleanup();

  _outMdvx.setWriteAsForecast();
  _outMdvx.updateMasterHeader();
  //
  // Write the MdvxField objects
  //
  string msg = "Writing MdvxField objects.";
  PMU_auto_register(msg.c_str());
  if(_params->debug)
    cout << msg << endl;
  
  if (_outMdvx.writeToDir(_params->outputUrl) != 0)
    {
      _errStr += "WARNING: ";
      _errStr += "Error writing data for time ";
      _errStr += DateTime(_outMdvx.getMasterHeader().time_centroid).dtime();
      _errStr +=  " to  ";
      _errStr +=  _params->outputUrl;
      cerr << "\n*** Skipping ***\n\n";
      cerr << _errStr;
      _clearErrStr();    

      msg = "WARNING: Could not write MdvxField objects.";
      PMU_auto_register(msg.c_str());
    }

  _outMdvx.clear();
  netcdfFile.close();

  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_checkFile
//
// Description:	checks that the file has the expected attributes, 
//		dimensions and variables
//
// Returns:	true or false
//
// Notes:	checks for all items and keeps an error count.
//
//

bool 
AccessA2Mdv::_checkFile(NcFile &ncf)
{
  const string methodName = _className + string( "::_checkFile" );

  int errorCount = 0;
  
  if (ncf.get_att(_params->netcdf_gattr_source) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  attribute missing: " << _params->netcdf_gattr_source << endl;
    errorCount++;
  }
  
  if (ncf.get_att(_params->netcdf_gattr_version) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  attribute missing: " << _params->netcdf_gattr_version << endl;
    errorCount++;
  }
  
  if (ncf.get_dim(_params->netcdf_dim_n_lat) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  dimension missing: " << _params->netcdf_dim_n_lat << endl;
    errorCount++;
  }
  
  if (ncf.get_dim(_params->netcdf_dim_n_lon) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  dimension missing: " << _params->netcdf_dim_n_lon << endl;
    errorCount++;
  }
  
  if(_params->inputDimension == 3)
  {
    if (ncf.get_dim(_params->netcdf_dim_n_lvl) == 0) {
      cerr << "ERROR - " << methodName << endl;
      cerr << "  dimension missing: " << _params->netcdf_dim_n_lvl << endl;
      errorCount++;
    }
  }

  if (ncf.get_dim(_params->netcdf_dim_n_time) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  dimension missing: " << _params->netcdf_dim_n_time << endl;
    errorCount++;
  }
  
  if (ncf.get_var(_params->netcdf_var_base_time) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  variable missing" << _params->netcdf_var_base_time << endl;
    errorCount++;
  }
  

  if (ncf.get_var(_params->netcdf_var_time_offset) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  variable missing" << _params->netcdf_var_time_offset << endl;
    errorCount++;
  }
  

  if (ncf.get_var(_params->netcdf_var_latitude) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  variable missing" << _params->netcdf_var_latitude << endl;
    errorCount++;
  }
  
  if (ncf.get_var(_params->netcdf_var_longitude) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  variable missing" << _params->netcdf_var_longitude << endl;
    errorCount++;
  }
  
  for(int i = 0; i < _params->output_fields_n; i++) {
    if (ncf.get_var(_params->_output_fields[i].nc_var_name) == 0) {
      cerr << "ERROR - " << methodName << endl;
      cerr << "  variable missing" << _params->_output_fields[i].nc_var_name << endl;
      errorCount++;
    }
  }
  
  if(errorCount > 0) {
    return false;
  }
  else {
    return true;
  }
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_printContents
//
// Description:	processes the file
//
// Returns:	
//
// Notes:
//
//

bool 
AccessA2Mdv::_printContents(const string& file_path)
{

  NcFile netcdfFile(file_path.c_str());

   if(!netcdfFile.is_valid()) {
    cerr << file_path << " is not a valid netcdf file." << endl;
    return false;
  }

  int numAttributes = netcdfFile.num_atts();
  int numDimensions = netcdfFile.num_dims();
  int numVariables = netcdfFile.num_vars();

  cout << "Printing contents of " << file_path << endl;

  // print out the global attributes
  //
  // all global attributes are strings
  //
  cout << "  Global attributes:" << endl;
  for(int i = 0; i < numAttributes; ++i) {
    NcAtt* att = netcdfFile.get_att(i);
    if(att == NULL)
    {
      cerr << "Global attribute " << i << " is not found in netcdf file." << endl;
    }
    else
    {
      string name = att->name();
      string val = att->as_string(0);
      cout << "    " << name << ": " << val << endl;
    }
  }
  cout << endl;

  // print out the dimensions
  cout << "  Dimensions:" << endl;
  for(int i = 0; i < numDimensions; ++i) {
    NcDim* dim = netcdfFile.get_dim(i);
    if(dim == NULL)
    {
      cerr << "Dimension " << i << " is not found in netcdf file." << endl;
    }
    else
    {
      string name = dim->name();
      long size = dim->size();
      cout << "    " << name << ": " << size << endl;
    }
  }
  cout << endl;

  // for each variable print:
  //  name, valid
  cout << "  Variables:" << endl;
  for(int i = 0; i < numVariables; ++i) {
    NcVar* var = netcdfFile.get_var(i);
    if(var == NULL)
    {
      cerr << "Variable " << i << " is not found in netcdf file." << endl;
    }
    else
    {
      string varName = var->name();
      cout << "    " << varName << ":" << endl;
    
      for(int j = 0; j < var->num_atts(); ++j) {
        NcAtt* att = var->get_att(j);
        if(att == NULL)
	{
          cerr << "Attribute " << j << " (Variable " << i << ") is not found in netcdf file." << endl; 
        }
        else
	{
          string name = att->name();
          cout << "      " << name << ": ";
          if(static_cast<nc_type>(att->type()) == NC_CHAR) {
    	    string val = att->as_string(0);
  	    cout << val << endl;
          }
          else {
	    for(int k = 0; k < att->num_vals(); ++k) {
	      string val = att->as_string(k);
	      cout << val << "   ";
	    }
	    cout << endl;
          }
	}
      }
    }
  }

  netcdfFile.close();

  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_getObservationTime
//
// Description:	
//
// Returns:	the observation time as unix time
//
// Notes:	assume the filename format is 
//              ACCESS-A_YYYYMMDDHH_FFF_pressure.nc, where
//		FFF is forecast hour
//
//

void
AccessA2Mdv::_getObservationTime(const string& file_path)
{
  //
  // pullout the obs time from the filename. why? time is not in the file
  //
  // 
  DateTime obsTime;
  size_t startPos;

  startPos = file_path.size() - 17;

  int year = atoi(file_path.substr(startPos, 4).c_str());
  startPos += 4;
  int month  = atoi(file_path.substr(startPos, 2).c_str());
  startPos += 2;
  int day = atoi(file_path.substr(startPos, 2).c_str());
  startPos += 2;
  int hour = atoi(file_path.substr(startPos, 2).c_str());
  _dateTime = obsTime.set(year, month, day, hour, 0, 0);

  // get forecast hour
  startPos += 4;
  int fhr = atoi(file_path.substr(startPos, 2).c_str());
  _navigation->setFhr(fhr);

  if(_params->debug) {
    cout << endl << "Observation time is:" << obsTime.getStr() << endl;
  }
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_getObservationTime
//
// Description:	
//
// Returns:	the observation time as unix time
//
// Notes:	assume the filename format is 
//              ACCESS-A_YYYYMMDDHH_FFF_pressure.nc, where
//		FFF is forecast hour
//
//

void
AccessA2Mdv::_getObservationTime(NcFile &ncf)
{
  //
  // pullout the obs time from the filename. why? time is not in the file
  //
  // 
  DateTime obsTime;

  string baseTime = ncf.get_var(_params->netcdf_var_base_date)->as_string(0);
  int year = atoi(baseTime.substr(0,4).c_str());
  int month = atoi(baseTime.substr(4,2).c_str());
  int day = atoi(baseTime.substr(6,2).c_str());
  int hour = ncf.get_var(_params->netcdf_var_base_time)->as_int(0) / 100;

  _dateTime = obsTime.set(year, month, day, hour, 0, 0);

  if(_params->debug) {
    cout << endl << "Observation time is:" << obsTime.getStr() << endl;
  }
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	AccessA2Mdv::_setMasterHeader
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
AccessA2Mdv::_setMasterHeader(const string& path, NcFile &ncf)
{
  // extract obs time from file name
  if(_params->time_source == Params::TIME_FROM_FNAME) {
    _getObservationTime(path);
  }
  else {
    _getObservationTime(ncf);
  }

  _outMdvx.clearMasterHeader();

  Mdvx::master_header_t hdr;
  memset(&hdr, 0, sizeof(Mdvx::master_header_t));

  hdr.time_gen = _dateTime;
  hdr.time_begin = _dateTime;
  hdr.time_end = _dateTime + _params->valid_time_offset;
  hdr.time_centroid = _dateTime;
  hdr.time_expire = _dateTime + _params->valid_time_offset;

  if(_params->inputDimension == 2)
  {
    hdr.data_dimension = 2;
    hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  }
  else
  {
    hdr.data_dimension = 3;
    hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
    hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  }

  hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  hdr.vlevel_included = 1;
  hdr.grid_orientation = Mdvx::ORIENT_NS_WE;
  hdr.data_ordering = Mdvx::ORDER_XYZ;

  string info = string(_params->data_set_name) + " data\n" +
    string(ncf.get_att(_params->netcdf_gattr_source)->as_string(0)) + "\n" +
    string(ncf.get_att(_params->netcdf_gattr_version)->as_string(0));


  char* cptr = STRcopy(&(hdr.data_set_info[0]), const_cast<char*>(info.c_str()), MDV_INFO_LEN);
  cptr = STRcopy(&(hdr.data_set_name[0]), _params->data_set_name, MDV_NAME_LEN);
  cptr = STRcopy(&(hdr.data_set_source[0]), _params->data_set_source, MDV_NAME_LEN);
      
  _outMdvx.setMasterHeader(hdr);
}


int AccessA2Mdv::round(float x)
{
  return (int)(x>=0.0 ? floor(x + 0.5) : ceil(x - 0.5));
}
