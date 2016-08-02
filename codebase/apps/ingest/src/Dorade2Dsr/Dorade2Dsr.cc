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
// Dorade2Dsr.cc
//
// Dorade2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2002
//
///////////////////////////////////////////////////////////////
//
// Dorade2Dsr reads Dorade radar beam-by-beam files and copies
// the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include "Dorade2Dsr.hh"
#include "File2Fmq.hh"
using namespace std;

// Constructor

Dorade2Dsr::Dorade2Dsr(int argc, char **argv)

{

  _input = NULL;
  _tiltNum = 0;
  _volNum = -1;
  _prevVolNum = -1;
  isOK = true;

  // set programe name

  _progName = "Dorade2Dsr";
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
    return;
  }

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Dorade2Dsr::Dorade2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Dorade2Dsr::Dorade2Dsr." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object

  if (_params.debug) {
    cerr << "n input files: " << _args.inputFileList.size() << endl;
  }
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - Dorade2Dsr" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    isOK = false;
    return;
  }

  if (_params.output_fmq_compress) {
    _rQueue.setCompressionMethod(TA_COMPRESSION_ZLIB);
  }

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  return;

}

// destructor

Dorade2Dsr::~Dorade2Dsr()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Dorade2Dsr::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.mode == Params::SIMULATE) {

    // simulate mode - go through the file list repeatedly
    
    while (true) {
      
      char *inputPath;
      _input->reset();
      while ((inputPath = _input->next()) != NULL) {
	PMU_auto_register("Simulate mode");
	if (_processFile(inputPath)) {
	  cerr << "ERROR = Dorade2Dsr::Run" << endl;
	  cerr << "  Processing file: " << inputPath << endl;
	  iret = -1;
	}
	umsleep(_params.simulate_wait_secs * 1000);
      } // while

    }

  } else {
    
    // loop until end of data
    
    char *inputPath;
    while ((inputPath = _input->next()) != NULL) {
      
      PMU_auto_register("Non-simulate mode");
      
      if (_processFile(inputPath)) {
	cerr << "ERROR = Dorade2Dsr::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
     
      if (_params.debug) {
	cerr << endl;
	cerr << "done with: " << inputPath << endl;
      }

    }

    if (_params.sendEOV) _rQueue.putEndOfVolume(_prevVolNum, time(NULL));

  } // if (_params.mode == Params::SIMULATE)
    
  return iret;

}

///////////////////////////////
// process file

int Dorade2Dsr::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << endl;
    cerr << "======================================================" << endl;
    cerr << "Processing file: " << input_path << endl << endl;
  }

  // instantiate the dorade mapper

  dd_mapper mapr;

  // instantiate an object for accessing an existing sweepfile

  dd_sweepfile_access sac;

  // open sweepfile and read in the first ray (i.e. map it!)
  
  if( !sac.access_sweepfile( input_path, &mapr ) ){ 
    cerr << "Cannot read in first ray, file: " << input_path << endl;
    return -1;
  }

  if (_params.debug) {
    _printHeaders(mapr);
  }

  // check that this is a valid file

  if (_params.end_of_vol_flag == Params::CHANGE_IN_SWEEP_VOL) {
    _volNum = mapr.volume_num();
  } else if (_params.end_of_vol_flag == Params::EVERY_FILE) {
    _volNum++;
  }

  DateTime startTime(mapr.year(), mapr.month(), mapr.day(),
		     mapr.hour(), mapr.minute(), mapr.second());
  //
  // Use the current time if we are in simulate mode, or if the
  // user has specifically asked us to.
  //
  if (
      (_params.mode == Params::SIMULATE) ||
      (_params.override_radar_time)
      ){
    if (_params.debug) {
      cerr << "Start time overridden from " << utimstr(startTime.utime());
      cerr << " to " << utimstr(time(NULL)) << endl;
    }
    startTime.set(time(NULL));
  }

  // set up the file output object
  
  File2Fmq f2fmq(_params, mapr, sac, _rQueue);
  
  // put start and end of vol flags as appropriate
  
  if (_params.end_of_vol_flag == Params::CHANGE_IN_SWEEP_VOL) {
    if (_prevVolNum != _volNum) {
      if (_prevVolNum >= 0) {
	if (_params.sendEOV) _rQueue.putEndOfVolume(_prevVolNum, startTime.utime());
      }
      _rQueue.putStartOfVolume(_volNum, startTime.utime());
      _tiltNum = 0;
      _prevVolNum = _volNum;
    } else {
      _tiltNum++;
    }
  }

  // put start of tilt flag

  _rQueue.putStartOfTilt(_tiltNum, startTime.utime());
  if (_params.end_of_vol_flag == Params::EVERY_FILE) {
    _rQueue.putStartOfVolume(_volNum, startTime.utime());
  }

  // put the params

  if (f2fmq.writeParams(input_path)) {
    cerr << "ERROR - Dorade2Dsr::_processFile" << endl;
    cerr << "  Cannot write the radar and field params to the queue" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }
  
  // put the beam data to the queue

  if (f2fmq.writeBeams(_volNum, _tiltNum, startTime.utime())) {
    cerr << "ERROR - Dorade2Dsr::_processFile" << endl;
    cerr << "  Cannot write the beam data to the queue" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // put end of tilt flag

  DateTime endTime(mapr.year(), mapr.month(), mapr.day(),
		   mapr.hour(), mapr.minute(), mapr.second());
  if (
      (_params.mode == Params::SIMULATE) ||
      (_params.override_radar_time)
      ){

    if (_params.debug) {
      cerr << "End time overridden from " << utimstr(endTime.utime());
      cerr << " to " << utimstr(time(NULL)) << endl;
    }
    endTime.set(time(NULL));
  }
  
  _rQueue.putEndOfTilt(_tiltNum, endTime.utime());
  if (_params.end_of_vol_flag == Params::EVERY_FILE) {
    if (_params.sendEOV) _rQueue.putEndOfVolume(_volNum, startTime.utime());
  }
  
  if (_params.mode == Params::REALTIME && _params.delete_input_file_after_use) {
    if (_params.debug) {
      cerr << "WARNING - unlinking input file: " << input_path << endl;
    }
    if (unlink(input_path)) {
      int errNum = errno;
      cerr << "ERROR - Dorade2Dsr::_processFile" << endl;
      cerr << "  Cannot unlink file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
  }

  return 0;

}

///////////////////////////////
// print data in file

void Dorade2Dsr::_printHeaders(dd_mapper &mapr)

{

  cerr << "Dorade header" << endl;
  cerr << "-------------" << endl;

  cerr << "  mapr date: "
       << mapr.year() << "/"
       << mapr.month() << "/"
       << mapr.day() << " "
       << mapr.hour() << ":"
       << mapr.minute() << ":"
       << mapr.second() << endl;
  
  cerr << "  proj_name: " << label(mapr.proj_name(), 20) << endl;
  cerr << "  radar_name: " << label(mapr.radar_name(), 8) << endl;

  cerr << "  volume_num: " << mapr.volume_num() << endl;
  cerr << "  sweep_num: " << mapr.sweep_num() << endl;
  cerr << "  new_vol: " << mapr.new_vol() << endl;
  cerr << "  new_sweep: " << mapr.new_sweep() << endl;
  cerr << "  new_ray: " << mapr.new_ray() << endl;
  cerr << "  new_mpb: " << mapr.new_mpb() << endl;
  cerr << "  volume_count: " << mapr.volume_count() << endl;
  cerr << "  sweep_count: " << mapr.sweep_count() << endl;
  cerr << "  total_ray_count: " << mapr.total_ray_count() << endl;
  cerr << "  sweep_ray_count: " << mapr.sweep_ray_count() << endl;
  cerr << "  sizeof_ray: " << mapr.sizeof_ray() << endl;
  cerr << "  complete: " << mapr.complete() << endl;
  cerr << "  found_ryib: " << mapr.found_ryib() << endl;
  cerr << "  error_state: " << mapr.error_state() << endl;
  cerr << "  swapped_data: " << mapr.swapped_data() << endl;
  cerr << "  radar_type: " << mapr.radar_type() << endl;
  cerr << "  radar_type_ascii: " << mapr.radar_type_ascii() << endl;
  cerr << "  scan_mode: " << mapr.scan_mode() << endl;
  cerr << "  scan_mode_mne: " << mapr.scan_mode_mne() << endl;
  cerr << "  rays_in_sweep: " << mapr.rays_in_sweep() << endl;
  cerr << "  latitude: " << mapr.latitude() << endl;
  cerr << "  longitude: " << mapr.longitude() << endl;
  cerr << "  altitude_km: " << mapr.altitude_km() << endl;
  cerr << "  azimuth: " << mapr.azimuth() << endl;
  cerr << "  elevation: " << mapr.elevation() << endl;
  cerr << "  fixed_angle: " << mapr.fixed_angle() << endl;
  cerr << "  roll: " << mapr.roll() << endl;
  cerr << "  rotation_angle: " << mapr.rotation_angle() << endl;
  cerr << "  number_of_cells: " << mapr.number_of_cells() << endl;
  cerr << "  meters_to_first_cell: " << mapr.meters_to_first_cell() << endl;
  cerr << "  meters_between_cells: " << mapr.meters_between_cells() << endl;
  cerr << "  min_cell_spacing: " << mapr.min_cell_spacing() << endl;
  cerr << "  meters_to_last_cell: " << mapr.meters_to_last_cell() << endl;
  cerr << "  return_num_samples: " << mapr.return_num_samples(0) << endl;

  cerr << "  num_fields: " << mapr.num_fields() << endl;
  for (int ii = 0; ii < mapr.num_fields(); ii++) {
    PARAMETER *fparam = mapr.parms[ii];
    cerr << endl;
    cerr << "  Field: " << ii << " name: "
	 << label(fparam->parameter_name, 8) << endl;
    cerr << "    scale: " << fparam->parameter_scale << endl;
    cerr << "    bias: " << fparam->parameter_bias << endl;
    cerr << "    bad_data_flag: " << fparam->bad_data << endl;
    cerr << "    field type: " << fparam->binary_format << endl;
    cerr << "    field units: " << label(fparam->param_units, 8) << endl;
  }

  cerr << endl;

}

///////////////////////////////
// print data in file

string Dorade2Dsr::label(const char *str, int maxLen)

{

  // null terminate

  TaArray<char> copy_;
  char *copy = copy_.alloc(maxLen + 1);
  memset(copy, 0, maxLen + 1);
  memcpy(copy, str, maxLen);
  
  // remove blanks

  int startPos = 0;
  for (int ii = 0; ii <= maxLen; ii++) {
    if (copy[ii] == ' ') {
      startPos++;
    } else {
      break;
    }
  }

  for (int ii = maxLen-1; ii >= 0; ii--) {
    if (copy[ii] == ' ') {
      copy[ii] = '\0';
    } else {
      break;
    }
  }

  return copy + startPos;

}

// c---------------------------------------------------------------------------

void dd_Testify(char * message)
{
    // sweepfiles codes need this routine
    int nn;
    char * mess = message;
    char str[512];

    // for now just use printf
    if(!mess || !(nn = strlen(mess))) {
	return;
    }
    if( nn > (int) sizeof(str) -1) {
	strncpy(str, mess, sizeof(str) -1);
	str[sizeof(str) -1] = '\0';
	mess = str;
    }
    printf("%s", mess);
}

