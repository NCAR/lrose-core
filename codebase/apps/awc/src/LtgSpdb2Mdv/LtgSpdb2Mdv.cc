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
// LtgSpdb2Mdv.cc
//
// LtgSpdb2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////
//
// Performs temporal smoothing on Mdv file data
//
///////////////////////////////////////////////////////////////

#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <euclid/CircularTemplate.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <rapformats/GenPt.hh>
#include <rapformats/LtgGroup.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "LtgSpdb2Mdv.hh"
#include "OutputFile.hh"

using namespace std;

// Constructor

LtgSpdb2Mdv::LtgSpdb2Mdv(int argc, char **argv)

{

  isOK = true;
  _trigger = NULL;

  // set programe name

  _progName = "LtgSpdb2Mdv";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = new char[strlen("unknown")+1];
  strcpy(_paramsPath, "unknown");
  
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the trigger object
  
  if (_params.debug >= Params::DEBUG_NORM)
    cerr << "*** Creating data trigger object" << endl;
  
  switch (_params.mode)
  {
  case Params::REALTIME :
  {
    if (_params.watch_latest_data_info) {
      string triggerUrl = _params.realtime_trigger_url;
      if(triggerUrl.empty())
      {
        triggerUrl = _params.input_url;
      }
      if (_params.debug >= Params::DEBUG_NORM)
      {
	cerr << "Initializing latest data REALTIME trigger:" << endl;
	cerr << "    url = " << triggerUrl << endl;
	cerr << "    max valid age = " << _params.max_realtime_valid_age << endl;
      }

      DsLdataTrigger *trigger = new DsLdataTrigger();
      if (trigger->init(triggerUrl,
			_params.max_realtime_valid_age,
			PMU_auto_register) != 0)
      {
	cerr << "ERROR: " << _progName << endl;
	cerr << "Error initializing REALTIME ldata trigger" << endl;
	cerr << "url = " << _params.input_url << endl;
	cerr << "max valid age = " << _params.max_realtime_valid_age << endl;
	isOK = false;
      }
      
      _trigger = trigger;
    } else {
      if (_params.debug >= Params::DEBUG_NORM)
      {
	cerr << "Initializing interval REALTIME trigger:" << endl;
	cerr << "    interval = " << _params.trigger_interval << endl;
      }
      
      DsIntervalTrigger *trigger = new DsIntervalTrigger();
      if (trigger->init(_params.trigger_interval,
			0, 1,
			PMU_auto_register) != 0)
      {
	cerr << "ERROR: " << _progName << endl;
	cerr << "Error initializing REALTIME interval trigger" << endl;
	cerr << "interval = " << _params.trigger_interval << " secs" << endl;
	isOK = false;
      }
      
      _trigger = trigger;
    }
    break;
  }
  
  case Params::ARCHIVE :
  {
    if (_params.debug >= Params::DEBUG_NORM)
    {
      cerr << "Initializing interval ARCHIVE trigger:" << endl;
      cerr << "    interval = " << _params.trigger_interval << endl;
      cerr << "    start time = " << DateTime::str(_args.startTime) << endl;
      cerr << "    end time = " << DateTime::str(_args.endTime) << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params.trigger_interval,
		      _args.startTime, _args.endTime) != 0)
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Error initializing ARCHIVE trigger" << endl;
      cerr << "interval = " << _params.trigger_interval << " secs" << endl;
      cerr << "start time = " << DateTime::str(_args.startTime) << endl;
      cerr << "end time = " << DateTime::str(_args.endTime) << endl;
      isOK = false;
    }
    
    _trigger = trigger;

    break;
  }

  case Params::TIME_LIST :
  {
    if (_params.debug >= Params::DEBUG_NORM)
    {
      cerr << "Initializing TIME_LIST trigger:" << endl;
      cerr << "    url = " << _params.trigger_url << endl;
      cerr << "    start time = " << DateTime::str(_args.startTime) << endl;
      cerr << "    end time = " << DateTime::str(_args.endTime) << endl;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params.trigger_url,
		      _args.startTime, _args.endTime) != 0)
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "url = " << _params.trigger_url << endl;
      cerr << "start time = " << DateTime::str(_args.startTime) << endl;
      cerr << "end time = " << DateTime::str(_args.endTime) << endl;
      isOK = false;
    }
    
    _trigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params.mode */
  
  // load up the computational kernel - this is used to create
  // a radius of influence around the lightning strike
  // The radius is in grid units.

  _loadKernel();

  return;

}

// destructor

LtgSpdb2Mdv::~LtgSpdb2Mdv()

{

  // free up

  delete _trigger;

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int LtgSpdb2Mdv::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // loop until end of data

  DateTime triggerTime;
  
  while (_trigger->nextIssueTime(triggerTime) == 0) {
    
    if (_params.debug) {
      cerr << "  Trigger time: " << triggerTime << endl;
    }

    // delay if required

    if (_params.processing_delay > 0) {
      for (int i = 0; i < _params.processing_delay; i++) {
	PMU_auto_register("Putting in processing delay");
	sleep (1);
      }
    }

    // create DsMdvx output object
    
    OutputFile out(_progName, _params, triggerTime.utime());
    DsMdvx &mdvx = out.getDsMdvx();

    // read in the lightning data

    vector<LTG_extended_t> strikes;

    if (_readLtg(triggerTime.utime(),
		 MdvxProj(mdvx.getMasterHeader(),
			  mdvx.getField(0)->getFieldHeader()),
		 strikes)) {
      cerr << "ERROR - LtgSpdb2Mdv::Run" << endl;
      cerr << "  Cannot read lightning data" << endl;
      cerr << "  URL: " << _params.input_url << endl;
      continue;
    }

    // load the rate field
    
    _loadRateField(strikes, mdvx);

    // load the distance field
    
    _loadDistanceField(strikes, mdvx);

    // load the derived field
    
    _loadDerivedField(mdvx);

    // write the file

    if (out.writeVol()) {
      cerr << "ERROR - LtgSpdb2Mdv::Run" << endl;
      cerr << mdvx.getErrStr() << endl;
    }
    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// load up the computational kernel - this is used to create
// a radius of influence around the lightning strike
// The radius is in grid units.

void LtgSpdb2Mdv::_loadKernel()
{

  // Figure out which grid squares are affected by the strike

  for (int x_offset = 0; x_offset <= _params.ltg_radius; x_offset++) {
    
    for (int y_offset = 0; y_offset <= _params.ltg_radius; y_offset++) {

      double dist = sqrt((double) x_offset * (double) x_offset +
			 (double) y_offset * (double) y_offset);

      if (dist <= _params.ltg_radius) {
	
	grid_offset_t offset;
	
	offset.x_offset = x_offset;
	offset.y_offset = y_offset;
	_kernel.push_back(offset);
	
	if (y_offset != 0) {
	  offset.x_offset = x_offset;
	  offset.y_offset = -y_offset;
	  _kernel.push_back(offset);
	}
	
	if (x_offset != 0) {
	  offset.x_offset = -x_offset;
	  offset.y_offset = y_offset;
	  _kernel.push_back(offset);
	}
	
	if (x_offset != 0 && y_offset != 0) {
	  offset.x_offset = -x_offset;
	  offset.y_offset = -y_offset;
	  _kernel.push_back(offset);
	}

      } // if (dist ....
      
    } // y_offset
    
  } // x_offset
  
  // Print the vector when debugging

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    vector<grid_offset_t>::iterator i;
    cerr << "Computational kernel offset list:" << endl;
    for (i = _kernel.begin(); i != _kernel.end(); i++) {
      cerr << "  " << i->x_offset << ", " << i->y_offset << endl;
    }
  }
  
}

//////////////////////////////////////////////////
// read in the lightning data

int LtgSpdb2Mdv::_readLtg(time_t trigger_time,
			  const MdvxProj &proj,
			  vector<LTG_extended_t> &strikes)
  
{
  
  strikes.erase(strikes.begin(), strikes.end());

  time_t start_time = trigger_time - _params.rate_compute_period;
  time_t end_time = trigger_time + _params.rate_extend_period;

  if (_params.debug) {
    cerr << "Retrieving ltg data:" << endl;
    cerr << "  Start time: " << DateTime::str(start_time) << endl;
    cerr << "  End time  : " << DateTime::str(end_time) << endl;
  }

  DsSpdb spdb;
  spdb.setProdId(_params.spdb_ltg_id);
  if (spdb.getInterval(_params.input_url,
		       start_time, end_time)) {
    cerr << "ERROR - LtgSpdb2Mdv::_readLtg" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  // Get the information needed for normalizing the strike longitudes
  // to the defined output grid.

  double min_lat, min_lon;
  proj.xyIndex2latlon(0, 0, min_lat, min_lon);
  
  // load up the strike vector
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  int nstrikesFound = 0;

  if (chunks.size() == 0) {
    if (_params.create_empty_files)
    {
      // found no data, so warn the user
      cerr << "WARNING - LtgSpdb2Mdv::_readLtg" << endl;
      cerr << "  No data available for interval requested" << endl;
      cerr << "  Creating empty MDV file" << endl;
    }
    else
    {
      // found no data, so do not create MDV file
      cerr << "ERROR - LtgSpdb2Mdv::_readLtg" << endl;
      cerr << "  No data available for interval requested" << endl;
      return -1;
    }
  }

  for (size_t ii = 0; ii < chunks.size(); ii++) {
    if (_params.spdb_ltg_id == SPDB_LTG_GROUP_ID) {
      _readGroups(chunks[ii], min_lon,
		  proj.getProjType() == Mdvx::PROJ_LATLON,
		  nstrikesFound, strikes);
    } else if (_params.spdb_ltg_id == SPDB_GENERIC_POINT_ID) {
      _readGenPts(chunks[ii], min_lon,
		  proj.getProjType() == Mdvx::PROJ_LATLON,
		  nstrikesFound, strikes);
    } else {
      if (chunks[ii].len >= (int) sizeof(ui32)) {
	ui32 cookie;
	memcpy(&cookie, chunks[ii].data, sizeof(cookie));
	if (cookie == LTG_EXTENDED_COOKIE) {
	  _readExtendedStrikes(chunks[ii], min_lon,
			       proj.getProjType() == Mdvx::PROJ_LATLON,
			       nstrikesFound, strikes);
	} else {
	  _readOriginalStrikes(chunks[ii], min_lon,
			       proj.getProjType() == Mdvx::PROJ_LATLON,
			       nstrikesFound, strikes);
	}
      }
    }
    
  } // ii

  if (_params.debug) {
    cerr << "  Retrieved " << chunks.size() << " chunks." << endl;
    cerr << "  Found     " << nstrikesFound << " strikes." << endl;
    cerr << "  Accepted  " << strikes.size() << " strikes." << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// read in the lightning group data

void LtgSpdb2Mdv::_readGroups(const Spdb::chunk_t &chunk,
			      const double min_lon,
			      const bool normalize_lon,
			      int &nstrikesFound,
			      vector<LTG_extended_t> &strikes) const
{
  static const string method_name = "LtgSpdb2Mdv::_readGroups()";
  
  nstrikesFound++;

  LtgGroup group;
  
  if (group.disassemble(chunk.data, chunk.len) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling lightning group" << endl;
    return;
  }
  
  LTG_extended_t extendedStrike;
  MEM_zero(extendedStrike);

  extendedStrike.time = group.getTime();
  extendedStrike.latitude = group.getLat();
  extendedStrike.longitude = group.getLon();
  extendedStrike.amplitude = group.getNetRadiance();

  // normalize strike longitude to output grid
  if (normalize_lon)
  {
    while (extendedStrike.longitude < min_lon)
      extendedStrike.longitude += 360.0;
    while (extendedStrike.longitude >= min_lon + 360.0)
      extendedStrike.longitude -= 360.0;
  }

  strikes.push_back(extendedStrike);

}

//////////////////////////////////////////////////
// read in the GenPt data

void LtgSpdb2Mdv::_readGenPts(const Spdb::chunk_t &chunk,
			      const double min_lon,
			      const bool normalize_lon,
			      int &nstrikesFound,
			      vector<LTG_extended_t> &strikes) const
{
  static const string method_name = "LtgSpdb2Mdv::_readGenPts()";
  
  nstrikesFound++;

  GenPt gen_pt;
  
  if (gen_pt.disassemble(chunk.data, chunk.len) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling GenPt data" << endl;
    return;
  }
  
  LTG_extended_t extendedStrike;
  MEM_zero(extendedStrike);

  extendedStrike.time = gen_pt.getTime();
  extendedStrike.latitude = gen_pt.getLat();
  extendedStrike.longitude = gen_pt.getLon();
  extendedStrike.amplitude = 0;

  // normalize strike longitude to output grid

  if (normalize_lon)
  {
    while (extendedStrike.longitude < min_lon)
      extendedStrike.longitude += 360.0;
    while (extendedStrike.longitude >= min_lon + 360.0)
      extendedStrike.longitude -= 360.0;
  }

  strikes.push_back(extendedStrike);

}

//////////////////////////////////////////////////
// read in the extended strike lightning data

void LtgSpdb2Mdv::_readExtendedStrikes(const Spdb::chunk_t &chunk,
				       const double min_lon,
				       const bool normalize_lon,
				       int &nstrikesFound,
				       vector<LTG_extended_t> &strikes) const
{
  int nn = chunk.len / sizeof(LTG_extended_t);
  LTG_extended_t *strikeArray = (LTG_extended_t *) chunk.data;
  for (int jj = 0; jj < nn; jj++) {
    nstrikesFound++;
    LTG_extended_t strike = strikeArray[jj];
    LTG_extended_from_BE(&strike);
    // normalize strike longitude to output grid
    if (normalize_lon)
    {
      while (strike.longitude < min_lon)
	strike.longitude += 360.0;
      while (strike.longitude >= min_lon + 360.0)
	strike.longitude -= 360.0;
    }
    // check whether to accept strike
    if (_acceptStrike(strike)) {
      strikes.push_back(strike);
    }
  } // jj
}

//////////////////////////////////////////////////
// read in the original strike lightning data

void LtgSpdb2Mdv::_readOriginalStrikes(const Spdb::chunk_t &chunk,
				       const double min_lon,
				       const bool normalize_lon,
				       int &nstrikesFound,
				       vector<LTG_extended_t> &strikes) const
{
  int nn = chunk.len / sizeof(LTG_strike_t);
  LTG_strike_t *strikeArray = (LTG_strike_t *) chunk.data;
  for (int jj = 0; jj < nn; jj++) {
    nstrikesFound++;
    LTG_strike_t strike = strikeArray[jj];
    LTG_from_BE(&strike);
    LTG_extended_t extendedStrike;
    MEM_zero(extendedStrike);
    extendedStrike.time = strike.time;
    extendedStrike.latitude = strike.latitude;
    extendedStrike.longitude = strike.longitude;
    extendedStrike.amplitude = strike.amplitude;
    extendedStrike.type = strike.type;
    // normalize strike longitude to output grid
    if (normalize_lon)
    {
      while (extendedStrike.longitude < min_lon)
	extendedStrike.longitude += 360.0;
      while (extendedStrike.longitude >= min_lon + 360.0)
	extendedStrike.longitude -= 360.0;
    }
    // check whether to accept strike
    if (_acceptStrike(extendedStrike)) {
      strikes.push_back(extendedStrike);
    }
  } // jj
}

//////////////////////////////////////////////////
// check whether to accept the strike

bool LtgSpdb2Mdv::_acceptStrike(const LTG_extended_t &strike) const
{

  bool accept = true;

  // Check the polarity of the strike
  
  if ((_params.polarity_flag == Params::POLAR_POS
       && strike.amplitude < 0) ||
      (_params.polarity_flag == Params::POLAR_NEG
       && strike.amplitude > 0)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "NOTE: Lightning data with wrong polarity" << endl;
      cerr << "  amplitude: " << strike.amplitude << endl;
      switch (_params.polarity_flag) {
      case Params::POLAR_POS:
	cerr << "  polarity_flag: POLAR_POS" << endl;
	break;
      case Params::POLAR_NEG:
	cerr << "  polarity_flag: POLAR_NEG" << endl;
	break;
      case Params::POLAR_BOTH:
	cerr << "  polarity_flag: POLAR_BOTH" << endl;
	break;
      } // switch
    }
    accept = false;
  } // polarity check
  
  // Check the amplitude
  
  double amp_magnitude = fabs(strike.amplitude);
  if ((_params.min_amplitude >= 0 &&
       amp_magnitude < _params.min_amplitude) ||
      (_params.max_amplitude >= 0 &&
       amp_magnitude > _params.max_amplitude)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "NOTE: Lightning data outside amplitude range" << endl;
      cerr << "  amplitude: " << strike.amplitude << endl;
      cerr << "  min_amplitude: " << _params.min_amplitude << endl;
      cerr << "  max_amplitude: " << _params.max_amplitude << endl;
    }
    accept = false;
  } // amplitude check

  return accept;

}

///////////////////////////////////////////////////
// load the rate field
    
void LtgSpdb2Mdv::_loadRateField(const vector<LTG_extended_t> &strikes,
				DsMdvx &mdvx)

{

  // get rate field

  MdvxField *fld = mdvx.getFieldByNum(0);
  if (fld == NULL) {
    return;
  }
  fl32 *rate = (fl32 *) fld->getVol();

  // create Mdvx projection object, for mapping the lat/lon of
  // the strikes into grid locations

  MdvxProj proj(mdvx.getMasterHeader(), fld->getFieldHeader());
  int nx = proj.getCoord().nx;
  int ny = proj.getCoord().ny;

  // loop through the strike array

  for (size_t ii = 0; ii < strikes.size(); ii++) {

    int ltg_x, ltg_y;

    if (proj.latlon2xyIndex(strikes[ii].latitude, strikes[ii].longitude,
			    ltg_x, ltg_y) == 0) {

      // strike is within grid - update the lightning total in each
      // of the appropriate grid squares

      for (size_t kk = 0; kk < _kernel.size(); kk++) {

	int grid_x = ltg_x + _kernel[kk].x_offset;
	int grid_y = ltg_y + _kernel[kk].y_offset;
	
	if (grid_x >= 0 && grid_x < nx &&
	    grid_y >= 0 && grid_y < ny) {
	  
	  int grid_index = nx * grid_y + grid_x;
	  
	  rate[grid_index]++;
	  
	}
	
      } // kk

    } else {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Rejecting ltg strike outside bounding box" << endl;
        cerr << "  lat, lon: "
             << strikes[ii].latitude << ", " << strikes[ii].longitude << endl;
      }

    } // if (proj.latlon2xyIndex(...

  } // ii

}

///////////////////////////////////////////////////
// load the distance field
    
void LtgSpdb2Mdv::_loadDistanceField(const vector<LTG_extended_t> &strikes,
				    DsMdvx &mdvx)

{

  if(!_params.add_distance_field)
    return;

  // get rate field
  MdvxField *fld = mdvx.getFieldByName(_params.distance_field_name);
  if (fld == NULL) {
    return;
  }
  fl32 *dist = (fl32 *) fld->getVol();

  // create Mdvx projection object, for mapping the lat/lon of
  // the strikes into grid locations

  Mdvx::field_header_t fhdr = fld->getFieldHeader();
  MdvxProj proj(mdvx.getMasterHeader(), fhdr);
  int nx = proj.getCoord().nx;
  int ny = proj.getCoord().ny;
  double dx = proj.getCoord().dx;
  double dy = proj.getCoord().dy;
  double strike_radius = hypot(_params.max_strike_radius/dx, 
			       _params.max_strike_radius/dy);

  // Create a circular template object.
  CircularTemplate circular_template(strike_radius);

  // loop through the strike array

  for (size_t ii = 0; ii < strikes.size(); ii++) {

    int ltg_x, ltg_y;

    if (proj.latlon2xyIndex(strikes[ii].latitude, strikes[ii].longitude,
			    ltg_x, ltg_y) == 0) {

      // loop through the template's points

      for (GridPoint *point =
	     circular_template.getFirstInGrid(ltg_x, ltg_y, nx, ny);
           point != (GridPoint *)NULL;
           point = circular_template.getNextInGrid()) {

	double grid_lat;
	double grid_lon;

	proj.xyIndex2latlon(point->x, point->y, grid_lat, grid_lon);

	double radius;
	double theta;
	PJGLatLon2RTheta(strikes[ii].latitude, strikes[ii].longitude, 
			 grid_lat, grid_lon, &radius, &theta);
	
	int grid_index = point->getIndex(nx, ny);
	
	if (dist[grid_index] == fhdr.missing_data_value) {
	  dist[grid_index] = radius;
	}
	else if (dist[grid_index] > radius) {
	  dist[grid_index] = radius;
	} // if

      } //  for (GridPoint *point = ...

    } // if (proj.latlon2xyIndex(...

  } // ii

}

///////////////////////////////////////////////////
// load the derived field
    
void LtgSpdb2Mdv::_loadDerivedField(DsMdvx &mdvx)

{

  if(!_params.add_derived_field)
    return;

  // get rate field
  
  MdvxField *rateFld = mdvx.getFieldByNum(0);
  if (rateFld == NULL) {
    return;
  }
  fl32 *rate = (fl32 *) rateFld->getVol();

  // get derived field
  
  MdvxField *derivedFld = mdvx.getFieldByName(_params.derived_field_name);
  if (derivedFld == NULL) {
    return;
  }
  fl32 *derived = (fl32 *) derivedFld->getVol();

  // loop through the grids

  int nVals = rateFld->getVolNumValues();

  for (int ipt = 0; ipt < nVals; ipt++, rate++, derived++) {
    int rateRounded = (int) (*rate + 0.5);
    *derived = _getDerivedVal(rateRounded);
  }

}

///////////////////////////////////////////////////
// Determines the deruved data value for
// the given number of strikes.  Returns 0.0 if the
// given number of strikes isn't found in the table.

double LtgSpdb2Mdv::_getDerivedVal(const int num_strikes) const

{

  // Make sure the number of strikes is reasonable
  
  if (num_strikes < 0) {
    return(0.0);
  }
  
  // Look through the table for the appropriate entry

  for (int i = 0; i < _params.ltg_factors_n - 1; i++)
  {
    if (num_strikes >= _params._ltg_factors[i].num_strikes &&
	num_strikes < _params._ltg_factors[i+1].num_strikes)
      return(_params._ltg_factors[i].ltg_value);
  }
  
  int last_entry = _params.ltg_factors_n - 1;
  
  if (num_strikes >= _params._ltg_factors[last_entry].num_strikes)
    return(_params._ltg_factors[last_entry].ltg_value);
  
  return(0.0);
  
}

