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
 *  $Id: MdvFixTimes.cc,v 1.5 2016/03/04 02:22:11 dixon Exp $
 */

# ifndef    lint
static char RCSid[] = "$Id: MdvFixTimes.cc,v 1.5 2016/03/04 02:22:11 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	MdvFixTimes
//
// Author:	G. M. Cunning
//
// Date:	Fri Apr 22 16:13:58 2005
//
// Description: modifies master and field header times.
//
//

// C++ include files
#include <iostream>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

// Local include files
#include "MdvFixTimes.hh"
#include "Params.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
MdvFixTimes *MdvFixTimes::_instance = 0;
 
// define any constants
const string MdvFixTimes::_className = "MdvFixTimes";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MdvFixTimes::MdvFixTimes(int argc, char **argv)
{
  _isOK = true;

  // Make sure the singleton wasn't already created.
  assert(_instance == 0);
 
  // Set the singleton instance pointer
  _instance = this;
 
  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();
  ucopyright(const_cast<char*>(_progName.c_str()));
 
  // get command line args
  _args = new Args();
  _args->parse(argc, argv, _progName);

  // get TDRP params
  _params = new Params();
  char *paramsPath = "unknown";
  if(_params->loadFromArgs(argc, argv, _args->override.list,
			  &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  } // endif -- _params.loadFromArgs(argc, argv, _args.override.list, ...

  // display ucopyright message and RCSid
  if(_params->debug != Params::DEBUG_VERBOSE) {
    cerr << RCSid << endl;
  } //endif -- _params->debug_mode != Params::DEBUG_VERBOSE


 
 // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(),
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
MdvFixTimes::~MdvFixTimes()
{
  // unregister process
  PMU_auto_unregister();

  // Free contained objects
  delete _params;
  delete _args;
  delete _input;
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
// Method Name:	MdvFixTimes::instance
//
// Description:	this method creates instance of MdvFixTimes object
//
// Returns:	returns pointer to self
//
// Notes:	this method implements the singleton pattern
//
//

MdvFixTimes*
MdvFixTimes::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new MdvFixTimes(argc, argv);
  }
  return(_instance);
}

MdvFixTimes*
MdvFixTimes::instance()
{
  assert(_instance != 0);
  
  return(_instance);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvFixTimes::initialize
//
// Description:	Initializes the class.
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//

void 
MdvFixTimes::initialize()
{

  // initialize the data input object
  _input = new DsMdvxInput();

  if(_input == 0) {
    _isOK = false;
    return;  
  }
    
  if (_params->mode == Params::REALTIME) {
    if (_input->setRealtime(_params->input_url, 600,
			    PMU_auto_register)) {
      _isOK = false;
    }
  } else if (_params->mode == Params::ARCHIVE) {
    if (_input->setArchive(_params->input_url,
			  _args->startTime,
			  _args->endTime)) {
      _isOK = false;
    }
  } else if (_params->mode == Params::FILELIST) {
    if (_input->setFilelist(_args->inputFileList)) {
      _isOK = false;
    }
  }
  
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvFixTimes::run
//
// Description:	runs the object
//
// Returns:	returns 0
//
// Notes:	
//
//

int 
MdvFixTimes::run()
{

  const string methodName = _className + string( "::run" );

  int iret = 0;

  // register with procmap
  PMU_auto_register("Running");

  if(_params->debug != Params::DEBUG_OFF) {
    cerr << "Running:" << endl;
  } // endif -- _params->debug_mode != Params::DEBUG_OFF

  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params->debug);

  // loop until end of data

  while (!_input->endOfData()) {

    PMU_auto_register("In main loop");
    
    // set up the Mdvx read

    mdvx.clearRead();
    mdvx.clearWrite(); 
 

    int num_fields;
    if (_params->set_field_names) {
      for (int i = 0; i < _params->field_names_n; i++) {
	mdvx.addReadField(_params->_field_names[i]);
      }
      num_fields = _params->field_names_n; 
    } else if (_params->set_field_nums) {
      for (int i = 0; i < _params->field_nums_n; i++) {
	mdvx.addReadField(_params->_field_nums[i]);
      }
      num_fields = _params->field_nums_n;
    }

    PMU_auto_register("Before read");
    

    if (_input->readVolumeNext(mdvx)) {
      cerr << "ERROR - MdvConvert::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input->getErrStr() << endl;
      iret = -1;
      continue;
    }

    if (_params->debug) {
      cerr << "Working on file: " << mdvx.getPathInUse() << endl;
    }
    
    // adjust forecast times, if requested
    if (_params->adjust_forecast_time)
    {
      const Mdvx::master_header_t in_master_hdr = mdvx.getMasterHeader();
      Mdvx::master_header_t out_master_hdr = in_master_hdr;

      time_t base_time;

      if(_params->adjust_gen_time) {
	out_master_hdr.time_gen = in_master_hdr.time_centroid;
      }

      if(_params->forecast_base_time == Params::TIME_GEN) {
	base_time = out_master_hdr.time_gen;
      }
      else if(_params->forecast_base_time == Params::TIME_BEGIN) {
	base_time = out_master_hdr.time_begin;
      }
      else if(_params->forecast_base_time == Params::TIME_CENTROID) {
	base_time = out_master_hdr.time_centroid;
      }
      else {
	cerr << "ERROR - MdvxConvert::Run" << endl;
	cerr << "  Error acquiring forcest base time type." << endl;
	continue;
      }
	  
     if (_params->output_as_forecast) {
       out_master_hdr.time_gen = base_time;
       out_master_hdr.time_begin = base_time + _params->forecast_delta + _params->begin_offset;
       out_master_hdr.time_end = base_time + _params->forecast_delta + _params->end_offset;
       out_master_hdr.time_centroid = base_time + _params->forecast_delta + _params->centroid_offset;
       out_master_hdr.time_expire = base_time + _params->forecast_delta + _params->expire_offset;
     }
     else {
       out_master_hdr.time_gen = base_time;
       out_master_hdr.time_begin = base_time + _params->forecast_delta + _params->begin_offset;
       out_master_hdr.time_end = base_time + _params->forecast_delta + _params->end_offset;
       out_master_hdr.time_centroid = base_time + _params->centroid_offset;
       out_master_hdr.time_expire = base_time + _params->forecast_delta + _params->expire_offset;
     }

      mdvx.setMasterHeader(out_master_hdr);
      mdvx.updateMasterHeader();

      for (int i = 0; i < num_fields; ++i) {
	MdvxField *field = mdvx.getField(i);

	if (field == 0) {
	  cerr << "ERROR - MdvxConvert::Run" << endl;
	  cerr << "  Error adjusting forecast time for  field #" << i <<
	    " in output file" << endl;
	  continue;
	}


	const Mdvx::field_header_t in_field_hdr = field->getFieldHeader();
	Mdvx::field_header_t out_field_hdr = in_field_hdr;
	
	out_field_hdr.forecast_delta = _params->forecast_delta;
	out_field_hdr.forecast_time = base_time + out_field_hdr.forecast_delta;

	field->setFieldHeader(out_field_hdr);

      }

    }


    // write out

    PMU_auto_register("Before write");


    if (_params->output_as_forecast){
      mdvx.setWriteAsForecast();
      if (_params->debug) {
	cerr << "Writing output as forecast" << endl << endl;
      }    
    }

    mdvx.setWriteLdataInfo();
    mdvx.setAppName(_progName);
    
    if(mdvx.writeToDir(_params->output_url)) {
      cerr << "ERROR - MdvConvert::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << mdvx.getErrStr() << endl;
      iret = -1;
      continue;
    }


  } // while

  return iret;
}

