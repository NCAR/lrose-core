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
// Dsr2Rapic.cc
//
// Dsr2Rapic object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////////////
//
// Dsr2Rapic reads an input radar FMQ, puts the data into a grid, and
// saves it out as an MDV file.
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <rapmath/math_macros.h>

#include "Dsr2Rapic.hh"
#include "CartTransform.hh"
#include "PpiTransform.hh"
#include "PolarTransform.hh"
#include "RhiTransform.hh"
#include "rdrutils.h"

#include "rapicThread.h"
#include "rdrscan.h"
LevelTable *GetStdBinarySpectWLevelTbl();

rapicThread *_rapicThread = NULL;
bool _appClosing = false;

using namespace std;

const double Dsr2Rapic::_smallRange = 0.0001;

const float Dsr2Rapic_Version = 1.0;

char *scan_mode_strings[] =
  { "UNKNOWN",
    "CALIBRATION",
    "SECTOR",
    "COPLANE",
    "RHI",
    "VERTICAL_POINTING",
    "TARGET",
    "MANUAL",
    "IDLE",
    "SURVEILLANCE",
    "VERTICAL_SWEEP" };
    
char* scanModeString(e_scanmode scanmode)
{
  if (scanmode < SCAN_UNKNOWN_MODE)
    scanmode = SCAN_UNKNOWN_MODE;
  if (scanmode > SCAN_VERTICAL_SWEEP_MODE)
    scanmode = SCAN_VERTICAL_SWEEP_MODE;
  return scan_mode_strings[int(scanmode) + 1];
} 

// Constructor

Dsr2Rapic::Dsr2Rapic(int argc, char **argv)

{

  isOK = true;
  _nBeamsRead = 0;

  _elevHist = NULL;
  _nElevHist = 0;
  _elevHistOffset = 0;
  _elevHistIntv = 0.1;
  _elevHistSearchWidth = 0;
  _nElev = 0;

  _deltaAz = 0;
  _nazPer45 = 0;
  _naz = 0;
  _nxy = 0;
  _nxyHalf = 0;

  _predomStartRange = 0;
  _predomGateSpacing = 0;
  _predomMaxNGates = 0;

  _startTime = 0;
  _endTime = 0;

  _radarInfoSet = false;
  _prevPrfGood = true;
  _radarLat = 0;
  _radarLon = 0;
  _radarAlt = 0;
  _beamWidth = 0;
  _countryCode = 36;
  _radarId = 98;
  _radarName = "CP2Bris";
  _endOfVol = false;
  _antenna = NULL;
  _antTransition = false;
  _tiltNum = -1;
  _antTransStartTime = 0.0;
  _lastVolTiltCount = 15;
  _lastTiltStartBeam = _lastTiltEndBeam = 0;
  _lastScanType = -1;

  _rapicScan = NULL;
  _lastRapicScanUpdateTime = 0;
  _rapicScanTimeout = 150;

  // set programe name

  _progName = "Dsr2Rapic";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  _rapicThread = new rapicThread(1.0,
				 "rpcomm.ini.dsr2rapic",
				 "rpdb.ini.dsr2rapic");
  if (_rapicThread)
    _rapicThread->startThread();

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }
  _nyquist = _params.nyquist_velocity;

  // create field refs vector

  _filterFieldNum = -1;
  for (int i = 0; i < _params.output_fields_n; i++) {

    Params::output_field_t &outField = _params._output_fields[i];

    // try to get filterField from output fields
    if (_params.check_filter)
      {
	if (strcmp(outField.dsr_name,
		   _params.filter_field_name) == 0) {
	  
	  if (_params.debug) {
	    cerr << "filterNoise using Field #" << i 
		 << " " << outField.dsr_name
		 << " for filter_field_name=" << _params.filter_field_name
		 << endl;
	  }
	  _filterFieldNum = i;
	}
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
		    encoding, compression);

    _fields.push_back(finfo);
    
  }

  // compute suitable delta az - there must be an integral number
  // of beams per 45 deg
  
  _nazPer45 = (int) (45.0 / _params.delta_az);
  _deltaAz = 45.0 / _nazPer45;
  _naz = _nazPer45 * 8;

  if (_deltaAz != _params.delta_az) {
    cerr << "WARNING - adjusting delta_az so that there are an" << endl
	 << "          number of beams per 45 degree arc." << endl;
    cerr << "  Params delta_az: " <<  _params.delta_az << endl;
    cerr << "  Using _deltaAz: " << _deltaAz << endl;
    cerr << endl;
  }
  
  // elevation histogram

  _elevHistIntv = _params. elev_hist_resolution;
  _nElevHist = (int) ((_params. elev_hist_end - _params. elev_hist_start) /
		      _elevHistIntv + 0.5);
  _elevHistOffset = (int) ((0.0 - _params. elev_hist_start) /
			   _elevHistIntv + 0.5);
  _elevHist = new int[_nElevHist];
  _elevHistSearchWidth = _params. elev_hist_search_width;

  if (_params.debug) {
    cerr << "========== Az / elev params ==========" << endl;
    cerr << "  _naz: " << _naz << endl;
    cerr << "  _nazPer45: " << _nazPer45 << endl;
    cerr << "  _deltaAz: " << _deltaAz << endl;
    cerr << "  _elevHistIntv: " << _elevHistIntv << endl;
    cerr << "  _nElevHist: " << _nElevHist << endl;
    cerr << "  _elevHistOffset: " << _elevHistOffset << endl;
    cerr << "  _elevHistSearchWidth: " << _elevHistSearchWidth << endl;
    cerr << "=================================" << endl;
  }

  // clear all arrays
  
  _clearAll();

  // create antenna object

  _antenna = new Antenna(_progName, _params);
  
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

}

/////////////////////////////////////////////////////////
// destructor

Dsr2Rapic::~Dsr2Rapic()

{

  _clearAll();

  if (_elevHist) {
    delete[] _elevHist;
  }
  if (_antenna != NULL) {
    delete _antenna;
  }

  if (_rapicScan)
    {
      if (_rapicScan->ShouldDelete(this, "~Dsr2Rapic"))
	delete _rapicScan;
      _rapicScan = NULL;
    }

  // unregister process

  PMU_auto_unregister();

  if (_rapicThread)
    delete _rapicThread;
}

//////////////////////////////////////////////////
// Run

int Dsr2Rapic::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    if (_params.debug) {
      cout << "Dsr2Rapic::Run:" << endl;
      cout << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
    }
    _run();
    sleep(2);
  }

  _appClosing = true;

  return 0;

}

//////////////////////////////////////////////////
// _run

int Dsr2Rapic::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Instantiate and initialize the DsRadar queue and message

  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;


  if (_params.debug)
    fprintf(stdout, "Attempting to open radar queue '%s'\n",
	    _params.input_fmq_url);

  if (_params.seek_to_end_of_input) {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Rapic::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  } else {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Rapic::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }
  
  if (_params.debug)
    fprintf(stdout, "Successfully opened radar queue '%s' - "
	    "seek_to_end_of_input = %d\n",
	    _params.input_fmq_url, _params.seek_to_end_of_input);
    
  // read beams from the queue and process them
  
  _nBeamsRead = 0;
  size_t nUsed = 0;
  
  while (true) {
    
    if (_readMsg(radarQueue, radarMsg) == 0) {
      
//       if (_params.debug)
// 	fprintf(stdout, "Mssg read from queue = %d\n", _nBeamsRead);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	if ((_nBeamsRead > 0) &&
	    _beamsStored.size() != nUsed) {
	  nUsed = _beamsStored.size();
          fprintf(stdout,
                  "  Reading queue, n read, n used, latest time, el, az: "
                  "%6d %6d %s %6.1f %6.1f\n",
                  int(_nBeamsRead), int(nUsed),
                  utimstr(_latestBeamTime), _latestBeamEl, _latestBeamAz);
		  fflush(stdout);
	}
      }
      else if (_params.debug) {
	if ((_nBeamsRead > 0) && (_nBeamsRead % 10 == 0) &&
	    _beamsStored.size() != nUsed) {
	  nUsed = _beamsStored.size();
          fprintf(stdout,
                  "  Reading queue, n read, n used, latest time, el, az: "
                  "%6d %6d %s %6.1f %6.1f\n",
                  int(_nBeamsRead), int(nUsed),
                  utimstr(_latestBeamTime), _latestBeamEl, _latestBeamAz);
	  fflush(stdout);
	}
      }
      
      bool _rapicScanTimedOut = _lastRapicScanUpdateTime &&
	((time(0) - _lastRapicScanUpdateTime) >  _rapicScanTimeout);
      if (_rapicScanTimedOut) {
	if (_params.debug)
	  cerr << "********* rapicScan not updating - Forcing End of volume ********* " << endl;
	_processEndOfVol();
	_endOfVol = true;
      }
	
    
      // at the end of a volume, process volume, then reset
      
      if (_endOfVol) {
	if (_params.debug) {
	  cerr << "  End of volume." << endl;
	  cerr << "    nbeams available:" << _beamsStored.size() << endl;
	}
// 	if ((int) _beamsStored.size() < _params.min_beams_in_vol) {
// 	  if (_params.debug) {
// 	    cerr << "  Too few beams in volume" << endl;
// 	    cerr << "  Not processing volume" << endl;
// 	  }
// 	} else {
// 	  _processVol();
// 	}
	_reset(radarMsg);
	_nBeamsRead = 0;
      }
      
    } //  if (_readMsg()
    
  } // while (true)
  
  return 0;

}

////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue, setting the flags about beam_data
// and _endOfVolume appropriately.
//

int Dsr2Rapic::_readMsg(DsRadarQueue &radarQueue,
			DsRadarMsg &radarMsg) 
  
{
  
   
  PMU_auto_register("Reading radar queue");
  _endOfVol = false;
  
  int contents;
  if (radarQueue.getDsMsg(radarMsg, &contents)) {
    return -1;
  }
    
  // is this a start of vol? If so, reset.
  
  if (_params.end_of_vol_decision != Params::AUTOMATIC &&
      (contents & DsRadarMsg::RADAR_FLAGS)) {
    const DsRadarFlags &flags = radarMsg.getRadarFlags();
    if (flags.startOfVolume) {
      _reset(radarMsg);
      if (_params.debug) {
	cerr << "=========== Start of volume ===============" << endl;
      }
    }
  }
  
  if (contents & DsRadarMsg::RADAR_FLAGS) {
    const DsRadarFlags &flags = radarMsg.getRadarFlags();
    if (flags.startOfTilt) {
      if (_params.debug) {
	timeval timenow;
	gettimeofday(&timenow, NULL);
	double timesecs = timenow.tv_sec + (timenow.tv_usec / 1000000.0);
	timesecs -= _antTransStartTime;
	cerr << "===== Start of " 
	     << scanModeString(e_scanmode(_lastScanType)) 
	     << " tilt " << flags.tiltNum+1 << " flag =====" 
	     << "== Transition took " << timesecs << "secs" << endl;
// 	else
// 	  cerr << "===== Start of tilt " << flags.tiltNum << " flag =====" 
// 	       << endl;
      }
      _tiltNum = flags.tiltNum;
      _antTransition = false;
    }
    if (flags.endOfTilt) {
      _lastTiltStartBeam = _startTiltBeamNum;
      _lastTiltEndBeam = _beamsStored.size() - 1;
      if (_params.debug)
	cerr << "===== End of " 
	     << scanModeString(e_scanmode(_lastScanType)) 
	     << " tilt " << flags.tiltNum+1 << " flag ====="
             << " Start/End beam numbers = " << _lastTiltStartBeam << "/" 
             << _lastTiltEndBeam << "(" 
	     << _lastTiltEndBeam - _lastTiltStartBeam + 1 
	     << ")" << endl;
      _antTransition = true;
      _startTiltBeamNum = -1;
      timeval timenow;
      gettimeofday(&timenow, NULL);
      _antTransStartTime = timenow.tv_sec + (timenow.tv_usec / 1000000.0);
      _processTilt(flags.tiltNum);
    }
    if (flags.startOfVolume) {
      if (_params.debug)
	cerr << "===== Start of " 
	     << scanModeString(e_scanmode(_lastScanType)) 
	     << " volume " << flags.volumeNum << " flag =====" << endl;
    }
    if (flags.endOfVolume) {
      if (_params.debug)
	cerr << "=========== End of " 
	     << scanModeString(e_scanmode(_lastScanType)) 
	     << " volume " << flags.volumeNum 
	     << " flag ===============" << endl;
      _endOfVol = true;
      _lastVolTiltCount = _tiltNum + 1;
    }
  }
      
  // set radar parameters if avaliable
  
  if (contents & DsRadarMsg::RADAR_PARAMS) {
    _loadRadarParams(radarMsg);
    if (_params.debug) {
      fprintf(stdout, "Msg contents=RADAR_PARAMS\n");
    }
  }

  // set field parameters if available
  
  if (contents & DsRadarMsg::FIELD_PARAMS) {
    _loadFieldParams(radarMsg);
    if (_params.debug) {
      fprintf(stdout, "Msg contents=FIELD_PARAMS\n");
    }
  }
  
  // If we have radar and field params, and there is good beam data,
  // add to the vector
  
  if (!_antTransition && (contents & DsRadarMsg::RADAR_BEAM) && radarMsg.allParamsSet()) {

  //  if (_params.debug) {
  //    fprintf(stdout, "Msg contents=RADAR_BEAM\n");
  //  }
    _nBeamsRead++;

    // if _filterFieldNum not found yet, try to find in beam msg
    // If found add field to be copied to beam.
    // This field won't be added to output rapic data 
    if (_params.check_filter && (_filterFieldNum < 0)) {
      vector<DsFieldParams *> &fparams = radarMsg.getFieldParams();
      for (size_t ii = 0; ii < fparams.size(); ii++) {
	if (strcmp(_params.filter_field_name, 
		   fparams[ii]->name.c_str()) == 0) {
	  FieldInfo finfo(fparams[ii]->name.c_str(),
			  "undefined",
			  "undefined",
			  Mdvx::ENCODING_INT8, 
			  Mdvx::COMPRESSION_ZLIB);
	  _filterFieldNum = _fields.size();
	  _fields.push_back(finfo);
	  break;
	}
	if (_params.debug && (_filterFieldNum >= 0))
	  cerr << "readMsg - Found matching input field=" 
	       << fparams[_filterFieldNum]->name << " for filter_field_name="
	       << "_params.filter_field_name" << endl
	       << " Field appended to _fields list for saving with beams"
	       << endl;
	  
      }
    }

    // see if we should keep beam based on geometry
    
    if (_preFilter(radarMsg)) {

      Beam *beam = new Beam(radarMsg, _params, _fields, _nyquist);
      
      // save the beam geometry
      
      _saveBeamGeom(beam);
      
      if (_startTiltBeamNum == -1)
	_startTiltBeamNum = _beamsStored.size();

      // save the beam
      
      _beamsStored.push_back(beam);

      
      // antenna check

      if (_params.end_of_vol_decision == Params::AUTOMATIC) {
	if (_antenna->addBeam(beam)) {
	  _endOfVol = true;
          _lastVolTiltCount = _tiltNum + 1;
	}
      }

      
    } // if (_preFilter ...

  } // if (contents ...
  
  // is this an end of vol?
  
  if ((int) _beamsStored.size() > _params.max_beams_in_vol) {
    _endOfVol = true;
    _lastVolTiltCount = _tiltNum + 1;
  }

  if (contents & DsRadarMsg::RADAR_FLAGS) {
    const DsRadarFlags &flags = radarMsg.getRadarFlags();
    
    if (_params.end_of_vol_decision != Params::AUTOMATIC) {
      if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG) {
	if (flags.endOfVolume) {
	  _endOfVol = true;
          _lastVolTiltCount = _tiltNum + 1;
	}
      } else if (flags.endOfTilt && flags.tiltNum == _params.last_tilt_in_vol) {
	_endOfVol = true;
        _lastVolTiltCount = _tiltNum + 1;
      } else if (flags.newScanType) {
	_endOfVol = true;
        _lastVolTiltCount = _tiltNum + 1;
      }
    }
  }

  if (_endOfVol) {
    if (_params.debug)
      cerr << "=========== End of volume - Tilt count="
	   << _lastVolTiltCount 
	   << " ===============" << endl;
    _processEndOfVol();
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// process the tilt

void Dsr2Rapic::_processTilt(int tiltnum)
{
  if (tiltnum < 0)
    {
      if (_params.debug)
	cerr << "Dsr2Rapic::_processTilt error tiltnum=" << tiltnum << endl;
      return;
    }
  if ((_lastTiltStartBeam < 0) || (_lastTiltEndBeam < 0))
    {
      if (_params.debug)
	cerr << "Dsr2Rapic::_processTilt error tiltnum=" << tiltnum 
	     << "_lastTiltStartBeam=" << _lastTiltStartBeam
	     << "_lastTiltEndBeam=" << _lastTiltEndBeam << endl;
      return;
    }
  if (_params.debug) {
    cerr << "**** Start Dsr2Rapic::_processTilt No " 
	 << tiltnum+1 
// 	 << " of " << _lastVolTiltCount
	 << " ****" 
	 << endl;
  }

  // load predominant geometry

  _loadPredominantGeom();

  // filter beams based on predominant geometry
  
  _filterBeams(_lastTiltStartBeam,
	       _lastTiltEndBeam);

  bool rhiflag = e_scanmode(_lastScanType) == SCAN_RHI_MODE;

  _loadElevTable(_lastTiltStartBeam,
		 _lastTiltEndBeam,
		 rhiflag);

  // if first tilt and _rapicScan exists assume first tilt of next vol, call end of vol
  if ((tiltnum == 0) && _rapicScan)
    _processEndOfVol();

  if (!_rapicScan)
    {
      _rapicScan = new rdr_scan(this, "Dsr2Rapic - Root scan");   
      if (_rapicScan)
	_rapicScan->data_source = COMM;
      if (_params.debug)
	cerr << "Dsr2Rapic::_processTilt - Created new _rapicScan scans=" 
	     << _rapicScan->completedScans()
	     << " tilts=" << _rapicScan->completedTilts() << endl;
    }
  
  if (!_rapicScan)
    return;

  rdr_scan *newtiltscan = NULL;
  char endstr[] = "END RADAR IMAGE";

  for (int i = 0; i < _params.output_fields_n; i++) {
    
    Params::output_field_t &outField = _params._output_fields[i];

    if (_rapicScan->thisScanComplete())  // add child scan 
      {
	newtiltscan = new rdr_scan(this, "Dsr2Rapic - Child scan",
				   _rapicScan, true);
	if (_params.debug)
	  cerr << "Adding tilt " << tiltnum 
	       << " rapic " << outField.dsr_name << " scan as child to "
	       << newtiltscan->rootScan()->ScanString() << endl;
      }
    else                    // root scan not complete, load with this tilt
      {
	newtiltscan = _rapicScan;
	if (_params.debug) 
	  cerr << "Adding tilt " << tiltnum
	       << outField.dsr_name << " scan as root to "
	       << newtiltscan->rootScan()->ScanString() << endl;	
      }
    newtiltscan->station = 0;
    newtiltscan->station = (newtiltscan->CountryCodeToID(_countryCode) << 8)
      | _radarId;
    newtiltscan->scan_time_t = _beamsStored[_lastTiltStartBeam]->time;
    newtiltscan->StartTm = _beamsStored[_lastTiltStartBeam]->time;
    newtiltscan->EndTm = _beamsStored[_lastTiltEndBeam]->time;
    UnixTime2DateTime(_beamsStored[_lastTiltStartBeam]->time,
		      newtiltscan->year,newtiltscan->month,newtiltscan->day,
		      newtiltscan->hour,newtiltscan->min,newtiltscan->sec);
    newtiltscan->rng_res = _predomGateSpacing * 1000.0;
    newtiltscan->start_rng = 
      newtiltscan->undefined_rng = _predomStartRange * 1000.0;
    newtiltscan->max_rng = (_predomStartRange + 
			   (_predomGateSpacing * _predomMaxNGates));
    switch (e_scanmode(_lastScanType)) 
      {
      case SCAN_SECTOR_MODE:
	newtiltscan->scan_type = VOL;
	newtiltscan->sectorScan = true;
	newtiltscan->sectorStartAngle = _sectorAz1;
	newtiltscan->sectorEndAngle = _sectorAz2;
	newtiltscan->sectorAngleIncreasing =  _azAngleIncreasing;
	newtiltscan->angle_res = fToRdrAngle(_azAngleRes);
	break;
      case SCAN_SURVEILLANCE_MODE:
	newtiltscan->scan_type = VOL;
	newtiltscan->sectorScan = false;
	newtiltscan->angle_res = fToRdrAngle(_azAngleRes);
	break;
      case SCAN_RHI_MODE:
	newtiltscan->scan_type = RHISet;
	newtiltscan->sectorScan = true;
	newtiltscan->sectorStartAngle = _rhiEl1;
	newtiltscan->sectorEndAngle = _rhiEl2;
	newtiltscan->sectorAngleIncreasing =  _elAngleIncreasing;
	newtiltscan->angle_res = fToRdrAngle(_elAngleRes);
	break;
      default :
	newtiltscan->scan_type = VOL;
      }
    newtiltscan->vol_scan_no = (tiltnum * _params.output_fields_n) + i + 1;
//     newtiltscan->vol_scans = _lastVolTiltCount * _params.output_fields_n;
    newtiltscan->vol_scans = 0;
    newtiltscan->vol_tilt_no = tiltnum + 1;
//     newtiltscan->vol_tilts = _lastVolTiltCount;
    newtiltscan->vol_tilts = 0;
    newtiltscan->scanCreatorString = "Dsr2Rapic";
    newtiltscan->creatorFieldString = "Dsr2Rapic";
    newtiltscan->creatorFieldVersion = Dsr2Rapic_Version;
    if (_elevTable.size())
      newtiltscan->set_angle = fToRdrAngle(_elevTable[0]);
    newtiltscan->data_type = e_data_type(int(outField.rapic_output_type));
    if (!_rapicScan->LvlTbls(newtiltscan->data_type)) {
      switch (newtiltscan->data_type)
	{
	case e_refl:
	  _rapicScan->_LvlTbls[e_refl]  = GetStdBinaryReflLevelTbl();
	  break;
	case e_vel:
	  _rapicScan->_LvlTbls[e_vel]  = GetStdBinaryVelLevelTbl();
	  break;
	case e_spectw:
	  _rapicScan->_LvlTbls[e_spectw]  = GetStdBinarySpectWLevelTbl();
	  break;
	case e_particle_id:
	  _rapicScan->_LvlTbls[e_particle_id] = GetStd64LvlPartIDLevelTbl();
	  if (_rapicScan->_LvlTbls[e_particle_id] &&
	      !_rapicScan->_LvlTbls[e_particle_id]->enumTypeStrings.size() &&
	      _params.particle_id_strings_n)
	    // if particle id strings available and not set in
	    // the lvltbl, set them now
	    _setPartIDStrings(_rapicScan->_LvlTbls[e_particle_id]);
	  break;
	default:
	  cerr << "Dsr2Rapic::_processTilt - ERROR - Trying to process unknown field - " 
	       << get_data_type_text(newtiltscan->data_type) << endl;
	  ;	
	}
    }
    if (_rapicScan->LvlTbls(newtiltscan->data_type))
      newtiltscan->NumLevels = 
	_rapicScan->LvlTbls(newtiltscan->data_type)->numLevels();
    else
      newtiltscan->NumLevels = 256;
    newtiltscan->data_source = RADARCONVERT;
    newtiltscan->nyquist = _beamsStored[_lastTiltStartBeam]->nyquistVel;
    newtiltscan->Write16lvlHeader();      // write header to expbuff
    _rapicEncodeField(i, newtiltscan, _lastTiltStartBeam, _lastTiltEndBeam);
    newtiltscan->add_line(endstr, strlen(endstr), false, true);
    newtiltscan->check_data();
    newtiltscan->setScanComplete(newtiltscan->dataValid());
    if (ScanMng && _rapicScan && !_rapicScan->ShownToScanClients) {
      ScanMng->NewDataAvail(_rapicScan);
      if (_params.debug) {
	cerr << "Dsr2Rapic::_processTilt - showing _rapicScan to ScanClients" 
	     << endl;
      }
    }
    _lastRapicScanUpdateTime = time(0);
    if (_params.debug) {
      cerr << "Dsr2Rapic::_processTilt - _rapicScan scans=" 
	   << _rapicScan->completedScans()
	   << " tilts=" << _rapicScan->completedTilts() << endl;
    }
  }
  
}

////////////////////////////////////////////////////////////////
// load the tilt field array

void Dsr2Rapic::_rapicEncodeField(int fieldnum,
				  rdr_scan *fieldscan,
				  int startbeam, 
				  int endbeam)

{

  if ((startbeam < 0) || (endbeam < 0))
    return;

  int maxrapicstrlen = _predomMaxNGates + 128;
  int linelen = 0;
  uchar *rapicstr = new uchar[maxrapicstrlen];
  s_radl radl(_predomMaxNGates);
  float *beamVals = new float[_predomMaxNGates];

  if (_params.debug) {
    cerr << "**** Start Dsr2Rapic::_rapicEncodeField ****" 
	 << " scan type = " <<  scanTypeText(fieldscan)
	 << " field = " << fieldnum << "("
	 << _fields[fieldnum].name << ")" 
	 << " startbeam=" << startbeam
	 << " endbeam=" << endbeam
	 << endl;
  }

  // resize the tilt field array
  
  if (endbeam < 0)
    endbeam = _beamsStored.size() - 1;

  if ((startbeam < 0) || (endbeam < 0))
    return;

  for (size_t ibeam = startbeam; (int)ibeam <= endbeam; ibeam++) {

    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      if (_params.debug)
	cerr << "rapicEncodeField - ignored beam without accept flag"
	     << endl;
      continue;
    }

    // azimuth

    int iaz = (int) ((beam->az + _params.az_correction) / _deltaAz + 0.5);
    if (iaz < 0) {
      iaz += _naz;
    } else if (iaz > _naz - 1) {
      iaz -= _naz;
    }
    
    float valscale = 1.0;
    switch (fieldscan->data_type)
      {
      case e_refl:
	radl.LvlTbl = GetStdBinaryReflLevelTbl();
	break;
      case e_vel:
	radl.LvlTbl = GetStdBinaryVelLevelTbl();
	valscale = 1/fieldscan->nyquist;
	break;
      case e_spectw:
	radl.LvlTbl = GetStdBinarySpectWLevelTbl();
	valscale = 1/fieldscan->nyquist;
	break;
      case e_particle_id:
	radl.LvlTbl = GetStd64LvlPartIDLevelTbl();
	if (radl.LvlTbl &&
	    !radl.LvlTbl->enumTypeStrings.size() &&
	    _params.particle_id_strings_n)
	  // if particle id strings available and not set in
	  // the lvltbl, set them now
	  _setPartIDStrings(radl.LvlTbl);
	valscale = 1.0;
	break;
      default:
	radl.LvlTbl = GetStdBinaryReflLevelTbl();	
      }
    for (int igate = 0; igate < _predomMaxNGates; igate ++)
      beamVals[igate] = valscale * beam->getValue(fieldnum, igate);
    radl.az = fToRdrAngle(beam->az);
    radl.el = fToRdrAngle(beam->elev);
    radl.numlevels = fieldscan->NumLevels;
    radl.ThresholdFloat(beamVals, _predomMaxNGates);
    if ((fieldscan->scan_type == RHI) ||
	(fieldscan->scan_type == RHISet)) 
      linelen = radl.EncodeEl(rapicstr, maxrapicstrlen);
    else
      linelen = radl.EncodeAz(rapicstr, maxrapicstrlen);
    fieldscan->add_line((char *)rapicstr, linelen, false, true);
  } // ibeam

  delete[] rapicstr;
  delete[] beamVals;
}


void Dsr2Rapic::_setPartIDStrings(LevelTable *lvltbl) {
  if (!lvltbl || !_params.particle_id_strings_n)
    return;
  lvltbl->enumTypeStrings.resize(_params.particle_id_strings_n);
  for (int i = 0; i < _params.particle_id_strings_n; i++) {
    lvltbl->enumTypeStrings[i] = _params._particle_id_strings[i];
  }
}


////////////////////////////////////////////////////////////////
// Handle end of volume
// Close _rapicScan and dereference it

void Dsr2Rapic::_processEndOfVol()
{
  if (!_rapicScan) {
	cerr << "Dsr2Rapic::_processEndOfVol - ERROR called with _rapicScan=NULL"
		<< endl;
	return;
  }
  char endvol[128];
  sprintf(endvol, "END SCAN SET %s %s",
	  stn_name(_rapicScan->station),
	  _rapicScan->volumeLabel());
  

  if (_params.debug)
    cerr << "Dsr2Rapic::_processEndOfVol ";
  if (_rapicScan)
    {
      if (_params.debug)
	cerr << " for " << _rapicScan->ScanString();
      _rapicScan->add_line(endvol, strlen(endvol), false);
      _rapicScan->data_finished();
    }
  cerr << endl;
  if (ScanMng)
    {
      ScanMng->FinishedDataAvail(_rapicScan);
    }
  if (_rapicScan && _rapicScan->ShouldDelete(this, "_processEndOfVol"))
    {
      if (_params.debug)
	cerr << "Deleting _rapicScan instance";
      delete _rapicScan;
      if (_params.debug)
	cerr << "_rapicScan instance deleted";
    }
  _rapicScan = NULL;
  _lastRapicScanUpdateTime = 0;
}

////////////////////////////////////////////////////////////////
// process the volume

void Dsr2Rapic::_processVol()

{
  
  PMU_auto_register("Processing volume");
  
  if (_params.debug) {
    cerr << "**** Start Dsr2Rapic::_processVol() ****" << endl;
  }

  // load predominant geometry

  _loadPredominantGeom();

  // filter beams based on predominant geometry
  
  _filterBeams();

  // load up elev table
  
  _loadElevTable();
  if (_elevTable.size() == 0) {
    if (_params.debug) {
      cerr << "WARNING - no elevations found" << endl;
    }
    if (_params.output_rhi_files) {
      if (_params.debug) {
	cerr << "  Trying RHI mode" << endl;
      }
      if (_processRhi()) {
	if (_params.debug) {
	  cerr << "----> WARNING - RHI mode failed" << endl;
	}
      }
    }
    return;
  }
  
  if (_params.debug) {
    _printElevsAndFields();
  }
  
  if (_params.debug) {
    cerr << "**** End Dsr2Rapic::_processVol() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// process in rhi mode

int Dsr2Rapic::_processRhi()

{
  
  PMU_auto_register("RHI mode");
  
  if (_params.debug) {
    cerr << "**** Start Dsr2Rapic::_processRhi() ****" << endl;
  }

  for (int ii = 0; ii < _params.rhi_files_n; ii++) {

    // create RHI object
    
    RhiTransform rhi(_params,
		     _params._rhi_files[ii].mdv_url,
		     _params._rhi_files[ii].oversampling_ratio,
		     _params._rhi_files[ii].interp_in_elevation,
		     _beamsStored, _fields,
		     _predomMaxNGates, _predomStartRange, _predomGateSpacing,
		     _beamWidth, _radarLat, _radarLon, _radarAlt);
    
    // load RHI data
    
    if (rhi.load()) {
      cerr << "  WARNING - Dsr2Rapic::_processRhi()" << endl;
      cerr << "    Loading RHI failed" << endl;
      return -1;
    }
    
    // write RHI volume
    
    if (rhi.writeVol(_predomRadarParams, _startTime, _endTime, _fields)) {
    return -1;
    }

  }
    
  if (_params.debug) {
    cerr << "**** End Dsr2Rapic::_processRhi() ****" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// load radar params

void Dsr2Rapic::_loadRadarParams(const DsRadarMsg &radarMsg)

{

  DsRadarParams *rparams = new DsRadarParams(radarMsg.getRadarParams());
  _radarParamsVec.push_back(rparams);
  
  if (!_radarInfoSet) {
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
    if (rparams->radarId)
      _radarId = rparams->radarId;
    if (rparams->radarName.size())
      _radarName =  rparams->radarName;
    _radarInfoSet = true;
    
  }

  if (_params.override_nyquist) {
    _nyquist = _params.nyquist_velocity;
  } else {
    _nyquist = (rparams->pulseRepFreq * (rparams->wavelength / 100.0)) / 4.0;
  }
  
}

////////////////////////////////////////////////////////////////
// load field params

void Dsr2Rapic::_loadFieldParams(const DsRadarMsg &radarMsg)

{

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
      } // if (!strcmp ...
    } // ifield
  } // ii
  
}

////////////////////////////////////////////////////////////////
// load the elevation table
//
// Returns true if elevation table has changed,
//         false otherwise

bool Dsr2Rapic::_loadElevTable(int startbeam, 
			       int endbeam,
			       bool rhimode)

{
  
  return _loadElevTableFromHist(startbeam, endbeam, rhimode);

}

////////////////////////////////////////////////////////////////
// load the elevation table
//
// Returns true if elevation table has changed,
//         false otherwise

bool Dsr2Rapic::_loadElevTableFromHist(int startbeam, 
				       int endbeam,
				       bool rhimode)

{

  // compute elevation histogram

  int nBeamsHist = _computeBeamGeom(startbeam, endbeam, rhimode);

  // search through the histogram, looking for peaks
  // within a given search angle

  vector<tilt_peak_t> peaks;
  int maxBeamsInTilt = 0;

  for (int i = _elevHistSearchWidth;
       i < _nElevHist - _elevHistSearchWidth; i++) {

    int count = _elevHist[i];
    
    // test for peak

    bool isPeak = true;
    if (count < _elevHist[i - 1] || count < _elevHist[i + 1]) {
      isPeak = false;
    }
    for (int j = 2; j <= _elevHistSearchWidth; j++) {
      if (count <= _elevHist[i - j] || count <= _elevHist[i + j]) {
	isPeak = false;
      }
    }
      
    if (isPeak) {
      double thisPeak = (i - _elevHistOffset) * _elevHistIntv;
      bool storePeak = false;
      if (peaks.size() == 0) {
	storePeak = true;
      } else {
	double prevPeak = peaks[peaks.size()-1].elev;
	double diff = thisPeak - prevPeak;
	if (diff > _elevHistIntv * 3) {
	  storePeak = true;
	}
      }
	
      if (storePeak) {

	// store the characteristics of this peak
	
	tilt_peak_t peak;
	peak.elev = thisPeak;
	peak.nbeams = 0;
	for (int k = i - _elevHistSearchWidth;
	     k <= i + _elevHistSearchWidth; k++) {
	  peak.nbeams += _elevHist[k];
	}
	if (peak.nbeams > maxBeamsInTilt) {
	  maxBeamsInTilt = peak.nbeams;
	}
	peaks.push_back(peak);
	
      }

    } // if (isPeak)

  } // i

  if (maxBeamsInTilt > _naz) {
    maxBeamsInTilt = _naz;
  }

//   if (_params.debug >= Params::DEBUG_VERBOSE) {
  if (_params.debug) {
    for (size_t ii = 0; ii < peaks.size(); ii++) {
      if (rhimode)
	cerr << "RHI az: " << peaks[ii].elev
	     << ", nbeams: " << peaks[ii].nbeams << endl;
      else
	cerr << "Tilt elev: " << peaks[ii].elev
	     << ", nbeams: " << peaks[ii].nbeams << endl;
    }
    cerr << "Max beams in tilt: " << maxBeamsInTilt << endl;
  }

  // go through the peaks, deciding if they are acceptable

  vector<double> elevArray;

  double sumBeamsUsed = 0.0;

  for (size_t ii = 0; ii < peaks.size(); ii++) {

    if (_params.check_min_beams_in_tilt &&
	peaks[ii].nbeams < _params.min_beams_in_tilt) {
//       if (_params.debug >= Params::DEBUG_VERBOSE) {
      if (_params.debug) {
	cerr << "Rejecting peak at " << peaks[ii].elev << endl;
	cerr << "  Too few beams:" << peaks[ii].nbeams << endl;
      }
      continue;
    }

    if (_params.check_min_fraction_in_tilt) {
      double fraction = (double) peaks[ii].nbeams / maxBeamsInTilt;
      if (fraction < _params.min_fraction_in_tilt) {
// 	if (_params.debug >= Params::DEBUG_VERBOSE) {
	if (_params.debug) {
	  cerr << "Rejecting peak at " << peaks[ii].elev << endl;
	  cerr << "  Fraction too low:" << fraction << endl;
	}
	continue;
      }
    }

    // accept

    elevArray.push_back(peaks[ii].elev);
    sumBeamsUsed += peaks[ii].nbeams;

  }

  // check for combined fraction

  double combinedFraction = sumBeamsUsed / nBeamsHist;
  if (_params.check_combined_fraction_in_all_tilts &&
      combinedFraction < _params.min_combined_fraction_in_all_tilts) {
    if (_params.debug) {
      cerr << "*** Combined fraction of beams too low" << endl;
      cerr << "    nBeamsHist: " << nBeamsHist << endl;
      cerr << "    sumBeamsUsed: " << sumBeamsUsed << endl;
      cerr << "    combinedFraction: " << combinedFraction << endl;
      cerr << "    minCombinedFraction: "
	   << _params.min_combined_fraction_in_all_tilts << endl;
    }
    _elevTable.clear();
    _nElev = _elevTable.size();
    return true;
  }
  
  bool elevsHaveChanged = false;
  if (elevArray.size() != _elevTable.size()) {
    elevsHaveChanged = true;
  } else {
    for (size_t ii = 0; ii < elevArray.size(); ii++) {
      if (elevArray[ii] != _elevTable[ii]) {
	elevsHaveChanged = true;
	break;
      }
    }
  }

  if (elevsHaveChanged) {
    if (_params.debug) {
      cerr << "Elevation table has changed" << endl;
    }
    _elevTable.clear();
    for (size_t ii = 0; ii < elevArray.size(); ii++) {
      _elevTable.push_back(elevArray[ii]);
    }
    _nElev = _elevTable.size();
  }
  
  return elevsHaveChanged;
  
}


////////////////////////////////////////////////////////////////
// compute beam geometry including 
// elevation histogram
// ppi/rhi angle resolution
// start/end az/el - az1/el1 always first beam angle, 
// use az/el angleincreasing flags for direction
//
// Returns the total number of beams used

int Dsr2Rapic::_computeBeamGeom(int startbeam, 
				int endbeam,
				bool rhimode)

{
  
  _clearElevHist();

  int nBeamsHist = 0;
  if (endbeam < 0)
    endbeam = _beamsStored.size() - 1;

  if ((startbeam < 0) || (endbeam < 0))
    return 0;

  Beam *prevbeam = NULL;
  float _deltaAngle = 0.0;
  float _deltaSum = 0.0;
  for (int ibeam = startbeam; (int)ibeam <= endbeam; ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->accept) {
      int elevBin;
      if (rhimode)
	elevBin = (int) (beam->az / _elevHistIntv + _elevHistOffset + 0.5);
      else
	elevBin = (int) (beam->elev / _elevHistIntv + _elevHistOffset + 0.5);	
      if (elevBin < 0) {
	elevBin = 0;
      }
      if (elevBin > _nElevHist - 1) {
	elevBin = _nElevHist - 1;
      }
      _elevHist[elevBin]++;
      nBeamsHist++;
      if (!prevbeam) {  // first beam, get az1
	if (rhimode)
	  _rhiEl1 = beam->elev;
	else
	  _sectorAz1 = beam->az;
      }
      else {
	if (rhimode)
	  _deltaAngle = beam->elev - prevbeam->elev;
	else
	  _deltaAngle = beam->az - prevbeam->az;
	if (_deltaAngle > 180.0)  // correct for zero crossing CCW
	  _deltaAngle -= 360.0;
	if (_deltaAngle < -180.0)  // zero crossing CW
	  _deltaAngle += 360.0;
	_deltaSum += _deltaAngle;
      }
      if (ibeam == endbeam) {
	if (rhimode) {
	  _elAngleRes = fabsf(_deltaSum / float(nBeamsHist - 1));
	  _elAngleIncreasing = _deltaSum > 0.0;
	  _rhiEl2 = beam->elev;
	  }
	else {
	  _azAngleRes = fabsf(_deltaSum / float(nBeamsHist - 1));
	  _azAngleIncreasing = _deltaSum > 0.0;
	  _sectorAz2 = beam->az;
	}
      }
    }
    prevbeam = beam;
  }

  if (_params.debug) {
    cerr << "Dsr2Rapic::_computeBeamGeom - startbeam="
	 << startbeam << " endbeam=" << endbeam
	 << " nBeamsHist=" << nBeamsHist 
	 << " _deltaSum=" << _deltaSum 
	 << " rhimode=" << rhimode << endl;
    if (rhimode) {
      cerr << "elAngleRes=" << _elAngleRes 
	   << " rhiEl1=" << _rhiEl1
	   << " rhiEl2=" << _rhiEl2
	   << " angleIncreasing=" << _elAngleIncreasing
	   << endl;
    }
    else {
      cerr << "azAngleRes=" << _azAngleRes 
	   << " sectorAz1=" << _sectorAz1
	   << " sectorAz2=" << _sectorAz2
	   << " angleIncreasing=" << _azAngleIncreasing
	   << endl;
    }
  }

  return nBeamsHist;

}

////////////////////////////////////////////////////////////////////
// Pre-filter data based on geometry - gate spacing and/or start gate
//

bool Dsr2Rapic::_preFilter(const DsRadarMsg &radarMsg)

{

  const DsRadarParams& rparams = radarMsg.getRadarParams();
  const DsRadarBeam &beam = radarMsg.getRadarBeam();

  if (beam.scanMode != _lastScanType) {
    if (_params.debug) {
      cerr << "@@@@ Scan type changed from " 
	   << scanModeString(e_scanmode(_lastScanType)) 
	   << " to " << scanModeString(e_scanmode(beam.scanMode)) 
	   << " @@@@" << endl;
//       if (!_endOfVol && (_tiltNum > 0)) {
// 	cerr << "@@@@ Forcing End of Vol @@@@" << endl;
// 	_endOfVol = true;
// 	_lastVolTiltCount = _tiltNum + 1;
//       }
    }
  }
  _lastScanType = beam.scanMode;

  // only process on SECTOR, SURVEILLANCE and RHI scan types
  if (!((e_scanmode(_lastScanType) == SCAN_SECTOR_MODE) ||
	(e_scanmode(_lastScanType) == SCAN_SURVEILLANCE_MODE) ||
	(e_scanmode(_lastScanType) == SCAN_RHI_MODE)))
    return false;

  if (_params.filter_gate_spacing) {
    double diff = fabs(_params.keep_gate_spacing - rparams.gateSpacing);
    if (diff > _smallRange) {
      return false;
    }
  }
  
  if (_params.filter_start_range) {
    double diff = fabs(_params.keep_start_range - rparams.startRange);
    if (diff > _smallRange) {
      return false;
    }
  }

  if (_params.filter_prf) {
    if (rparams.pulseRepFreq < _params.min_prf ||
	rparams.pulseRepFreq > _params.max_prf) {
      if (_prevPrfGood &&
	  _params.end_of_vol_decision == Params::AUTOMATIC &&
	  _params.set_end_of_vol_on_prf_change) {
	// set end of vol flag
	_prevPrfGood = false;
	_endOfVol = true;
	_lastVolTiltCount = _tiltNum + 1;
	if (_params.debug) {
	  cerr << "  PRF went out of range: " << rparams.pulseRepFreq << endl;
	}
      }
      return false;
    }
  }
  _prevPrfGood = true;
  
  if (_params.filter_elev) {
    if (beam.elevation < _params.min_elev ||
        beam.elevation > _params.max_elev) {
      return false;
    }
  }

  return true;

}

////////////////////////////////////////////////////////////////
// decide whether to select or reject beams
// also set start and end times

void Dsr2Rapic::_filterBeams(int startbeam, 
			     int endbeam)

{
  
  _startTime = 0;
  _endTime = 0;

  // check beam geometry

  int nBeamsUsed = 0;
  if (endbeam < 0)
    endbeam = _beamsStored.size() - 1;
  if ((startbeam < 0) || (endbeam < 0))
    return;
  for (size_t ibeam = startbeam; (int)ibeam <= endbeam; ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (_geomMatches(beam->startRange, _predomStartRange,
		     beam->gateSpacing, _predomGateSpacing)) {
      nBeamsUsed++;
      if (_startTime == 0) {
	_startTime = beam->time;
      }
      _endTime = beam->time;
    } else {
      if (_params.debug) {
	cerr << "_filterBeams - ignoring beam #" << ibeam - startbeam
	     << endl;
      }
      beam->accept = false;
    }
  }

  if (_params.debug) {
    cerr << "  nBeamsUsed: " << nBeamsUsed << endl;
  }

  // if required, filter the noise

  if (_params.check_filter) {
    _filterNoise();
  }
  
}

////////////////////////////////////////////////////////////////
// filter noise from accepted beams
//

void Dsr2Rapic::_filterNoise(int startbeam, 
			     int endbeam)

{
  
  if (_filterFieldNum < 0) {
    int i = 0;
    int numfields = _beamsStored[startbeam]->_fields.size();
    while ((_filterFieldNum < 0) && 
	   (i < numfields)) {
      if (strcmp(_beamsStored[startbeam]->_fields[i].dsrName.c_str(),
		 _params.filter_field_name) == 0) {
	
	if (_params.debug) {
	  cerr << "filterNoise using Field #" << i 
	       << " " << _beamsStored[startbeam]->_fields[i].dsrName
	       << " for filter_field_name=" << _params.filter_field_name
	       << endl;
	}
	_filterFieldNum = i;
      } else {
	i++;
      }
    }
  }

  if (_filterFieldNum < 0) {
    cerr << "WARNING - Dsr2Rapic::_filterNoise()" << endl;
    cerr << "  No matching field found for filter_field_name=" 
	 << _params.filter_field_name << endl;
    cerr << "  Filtering cannot be done." << endl;
    return;
  }

  bool filterOndBZ = 
    (strcasecmp(_beamsStored[startbeam]->_fields[_filterFieldNum].units.c_str(),
		"dbz") == 0);

  TaArray<fl32> dbzNoiseThreshold_;
  fl32 *dbzNoiseThreshold = NULL;
  if (filterOndBZ) {
    // Set dbz test level according to the signal/noise threshold.
    
    dbzNoiseThreshold = dbzNoiseThreshold_.alloc(_predomMaxNGates);
    
    double range = _predomStartRange;
    for (int igate = 0; igate < _predomMaxNGates; igate++,
	   range += _predomGateSpacing) {
      dbzNoiseThreshold[igate] =
	(_params.filter_threshold_min + _params.noise_dbz_at_100km +
	 20.0 * (log10(range) - log10(100.0)));
    } // igate
  }
    
  // clear the gates with reflectivity values below the signal/noise

  if (_params.debug) {
    cerr << "  Deleting noise gates ..." << endl;
  }

  if (endbeam < 0)
    endbeam = _beamsStored.size() - 1;
  if ((startbeam < 0) || (endbeam < 0))
    return;
  for (size_t ibeam = startbeam; (int)ibeam <= endbeam; ibeam++) {

    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }

    int runLength = 0;
    bool filterPassed = false;
    float filterMin = _params.filter_threshold_min;
    float filterMax = _params.filter_threshold_max;
    for (int igate = 0; igate < _predomMaxNGates; igate++) {
      if (filterOndBZ) {
	filterPassed = 
	  beam->getValue(_filterFieldNum, igate) >= dbzNoiseThreshold[igate];
      } else {
	float filterVal = beam->getValue(_filterFieldNum, igate);
	filterPassed = (filterVal >= filterMin) &&
	  (filterVal <= filterMax);
      }
      if (filterPassed) {
	runLength++;
      } else {
	if (runLength > 0 && runLength < _params.filter_min_valid_run) {
	  // clear a short run
	  for (int jgate = igate - runLength; jgate < igate; jgate++) {
	    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
	      beam->setMissing(ifield, jgate);
	    } // ifield
	  } // jgate
	} // if (runLength > 0 ...
	// clear this gate
	for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
	  beam->setMissing(ifield, igate);
	} // ifield
	runLength = 0;
      } // if (beam->getValue( ...
    } // igate

  } // ibeam

}

////////////////////////////////////////////////////////////////
// save the beam geometry to the beamGeomVec 

void Dsr2Rapic::_saveBeamGeom(const Beam *beam)

{

  _latestBeamTime = beam->time;
  _latestBeamEl = beam->elev;
  _latestBeamAz = beam->az;

  // check if we already have a matching geometry

  for (size_t ii = 0; ii < _beamGeomVec.size(); ii++) {

    if (beam->startRange == _beamGeomVec[ii]->startRange &&
	beam->gateSpacing == _beamGeomVec[ii]->gateSpacing) {
      _beamGeomVec[ii]->nBeams++;
      if (_beamGeomVec[ii]->maxNGates < beam->nGates) {
	_beamGeomVec[ii]->maxNGates = beam->nGates;
      }
      return;
    }

  } // ii 

  // no match, add one

  BeamGeom *geom = new BeamGeom(beam->startRange,
				beam->gateSpacing);
  _beamGeomVec.push_back(geom);

}

////////////////////////////////////////////////////////////////
// load the predominant geometry into the 'in use' member
//
// Returns true if predominant beam geometry has changed,
//         false otherwise

bool Dsr2Rapic::_loadPredominantGeom()
  
{
  
  if (_beamGeomVec.size() == 0) {
    return false;
  }
  
  // find the predominant geometry, use this
  
  int predominant = 0;
  int maxBeams = 0;
  for (size_t ii = 0; ii <  _beamGeomVec.size(); ii++) {
    if (_beamGeomVec[ii]->nBeams > maxBeams) {
      maxBeams = _beamGeomVec[ii]->nBeams;
      predominant = (int) ii;
    }
  } // ii
  
  const BeamGeom &geom = *_beamGeomVec[predominant];
  
  bool changed = false;
  if (!_geomMatches(geom.startRange, _predomStartRange,
		    geom.gateSpacing, _predomGateSpacing,
		    geom.maxNGates, _predomMaxNGates)) {
    _predomGateSpacing = geom.gateSpacing;
    _predomStartRange = geom.startRange;
    _predomMaxNGates = geom.maxNGates;
    if (_params.debug) {
      cerr << "--> Beam geometry has changed" << endl;
      cerr << "    _predomStartRange: " << _predomStartRange << endl;
      cerr << "    _predomGateSpacing: " << _predomGateSpacing << endl;
      cerr << "    _predomMaxNGates: " << _predomMaxNGates << endl;
    }
    changed = true;
  }
  
  // set the predominant radar params to the first which matches
  // the beam geometry
  
  if (changed) {
    for (size_t ii = 0; ii < _radarParamsVec.size(); ii++) {
      if (_geomMatches(_radarParamsVec[ii]->startRange, _predomStartRange,
		       _radarParamsVec[ii]->gateSpacing, _predomGateSpacing)) {
	_predomRadarParams = *_radarParamsVec[ii];
	break;
      }
    }
  }
    
  return changed;

}

///////////////////////////////////////////////////////////////
// check for matching geometry

bool Dsr2Rapic::_geomMatches(double startRangeNew, double startRangePrev,
			     double gateSpacingNew, double gateSpacingPrev,
			     int nBeamsNew /* = 0 */,
			     int nBeamsPrev /* = 0 */)
  
{
  
  if (fabs(startRangeNew - startRangePrev) < _smallRange &&
      fabs(gateSpacingNew - gateSpacingPrev) < _smallRange &&
      nBeamsNew <= nBeamsPrev) {
    return true;
  } else {
    return false;
  }

}

////////////////////////////////////////////////////////////////
// bridge a missing beam in azimuth

void Dsr2Rapic::_bridgeMissingInAzimuth()

{

//   for (int ielev = 0; ielev < _nElev; ielev++) {

//     for (int iaz = 1; iaz < _naz; iaz++) {

//       if (_ppiArray[ielev][iaz] == NULL) {
	
// 	if (_ppiArray[ielev][iaz - 1] != NULL &&
// 	    _ppiArray[ielev][iaz + 1] != NULL) {
// 	  Beam *beam = new Beam(*_ppiArray[ielev][iaz - 1],
// 				*_ppiArray[ielev][iaz + 1], 0.5);
// 	  _ppiArray[ielev][iaz] =beam;
// 	  _beamsInterp.push_back(beam);
// 	}
	
//       } // if (_ppiArray[ielev][iaz] == NULL)

//     } // iaz

//     // north beam missing?

//     if (_ppiArray[ielev][0] == 0) {
      
//       if (_ppiArray[ielev][_naz - 1] != NULL &&
// 	  _ppiArray[ielev][1] != NULL) {
// 	Beam *beam = new Beam(*_ppiArray[ielev][_naz - 1],
// 			      *_ppiArray[ielev][1], 0.5);
// 	_ppiArray[ielev][0] =beam;
// 	_beamsInterp.push_back(beam);
//       }

//     }

//   } // ielev

}

////////////////////////////////////////////////////////////////
// bridge a missing beam in elevation

void Dsr2Rapic::_bridgeMissingInElevation()

{

//   for (int iaz = 1; iaz <= _naz; iaz++) {

//     for (int ielev = 1; ielev < _nElev - 1; ielev++) {

//       if (_ppiArray[ielev][iaz] == NULL) {
	
// 	if (_ppiArray[ielev - 1][iaz] != NULL &&
// 	    _ppiArray[ielev + 1][iaz] != NULL) {
// 	  Beam *beam = new Beam(*_ppiArray[ielev - 1][iaz],
// 				*_ppiArray[ielev + 1][iaz], 0.5);
// 	  _ppiArray[ielev][iaz] = beam;
// 	  _beamsInterp.push_back(beam);
// 	}
	
//       } // if (_ppiArray[ielev][iaz] == NULL)

//     } // ielev

//   } // iaz

}

/////////////////////////////////////////////////////////////////
// Print elevation and fields

void Dsr2Rapic::_printElevsAndFields()

{
  
  cerr << "Elevation histogram" << endl;
  cerr << "===================" << endl;
  for (int i = 0; i < _nElevHist; i++) {
    if (_elevHist[i] > 0) {
      cerr << "  Elev: " << (i - _elevHistOffset) * _elevHistIntv
	   << ", count: " << _elevHist[i] << endl;
    }
  } // i
  cerr << endl;

  cerr << "Elevation table" << endl;
  cerr << "===============" << endl;
  for (size_t i = 0; i < _elevTable.size(); i++) {
    cerr << "  Elev #: " << i << ", angle: " << _elevTable[i] << endl;
  } // i
  cerr << endl;
  
  cerr << "Beam geometry table" << endl;
  cerr << "===================" << endl;
  for (size_t i = 0; i < _beamGeomVec.size(); i++) {
    cerr << "  Geometry #: " << i << endl;
    cerr << "    Start range: " << _beamGeomVec[i]->startRange << endl;
    cerr << "    Gate spacing: " << _beamGeomVec[i]->gateSpacing << endl;
    cerr << "    Max gates: " << _beamGeomVec[i]->maxNGates << endl;
    cerr << "    N beams: " << _beamGeomVec[i]->nBeams << endl;
  } // i
  cerr << endl;

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
      cerr << "    isLoaded: " << _fields[ii].isLoaded << endl;
    }
  } // i
  cerr << endl;

}
  
/////////////////////
// prepare for volume

void Dsr2Rapic::_prepareForVol()

{
  _prepareBeamsForVol();
  _clearRadarParamsVec();
  _clearBeamGeomVec();
  _clearElevHist();
  _clearPpiArray();
  _clearFieldFlags();
}

/////////////////////////////
// prepare beams for next vol

void Dsr2Rapic::_prepareBeamsForVol()
{

  if (_params.nbeams_overlap_per_vol > 0) {
    
    // prepare beams read by saving some beams
    // for start of next vol

    _saveBeamsOverlap();
    
  } else {
    
    // clear read beams
    
    _clearBeamsStored();
    
  }

  // clear interpolated beams
  
  _clearBeamsInterp();

}

/////////////////////
// save overlap beams

void Dsr2Rapic::_saveBeamsOverlap()
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

void Dsr2Rapic::_reset(const DsRadarMsg &radarMsg)

{
  _prepareForVol();
  if (radarMsg.allParamsSet()) {
    DsRadarParams *rparams = new DsRadarParams(radarMsg.getRadarParams());
    _radarParamsVec.push_back(rparams);
  }
}

void Dsr2Rapic::_clearAll()

{
  _clearBeams();
  _clearRadarParamsVec();
  _clearBeamGeomVec();
  _clearElevHist();
  _clearPpiArray();
  _clearFieldFlags();
}

void Dsr2Rapic::_clearBeams()
{
  
  _clearBeamsStored();
  _clearBeamsInterp();

}

void Dsr2Rapic::_clearBeamsStored()
{

  for (size_t ii = 0; ii < _beamsStored.size(); ii++) {
    delete _beamsStored[ii];
  }
  _beamsStored.clear();

}
  
void Dsr2Rapic::_clearBeamsInterp()
{

  for (size_t ii = 0; ii < _beamsInterp.size(); ii++) {
    delete _beamsInterp[ii];
  }
  _beamsInterp.clear();

}

void Dsr2Rapic::_clearRadarParamsVec()
{
  for (size_t ii = 0; ii < _radarParamsVec.size(); ii++) {
    delete _radarParamsVec[ii];
  }
  _radarParamsVec.clear();
}

void Dsr2Rapic::_clearBeamGeomVec()
{
  for (size_t ii = 0; ii < _beamGeomVec.size(); ii++) {
    delete _beamGeomVec[ii];
  }
  _beamGeomVec.clear();
}

void Dsr2Rapic::_clearElevHist()
{
  memset(_elevHist, 0, _nElevHist * sizeof(int));
}

void Dsr2Rapic::_clearPpiArray()
{
//   if (_ppiArray != NULL) {
//     ufree2((void **) _ppiArray);
//     _ppiArray = NULL;
//   }
}

void Dsr2Rapic::_clearFieldFlags()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii].isLoaded = false;
  }
}

bool appClosing()
{
  return _appClosing;
}

void HandleSignal(int, siginfo*, void*)
{
}
