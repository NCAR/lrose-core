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
// TitanSelect.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2004
//
///////////////////////////////////////////////////////////////
//
// TitanSelect selects specified Titan track data from the
// database and prints it to stdout.
// It watches the Rview shmem segments for user activity
// and reacts when a new track is specified.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/ushmem.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include "TitanSelect.hh"
using namespace std;

// Constructor

TitanSelect::TitanSelect(int argc, char **argv)
  
{
  
  isOK = true;

  // set programe name

  _progName = "TitanSelect";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // Attach shared memory segments
  
  if ((_coordShmem =
       (coord_export_t *) ushm_create(_params.rview_shmem_key,
				      sizeof(coord_export_t),
				      0666)) == NULL) {
    cerr << "ERROR: " << _progName << endl;
    cerr <<
      "  Cannot create/attach Rview's coord export shmem segment." << endl;
    isOK = false;
  }
  
  if ((_titanShmem =
       (time_hist_shmem_t *) ushm_create(_params.rview_shmem_key + 1,
					 sizeof(time_hist_shmem_t),
					 0666)) == NULL) {
    cerr << "ERROR: " << _progName << endl;
    cerr <<
      "  Cannot create/attach Rview's titan shmem segment." << endl;
    isOK = false;
  }
  
  // initialize storm track parameters
  
  _stormTime = 0;
  _complexTrackNum = 0;
  _simpleTrackNum = 0;
  
  return;

}

// destructor

TitanSelect::~TitanSelect()

{

  // detach from shared memory

  ushm_detach(_coordShmem);
  ushm_detach(_titanShmem);
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TitanSelect::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  while (!_coordShmem->shmem_ready) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Now, operate forever
  
  while (true) {
    
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    if(_titanShmem->shmem_ready && _titanShmem->main_display_active) {
      
      if (_titanShmem->time != _stormTime ||
	  _titanShmem->complex_track_num != _complexTrackNum ||
	  _titanShmem->simple_track_num != _simpleTrackNum) {

	_stormTime = _titanShmem->time;
	_complexTrackNum = _titanShmem->complex_track_num;
	_simpleTrackNum = _titanShmem->simple_track_num;
	
	if (_titanShmem->simple_track_num != -1) {
	
	  if (_params.debug) {
	    cerr << "---> Titan shmem <---" << endl;
	    cerr << "     Time: " << utimstr(_stormTime) << endl;
	    cerr << "     Complex track num: " << _complexTrackNum << endl;
	    cerr << "     Simple track num: " << _simpleTrackNum << endl;
	  }

	  if (_getData() == 0) {
	    if (_doPrint()) {
	      _doPrintNoData();
	    }
	  } else {
	    _doPrintNoData();
	  }
	  
	} // if (_titanShmem->simple_track_num != -1)

      } // if (_titanShmem->time != _stormTime ...
      
    } // if(_titanShmem->shmem_ready ...

    umsleep(_params.sleep_msecs);
    
  } // while(true)
  
  return (0);

}

////////////////////////////////////////////
// get the data
//
// Returns 0 on success, -1 on failure

int TitanSelect::_getData()
  
{

  _titan.clearRead();
  _titan.setReadClosest(_stormTime, 0);
  _titan.setReadSingleTrack(_complexTrackNum);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _titan.printReadRequest(cerr);
  }
  if (_titan.read(_params.input_url)) {
    if (_params.log_read_errors) {
      MsgLog msgLog;
      msgLog.setApplication(_progName, _params.instance);
      msgLog.setSuffix("log");
      msgLog.setAppendMode();
      msgLog.setDayMode();
      msgLog.setOutputDir(_params.errors_log_dir);
      msgLog.postMsg("ERROR - TitanSelect::_getData");
      msgLog.postMsg("  Retrieving data.");
      msgLog.postMsg("%s", _titan.getErrStr().c_str());
    } else {
      cerr << "ERROR - TitanSelect::_getData" << endl;
      cerr << "  Retrieving data." << endl;
      cerr << "  " << _titan.getErrStr() << endl;
    }
    return -1;
  }

  return 0;

}

////////////////////////////////////
// normal print

int TitanSelect::_doPrint()
  
{

  // check we have a complex track
  
  if (_titan.complex_tracks().size() == 0) {
    if (_params.debug) {
      cerr << "----> no complex track" << endl;
    }
    return -1;
  }

  // find the relevant simple track

  const vector<TitanSimpleTrack *> &sTracks =
    _titan.complex_tracks()[0]->simple_tracks();
  int iSimple = -1;
  for (size_t ii = 0; ii < sTracks.size(); ii++) {
    if (sTracks[ii]->simple_params().simple_track_num == _simpleTrackNum) {
      iSimple = (int) ii;
      break;
    }
  }
  if (iSimple < 0) {
    if (_params.debug) {
      cerr << "----> cannot find simple track" << endl;
    }
    return -1;
  }

  // find the relevant entry
  
  const vector<TitanTrackEntry *> entries = sTracks[iSimple]->entries();
  int iEntry = -1;
  double minTimeDiff = 1.0e99;
  for (size_t ii = 0; ii < entries.size(); ii++) {
    double timeDiff =
      fabs((double) entries[ii]->entry().time - (double) _stormTime);
    if (timeDiff < minTimeDiff) {
      iEntry = (int) ii;
      minTimeDiff = timeDiff;
    }
  }
  if (iEntry < 0) {
    if (_params.debug) {
      cerr << "----> cannot find track entry" << endl;
    }
    return -1;
  }

  const track_file_entry_t &entry = entries[iEntry]->entry();
  const storm_file_global_props_t &gprops = entries[iEntry]->gprops();

  // scroll down

  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // print delimiter

  if (_params.length_delimiter_line > 0) {
    for (int i = 0; i < _params.length_delimiter_line; i++) {
      cout << "-";
    }
    cout << endl;
  }

  // print out props

  cout << "      TITAN storm properties" << endl;
  cout << endl;
  cout << setw(11) << "Time " << utimstr(entry.time) << endl;
  cout << endl;
  cout << setw(25) << "Complex track num: " << entry.complex_track_num << endl;
  cout << setw(25) << "Simple  track num: " << entry.simple_track_num << endl;
  cout << endl;

  for (int ii = 0; ii < _params.property_list_n; ii++) {
    _printProperty(_params._property_list[ii],
		   entry, gprops);
  }

  return 0;

}

//////////////////////////////////////
// print no data message

void TitanSelect::_doPrintNoData()
  
{
  
  // scroll down
  
  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // print delimiter

  if (_params.length_delimiter_line > 0) {
    for (int i = 0; i < _params.length_delimiter_line; i++) {
      cout << "-";
    }
    cout << endl;
  }

  // Print no data message
  
  cout << "No data found" << endl;
  cout << endl;
  
}

//////////////////////////////////////
// print specific property

void TitanSelect::_printProperty(Params::property_t property,
				 const track_file_entry_t &entry,
				 const storm_file_global_props_t &gprops)

{

  switch (property) {

  case Params::CENTROID_POSITION:
    cout << setw(25) << "Projected area centroid x: "
	 << gprops.proj_area_centroid_x << endl;
    cout << setw(25) << "Projected area centroid y: "
	 << gprops.proj_area_centroid_y << endl;
    break;
    
  case Params::TOP:
    cout << setw(25) << "Tops (km): " << gprops.top << endl;
    break;
    
  case Params::BASE:
    cout << setw(25) << "Base (km): " << gprops.base << endl;
    break;

  case Params::VOLUME:
    cout << setw(25) << "Volume (km3): " << gprops.volume << endl;
    break;

  case Params::AREA_MEAN:
    cout << setw(25) << "Mean area (km2): " << gprops.area_mean << endl;
    break;

  case Params::PRECIP_FLUX:
    cout << setw(25) << "Precip flux (m3/s): " << gprops.precip_flux << endl;
    break;

  case Params::MASS:
    cout << setw(25) << "Mass (ktons): " << gprops.mass << endl;
    break;

  case Params::TILT_ANGLE:
    cout << setw(25) << "Tilt angle (deg): " << gprops.tilt_angle << endl;
    break;

  case Params::TILT_DIRN:
    cout << setw(25) << "Tilt direction (deg): " << gprops.tilt_dirn << endl;
    break;

  case Params::DBZ_MAX:
    cout << setw(25) << "Max DBZ: " << gprops.dbz_max << endl;
    break;

  case Params::DBZ_MEAN:
    cout << setw(25) << "Mean DBZ: " << gprops.dbz_mean << endl;
    break;

  case Params::DBZ_MAX_GRADIENT:
    cout << setw(25) << "Max DBZ grad (dBZ/km): "
	 << gprops.dbz_max_gradient << endl;
    break;

  case Params::DBZ_MEAN_GRADIENT:
    cout << setw(25) << "Mean DBZ grad (dBZ/km): "
	 << gprops.dbz_mean_gradient << endl;
    break;

  case Params::HT_OF_DBZ_MAX:
    cout << setw(25) << "Ht of max DBZ (km): "
	 << gprops.ht_of_dbz_max << endl;
    break;

  case Params::PROJ_AREA:
    cout << setw(25) << "Projected area (km2): "
	 << gprops.proj_area << endl;
    break;

  case Params::HAIL_PRESENT:
    cout << setw(25) << "Is hail present?: "
	 << (gprops.hail_present? "yes" : "no") << endl;
    break;

  case Params::VIL_FROM_MAXZ:
    cout << setw(25) << "VIL from max Z (km/m2): "
	 << gprops.vil_from_maxz << endl;
    break;

  case Params::LTG_COUNT:
    cout << setw(25) << "Ltg count: " << gprops.ltg_count << endl;
    break;

  case Params::FOKRCATEGORY:
    cout << setw(25) << "Hail - FOKR Cat: "
	 << gprops.add_on.hail_metrics.FOKRcategory << endl;
    break;

  case Params::POH:
    cout << setw(25) << "Hail - POH: "
	 <<  gprops.add_on.hail_metrics.waldvogelProbability << endl;
    break;

  case Params::HAILMASSALOFT:
    cout << setw(25) << "Hail - mass aloft: "
	 << gprops.add_on.hail_metrics.hailMassAloft << endl;
    break;

  case Params::VIHM:
    cout << setw(25) << "Hail - Vihm: "
	 << gprops.add_on.hail_metrics.vihm << endl;
    break;

  } // switch

}

