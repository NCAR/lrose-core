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
// MdvMerge2 object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////

#include <algorithm>
#include <string>
#include <cstring>

#include <toolsa/Path.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <Mdv/MdvxUrlWatcher.hh>

#include "MdvMerge2.hh"
#include "OutputFile.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

MdvMerge2::MdvMerge2(int argc, char **argv)

{

  isOK = true;
  _closestRange = NULL;
  _closestFlag = NULL;
  _trigger = NULL;

  // set programe name

  _progName = "MdvMerge2";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
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
  
  // setup output grid params
  
  if (_initGrids()) {
    isOK = false;
    return;
  }

  // create trigger

  if (_createTrigger()) {
    isOK = false;
    return;
  }

  // create input objects

  for (int ii = 0; ii < _params.input_urls_n; ii++) {
    InputFile *input = new InputFile(_progName, _params,
				     _params._input_urls[ii],
				     _outProj);
    _inputs.push_back(input);
  }
  
  // init process mapper registration
  int pmuRegSec = PROCMAP_REGISTER_INTERVAL;

  if(_params.procmap_register_interval_secs > PROCMAP_REGISTER_INTERVAL) {
    pmuRegSec = _params.procmap_register_interval_secs;
  }

  if (_params.mode == Params::REALTIME)
    PMU_auto_init(const_cast<char*>(_progName.c_str()), _params.instance, pmuRegSec);
  
  return;

}

//////////////////////////
// destructor

MdvMerge2::~MdvMerge2()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  for (int ii = 0; ii < _params.merge_fields_n; ii++) {
    switch ( _params._merge_fields[ii].merge_encoding ){
      case Params::FLOAT32:
        delete[] (fl32 *) _merged[ii];
        break;
      case Params::INT16 :
      delete[] (ui16 *) _merged[ii];
      break;
      default :
        delete[] (ui08 *) _merged[ii];
        break;
    }
  }

  for (size_t ii = 0; ii < _count.size(); ii++) {
    delete[] _count[ii];
  }
  
  for (size_t ii = 0; ii < _latestTime.size(); ii++) {
    delete[] _latestTime[ii];
  }

  for (size_t ii = 0; ii < _inputs.size(); ii++) {
    delete _inputs[ii];
  }

  if (_closestRange) {
    delete[] _closestRange;
  }

  if (_closestFlag) {
    delete[] _closestFlag;
  }

  if (_trigger) {
    delete[] _trigger;
  }

}

//////////////////////////////////////////////////
// Run

int MdvMerge2::Run ()
{
  if(_params.trigger == Params::FCST_FILES_TRIGGER) {
    return _runForecast();
  } else {
    return _runObs();
  }
}
  
//////////////////////////////////////////////////
// Run in forecast mode

int MdvMerge2::_runForecast()
{

  int iret = 0;
  
  PMU_auto_register("MdvMerge2::_runForecast");

  // set up lead times

  vector<double> leadtimes_hours;
  for( int i = 0; i < _params.fcstLeadTimes_n; i++) {
    leadtimes_hours.push_back(_params._fcstLeadTimes[i] / 3600);
  }
  
  // create the input watcher

  MdvxUrlWatcher *URLW;
  if ( _params.mode == Params::REALTIME) {

    MdvxUrlWatcher *realtimeURLW = new MdvxUrlWatcher(_params.fcst_file_trigger_url,
                                                      _params.max_realtime_valid_age,
                                                      leadtimes_hours,
                                                      true,
                                                      _params.debug);
    URLW = realtimeURLW;

  } else {

    MdvxUrlWatcher *archiveURLW= new MdvxUrlWatcher(_params.fcst_file_trigger_url,
                                                    _args.startTime, _args.endTime,
                                                    true,
                                                    _params.debug);
    URLW = archiveURLW;

  }
  
  // loop through input data
  
  while (true) {
    
    if (URLW->get_data()) {
      
      time_t genTime = URLW->get_time();
      
      if (_params.debug) {
        cerr << "Found new data with gen time "
             << DateTime::str( genTime ) << endl;
      }
      
      vector<double>::iterator it;
      for (it = leadtimes_hours.begin(); it != leadtimes_hours.end(); ++it)	{
        
        if (_params.debug) {
          cerr << "Process forecast lead time of " << *it * 3600 << endl;
        }
	
        int leadTime = int (*it * 3600.0 + 0.5);
	
        // initialize the merge
        
        _initMerge();
	
        iret = _processData(genTime, leadTime);
        
      } // it
      
    } else {
      
      if ( _params.mode == Params::ARCHIVE) {
        break;
      }
      
    } // if (URLW->get_data())

    if (_args.runOnce) {
      break;
    }
    
  } // while (true)
  
  if (URLW) {
    delete (URLW);
  }
  
  return iret;

}

//////////////////////////////////////////////////
// Run in observational mode

int MdvMerge2::_runObs()
{
  
  int iret = 0;
  
  PMU_auto_register("MdvMerge2::_runObs");


  time_t triggerTime;

  while ((triggerTime = _trigger->next()) >= 0) {  

    if (_params.debug) {
      cerr << "----> Trigger time: " << utimstr(triggerTime) << endl;
    }

    // In REALTIME mode, sleep after triggering, if desired.
    
    if (_params.mode == Params::REALTIME) {
      if ((_params.sleepAfterTrigger) && (_params.debug)) {
	cerr << " Sleeping for " << _params.sleepAfterTrigger;
	cerr << " seconds before processing" << endl;
      }
      for (int iSleep=0; iSleep < _params.sleepAfterTrigger; iSleep++) {
	PMU_auto_register("Sleeping before processing data");
	umsleep(1000);
      } 
    }

    // initialize the merge

    _initMerge();
    
    iret = _processData(triggerTime, 0);
    
    if (_args.runOnce) {
      break;
    }
    
  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// initialize the output grids

int MdvMerge2::_initGrids()
  
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

  // alloc gridded arrays

  _nz = coord.nz;
  _nxy = coord.nx * coord.ny;
  _nxyz = _nxy * _nz;

  bool needRange = false;
  for (int ii = 0; ii < _params.merge_fields_n; ii++) {
    
    switch ( _params._merge_fields[ii].merge_encoding ){
      case Params::INT8:
        {
          ui08 *ui08Merged = new ui08[_nxyz];
          _merged.push_back(ui08Merged);
        }
        break;
      case Params::INT16: 
        {
          ui16 *ui16Merged = new ui16[_nxyz];
          _merged.push_back(ui16Merged);
        }
        break;
      default:
        {
          fl32 *fl32Merged = new fl32[_nxyz];
          _merged.push_back(fl32Merged);
        }
        break;
    }

    if (_params._merge_fields[ii].merge_method == Params::MERGE_MEAN) {
      ui08 *count = new ui08[_nxyz];
      _count.push_back(count);
    } else {
      _count.push_back(NULL);
    }
    
    if (_params._merge_fields[ii].merge_method == Params::MERGE_LATEST) {
      time_t *latest_time_ptr = new time_t[_nxyz];
      memset(latest_time_ptr, 0, _nxyz * sizeof(time_t));
      _latestTime.push_back(latest_time_ptr);
    } else {
      _latestTime.push_back(NULL);
    }

    if (_params._merge_fields[ii].merge_method == Params::MERGE_CLOSEST) {
      needRange = true;
    }
      
  } // ii

  if (needRange) {
    _closestRange = new fl32[_nxyz];
    _closestFlag = new int[_nxyz];
  }

  return 0;

}

//////////////////////////////////////////////////
//  create trigger

int MdvMerge2::_createTrigger()
{

  if (_params.mode == Params::REALTIME) {

    // Check for errors in parameters
    
    if (_params.time_trigger_interval == 0){
      cerr << "ERROR - MdvMerge2::_createTrigger()" << endl;
      cerr << "  REALTIME mode - param time_trigger_interval is 0" << endl;
      cerr << "  edit params and re-run" << endl;
      return -1;
    }

    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new RealtimeTimeTrigger(_progName, _params);
    }
    else if (_params.trigger == Params::FILE_TRIGGER)
    {
      _trigger = new RealtimeFileTrigger(_progName, _params);
    }
    // FCST_FILES_TRIGGER
  } else {

    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new ArchiveTimeTrigger(_progName, _params,
				       _args.startTime,  _args.endTime);
    } else {
      _trigger = new ArchiveFileTrigger(_progName, _params,
				       _args.startTime, _args.endTime);
    }
  }

  return 0;
  
}

//////////////////////////////////////////////////
// initialize merge - set to missing values

void MdvMerge2::_initMerge()
{
  
  for (int ifld = 0; ifld < _params.merge_fields_n; ifld++) {
    
    if (_params._merge_fields[ifld].merge_encoding == Params::FLOAT32) {
      fl32 *merged = (fl32 *) _merged[ifld];
      for (int ii = 0; ii < _nxyz; ii++, merged++) {
	*merged = OutputFile::missingFl32;
      }
    } else if (_params._merge_fields[ifld].merge_encoding == Params::INT16) {
      memset(_merged[ifld], 0, _nxyz * sizeof(ui16));
    } else {
      memset(_merged[ifld], 0, _nxyz * sizeof(ui08));
    }

  } // ifld

  // init closest range to large value so that first data set
  // will all be valid

  if (_closestRange != NULL) {
    for (int ii = 0; ii < _nxyz; ii++) {
      _closestRange[ii] = 1.0e6;
    }
  }

}

//////////////////////////////////////////////////
// Read and process data - return true if successful otherwise false

int MdvMerge2::_processData(time_t fileTime, int leadTime)
{
  int iret = 0;

  // read relevant input files
    
  bool dataAvail = false;
  string dataSetInfo = "Data merged from following files:\n";
  time_t startTime = -1;
  time_t endTime = -1;
    
  for (size_t ii = 0; ii < _inputs.size(); ii++) {

    // read in data
    
    if (_inputs[ii]->process(fileTime, leadTime,
			     _nxy, _nz,
                             _merged, _count, _latestTime,
                             _closestRange, _closestFlag,
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
	endTime = MAX(endTime, _inputs[ii]->getEndTime());
      }
	      
    } else{

      if (_inputs[ii]->getIsRequired()){
        return iret;
      }
      
    } // if (_inputs[ii]->read(triggerTime) == 0)

  } // ii
    
  if ( !dataAvail)
  {
    return iret;
  }
  
  // write the data out

  OutputFile outFile(_progName, _params, _outProj,
		     _exampleMhdr, _exampleFhdrs);
  
  // If we're in archive mode and the start time is equal
  // to the end time, use that time.
  time_t writeTime = fileTime;
  if ((_args.runOnce) && 
      (_params.mode == Params::ARCHIVE) &&
      (_args.startTime == _args.endTime) &&
      (_args.startTime != 0L)) {
    writeTime = _args.startTime;
    startTime = endTime = writeTime;
    if (_params.debug) {
      cerr << "Start time equals end time in run once archive mode :" << endl;
      cerr << "      Start/End/Centroid times set to " << utimstr(writeTime) << endl;
    }
  }
  
  if ( (_params.mode == Params::ARCHIVE && _params.write_as_forecast) ||
       (_params.trigger == Params::FCST_FILES_TRIGGER && _params.write_as_forecast)
     )
  {
    writeTime = fileTime + leadTime;
    startTime = fileTime;
  }
  
  PMU_force_register("Writing file");
  
  if (outFile.write(writeTime, startTime, endTime,
                    _merged, dataSetInfo)) {
    cerr << "ERROR - MdvMerge2::_processData" << endl;
    cerr << "  Cannot write output file" << endl;
    iret = -1;
  }
  
  return iret;

}

