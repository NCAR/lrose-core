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
//////////////////////////////////////////////////////////////////////
// $Id: BeamWriter.cc,v 1.8 2016/03/06 23:53:42 dixon Exp $
//
// Data managment class
/////////////////////////////////////////////////////////////////////

#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <cmath>
#include "BeamWriter.hh"
#include "DropEOVStrategy.hh"
#include "Edge2Dsr.hh"
using namespace std;




BeamWriter::BeamWriter() :
  _objectInitialized(false),
  _radarParamsInitialized(false),
  _fieldParamsInitialized(false),
  _debug(false),
  _summaryInterval(0),
  _lastParamsUpdate(0),
  _volumeNum(0),
  _prevVolNum(0),
  _endOfVolumeWritten(false),
  _prevElevation(-1.0),
  _prevTiltNum(0),
  _endOfTiltWritten(false),
  _eovStrategy(new DropEOVStrategy(10.0)),
  _overrideLocation(false),
  _radarParams(&(_radarMsg.getRadarParams())),
  _radarBeam(&(_radarMsg.getRadarBeam()))
{
}

BeamWriter::~BeamWriter() 
{
  delete _scanStrategy;

  delete _eovStrategy;
}

bool BeamWriter::init(const string &output_fmq_url,
		      const bool output_fmq_compress,
		      const int output_fmq_nslots,
		      const int output_fmq_size,
                      bool write_blocking,
		      DsFmq *archive_queue,
		      const ScanStrategy &scan_strategy,
		      const int n_elevations,
		      const double max_elev_diff,
		      const double max_diff_from_scan,
		      const double max_diff_from_target,
		      MsgLog *msg_log)
{ 
  // Set up output fmq

  if (_outputQueue.init(output_fmq_url.c_str(),
			PROGRAM_NAME,
			DEBUG_ENABLED,
			DsFmq::READ_WRITE, DsFmq::END,
			output_fmq_compress,
			output_fmq_nslots, 
			output_fmq_size,
			1000, msg_log )) {
    POSTMSG(ERROR, "Could not initialize radar queue '%s'", 
	    output_fmq_url.c_str());
    return false;
  }

  if (write_blocking) {
    _outputQueue.setBlockingWrite();
  }

  // Copy the necessary information

  _archiveQueue = archive_queue;
  
  _scanStrategy = new ScanStrategy(scan_strategy);
  
  _elevationDiffForTilt = max_elev_diff;
  _elevationDiffForScan = max_diff_from_scan;
  _elevationDiffInTarget = max_diff_from_target;
   
  _objectInitialized = true;
  
  return true;
}


bool BeamWriter::initRadarParams(const int radar_id,
				 const int n_gates_out,
				 const int polarization_code,
				 const double radar_constant,
				 const double gate_spacing,
				 const double beam_width,
				 const double wavelength,
				 const double peak_xmit_pwr,
				 const double receiver_mds,
				 const double receiver_gain,
				 const double antenna_gain,
				 const double system_gain,
				 const string &radar_name,
				 const string &site_name,
				 const string &scan_type_name)
{
  _radarParams->radarId           = radar_id;
  _radarParams->radarType         = DS_RADAR_GROUND_TYPE;
  _radarParams->numFields         = EdgeMsg::NUM_FIELDS;
  _radarParams->numGates          = n_gates_out;
  _radarParams->scanMode          = DS_RADAR_SURVEILLANCE_MODE;
  _radarParams->polarization      = polarization_code;

  _radarParams->radarConstant     = radar_constant;

  _radarParams->gateSpacing       = gate_spacing;
  _radarParams->startRange        = gate_spacing / 2.0;
  _radarParams->horizBeamWidth    = beam_width;
  _radarParams->vertBeamWidth     = beam_width;
  _radarParams->wavelength        = wavelength;

  _radarParams->xmitPeakPower     = peak_xmit_pwr;
  _radarParams->receiverMds       = receiver_mds;
  _radarParams->receiverGain      = receiver_gain;
  _radarParams->antennaGain       = antenna_gain;
  _radarParams->systemGain        = system_gain;

  char fullName[1024];
  sprintf( fullName, "%s at %s", radar_name.c_str(), site_name.c_str() );
  _radarParams->radarName = fullName;
  _radarParams->scanTypeName = scan_type_name.c_str();

  _radarParamsInitialized = true;
  
  return true;
}


bool BeamWriter::initFieldParams(const double dbz_scale,
				 const double dbz_bias,
				 const double vel_scale,
				 const double vel_bias,
				 const double sw_scale,
				 const double sw_bias,
				 const double uncorr_dbz_scale,
				 const double uncorr_dbz_bias)
{
  // Set DsRadarParams that are constant
  //
  //
  // Initialize output field parameters
  //
  vector< DsFieldParams* > &field_params = _radarMsg.getFieldParams();
  DsFieldParams*  fparams;

  fparams = new DsFieldParams("DBZ", "dBZ", 
			      dbz_scale, 
			      dbz_bias );
  field_params.push_back(fparams);

  fparams = new DsFieldParams("VEL", "m/s", 
			      vel_scale, 
			      vel_bias );
  field_params.push_back(fparams);

  fparams = new DsFieldParams("SPW", "m/s", 
			      sw_scale,
			      sw_bias );
  field_params.push_back(fparams);

  fparams = new DsFieldParams("UNC DBZ", "dBZ", 
			      uncorr_dbz_scale,
			      uncorr_dbz_bias );
  field_params.push_back(fparams);

  _fieldParamsInitialized = true;
  
  return true;
}


void BeamWriter::setLatLonOverride(const double latitude,
				   const double longitude,
				   const double altitude)
{
  _radarParams->latitude  = latitude;
  _radarParams->longitude = longitude;
  _radarParams->altitude  = altitude;

  _overrideLocation   = true;
}


void BeamWriter::setDiagnostics(const bool debug_flag,
				const int summary_interval)
{
  _debug = debug_flag;
  
  _summaryInterval = summary_interval;
}


void BeamWriter::writeBeam(const EdgeMsg &edge_msg)
{
  bool writeIt = true;

  // Make sure everything is initialized
  
  if (!_objectInitialized)
  {
    POSTMSG(ERROR,
	    "BeamWriter::init() MUST be called before calling BeamWriter::writeBeam()");
    
    return;
  }
  
  if (!_radarParamsInitialized)
  {
    POSTMSG(ERROR,
	    "BeamWriter::initRadarParams() MUST be called before calling BeamWriter::writeBeam()");

    return;
  }
  
  if (!_fieldParamsInitialized)
  {
    POSTMSG(ERROR,
	    "BeamWriter::initFieldParams() MUST be called before calling BeamWriter::writeBeam()");

    return;
  }
  
  //
  // Put new scan type flag out the first time only
  //
  PMU_auto_register( "Writing flags" );
  static bool firstCall = true;
  if (firstCall)
    _outputQueue.putNewScanType( _radarParams->scanType );

  //
  // Find the tilt number. Base the tilt number on our target elevation
  // since this is the elevation we're trying to get the beam to.
  //
  double elevation       = edge_msg.getElevation();
  double targetElevation = edge_msg.getTargetElev();
  _tiltNum = _scanStrategy->getTiltNum(targetElevation);

  //
  // Find the volume number
  //
  if (_eovStrategy->isEndOfVolume(elevation))
    ++_volumeNum;

  //
  // Put end of tilt and end of volume flags in
  // as necessary.  This must be done before beams
  // are thrown out for any reason so the triggering
  // isn't delayed.
  //
  if(!firstCall && !_endOfTiltWritten &&
     _tiltNum != _prevTiltNum )
  {
    if( _debug )
    {
      fprintf( stderr, "\n-------------------> end of tilt %d\n",
	       _prevTiltNum );
    }
    _outputQueue.putEndOfTilt( _prevTiltNum );
    _endOfTiltWritten = true;
  }
   
  if(!firstCall && _volumeNum != _prevVolNum &&
     !_endOfVolumeWritten)
  {
    if( _debug )
    {
      fprintf( stderr, "\n-------------------> end of volume %d\n",
	       _prevVolNum );
    }
    _outputQueue.putEndOfVolume( _prevVolNum );
    if( _archiveQueue ) 
      _archiveQueue->writeMsg( DsFmq::eof );
    _endOfVolumeWritten = true;
  }
   
  //
  // If this is not the first beam and the differenct
  // in elevation from the previous beam is too great
  // don't write the beam
  //
  if( _prevElevation != -1 && 
      _prevElevation - elevation > _elevationDiffForTilt )
    writeIt = false;
   

  //
  // If the actual elevation is fairly close to the target
  // elevation, we think we are not in the middle of changing
  // tilts.  If this is the case and the target elevation does
  // not look like what we were expecting from the scan strategy
  // don't write the beam.  If the actual elevation is not
  // close to the target elevation, we are in the middle of
  // changing tilts.  If this is the case we don't want to
  // be using the target elevation to judge whether to write
  // the beam out or not.
  //
  if( (fabs(targetElevation - elevation) < _elevationDiffInTarget) &&
      (fabs(targetElevation - _scanStrategy->getElevation(_tiltNum))
                 > _elevationDiffForScan) ) 
    writeIt = false;

  if( !writeIt )
  {
    POSTMSG( DEBUG, "Skipping beam at elevation %f", elevation );

    _prevElevation = elevation;
    return;
  }
  
  //
  // Put start of tilt and start of volume flags
  // in as necessary
  //
  time_t dataTime = edge_msg.getDataTime();
  bool   newTilt  = false;
  if( firstCall || _tiltNum != _prevTiltNum )
  {
    if( _debug )
    {
      fprintf( stderr, "-------------------> start of tilt %d\n",
	       _tiltNum );
    }
    _outputQueue.putStartOfTilt( _tiltNum, dataTime );
    newTilt = true;
    _endOfTiltWritten = false;
  }
   
  if( firstCall || _volumeNum != _prevVolNum )
  {
    if( _debug )
    {
      fprintf( stderr, "\n-------------------> start of volume %d\n",
	       _volumeNum );
    }
    _outputQueue.putStartOfVolume( _volumeNum, dataTime );
    _endOfVolumeWritten = false;
  }

  //
  // Update the parameters if necessary
  //
  PMU_auto_register( "Updating parameters" );
  int msgContent      = 0;
  int timeSinceUpdate = dataTime - _lastParamsUpdate;
  if( firstCall || newTilt || timeSinceUpdate > 5 )
  {
    POSTMSG( DEBUG, "Adding parameters to beam" );
    msgContent |= (int) DsRadarMsg::RADAR_PARAMS;
    msgContent |= (int) DsRadarMsg::FIELD_PARAMS;
    _updateParams(edge_msg);
    _lastParamsUpdate = dataTime;
  }

  //
  // Update the beam information
  //
  msgContent |= (int) DsRadarMsg::RADAR_BEAM;
  _radarBeam->dataTime   = dataTime;
  _radarBeam->azimuth    = edge_msg.getAzimuth();
  _radarBeam->elevation  = elevation;
  _radarBeam->targetElev = edge_msg.getTargetElev();
  _radarBeam->volumeNum  = _volumeNum;
  _radarBeam->tiltNum    = _tiltNum;
  _radarBeam->loadData(edge_msg.getDataPtr(),
		       edge_msg.getDataLen());

  //
  // Write diagnostics
  //
  if( _debug )
      _printSummary();

  //
  // Write the beam to the fmq
  //
  POSTMSG( DEBUG, "Writing beam: az = %f, el = %f",
	   _radarBeam->azimuth, _radarBeam->elevation);
  if (_outputQueue.putDsBeam(_radarMsg, msgContent) != 0)
    POSTMSG(ERROR, "Error writing beam to FMQ");

  PMU_auto_register( "Writing beam" );

  //
  // Set previous tilt and volume
  //
  _prevTiltNum = _tiltNum;
  _prevVolNum  = _volumeNum;
  _prevElevation = elevation;

  //
  // No longer first call
  //
  firstCall   = false;
}


void BeamWriter::_updateParams(const EdgeMsg &edge_msg)
{
  _radarParams->pulseRepFreq    = edge_msg.getPrf();
  _radarParams->samplesPerBeam  = edge_msg.getSamples();
  _radarParams->scanType        = edge_msg.getScanType();
  _radarParams->pulseWidth      = edge_msg.getPulseWidth();
  _radarParams->unambigVelocity = edge_msg.getNyquist();

  if( !_overrideLocation ) {
    _radarParams->latitude  = edge_msg.getLat();
    _radarParams->longitude = edge_msg.getLon();
    _radarParams->altitude  = edge_msg.getAlt();
  }
   
}


void BeamWriter::_printSummary()
{
  static int count = 0;

  if (count == 0)
  {
    fprintf(stderr,
	    " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF     Date     Time\n");

    //
    // Parse the time of the beam
    //
    DateTime  data_time(_radarBeam->dataTime);

    fprintf(stderr,
	    "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld\n",
	    (long) _radarBeam->volumeNum,
	    (long) _radarBeam->tiltNum,
	    (double) _radarBeam->targetElev,
	    (double) _radarBeam->elevation,
	    (double) _radarBeam->azimuth,
	    (long) _radarParams->numGates,
	    (long) (_radarParams->gateSpacing * 1000),
	    (long) _radarParams->pulseRepFreq,
	    (long) data_time.getYear(),
	    (long) data_time.getMonth(),
	    (long) data_time.getDay(),
	    (long) data_time.getHour(),
	    (long) data_time.getMin(),
	    (long) data_time.getSec());
//    fprintf(stderr,
//	    "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld \n",
//	    (long) _radarBeam->volumeNum,
//	    (long) _radarBeam->tiltNum,
//	    (double) _radarBeam->targetElev,
//	    (double) _radarBeam->elevation,
//	    (double) _radarBeam->azimuth,
//	    (long) _radarParams->numGates,
//	    (long) (_radarParams->gateSpacing * 1000),
//	    (long) _radarParams->pulseRepFreq);
  }

  count++;
  if (count == _summaryInterval)
  {
    count = 0;
  }

  return;
}
