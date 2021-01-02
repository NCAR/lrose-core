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
// Tracks2Ascii.cc
//
// Tracks2Ascii object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#include "Tracks2Ascii.hh"
#include "CompleteTrack.hh"
#include "TrackEntry.hh"
#include "InitProps.hh"
#include "TrackFile.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
using namespace std;

// Constructor

Tracks2Ascii::Tracks2Ascii(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "Tracks2Ascii";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // input file object

  if (_args.inputFileList.size() > 0) {
    
    _input = new DsInputPath((char *)_progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
      
  } else {

    // check that start and end times are set

    if (_args.startTime == 0 || _args.endTime == 0) {
      fprintf(stderr, "ERROR: %s\n", _progName.c_str());
      fprintf(stderr,
	      "You must either specify a file list using -f,\n"
	      " or you must specify -start and -end.\n");
      isOK = FALSE;
      return;
    }
      
    _input = new DsInputPath((char *) _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime,
			     _args.endTime);
    _input->setSearchExt(TRACK_HEADER_FILE_EXT);
  }

  return;

}

//////////////
// destructor

Tracks2Ascii::~Tracks2Ascii()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete(_input);
  }

}

//////////////////////////////////////////////////
// PrintComments()
//
// Print comments at start of file
//

void Tracks2Ascii::_printComments ()
{

  date_time_t file_time;
  ulocaltime(&file_time);
  fprintf(stdout, "#File create time: %s\n", utimestr(&file_time));

  fprintf(stdout, "#File name(s):\n");
  _input->reset();
  char *trackFilePath;
  while ((trackFilePath = _input->next()) != NULL) {
    fprintf(stdout, "#    %s\n", trackFilePath);
  }

  if (_params.target_entity == Params::COMPLETE_TRACK) {

    fprintf(stdout, "#target_entity: complete_track\n");
    
  } else if (_params.target_entity == Params::TRACK_ENTRY) {
    
    fprintf(stdout, "#target_entity: track_entry\n");
    fprintf(stdout, "#sample_interval (secs): %d\n",
	   (int) _params.sample_interval);
    fprintf(stdout, "#scan_interval (secs): %d\n",
	   (int) _params.scan_interval);

    if (_params.refl_histogram_only) {
      if (_params.refl_histogram_vol) {
	fprintf(stdout, "#Reflectivity histograms for volume\n");
      } else {
	fprintf(stdout, "#Reflectivity histograms for area\n");
      }
    }
    
  } else if (_params.target_entity == Params::INITIAL_PROPS) {
    
    fprintf(stdout, "#target_entity: initial_props\n");
    fprintf(stdout, "#initial_props_nscans: %d\n",
	   (int) _params.initial_props_nscans);
    
  }

  if (_params.use_simple_tracks) {
    fprintf(stdout, "#simple tracks used\n");
  }
   
  if (_params.use_complex_tracks) {
    fprintf(stdout, "#complex tracks used\n");
  }
  
  if (_params.use_box_limits) {

    fprintf(stdout, "#Box limits (%g, %g) to (%g, %g)\n",
	   _params.box.min_x, _params.box.min_y,
	   _params.box.max_x, _params.box.max_y);
    
    if (_params.target_entity == Params::COMPLETE_TRACK) {

      fprintf(stdout, "#min percent in box: %g\n", _params.box.min_percent);
      fprintf(stdout, "#min nscans  in box: %d\n", (int) _params.box.min_nscans);

    }
    
  } /* if (_params.use_box_limits) */

  if (_params.check_too_close) {
    fprintf(stdout, "#max_nscans_too_close: %d\n",
	   (int) _params.max_nscans_too_close);
  }

  if (_params.check_too_far) {
    fprintf(stdout, "#max_nscans_too_far: %d\n",
	   (int) _params.max_nscans_too_far);
  }

  if (_params.check_vol_at_start) {
    fprintf(stdout, "#max_vol_at_start: %g\n",
	   _params.max_vol_at_start);
  }

  if (_params.check_vol_at_end) {
    fprintf(stdout, "#max_vol_at_end: %g\n",
	   _params.max_vol_at_end);
  }

  if (_params.use_case_tracks_file) {
    fprintf(stdout, "#using case tracks file: %s\n",
            _params.case_tracks_file_path);
  }

}

//////////////////////////////////////////////////
// Run

int Tracks2Ascii::Run ()
{

  PMU_auto_register("Tracks2Ascii::Run");

  // print comments to start of output file

  _printComments();

  // create compute entity

  Entity *entity;

  if (_params.target_entity == Params::COMPLETE_TRACK) {
    entity = new CompleteTrack(_progName, _params);
  } else if (_params.target_entity == Params::TRACK_ENTRY) {
    entity = new TrackEntry(_progName, _params);
  } else if (_params.target_entity == Params::INITIAL_PROPS) {
    entity = new InitProps(_progName, _params);
  } else {
    fprintf(stderr, "ERROR - %s:Run\n", _progName.c_str());
    fprintf(stderr, "No valid entity set.\n");
    return (-1);
  }
  if (!entity->isOK) {
    return -1;
  }
  
  // loop through the track files

  int nComplex = 0;
  int nSimple = 0;

  char *trackFilePath;
  _input->reset();
  while ((trackFilePath = _input->next()) != NULL) {

    fprintf(stderr, "Processing track file %s\n", trackFilePath);

    TrackFile *trackFile = new TrackFile(_progName, _params,
					 entity, trackFilePath);
    
    trackFile->process();

    nComplex += trackFile->ncomplex();
    nSimple += trackFile->nsimple();

    delete (trackFile);

  } // while

  delete (entity);

  fprintf(stdout, "# n_complex: %d\n", nComplex);
  fprintf(stdout, "# n_simple : %d\n", nSimple);
  fprintf(stdout, "# n_total  : %d\n", nComplex + nSimple);

  return (0);

}

