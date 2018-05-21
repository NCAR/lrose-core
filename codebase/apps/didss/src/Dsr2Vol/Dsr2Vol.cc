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
////////////////////////////////////////////////////////////////////////
// Dsr2Vol.cc
//
// Dsr2Vol object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////////////
//
// Dsr2Vol reads an input radar FMQ, puts the data into a grid, and
// saves it out as an MDV file.
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>

#include "Dsr2Vol.hh"
#include "CartTransform.hh"
#include "PpiTransform.hh"
#include "PolarTransform.hh"
#include "RhiTransform.hh"

using namespace std;

const double Dsr2Vol::_smallVal = 0.0001;

// Constructor

Dsr2Vol::Dsr2Vol(int argc, char **argv)

{

  isOK = true;
  _nBeamsRead = 0;

  _prevElevMoving = -999;
  _prevAzMoving = -999;

  _startTime = 0;
  _endTime = 0;

  _prevPrfGood = true;
  _radarLat = 0;
  _radarLon = 0;
  _radarAlt = 0;
  _beamWidth = 0;

  _prevVolNum = -99999;
  _volNum = -1;
  _endOfVol = false;
  _ppiMgr = NULL;
  _antenna = NULL;
  _beamGeomMgr = NULL;
  _latestRadarParams = NULL;
  _calib = NULL;

  _scanMode = SCAN_MODE_UNKNOWN;
  _scanInfoFromHeaders = false;

  // set programe name

  _progName = "Dsr2Vol";
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
  _nyquist = _params.nyquist_velocity;

  // create field refs vector

  _dbzFieldNum = -1;
  _snrFieldNum = -1;
  _thresholdFieldNum = -1;
  for (int i = 0; i < _params.output_fields_n; i++) {

    Params::output_field_t &outField = _params._output_fields[i];
    
    if (outField.is_dbz && _dbzFieldNum < 0) {
      _dbzFieldNum = i;
    }

    if (strcmp(outField.dsr_name, _params.threshold_field_name) == 0) {
      _thresholdFieldNum = i;
    }

    if (strcmp(outField.dsr_name, _params.snr_field_name) == 0) {
      _snrFieldNum = i;
    }

    Mdvx::encoding_type_t encoding = Mdvx::ENCODING_INT8;
    if (outField.encoding == Params::ENCODING_INT16) {
      encoding = Mdvx::ENCODING_INT16;
    } else if (outField.encoding == Params::ENCODING_FLOAT32) {
      encoding = Mdvx::ENCODING_FLOAT32;
    }
    
    Mdvx::compression_type_t compression = Mdvx::COMPRESSION_ZLIB;
    if (_params.output_compression == Params::RLE_COMPRESSION) {
      compression = Mdvx::COMPRESSION_RLE;
    } else if (_params.output_compression == Params::BZIP_COMPRESSION) {
      compression = Mdvx::COMPRESSION_BZIP;
    } else if (_params.output_compression == Params::GZIP_COMPRESSION) {
      compression = Mdvx::COMPRESSION_GZIP;
    } else if (_params.output_compression == Params::NO_COMPRESSION) {
      compression = Mdvx::COMPRESSION_NONE;
    }

    FieldInfo finfo(outField.dsr_name,
		    outField.output_name,
		    outField.output_units,
		    outField.transform,
		    outField.is_dbz,
		    outField.interp_db_as_power,
		    outField.is_vel,
		    outField.allow_interp,
		    encoding, compression);

    _fields.push_back(finfo);
    
  }

  // print out field info

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printFields();
  }
  
  // clear all arrays
  
  _clearAll();

  // create antenna angle manager

  _antenna = new Antenna(_progName, _params);
  _endOfVolAutomatic =
    (_params.end_of_vol_decision == Params::AUTOMATIC);
  
  // create beam geometry manager

  _beamGeomMgr = new BeamGeomMgr(_params);
  
  // set up the PPI manager

  _ppiMgr = new PpiMgr(_progName, _params,
                       _fields, _beamsStored, *_beamGeomMgr);

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

}

/////////////////////////////////////////////////////////
// destructor

Dsr2Vol::~Dsr2Vol()

{

  _clearAll();

  if (_ppiMgr != NULL) {
    delete _ppiMgr;
  }

  if (_antenna != NULL) {
    delete _antenna;
  }

  if (_beamGeomMgr != NULL) {
    delete _beamGeomMgr;
  }

  if (_latestRadarParams != NULL) {
    delete _latestRadarParams;
  }

  if (_calib != NULL) {
    delete _calib;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Dsr2Vol::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    if (_params.debug) {
      cerr << "Dsr2Vol::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
    }
    umsleep(1000);
  }

  return 0;

}

//////////////////////////////////////////////////
// _run

int Dsr2Vol::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Instantiate and initialize the DsRadar queue and message

  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;

  if (_params.seek_to_end_of_input) {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Vol::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  } else {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Vol::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }
  
  // read beams from the queue and process them
  
  _nBeamsRead = 0;
  int nUsed = 0;
  time_t timeLastMsg = time(NULL);
  
  while (true) {

    PMU_auto_register("Reading radar FMQ");

    bool gotMsg = false;
    if (_readMsg(radarQueue, radarMsg, gotMsg)) {
      umsleep(100);
    }
    
    if (gotMsg) {
      timeLastMsg = time(NULL);
    } else {
      umsleep(100);
      if (_nBeamsRead > _params.min_beams_in_vol &&
	  _params.write_end_of_vol_when_data_stops) {
        time_t now = time(NULL);
        int timeSinceLastMsg = now - timeLastMsg;
        if (timeSinceLastMsg > _params.nsecs_no_data_for_end_of_vol) {
	  _endOfVol = true;
          timeLastMsg = now;
	}
      }
      if (!_endOfVol) {
	continue;
      }
    }
    
    if (_params.debug) {
      if ((_nBeamsRead > 0) && (_nBeamsRead % 90 == 0) &&
	  (int) _beamsStored.size() != nUsed) {
	nUsed = (int) _beamsStored.size();
	fprintf(stderr,
		"  Reading queue, n read, n used, latest time, el, az: "
		"%6d %6d %s %7.2f %7.2f\n",
		_nBeamsRead, nUsed,
		utimstr(_latestBeamTime), _latestBeamEl, _latestBeamAz);
      }
    }
    
    // at the end of a volume, process volume, then reset
    
    if (_endOfVol) {
      if (_params.debug) {
	cerr << "  End of volume." << endl;
	cerr << "    nbeams available:" << _beamsStored.size() << endl;
      }
      if ((int) _beamsStored.size() < _params.min_beams_in_vol) {
	if (_params.debug) {
	  cerr << "  Too few beams in volume" << endl;
	  cerr << "  Not processing volume" << endl;
	}
      } else {
	_processVol();
      }
      _reset(radarMsg);
      _nBeamsRead = 0;
    }
    
  } // while (true)
  
  return 0;

}

////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue, setting the flags about beam_data
// and _endOfVolume appropriately.
//
// Sets gotMsg to true if message was read
//

int Dsr2Vol::_readMsg(DsRadarQueue &radarQueue,
		      DsRadarMsg &radarMsg,
		      bool &gotMsg) 
  
{
  
   
  PMU_auto_register("Reading radar queue");
  _endOfVol = false;
  
  int contents;
  if (radarQueue.getDsMsg(radarMsg, &contents, &gotMsg)) {
    return -1;
  }
  if (!gotMsg) {
    return -1;
  }
    
  // set radar parameters if avaliable
  
  if (contents & DsRadarMsg::RADAR_PARAMS) {
    _loadRadarParams(radarMsg);
  }

  // set radar calibration if avaliable
  
  if (contents & DsRadarMsg::RADAR_CALIB) {
    _loadRadarCalib(radarMsg);
  }

  // set status XML if avaliable
  
  if (contents & DsRadarMsg::STATUS_XML) {
    _statusXml = radarMsg.getStatusXml();
  }

  // set field parameters if available
  
  if (contents & DsRadarMsg::FIELD_PARAMS) {
    _loadFieldParams(radarMsg);
  }
  
  // If we have radar and field params, and there is good beam data,
  // add to the vector
  
  if ((contents & DsRadarMsg::RADAR_BEAM) && radarMsg.allParamsSet()) {

    _nBeamsRead++;
    
    // see if we should keep beam based on geometry and movement
    
    if (_preFilter(radarMsg)) {

      Beam *beam = new Beam(radarMsg, _params, _fields, _nyquist);
      
      // save the beam geometry
      
      _saveLatestBeamInfo(beam);
      
      // save the beam
      
      _beamsStored.push_back(beam);

      // end of vol condition

      if (beam->volNum != -1 && beam->volNum != _prevVolNum) {
        if (_prevVolNum != -99999 &&
            _params.end_of_vol_decision == Params::CHANGE_IN_VOL_NUM) {
	  _endOfVol = true;
	}
        _prevVolNum = beam->volNum;
      }
      _volNum = beam->volNum;
      
      // decreasing elevation condition

      
      if (beam->elev >= 0 && beam->elev != _prevEl) {
        if (_prevEl != -99 && beam->elev < _prevEl &&
            abs(beam->elev - _prevEl) >= _params.min_elevation_decrease &&
	    _params.end_of_vol_decision == Params::DECREASE_IN_ELEV) {
	  if (_params.debug) {
	    cerr << "DECREASE_IN_ELEV >= " << _params.min_elevation_decrease << " detected. Setting _endOfVol = true.\n";
          }
	  _endOfVol = true;
	}
        _prevEl = beam->elev;
      }

      if (_endOfVolAutomatic) {
	if (_antenna->addBeam(beam)) {
	  _endOfVol = true;
	}
      }
      
    } // if (_preFilter ...
    
  } // if (contents ...
  
  // is this an end of vol?
  
  if ((int) _beamsStored.size() > _params.max_beams_in_vol) {
    _endOfVol = true;
  }

  if (!_endOfVolAutomatic &&
      _params.end_of_vol_decision != Params::CHANGE_IN_VOL_NUM &&
      (contents & DsRadarMsg::RADAR_FLAGS)) {

    const DsRadarFlags &flags = radarMsg.getRadarFlags();

    if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG &&
        flags.endOfVolume) {

      _endOfVol = true;
      
    } else if (_params.end_of_vol_decision == Params::LAST_TILT_IN_VOL &&
               flags.endOfTilt &&
               flags.tiltNum == _params.last_tilt_in_vol) {
      
      _endOfVol = true;

    } else if (flags.newScanType) {

      _endOfVol = true;

    }

  }
  
  if (_params.debug && _endOfVol) {
      cerr << "=========== End of volume flag ===============" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// process the volume

void Dsr2Vol::_processVol()

{
  
  PMU_auto_register("Processing volume");
  
  if (_params.debug) {
    cerr << "**** Start Dsr2Vol::_processVol() ****" << endl;
  }

  // load the current scan mode - PPI or RHI

  _loadCurrentScanMode();

  // load predominant geometry

  _beamGeomMgr->loadPredominantGeom();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _beamGeomMgr->printPredom(cerr);
  }

  // load predominant radar params

  int paramsIndex = _beamGeomMgr->findPredomRadarParams(_radarParamsVec);

  DsRadarParams predomRadarParams;
  if (paramsIndex >= 0) {
    predomRadarParams = *_radarParamsVec[paramsIndex];
  } else {
    cerr << "ERROR - _processVol" << endl;
    cerr << "  Cannot find predominant radar params" << endl;
    cerr << "  Cannot proceed" << endl;
    return;
  }
  if (_params.override_radar_location) {
    predomRadarParams.latitude = _params.radar_location.latitude;
    predomRadarParams.longitude = _params.radar_location.longitude;
    predomRadarParams.altitude = _params.radar_location.altitude;
  }
  if (_params.override_beam_width) {
    predomRadarParams.horizBeamWidth = _params.beam_width;
    predomRadarParams.vertBeamWidth = _params.beam_width;
  }
  if (_params.override_nyquist) {
    predomRadarParams.unambigVelocity = _params.nyquist_velocity;
  }
  
  // filter beams based on predominant geometry
  
  _filterOnGeom();

  // censor gates using data thresholds
  
  _censorGateData();

  // process the volume according to the scan mode
  // if _params.process_rhi_as_ppi is TRUE, the PPI method will be 
  // called even in RHI mode

  if (_scanMode == SCAN_MODE_RHI) {
    if (_params.output_rhi_files) {
      _processRhi(predomRadarParams);
    }
    if (_params.output_rhi_cart_files) {
      _ppiMgr->processPpi(_volNum, _scanMode, _scanInfoFromHeaders,
                          predomRadarParams,
                          _calib, _statusXml,
                          _startTime, _endTime,
                          _beamWidth, _nyquist,
                          _radarLat, _radarLon, _radarAlt);
    }
  } else {
    _ppiMgr->processPpi(_volNum, _scanMode, _scanInfoFromHeaders,
                        predomRadarParams,
                        _calib, _statusXml,
                        _startTime, _endTime,
                        _beamWidth, _nyquist,
                        _radarLat, _radarLon, _radarAlt);
  }

  if (_params.debug) {
    cerr << "**** End Dsr2Vol::_processVol() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// process in rhi mode

void Dsr2Vol::_processRhi(const DsRadarParams &predomRadarParams)

{
  
  PMU_auto_register("RHI mode");
  
  if (_params.debug) {
    cerr << "**** Start Dsr2Vol::_processRhi() ****" << endl;
    cerr << "     Processing RHI volume" << endl;
  }

  for (int ii = 0; ii < _params.rhi_files_n; ii++) {

    // create RHI object
    
    RhiTransform rhi(_params,
		     _params._rhi_files[ii].mdv_url,
                     _fields,
		     _params._rhi_files[ii].oversampling_ratio,
		     _params._rhi_files[ii].interp_in_elevation,
		     _beamsStored,
		     _scanInfoFromHeaders,
                     _beamGeomMgr->getPredomMaxNGates(),
                     _beamGeomMgr->getPredomStartRange(),
                     _beamGeomMgr->getPredomGateSpacing(),
                     _beamGeomMgr->getPredomAngularRes(),
		     _beamWidth, _radarLat, _radarLon, _radarAlt);
    
    // load RHI data
    
    if (rhi.load()) {
      cerr << "  WARNING - Dsr2Vol::_processRhi()" << endl;
      cerr << "    Loading RHI failed" << endl;
      return;
    }
    
    // write RHI volume
    
    rhi.writeVol(predomRadarParams, _calib, _statusXml,
                 _volNum, _startTime, _endTime);

  }
  
  if (_params.debug) {
    cerr << "**** End Dsr2Vol::_processRhi() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// load radar params

void Dsr2Vol::_loadRadarParams(const DsRadarMsg &radarMsg)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> got RADAR_PARAMS" << endl;
  }

  DsRadarParams *rparams = new DsRadarParams(radarMsg.getRadarParams());
  _radarParamsVec.push_back(rparams);
  
  if (_params.override_radar_location) {
    _radarLat = _params.radar_location.latitude;
    _radarLon = _params.radar_location.longitude;
    _radarAlt = _params.radar_location.altitude;
  } else {
    _radarLat = rparams->latitude;
    _radarLon = rparams->longitude;
    _radarAlt = rparams->altitude;
  }
  
  // check to make sure altitude is in km, not meters
  
  if (_radarAlt > 8.0 && _params.debug){
    cerr << "WARNING : Sensor altitude is " << _radarAlt
         << " Km." << endl;
    cerr << "  Are the right units being used for altitude?" << endl;
    cerr << "  Incorrect altitude results in bad cart remapping." << endl;
  }
  
  if (_params.override_beam_width) {
    _beamWidth = _params.beam_width;
  } else {
    _beamWidth = rparams->vertBeamWidth;
  }
  if (_beamWidth < 0.5) {
    _beamWidth = 1.0;
  }

  if (_params.override_nyquist) {
    _nyquist = _params.nyquist_velocity;
  } else {
    _nyquist = (rparams->pulseRepFreq * (rparams->wavelength / 100.0)) / 4.0;
  }

  // save latest radar params for use after a reset

  if (_latestRadarParams == NULL) {
    _latestRadarParams = new DsRadarParams();
  }
  *_latestRadarParams = *rparams;

}

////////////////////////////////////////////////////////////////
// load radar calib

void Dsr2Vol::_loadRadarCalib(const DsRadarMsg &radarMsg)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> got RADAR_CALIB" << endl;
  }

  if (_calib == NULL) {
    _calib = new DsRadarCalib();
  }
  *_calib = radarMsg.getRadarCalib();
}

////////////////////////////////////////////////////////////////
// load field params

void Dsr2Vol::_loadFieldParams(const DsRadarMsg &radarMsg)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> got FIELD_PARAMS" << endl;
  }

  for (size_t ii = 0; ii < radarMsg.getFieldParams().size(); ii++) {
    const DsFieldParams *rfld = radarMsg.getFieldParams(ii);
    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
      FieldInfo &field = _fields[ifield];
      if (field.dsrName == rfld->name) {
	field.byteWidth = rfld->byteWidth;
	field.missingDataValue = rfld->missingDataValue;
	field.scale = rfld->scale;
	field.bias = rfld->bias;
	if (field.name.size() == 0) {
	  field.name = rfld->name;
	}
	if (field.units.size() == 0) {
	  field.units = rfld->units;
	}
	field.isLoaded = true;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "=========>>   field name: " << field.dsrName << endl;
        }
      } // if (!strcmp ...
    } // ifield
  } // ii
  
}

////////////////////////////////////////////////////////////////////
// Pre-filter data based on geometry - gate spacing and/or start gate
//

bool Dsr2Vol::_preFilter(const DsRadarMsg &radarMsg)

{

  const DsRadarParams& rparams = radarMsg.getRadarParams();

  if (_params.filter_gate_spacing) {
    double diff = fabs(_params.keep_gate_spacing - rparams.gateSpacing);
    if (diff > _smallVal) {
      return false;
    }
  }
  
  if (_params.filter_start_range) {
    double diff = fabs(_params.keep_start_range - rparams.startRange);
    if (diff > _smallVal) {
      return false;
    }
  }

  if (_params.filter_prf) {
    if (rparams.pulseRepFreq < _params.min_prf ||
	rparams.pulseRepFreq > _params.max_prf) {
      if (_prevPrfGood &&
	  _endOfVolAutomatic &&
	  _params.set_end_of_vol_on_prf_change) {
	// set end of vol flag
	_prevPrfGood = false;
	_endOfVol = true;
	if (_params.debug) {
	  cerr << "  PRF went out of range: " << rparams.pulseRepFreq << endl;
	}
      }
      return false;
    }
  }
  _prevPrfGood = true;

  const DsRadarBeam &beam = radarMsg.getRadarBeam();

  if (_params.filter_elev) {
    if (beam.elevation < _params.min_elev ||
        beam.elevation > _params.max_elev) {
      return false;
    }
  }

  // check for antenna transitions
  
  if (_params.filter_antenna_transitions) {
    if (beam.antennaTransition) {
      return false;
    }
  }

  // check for movement if required
  
  if (_params.check_antenna_moving) {
    const DsBeamHdr_t *beam_hdr = beam.getBeamHdr();
    if (!_isAntennaMoving(beam_hdr)) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "WARNING - antenna not moving, will ignore beam" << endl;
	cerr << "  el, az: "
	     << beam_hdr->elevation << ", "
	     << beam_hdr->azimuth << endl;
      }
      return false;
    }
  }

  return true;

}

////////////////////////////////////////////////////////////////
// decide whether to select or reject beams based on geometry
// also set start and end times

void Dsr2Vol::_filterOnGeom()

{
  
  _startTime = 0;
  _endTime = 0;

  // check beam geometry

  int nBeamsUsed = 0;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (_beamGeomMgr->matchesPredom(beam->startRange, beam->gateSpacing)) {
      nBeamsUsed++;
      if (_startTime == 0) {
	_startTime = beam->time;
      }
      _endTime = beam->time;
    } else {
      beam->accept = false;
    }
  }

  if (_params.debug) {
    cerr << "  nBeamsUsed: " << nBeamsUsed << endl;
  }

}

////////////////////////////////////////////////////////////////
// censor gates using data thresholds

void Dsr2Vol::_censorGateData()

{

  // any censoring active?

  if (!_params.filter_output_using_thresholds && !_params.check_sn) {
    return;
  }

  // initialize

  int nGates = _beamGeomMgr->getPredomMaxNGates();
  int flagCheck = 0;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    for (int igate = 0; igate < beam->nGates; igate++) {
      beam->censorFlag[igate] = 0;
    }
  }

  // censor using thresholds?

  if (_params.filter_output_using_thresholds) {
    if (_thresholdFieldNum >= 0) {
      _filterUsingThresholds();
      flagCheck++;
    } else {
      cerr << "WARNING - Dsr2Vol" << endl;
      cerr << "  Thresholding using specified field was requested." << endl;
      cerr << "  However, field could not be found: "
           << _params.threshold_field_name << endl;
      cerr << "  Thresholding will not be performed." << endl;
    }
  }

  // censor using SNR

  if (_params.check_sn) {
    if (_snrFieldNum >= 0) {
      _filterUsingSnr();
      flagCheck++;
    } else if (_dbzFieldNum >= 0) {
      _filterUsingDbz();
      flagCheck++;
    } else {
      cerr << "WARNING - Dsr2Vol" << endl;
      cerr << "  SNR thresholding was requested." << endl;
      cerr << "  However, no field is marked as is_dbz "
           << "and no SNR field is specified." << endl;
      cerr << "  Therefore, SNR thresholding will not be performed." << endl;
    }
  }

  // censor gate data

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }

    // check that uncensored runs meet the minimum length
    // those which do not are censored
    
    if (_params.filtering_min_valid_run > 1) {
      int runLength = 0;
      bool doCheck = false;
      for (int igate = 0; igate < beam->nGates; igate++) {
        if (beam->censorFlag[igate] < flagCheck) {
          doCheck = false;
          runLength++;
        } else {
          doCheck = true;
        }
        // last gate?
        if (igate == nGates - 1) doCheck = true;
        // check run length
        if (doCheck) {
          if (runLength < _params.filtering_min_valid_run) {
            // clear the run which is too short
            for (int jgate = igate - runLength; jgate < igate; jgate++) {
              beam->censorFlag[jgate] = flagCheck;
            } // jgate
          }
          runLength = 0;
        } // if (doCheck ...
      } // igate
    }
    
    for (int igate = 0; igate < beam->nGates; igate++) {
      if (beam->censorFlag[igate] >= flagCheck) {
        for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
          beam->setMissing(ifield, igate);
        }
      }
    } // igate

  } // ibeam
  
}

////////////////////////////////////////////////////////////////
// filter noise using thresholds
//

void Dsr2Vol::_filterUsingThresholds()

{

  double minThreshold = _params.threshold_min_value;
  double maxThreshold = _params.threshold_max_value;
  
  if (_params.debug) {
    cerr << "  Thresholding on field: " << _params.threshold_field_name << endl;
    cerr << "  Min value: " << _params.threshold_min_value << endl;
    cerr << "  Max value: " << _params.threshold_max_value << endl;
  }

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }
    for (int igate = 0; igate < beam->nGates; igate++) {
      double val = beam->getValue(_thresholdFieldNum, igate);
      if ((val == Beam::missFl32) ||
          (val < minThreshold || val > maxThreshold)) {
        beam->censorFlag[igate] += 1;
      }
    } // igate

  } // ibeam

}

////////////////////////////////////////////////////////////////
// filter noise using the SNR field
//

void Dsr2Vol::_filterUsingSnr()

{
  
  // Set dbz test level according to the signal/noise threshold.

  double snrThreshold = _params.sn_threshold;

  // clear the gates with reflectivity values below the signal/noise

  if (_params.debug) {
    cerr << "INFO - Dsr2Vol::_filterUsingSnr()" << endl;
    cerr << "  Thresholding on SNR of: " << _params.sn_threshold << endl;
  }

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }
    for (int igate = 0; igate < beam->nGates; igate++) {
      double val = beam->getValue(_snrFieldNum, igate);
      if (val == Beam::missFl32 || val < snrThreshold) {
        beam->censorFlag[igate] += 1;
      }
    } // igate

  } // ibeam

}

////////////////////////////////////////////////////////////////
// filter noise using the DBZ field
//

void Dsr2Vol::_filterUsingDbz()

{

  // Set dbz test level according to the signal/noise threshold.

  double snrThreshold = _params.sn_threshold;
  int nGates = _beamGeomMgr->getPredomMaxNGates();
  double startRange = _beamGeomMgr->getPredomStartRange();
  double gateSpacing = _beamGeomMgr->getPredomGateSpacing();

  TaArray<fl32> dbzNoiseThreshold_;
  fl32 *dbzNoiseThreshold = dbzNoiseThreshold_.alloc(nGates);
  
  double range = startRange;
  for (int igate = 0; igate < nGates; igate++, range += gateSpacing) {
    dbzNoiseThreshold[igate] =
      (snrThreshold + _params.noise_dbz_at_100km +
       20.0 * (log10(range) - log10(100.0)));
  } // igate

  // clear the gates with reflectivity values below the signal/noise

  if (_params.debug) {
    cerr << "INFO - Dsr2Vol::_filterUsingDbz()" << endl;
    cerr << "  No valid SNR field available, using DBZ instead" << endl;
    cerr << "  Thresholding on SNR of: " << _params.sn_threshold << endl;
  }

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }
    for (int igate = 0; igate < beam->nGates; igate++) {
      double val = beam->getValue(_dbzFieldNum, igate);
      if (val == Beam::missFl32 || val < dbzNoiseThreshold[igate]) {
        beam->censorFlag[igate] += 1;
      }
    } // igate

  } // ibeam

}

////////////////////////////////////////////////////////////////
// save the latest beam info

void Dsr2Vol::_saveLatestBeamInfo(const Beam *beam)

{

  _latestBeamTime = beam->time;
  _latestBeamEl = beam->elev;
  _latestBeamAz = beam->az;

  _beamGeomMgr->saveGeom(beam);

}

////////////////////////////////////////////////////////////////
// is antenna moving

bool Dsr2Vol::_isAntennaMoving(const DsBeamHdr_t *beamHdr)

{

  double elev = beamHdr->elevation;
  double az = beamHdr->azimuth;
  double deltaElev = fabs(elev - _prevElevMoving);
  double deltaAz = fabs(az - _prevAzMoving);
  bool isMoving;
  if (deltaElev > _params.min_angle_change ||
      deltaAz > _params.min_angle_change) {
    isMoving = true;
  } else {
    isMoving = false;
  }

  _prevElevMoving = elev;
  _prevAzMoving = az;

  return isMoving;

}

////////////////////////////////////////////////////////////////
// load the current scan mode
//
// returns 0 on success, -1 on failure

int Dsr2Vol::_loadCurrentScanMode()
  
{
  
  if (_beamsStored.size() == 0) {
    _scanMode = SCAN_MODE_UNKNOWN;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "loadCurrentScanMode()" << endl;
  }

  if (_params.use_input_scan_mode) {
    
    if (_findScanModeFromHeaders() == 0) {
      _scanInfoFromHeaders = true;
      _endOfVolAutomatic =
	(_params.end_of_vol_decision == Params::AUTOMATIC);
    } else {
      _findScanModeFromAntenna();
      _scanInfoFromHeaders = false;
      _endOfVolAutomatic = true;
    }

  } else {

    _findScanModeFromAntenna();
    _scanInfoFromHeaders = false;
    _endOfVolAutomatic =
      (_params.end_of_vol_decision == Params::AUTOMATIC);

  }

  // special case - vertical scan

  if (_isVertScan()) {
    _scanMode = SCAN_MODE_VERT;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_scanMode == SCAN_MODE_RHI) {
      cerr << "  currentScanMode: RHI" << endl;
    } else if (_scanMode == SCAN_MODE_PPI) {
      cerr << "  currentScanMode: PPI" << endl;
    } else if (_scanMode == SCAN_MODE_SECTOR) {
      cerr << "  currentScanMode: SECTOR" << endl;
    } else if (_scanMode == SCAN_MODE_SURVEILLANCE) {
      cerr << "  currentScanMode: SURVEILLANCE" << endl;
    } else if (_scanMode == SCAN_MODE_VERT) {
      cerr << "  currentScanMode: VERT" << endl;
    } else {
      cerr << "  currentScanMode: UNKNOWN" << endl;
    }      
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// find the current scan mode from the data headers
//
// returns 0 on success, -1 on failure

int Dsr2Vol::_findScanModeFromHeaders()
  
{
  
  // find predominant scan mode
  
  double countSur = 0;
  double countSec = 0;
  double countRhi = 0;
  double countTotal = 0;

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->scanMode == DS_RADAR_SURVEILLANCE_MODE ||
	beam->scanMode == DS_RADAR_VERTICAL_POINTING_MODE) {
      countSur++;
    } else if (beam->scanMode == DS_RADAR_SECTOR_MODE) {
      countSec++;
    } else if (beam->scanMode == DS_RADAR_RHI_MODE) {
      countRhi++;
    }
    countTotal++;
  }

  double fractionGood = (countSur + countSec + countRhi) / countTotal;
  if (fractionGood < 0.8) {
    return -1;
  }
  
  if (countRhi > (countSur + countSec)) {
    _scanMode = SCAN_MODE_RHI;
  } else if (countSec > countSur) {
    _scanMode = SCAN_MODE_SECTOR;
  } else {
    _scanMode = SCAN_MODE_SURVEILLANCE;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  countSur: " << countSur << endl;
    cerr << "  countSec: " << countSec << endl;
    cerr << "  countRhi: " << countRhi << endl;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// find the current scan mode from antenna angles

void Dsr2Vol::_findScanModeFromAntenna()
  
{
  
  // find which slews more - azimuth or elevation
  
  double azTravel = 0.0;
  double elTravel = 0.0;
  
  Beam *beam = _beamsStored[0];
  double prevEl = beam->elev;
  double prevAz = beam->az;
  
  for (size_t ibeam = 1; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    double el = beam->elev;
    double az = beam->az;
    double elDiff = el - prevEl;
    if (elDiff > 180) {
      elDiff -= 360.0;
    } else if (elDiff < -180) {
      elDiff += 360.0;
    }
    double azDiff = az - prevAz;
    if (azDiff > 180) {
      azDiff -= 360.0;
    } else if (azDiff < -180) {
      azDiff += 360.0;
    }
    elTravel += fabs(elDiff);
    azTravel += fabs(azDiff);
    prevEl = el;
    prevAz = az;
  }
  
  // at this point we can only determine whether it is an
  // RHI or not. In PpiMgr, we can determine whether it is a
  // sector or surveillance
  
  if (elTravel > azTravel) {
    _scanMode = SCAN_MODE_RHI;
  } else {
    _scanMode = SCAN_MODE_PPI; // generic non-rhi mode
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  elTravel: " << elTravel << endl;
    cerr << "  azTravel: " << azTravel << endl;
  }

}

////////////////////////////////////////////////////////////////
// check if this is a vertically-pointing scan

bool Dsr2Vol::_isVertScan()
  
{

  // check if this is even an option

  if (!_params.separate_vert_files) {
    return false;
  }

  // count number of beams above min elevation
  
  double countAbove = 0.0;
  double countTotal = 0.0;

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    double el = beam->elev;
    if (el > _params.min_elevation_for_vert_files) {
      countAbove++;
    }
    countTotal++;
  }

  double fractionAbove = countAbove / countTotal;

  if (fractionAbove > _params.min_vert_fraction_for_vert_files) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////////////
// Print elevation and fields

void Dsr2Vol::_printFields()

{
  
  cerr << "Field array" << endl;
  cerr << "===========" << endl;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    cerr << "  Field #, name: " << ii << " " << _fields[ii].name << endl;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "    byteWidth: " << _fields[ii].byteWidth << endl;
      cerr << "    missingDataVal: " << _fields[ii].missingDataValue << endl;
      cerr << "    scale: " << _fields[ii].scale << endl;
      cerr << "    bias: " << _fields[ii].bias << endl;
      cerr << "    units: " << _fields[ii].units << endl;
      cerr << "    isDbz: " << _fields[ii].isDbz << endl;
      cerr << "    isLoaded: " << _fields[ii].isLoaded << endl;
    }
  } // i
  cerr << endl;

}
  
/////////////////////
// prepare for volume

void Dsr2Vol::_prepareForVol()

{
  _prepareBeamsForVol();
  _clearRadarParamsVec();
  if (_beamGeomMgr) {
    _beamGeomMgr->clear();
  }
  _clearFieldFlags();
}

/////////////////////////////
// prepare beams for next vol

void Dsr2Vol::_prepareBeamsForVol()
{

  if (_params.nbeams_overlap_per_vol > 0) {
    
    // prepare beams read by saving some beams
    // for start of next vol

    _saveBeamsOverlap();
    
  } else {
    
    // clear read beams
    
    _clearBeamsStored();
    
  }

}

/////////////////////
// save overlap beams

void Dsr2Vol::_saveBeamsOverlap()
{

  // save last beams to have been read

  size_t nSave = (size_t) _params.nbeams_overlap_per_vol;
  if (nSave > _beamsStored.size()) {
    nSave = _beamsStored.size();
  }

  vector<Beam *> buf;
  for (size_t ii = _beamsStored.size() - nSave;
       ii < _beamsStored.size(); ii++) {
    buf.push_back(_beamsStored[ii]);
  }

  // clear read beams
  
  for (size_t ii = 0; ii < _beamsStored.size() - nSave; ii++) {
    delete _beamsStored[ii];
  }
  _beamsStored.clear();
  
  // copy tmp beams into start of buffer

  for (size_t ii = 0; ii < buf.size(); ii++) {
    _beamsStored.push_back(buf[ii]);
  }
  
}

/////////////////////////////////////////////////////////////////
// reset and clear methods

void Dsr2Vol::_reset(const DsRadarMsg &radarMsg)

{
  _prepareForVol();
  if (_latestRadarParams != NULL) {
    DsRadarParams *rparams = new DsRadarParams(*_latestRadarParams);
    _radarParamsVec.push_back(rparams);
  }
}

void Dsr2Vol::_clearAll()

{
  _clearBeamsStored();
  _clearRadarParamsVec();
  if (_beamGeomMgr) {
    _beamGeomMgr->clear();
  }
  _clearFieldFlags();
}

void Dsr2Vol::_clearBeamsStored()
{

  for (size_t ii = 0; ii < _beamsStored.size(); ii++) {
    delete _beamsStored[ii];
  }
  _beamsStored.clear();

}
  
void Dsr2Vol::_clearRadarParamsVec()
{
  for (size_t ii = 0; ii < _radarParamsVec.size(); ii++) {
    delete _radarParamsVec[ii];
  }
  _radarParamsVec.clear();
}

void Dsr2Vol::_clearFieldFlags()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii].isLoaded = false;
  }
}

