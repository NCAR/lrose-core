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
///////////////////////////////////////////////////////////////
// MdvStitch object
//
// Jason Craig, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2008
//
///////////////////////////////////////////////////////////////

#include <algorithm>
#include <string>
#include <cstring>

#include <toolsa/Path.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>

#include "MdvStitch.hh"
#include "OutputFile.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

MdvStitch::MdvStitch(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "MdvStitch";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check params
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  In ARCHIVE mode you must specify start and end times" << endl;
      cerr << "  Run '" << _progName << " -h' for usage" << endl;
      _args.usage(cerr);
      isOK = false;
    }
  }

  if (_params.input_urls_n < 1) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << "  Number of input_urls: " << _params.input_urls_n << endl;
    cerr << "  You must specify at least 1 input_url." << endl;
    isOK = false;
  }
  if (_params.merge_fields_n < 1) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << "  Number of merge_fields: " << _params.merge_fields_n << endl;
    cerr << "  You must specify at least 1 merge field." << endl;
    isOK = false;
  }
  
  if (!isOK) {
    return;
  }
  
  // setup output Projection info
  
  _initOutputProj();

  // create trigger

  _createTrigger();

  // create input objects

  for (int ii = 0; ii < _params.input_urls_n; ii++) {
    InputFile *input = new InputFile(_progName, _params,
				     _params._input_urls[ii],
				     _outProj);
    _inputs.push_back(input);
  }
  
  // init process mapper registration
  
  PMU_auto_init(const_cast<char*>(_progName.c_str()), _params.instance, 
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

//////////////////////////
// destructor

MdvStitch::~MdvStitch()

{

  // unregister process

  PMU_auto_unregister();

  for (size_t ii = 0; ii < _inputs.size(); ii++) {
    delete _inputs[ii];
  }

}

//////////////////////////////////////////////////
// Run

int MdvStitch::Run ()
{

  int iret = 0;
  PMU_auto_register("MdvStitch::Run");
 
  // create the input file objects

  // loop through times
  
  time_t triggerTime;
  while ((triggerTime = _trigger->next()) >= 0) {
    
    if (_params.debug) {
      cerr << "----> Trigger time: " << utimstr(triggerTime) << endl;
    }

    // In REALTIME mode, sleep after triggering, if desired.

    if (_params.mode == Params::REALTIME){

      if ((_params.sleepAfterTrigger) && (_params.debug)) {
	cerr << " Sleeping for " << _params.sleepAfterTrigger;
	cerr << " seconds before processing" << endl;
      }
      
      for (int iSleep=0; iSleep < _params.sleepAfterTrigger; iSleep++){
	PMU_auto_register("Sleeping before processing data");
	sleep(1);
      }

    }

    // read relevant input files
    
    bool dataAvail = false;
    string dataSetInfo = "Data merged from following files:\n";
    time_t startTime = -1;
    time_t endTime = -1;
    
    for (size_t ii = 0; ii < _inputs.size(); ii++) {

      // read in data
      PMU_auto_register("Processing data");
      if (_inputs[ii]->process(triggerTime, _nxy, _merged, _count,
			       _exampleMhdr, _exampleFhdrs) == 0) {
	
	// success
	
	dataAvail = true;
	
	// append to info string

	char infoStr[1024];
	sprintf(infoStr, "  %s\n", _inputs[ii]->getPathInUse().c_str());
	dataSetInfo += infoStr;

	// update times

	if (startTime < 0) {
	  startTime = _inputs[ii]->getStartTime();
	} else {
	  startTime = MIN(startTime, _inputs[ii]->getStartTime());
	}
	if (endTime < 0) {
	  endTime = _inputs[ii]->getEndTime();
	} else {
	  endTime = MIN(endTime, _inputs[ii]->getEndTime());
	}

      } // if (_inputs[ii]->read(triggerTime) == 0)

    } // ii
    
    if (!dataAvail) {
      continue;
    }
    
    // write the data out
    
    OutputFile outFile(_progName, _params, _outProj,
		       _exampleMhdr, _exampleFhdrs);
    
    // If we're in archive mode and the start time is equal
    // to the end time, use that time.
    time_t writeTime = triggerTime;
    if ((_args.runOnce) && 
	(_params.mode == Params::ARCHIVE) &&
	(_args.startTime == _args.endTime) &&
	(_args.startTime != 0L)
	){
      writeTime = _args.startTime;
      startTime = endTime = writeTime;
      if (_params.debug) {
	cerr << "Start time equals end time in run once archive mode :" << endl;
	cerr << "      Start/End/Centroid times set to " << utimstr(writeTime) << endl;
      }
    }

    if (outFile.write(writeTime, startTime, endTime,
  		      _merged, dataSetInfo)) {
      cerr << "ERROR - MdvStitch::Run" << endl;
      cerr << "  Cannot write output file" << endl;
      iret = -1;
    }

    // free up
    
    for (int ii = 0; ii < _params.merge_fields_n; ii++) {
      
      switch ( _exampleFhdrs[ii].encoding_type ){
	
      case Mdvx::ENCODING_FLOAT32 :
	free((fl32 *)_merged[ii]);
	break;
	
      case Mdvx::ENCODING_INT16 :
	free((ui16 *)_merged[ii]);
	break;
	
      default :
	free((ui08 *)_merged[ii]);
	break;
	
      }
    }
    _merged.clear();

    if (_args.runOnce) {
      break;
    }

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
//  create trigger

void MdvStitch::_createTrigger()
{

  if (_params.mode == Params::REALTIME) {

    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new RealtimeTimeTrigger(_progName, _params);
    } else {
      _trigger = new RealtimeFileTrigger(_progName, _params);
    }

  } else {

    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new ArchiveTimeTrigger(_progName, _params,
				       _args.startTime,  _args.endTime);
    } else {
      _trigger = new ArchiveFileTrigger(_progName, _params,
				       _args.startTime, _args.endTime);
    }
  }
  
}

//////////////////////////////////////////////////
// initialize the output grids

void MdvStitch::_initOutputProj()
  
{
  
  // load up grid params

  Mdvx::coord_t coord;
  MEM_zero(coord);
  coord.nx = _params.output_grid.nx;
  coord.ny = _params.output_grid.ny;
  coord.minx = _params.output_grid.minx;
  coord.miny = _params.output_grid.miny;
  coord.dx = _params.output_grid.dx;
  coord.dy = _params.output_grid.dy;
  if(_params.use_specified_vlevels) {
    coord.minz = _params._vlevel_array[0];
    coord.nz = _params.vlevel_array_n;
  } else {
    coord.minz = _params.output_grid.minz;
    coord.nz = _params.output_grid.nz;
  }
  coord.dz = _params.output_grid.dz;
  
  if (_params.output_projection == Params::OUTPUT_PROJ_FLAT) {
    coord.proj_origin_lat = _params.output_origin.lat;
    coord.proj_origin_lon = _params.output_origin.lon;
    coord.proj_params.flat.rotation = _params.output_rotation;
    coord.proj_type = Mdvx::PROJ_FLAT;
  } else if (_params.output_projection == Params::OUTPUT_PROJ_LAMBERT) {
    coord.proj_origin_lat = _params.output_origin.lat;
    coord.proj_origin_lon = _params.output_origin.lon;
    coord.proj_params.lc2.lat1 = _params.output_std_parallels.lat_1;
    coord.proj_params.lc2.lat2 = _params.output_std_parallels.lat_2;
    coord.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  } else {
    coord.proj_type = Mdvx::PROJ_LATLON;
  }
  
  _outProj.init(coord);

}
