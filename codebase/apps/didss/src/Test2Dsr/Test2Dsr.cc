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
// Test2Dsr.cc
//
// Test2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////
//
// Test2Dsr produces a forecast image based on (a) motion data
// provided in the form of (u,v) components on a grid and
// (b) image data on a grid.
//
///////////////////////////////////////////////////////////////

#include "Test2Dsr.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
#include <Fmq/DsRadarQueue.hh>
using namespace std;

// Constructor

Test2Dsr::Test2Dsr(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = "Test2Dsr";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			    &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
  }

  if (!OK) {
    return;
  }

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // determine the number of fields

  _nFields = 1;
  if (_params.output_vel_field) {
    _nFields += 1;
  }
  if (_params.output_geom_fields) {
    _nFields += 4;
  }

  return;

}

// destructor

Test2Dsr::~Test2Dsr()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Test2Dsr::Run ()
{

  PMU_auto_register("Test2Dsr::Run");

  // set up the sampling object

  Sampling sampling(_progName, _params, _nFields);
  if (!sampling.OK) {
    fprintf(stderr, "ERROR - %s:Run\n", _progName.c_str());
    fprintf(stderr, "Cannot set up sampling object\n");
    return (-1);
  }
  
  // create the output message and queue

  DsRadarMsg radarMsg;
  DsRadarQueue radarQueue;

  DsFmq::openMode open_mode = DsFmq::READ_WRITE;
  //   if (_params.write_blocking) {
  //     open_mode = DsFmq::BLOCKING_READ_ONLY;
  //   }
  
  while (radarQueue.init(_params.output_fmq_path,
			 _progName.c_str(),
			 _params.debug,           
			 open_mode, DsFmq::END, 
			 _params.output_fmq_compress,
			 _params.output_fmq_nslots,
			 _params.output_fmq_size )) {
    fprintf(stderr, "ERROR - %s:radarQueue::init failed.\n",
	    _progName.c_str());
    sleep (1);
  }

  // load up radar and field params

  _loadParams(radarMsg, sampling);

  // initialize start flags

  int start_of_tilt = TRUE;
  int start_of_volume = TRUE;
  int end_of_tilt;
  int end_of_volume;
  int new_scan_type;
  time_t lastParamsUpdate = 0;

  // loop forever, getting beams
  
  int forever = TRUE;
  while (forever) {

    PMU_auto_register("Creating next beam");

    // load next beam

    DsBeamHdr_t beamHdr;
    int nData;
    ui08 *beamData =
      sampling.loadBeam(&beamHdr, &nData, &new_scan_type,
			&end_of_tilt, &end_of_volume);

    // set beam params

    DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
    int msgContent = (int)DsRadarMsg::RADAR_BEAM;

    radarBeam.dataTime = beamHdr.time;

    radarBeam.volumeNum = beamHdr.vol_num;
    radarBeam.tiltNum = beamHdr.tilt_num;

    radarBeam.azimuth = beamHdr.azimuth;
    radarBeam.elevation = beamHdr.elevation;
    radarBeam.targetElev = beamHdr.target_elev;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
	      "time, az, el, volnum, tiltnum: "
	      "%12s, %5.1f, %5.1f, %5d, %5d\n",
	      utimstr(radarBeam.dataTime),
	      radarBeam.azimuth,
	      radarBeam.elevation,
	      radarBeam.volumeNum,
	      radarBeam.tiltNum);
    }

    //
    // Include radar and field parameters every 5 secs and at
    // start of each tilt
    //

    time_t timeSinceUpdate = radarBeam.dataTime - lastParamsUpdate;
    if (start_of_tilt || timeSinceUpdate > 5) {
      msgContent |= (int)DsRadarMsg::RADAR_PARAMS;
      msgContent |= (int)DsRadarMsg::FIELD_PARAMS;
      lastParamsUpdate = radarBeam.dataTime;
    }

    // load the beam data into the radarBeam class

    radarBeam.loadData(beamData, nData);

    // send flag messages

    if (new_scan_type) {
      radarQueue.putNewScanType(_params.scan_strategy_id);
    }
    if (start_of_tilt) {
      radarQueue.putStartOfTilt(beamHdr.tilt_num, beamHdr.time);
    }
    if (start_of_volume) {
      radarQueue.putStartOfVolume(beamHdr.vol_num, beamHdr.time);
    }

    PMU_auto_register("Sending ....");

    // serialize the class into a buffer and write to the message queue

    if (radarQueue.putDsBeam(radarMsg, msgContent)) {
      fprintf(stderr, "ERROR - Test2Dsr::Run:putDsBeam\n");
      fprintf(stderr, "Cannot put beam to queue.\n");
    }

    // send flag messages

    if (end_of_tilt) {
      radarQueue.putEndOfTilt(beamHdr.tilt_num, beamHdr.time);
    }
    if (end_of_volume) {
      radarQueue.putEndOfVolume(beamHdr.vol_num, beamHdr.time);
    }

    // set start flags for next beam

    if (end_of_volume) {
      start_of_volume = TRUE;
    } else {
      start_of_volume = FALSE;
    }
    if (end_of_tilt) {
      start_of_tilt = TRUE;
    } else {
      start_of_tilt = FALSE;
    }

    PMU_auto_register("Zzzz ....");

    if (end_of_volume) {
      sleep(_params.vol_wait_secs);
    }
    umsleep(_params.beam_wait_msecs);

  } // while

  return (0);

}

/////////////////
// _loadParams()
//
// Load up params
//

void Test2Dsr::_loadParams(DsRadarMsg &radarMsg,
			   Sampling &sampling)
  
{

  //
  // Initialize constant radar parameters
  //

  DsRadarParams &radarParams    = radarMsg.getRadarParams();

  radarParams.radarId           = _params.radar_params.radar_id;
  radarParams.radarType         = DS_RADAR_GROUND_TYPE;
  radarParams.numFields         = _nFields;
  radarParams.numGates          = _params.radar_params.num_gates;
  radarParams.samplesPerBeam    = _params.radar_params.samples_per_beam;
  radarParams.scanType          = _params.scan_strategy_id;
  radarParams.scanMode          = DS_RADAR_SURVEILLANCE_MODE;
  radarParams.polarization      = DS_POLARIZATION_HORIZ_TYPE;

  radarParams.radarConstant     = _params.radar_params.radar_constant;
  radarParams.altitude          = _params.radar_params.altitude;
  radarParams.latitude          = _params.radar_params.latitude;
  radarParams.longitude         = _params.radar_params.longitude;

  radarParams.gateSpacing       = _params.radar_params.gate_spacing;
  radarParams.startRange        = _params.radar_params.start_range;
  radarParams.horizBeamWidth    = _params.radar_params.beam_width;
  radarParams.vertBeamWidth     = _params.radar_params.beam_width;

  radarParams.pulseWidth        = _params.radar_params.pulse_width;
  radarParams.pulseRepFreq      = _params.radar_params.prf;
  radarParams.wavelength        = _params.radar_params.wavelength;

  radarParams.xmitPeakPower     = _params.radar_params.xmit_peak_pwr;
  radarParams.receiverMds       = _params.radar_params.receiver_mds;
  radarParams.receiverGain      = _params.radar_params.receiver_gain;
  radarParams.antennaGain       = _params.radar_params.antenna_gain;
  radarParams.systemGain        = _params.radar_params.system_gain;

  radarParams.unambigVelocity   = _params.radar_params.unambig_velocity;
  radarParams.unambigRange      = _params.radar_params.unambig_range;

  char textStr[1024];
  sprintf(textStr, "Test radar data - generated by Test2Dsr");
  radarParams.radarName = textStr;
  radarParams.scanTypeName = _params.scan_strategy_name;

  //
  // Initialize output field parameters
  //

  vector< DsFieldParams* > &fieldParams = radarMsg.getFieldParams();
  DsFieldParams*  fparams;

  fparams = new DsFieldParams("DBZ", "dBZ",
			      sampling.zScale(), sampling.zBias());
  fieldParams.push_back(fparams);

  if (_nFields > 1) {
    fparams = new DsFieldParams("VEL", "m/s",
				sampling.vScale(), sampling.vBias());
    fieldParams.push_back(fparams);
  }

  if (_nFields > 2) {
    fparams = new DsFieldParams("HEIGHT", "km",
				sampling.htScale(), sampling.htBias());
    fieldParams.push_back(fparams);
    fparams = new DsFieldParams("ELE", "deg", 1.0, 1.0);
    fieldParams.push_back(fparams);
    fparams = new DsFieldParams("AZ", "deg", 1.0, 1.0);
    fieldParams.push_back(fparams);
    fparams = new DsFieldParams("GATE", "count", 1.0, 1.0);
    fieldParams.push_back(fparams);
  }

}
