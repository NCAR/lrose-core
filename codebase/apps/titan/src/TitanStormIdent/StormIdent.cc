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
// StormIdent.cc
//
// StormIdent object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Dave Albo
//
// August 2014
//
///////////////////////////////////////////////////////////////

#include "StormIdent.hh"
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>

const int StormIdent::maxEigDim = 3;
const float StormIdent::missingVal = -9999.0;

// Constructor

StormIdent::StormIdent(const string &prog_name,
		       const Params &params) :
  Worker(prog_name, params) ,
  _inputMdv(_progName, _params),
  _thresh(_progName, _params),
  _identify(_progName, _params, _inputMdv, _sfile, _thresh)
{

  _fatalError = false;

}

// destructor

StormIdent::~StormIdent()

{

}

//////////////////////////////////////////////////
// runRealtime

int StormIdent::runRealtime()
{
  
  // register with procmap
  
  PMU_auto_register("StormIdent::runRealtime");

  storm_file_params_t sparams;
  _loadStormParams(&sparams);

  // copy storm file params to header
  _sfile._header.params = sparams;
  

  if (_params.forecast_mode)  {
    return _runForecast();
  }
  else {
    return _runFlat();
  }
}

      
//////////////////////////////////////////////////
// runArchive

int StormIdent::runArchive(time_t start_time,
			   time_t end_time)

{

  if (_params.debug) {
    cerr << "***************** ARCHIVE MODE **********************" << endl;
    cerr << "  startTime: " << DateTime::str(start_time) << endl;
    cerr << "  endTime: " << DateTime::str(end_time) << endl;
    cerr << "*****************************************************" << endl;
  }
  
  // register with procmap
  
  PMU_auto_register("StormIdent::runArchive");
  
  storm_file_params_t sparams;
  _loadStormParams(&sparams);

  // copy storm file params to header
  _sfile._header.params = sparams;

  if (_params.forecast_mode)  {
    return _runForecast(start_time, end_time);
  }
  else {
    return _runFlat(start_time, end_time);
  }

}

//////////////////////////////////////////////////
// runForecast

int StormIdent::_runForecast(void)
{
  DsUrlTrigger trigger(_params.input_url, DsUrlTrigger::FCST_LEAD, 
		       _params.debug >= Params::DEBUG_EXTRA);
  trigger.setMaxValidAge(_params.max_realtime_valid_age);

  time_t gt;
  int lt;
  int scanNum = 1;
  while (trigger.nextTime(gt, lt)) {
    cerr << "------------------Triggered " << DateTime::strn(gt) 
	 << " + " << lt 
	 << " ----------------" << endl;
    if (_processScan(scanNum, gt, lt) == 0) {

      if (_fatalError) {

	cerr << "ERROR - StormIdent::runForecast" << endl;
	return -1;

      } else {

	// increment scan num
	
	scanNum++;
      }
    }
  }
  return 0;
}



//////////////////////////////////////////////////
// runFlat

int StormIdent::_runFlat(void)
{

  DsUrlTrigger trigger(_params.input_url, DsUrlTrigger::OBS,
		       _params.debug >= Params::DEBUG_EXTRA);
  trigger.setMaxValidAge(_params.max_realtime_valid_age);
  time_t t;
  int scanNum = 1;
  while (trigger.nextTime(t)) {
    cerr << "------------------ Triggered " << DateTime::strn(t) 
	 << " ------------------" << endl;
    if (_processScan(scanNum, t) == 0) {

      if (_fatalError) {

	cerr << "ERROR - StormIdent::runFlat" << endl;
	return -1;

      } else {

	// _writeLdataInfo();

	// increment scan num
	
	scanNum++;
      }
    }
  }
  return 0;
}

//////////////////////////////////////////////////
// runForecast

int StormIdent::_runForecast(time_t start_time, time_t end_time)
{
  DsUrlTrigger trigger(start_time, end_time,
		       _params.input_url, DsUrlTrigger::FCST_LEAD, 
		       _params.debug >= Params::DEBUG_EXTRA);

  time_t gt;
  int lt;
  int scanNum = 1;

  if (_params.low_dbz_threshold_spdb_archive_rerun)
  {
    _thresh.set_archive_rerun(end_time);
  }

  while (trigger.nextTime(gt, lt)) {
    cerr << "------------------Triggered " << DateTime::strn(gt) 
	 << " + " << lt 
	 << " ----------------" << endl;
    if (_processScan(scanNum, gt, lt) == 0) {

      if (_fatalError) {

	cerr << "ERROR - StormIdent::runForecast" << endl;
	return -1;

      } else {

	// increment scan num
	
	scanNum++;
      }
    }
  }
  return 0;
}



//////////////////////////////////////////////////
// runFlat

int StormIdent::_runFlat(time_t start_time, time_t end_time)
{

  DsUrlTrigger trigger(start_time, end_time,
		       _params.input_url, DsUrlTrigger::OBS,
		       _params.debug >= Params::DEBUG_EXTRA);

  time_t t;
  int scanNum = 1;
  while (trigger.nextTime(t)) {
    cerr << "------------------Triggered " << DateTime::strn(t) 
	 << " ----------------" << endl;
    if (_processScan(scanNum, t) == 0) {

      if (_fatalError) {

	cerr << "ERROR - StormIdent::runFlat" << endl;
	return -1;

      } else {

	// increment scan num
	
	scanNum++;
      }
    }
  }
  return 0;
}

//////////////////
// process a scan

int StormIdent::_processScan(int scan_num, time_t scan_time)

{

  char pmu_message[128];
  sprintf(pmu_message, "Processing %s\n", utimstr(scan_time));
  PMU_auto_register(pmu_message);
  _fatalError = false;
    
  // read in MDV data
  
  if (_inputMdv.read(scan_time)) {
    cerr << "ERROR - StormIdent::_identAndTrack" << endl;
    cerr << "  Cannot read data for time: " << utimstr(scan_time) << endl;
    return -1;
  }

  // set low dbz threshold to parameterized value
  _thresh.set_to_defaults();

  // identify storms
  
  if (_identify.run(scan_num, scan_time)) {
    _fatalError = false;
    return -1;
  }
    
  return 0;
  
}
    
//////////////////
// process a scan

int StormIdent::_processScan(int scan_num, time_t scan_gen_time,
			     int scan_lead_time)

{

  char pmu_message[128];
  sprintf(pmu_message, "Processing %s + %d\n", utimstr(scan_gen_time), 
	  scan_lead_time);
  PMU_auto_register(pmu_message);
  
  _fatalError = false;
    
  // read in MDV data
  
  if (_inputMdv.read(scan_gen_time, scan_lead_time)) {
    cerr << "ERROR - StormIdent::_identAndTrack" << endl;
    cerr << "  Cannot read data for time: " << utimstr(scan_gen_time) << "+" 
	 << scan_lead_time << endl;
    return -1;
  }

  // dynamically set threshold for matching gen/lead times

  _thresh.set_dynamically(scan_gen_time, scan_lead_time);

  // reset the values in the file object, in case they were modified:

  _sfile._header.params.low_dbz_threshold = _thresh.get_low_threshold();
  _sfile._header.params.high_dbz_threshold = _thresh.get_high_threshold();
  _sfile._header.params.dbz_hist_interval = _thresh.get_hist_interval();

  // identify storms
  
  if (_identify.run(scan_num, scan_gen_time, scan_lead_time)) {
    _fatalError = false;
    return -1;
  }
    
  return 0;
  
}
    
/////////////////////////////////////////////////////
// _loadStormParams()
//
// Load up storm file params

void StormIdent::_loadStormParams(storm_file_params_t *sparams)

{
  
  // zero out

  memset (sparams, 0, sizeof(storm_file_params_t));

  // fill in parts of file header
  
  sparams->low_dbz_threshold = _thresh.get_low_threshold();
  sparams->high_dbz_threshold = _thresh.get_high_threshold();
  sparams->hail_dbz_threshold = _params.hail_dbz_threshold;
  sparams->dbz_hist_interval = _params.dbz_hist_interval;
  sparams->base_threshold = _params.base_threshold;
  sparams->top_threshold = _params.top_threshold;
  sparams->min_storm_size = _params.min_storm_size;
  sparams->max_storm_size = _params.max_storm_size;
  sparams->z_p_coeff = _params.ZR.coeff;
  sparams->z_p_exponent = _params.ZR.expon;
  sparams->z_m_coeff = _params.ZM.coeff;
  sparams->z_m_exponent = _params.ZM.expon;
  sparams->sectrip_vert_aspect = _params.sectrip_vert_aspect;
  sparams->sectrip_horiz_aspect = _params.sectrip_horiz_aspect;
  sparams->sectrip_orientation_error = _params.sectrip_orientation_error;
  sparams->n_poly_sides = N_POLY_SIDES;
  sparams->poly_start_az = 0.0;
  sparams->poly_delta_az = 360.0 / (double) N_POLY_SIDES;
  sparams->vel_available = _params.vel_available;
  sparams->hail_z_m_coeff = _params.hail_ZM.coeff;
  sparams->hail_z_m_exponent = _params.hail_ZM.expon;

  // Indication of how to interpret the add_on union in gprops

  if ( !strcmp(_params.special_feature, "hail") ) {
    sparams->gprops_union_type = UNION_HAIL;
  }
  else {
    sparams->gprops_union_type = UNION_NONE;
  }

  if (_params.set_dbz_threshold_for_tops) {
    sparams->tops_dbz_threshold = _params.tops_dbz_threshold;
  } else {
    sparams->tops_dbz_threshold = _thresh.get_low_threshold();
  }
  
  if (_params.precip_computation_mode == Params::PRECIP_FROM_COLUMN_MAX) {
    sparams->precip_computation_mode = TITAN_PRECIP_FROM_COLUMN_MAX;
    sparams->precip_plane_ht = -9999;
  } else if (_params.precip_computation_mode == Params::PRECIP_AT_SPECIFIED_HT) {
    sparams->precip_computation_mode = TITAN_PRECIP_AT_SPECIFIED_HT;
    sparams->precip_plane_ht = _params.precip_plane_ht;
  } else if (_params.precip_computation_mode == Params::PRECIP_AT_LOWEST_VALID_HT) {
    sparams->precip_computation_mode = TITAN_PRECIP_AT_LOWEST_VALID_HT;
    sparams->precip_plane_ht = _params.base_threshold;
  } else if (_params.precip_computation_mode == Params::PRECIP_FROM_LOWEST_AVAILABLE_REFL) {
    sparams->precip_computation_mode = TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL;
    sparams->precip_plane_ht = _params.base_threshold;
  }

}
