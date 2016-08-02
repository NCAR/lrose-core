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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:41 $
//   $Id: TiltNumBeamWriter.cc,v 1.2 2016/03/06 23:53:41 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TiltNumBeamWriter : Class of objects that write beam data in the Dsr
 *                     format.  This version of the beam writer tries to
 *                     detect end of tilt conditions and doesn't send
 *                     beams that are outside of the defined scan strategy.
 *                     This is needed if you are running ApRemoval downstream.
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include <toolsa/pmu.h>

#include "TiltNumBeamWriter.hh"

using namespace std;


TiltNumBeamWriter::TiltNumBeamWriter() :
  BeamWriter(),
  _objectInitialized(false),
  _getTiltNumFromHeader(true),
  _getVolNumFromHeader(true),
  _getBeamTimeFromHeader(true),
  _lastParamsUpdate(0),
  _maxDiffFromScan(0.0),
  _maxLegalElevDrop(90.0),
  _beamDropping(false),
  _prevBeamNum(-1)
{
}

TiltNumBeamWriter::~TiltNumBeamWriter() 
{
  delete _radarQueue;
  delete _eovStrategy;
}

bool TiltNumBeamWriter::init(DsRadarQueue *radar_queue,
			     const ScanStrategy &scan_strategy,
			     const double max_diff_from_scan,
			     EOVStrategy *end_of_volume_strategy,
			     const double max_legal_elev_drop,
			     const bool get_tilt_num_from_header,
			     const bool get_vol_num_from_header,
			     const bool get_beam_time_from_header,
			     const bool debug)
{ 
  static const string method_name = "TiltNumBeamWriter::init()";
  
  // Initialize the underlying object

  _init(radar_queue, scan_strategy, end_of_volume_strategy, debug);
  
  // Copy the incoming objects

  _maxDiffFromScan = max_diff_from_scan;
  _maxLegalElevDrop = max_legal_elev_drop;

  // Copy the flags

  _getTiltNumFromHeader = get_tilt_num_from_header;
  _getVolNumFromHeader = get_vol_num_from_header;
  _getBeamTimeFromHeader = get_beam_time_from_header;

  _objectInitialized = true;
  
  return true;
}


void TiltNumBeamWriter::writeBeam(const HiqRadarMsg &radar_msg,
				  HiqBeamMsg &beam_msg)
{
  static const string method_name = "TiltNumBeamWriter::writeBeam()";
  
  // Make sure everything is initialized
  
  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "TiltNumBeamWriter::init() MUST be called before calling TiltNumBeamWriter::writeBeam()" << endl;
    
    return;
  }
  
  if (!_radarParamsInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "TiltNumBeamWriter::initRadarParams() MUST be called before calling TiltNumBeamWriter::writeBeam()" << endl;

    return;
  }
  
  if (!_fieldParamsInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "TiltNumBeamWriter::initFieldParams() MUST be called before calling TiltNumBeamWriter::writeBeam()" << endl;

    return;
  }
  
  // Put new scan type flag out the first time only

  PMU_auto_register("Writing flags");
  if (_debug)
    cerr << "Writing flag" << endl;

  if (_prevVolNum < 0)
  {
    _radarQueue->putNewScanType(beam_msg.getScanType());
    _prevVolNum = 0;
  }
  
  // Check to see if we've missed any beams.  Do this right at the
  // beginning so we don't flag skipped beams as missing.

  int curr_beam_num = beam_msg.getRayCount();
  bool missing_previous_beams;

  if (_prevBeamNum >= 0 &&
      curr_beam_num != _prevBeamNum + 1)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Missing beams" << endl;
    cerr << "   current beam number = " << curr_beam_num << endl;
    cerr << "   previous beam number = " << _prevBeamNum << endl;

    missing_previous_beams = true;
  }
  else
  {
    missing_previous_beams = false;
  }

  _prevBeamNum = curr_beam_num;

  // Get the beam's data time.

  time_t data_time;

  if (_getBeamTimeFromHeader)
    data_time = beam_msg.getDataTime();
  else
    data_time = time((time_t *)0);

  // Determine the tilt number for this beam.

  int tilt_num;

  if (_getTiltNumFromHeader)
    tilt_num = beam_msg.getTiltNumber();
  else
    tilt_num = _scanStrategy.getTiltNum(beam_msg.getElevation());

  // If we're not in a valid tilt, don't write out the beam

  double target_elevation = _scanStrategy.getElevation(tilt_num);

  if (tilt_num < 0 ||
      target_elevation < 0 ||
      fabs(target_elevation - beam_msg.getElevation()) > _maxDiffFromScan)
  {
    if (_debug)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Beam outside of scan strategy -- skipping: el = "
	   << beam_msg.getElevation() 
	   << ", az = " << beam_msg.getAzimuth() << endl;
    }

    _prevElevation = beam_msg.getElevation();

    return;
  }

  // See if the beam is currently in the drop between volumes.
  // We think the beam is dropping if this is not the first beam and
  // the differenct in elevation from the previous beam is too great
  // or if we have been dropping and we haven't gotten back
  // to the first tilt again yet.

  if ((_prevElevation != -1 && 
       _prevElevation - beam_msg.getElevation() > _maxLegalElevDrop) ||
      (_beamDropping &&
	tilt_num != 0))
//       (beam_msg.getElevation() < _prevElevation ||
//	tilt_num != 0)))
  {
    if (_debug)
      cerr << "Skipping beam assumed to be in drop to starting tilt: el = "
	   << beam_msg.getElevation()
	   << ", az = " << beam_msg.getAzimuth() << endl;

    _prevElevation = beam_msg.getElevation();

    _beamDropping = true;

    return;
  }
  
  _beamDropping = false;

  // Calculate the volume number.  This needs to be immediately before
  // outputting start/end of volume so that we don't exit after updating
  // the volume number but before outputting the flags.

  int volume_num;

  if (_getVolNumFromHeader)
  {
    volume_num = beam_msg.getVolumeNumber();
  }
  else if (_scanStrategy.getNumTilts() == 1)
  {
    if (_prevAzimuth >= 0.0 &&
	_prevAzimuth > beam_msg.getAzimuth())
      volume_num = _prevVolNum + 1;
    else
      volume_num = _prevVolNum;
  }
  else
  {
    if (_eovStrategy->isEndOfVolume(beam_msg.getElevation()))
      volume_num = _prevVolNum + 1;
    else
      volume_num = _prevVolNum;
  }

  // If we get here, we are going to output a beam.  Now check for
  // end of tilt and/or volume so we can send those flags along with
  // the beam.  We only want to send these flags with a valid beam
  // so we aren't flooding the output queue with start/end of tilt
  // flags during the beam drop between volumes.

  bool new_tilt = _putStartEndFlags(volume_num, tilt_num, data_time);
   
  // Set previous tilt and volume since we don't use these below
  // this point.

  if (tilt_num >= 0)
    _prevTiltNum = tilt_num;
  _prevVolNum  = volume_num;

  // We don't use _prevElevation or _prevAzimuth after this so we can reset
  // them here.
  // This allows us to exit on errors below without having to set
  // this value each time.

  _prevElevation = beam_msg.getElevation();
  _prevAzimuth = beam_msg.getAzimuth();

  // Update the parameters if necessary

  PMU_auto_register( "Updating parameters" );

  int msg_content = 0;
  int time_since_update = data_time - _lastParamsUpdate;

  if (new_tilt || time_since_update > 5)
  {
    if (_debug)
    {
      cerr << "Adding parameters to beam:" << endl;

      _radarParams->print(cerr);
    }

    msg_content |= (int) DsRadarMsg::RADAR_PARAMS;
    msg_content |= (int) DsRadarMsg::FIELD_PARAMS;

    _lastParamsUpdate = data_time;
  }

  // Update the beam header information.

  msg_content |= (int) DsRadarMsg::RADAR_BEAM;
  _radarBeam->dataTime   = data_time;
  _radarBeam->azimuth    = beam_msg.getAzimuth();
  _radarBeam->elevation  = beam_msg.getElevation();
  _radarBeam->targetElev = _scanStrategy.getElevation(tilt_num);
  _radarBeam->volumeNum  = volume_num;
  _radarBeam->tiltNum    = tilt_num;

  // Update the beam data

  if (!_updateBeamData(radar_msg, beam_msg, *_radarBeam))
    return;

  // Write diagnostics

  if (_printSummaryFlag)
      _printSummary();

  // Write the beam to the fmq

  PMU_auto_register("Writing beam");

  if (_debug)
    cerr << "Writing beam: az = " << _radarBeam->azimuth
	 << ", el = " << _radarBeam->elevation << endl;
  
  if (_radarQueue->putDsBeam(_radarMsg, msg_content) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing beam to FMQ" << endl;
  }
  
}
