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
//   $Date: 2016/03/06 23:53:39 $
//   $Id: BeamWriter.cc,v 1.20 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.20 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BeamWriter : Class of objects that write beam data in the Dsr format.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

#include "BeamWriter.hh"

#include "DualPol1Products.hh"
#include "DualPol3Products.hh"
#include "DualPolFull1Products.hh"
#include "DualPrtProducts.hh"
#include "NewSimpleProducts.hh"
#include "PolyProducts.hh"
#include "SimpleProducts.hh"

using namespace std;


BeamWriter::BeamWriter() :
  _objectInitialized(false),
  _radarParamsInitialized(false),
  _fieldParamsInitialized(false),
  _debug(false),
  _printSummaryFlag(false),
  _summaryInterval(0),
  _getTiltNumFromHeader(true),
  _getVolNumFromHeader(true),
  _getBeamTimeFromHeader(true),
  _lastParamsUpdate(0),
  _maxDiffFromScan(0.0),
  _eovStrategy(0),
  _maxLegalElevDrop(90.0),
  _maxElevDiff(90.0),
  _fixElevations(false),
  _fixAzimuths(false),
  _beamDropping(false),
  _prevVolNum(0),
  _prevElevation(-1.0),
  _prevElevationBad(false),
  _prevAzimuth(-1.0),
  _prevAzimuthDiff(1.0),
  _prevAzimuthBad(false),
  _azimuthIncrement(1.0),
  _prevTiltNum(-1),
  _prevBeamNum(-1),
  _overrideLocation(false),
  _reflScale(1.0),
  _reflBias(0.0),
  _velScale(1.0),
  _velBias(0.0),
  _swScale(1.0),
  _swBias(0.0),
  _coherentReflScale(1.0),
  _coherentReflBias(0.0),
  _ncpScale(1.0),
  _ncpBias(0.0),
  _powerScale(1.0),
  _powerBias(1.0),
  _radarParams(&(_radarMsg.getRadarParams())),
  _radarBeam(&(_radarMsg.getRadarBeam())),
  _radarQueue(0)
{
}

BeamWriter::~BeamWriter() 
{
  delete _radarQueue;
  delete _eovStrategy;
}

bool BeamWriter::init(DsRadarQueue *radar_queue,
		      const ScanStrategy &scan_strategy,
		      const double max_diff_from_scan,
		      EOVStrategy *end_of_volume_strategy,
		      const double max_legal_elev_drop,
		      const double max_elev_diff,
		      const double max_azimuth_diff,
		      const double azimuth_increment,
		      const bool get_tilt_num_from_header,
		      const bool get_vol_num_from_header,
		      const bool get_beam_time_from_header,
		      const bool fix_elevations,
		      const bool fix_azimuths,
		      const bool debug)
{ 
  static const string method_name = "BeamWriter::init()";
  
  _debug = debug;
  
  // Copy the incoming objects

  _radarQueue = radar_queue;
  _scanStrategy = scan_strategy;
  _eovStrategy = end_of_volume_strategy;

  _maxDiffFromScan = max_diff_from_scan;
  _maxLegalElevDrop = max_legal_elev_drop;
  _maxElevDiff = max_elev_diff;
  _maxAzimuthDiff = max_azimuth_diff;
  _azimuthIncrement = azimuth_increment;

  // Copy the flags

  _getTiltNumFromHeader = get_tilt_num_from_header;
  _getVolNumFromHeader = get_vol_num_from_header;
  _getBeamTimeFromHeader = get_beam_time_from_header;

  _fixElevations = fix_elevations;
  _fixAzimuths = fix_azimuths;

  _objectInitialized = true;
  
  return true;
}


bool BeamWriter::initRadarParams(const int radar_id,
				 const string scan_type_name)
{
  _radarParams->radarType = DS_RADAR_GROUND_TYPE;
  _radarParams->numFields = NUM_OUTPUT_FIELDS;     // number of fields
  _radarParams->scanMode = DS_RADAR_SURVEILLANCE_MODE;

  _radarParams->radarId = radar_id;                // unique number
  _radarParams->scanTypeName = scan_type_name;     // name of scanType (string)

  _radarParamsInitialized = true;
  
  return true;
}


bool BeamWriter::initFieldParams(const double dbz_scale,
				 const double dbz_bias,
				 const double vel_scale,
				 const double vel_bias,
				 const double sw_scale,
				 const double sw_bias,
				 const double coherent_dbz_scale,
				 const double coherent_dbz_bias,
				 const double ncp_scale,
				 const double ncp_bias,
				 const double power_scale,
				 const double power_bias)
{
  // Initialize output field parameters.  The MUST match the
  // order in the output_field_offsets_t enumeration!!

  vector< DsFieldParams* > &field_params = _radarMsg.getFieldParams();
  DsFieldParams*  fparams;

  fparams = new DsFieldParams("DBZ", "dBZ", 
			      dbz_scale, 
			      dbz_bias );
  field_params.push_back(fparams);

  _reflScale = dbz_scale;
  _reflBias = dbz_bias;

  fparams = new DsFieldParams("VEL", "m/s", 
			      vel_scale, 
			      vel_bias );
  field_params.push_back(fparams);

  _velScale = vel_scale;
  _velBias = vel_bias;

  fparams = new DsFieldParams("SPW", "m/s", 
			      sw_scale,
			      sw_bias );
  field_params.push_back(fparams);

  _swScale = sw_scale;
  _swBias = sw_bias;

  fparams = new DsFieldParams("COH DBZ", "dBZ", 
			      coherent_dbz_scale,
			      coherent_dbz_bias );
  field_params.push_back(fparams);

  _coherentReflScale = coherent_dbz_scale;
  _coherentReflBias = coherent_dbz_bias;

  fparams = new DsFieldParams("NCP", "none",
			      ncp_scale,
			      ncp_bias );
  field_params.push_back(fparams);

  _ncpScale = ncp_scale;
  _ncpBias = ncp_bias;

  fparams = new DsFieldParams("POWER", "none",
			      power_scale,
			      power_bias );
  field_params.push_back(fparams);

  _powerScale = power_scale;
  _powerBias = power_bias;

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


void BeamWriter::setDiagnostics(const bool print_summary,
				const int summary_interval)
{
  _printSummaryFlag = print_summary;
  
  _summaryInterval = summary_interval;
}


void BeamWriter::updateParams(const BinetRadarMsg &radar_msg,
			      const BinetBeamMsg &beam_msg)

{
  _radarParams->numGates = beam_msg.getNumGates();
  _radarParams->samplesPerBeam = beam_msg.getSamplesPerBeam();
  _radarParams->scanType = beam_msg.getScanType();

  switch (radar_msg.getPolarization())
  {
  case 'H' :
    _radarParams->polarization = DS_POLARIZATION_HORIZ_TYPE;
    break;

  case 'V' :
    _radarParams->polarization = DS_POLARIZATION_VERT_TYPE;
    break;
  }

  _radarParams->radarConstant = radar_msg.getRadarConstant();

  if(!_overrideLocation)
  {
    _radarParams->altitude  = beam_msg.getRadarAltitude();
    _radarParams->latitude  = beam_msg.getRadarLatitude();
    _radarParams->longitude = beam_msg.getRadarLongitude();
  }

  _radarParams->gateSpacing = beam_msg.getGateSpacing() / 1000.0;  // km
  _radarParams->startRange = beam_msg.getStartRange() / 1000.0;    // km
  _radarParams->horizBeamWidth = radar_msg.getHorizBeamWidth();    // degrees
  _radarParams->vertBeamWidth = radar_msg.getVertBeamWidth();      // degrees
  _radarParams->pulseWidth = beam_msg.getRcvrPulseWidth();         // micro-sec
  _radarParams->pulseRepFreq = beam_msg.getPrf();                  // (/s)
  _radarParams->wavelength = radar_msg.getWavelength();            // cm
  _radarParams->xmitPeakPower = radar_msg.getPeakPower();          // watts
  _radarParams->receiverMds = radar_msg.getNoisePower();           // dBm
  _radarParams->receiverGain = radar_msg.getReceiverGain();        // dB
  _radarParams->antennaGain = radar_msg.getAntennaGain();          // dB
  _radarParams->systemGain = radar_msg.getAntennaGain();           // dB
  _radarParams->unambigVelocity =
    beam_msg.getPrf() * radar_msg.getWavelength() / 400.0;         // m/s 
  _radarParams->unambigRange = beam_msg.getUnambiguousRange();     // km

  _radarParams->radarName = radar_msg.getRadarName();
}


void BeamWriter::writeBeam(const BinetRadarMsg &radar_msg,
			   BinetBeamMsg &beam_msg)
{
  static const string method_name = "BeamWriter::writeBeam()";
  
  // Make sure everything is initialized
  
  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "BeamWriter::init() MUST be called before calling BeamWriter::writeBeam()" << endl;
    
    return;
  }
  
  if (!_radarParamsInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "BeamWriter::initRadarParams() MUST be called before calling BeamWriter::writeBeam()" << endl;

    return;
  }
  
  if (!_fieldParamsInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "BeamWriter::initFieldParams() MUST be called before calling BeamWriter::writeBeam()" << endl;

    return;
  }
  
  // Put new scan type flag out the first time only

  PMU_auto_register("Writing flags");
  if (_debug)
    cerr << "Writing flag" << endl;

  if (_prevTiltNum < 0)
    _radarQueue->putNewScanType(beam_msg.getScanType());

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

  // We are periodically getting beams for wild elevations from the
  // Binet processor on the Al Ain radar.  Correct these elevation
  // values if possible.

  if (_fixElevations &&
      !_fixElevation(beam_msg, missing_previous_beams))
    return;

  // We are also periodically getting bad azimuth values in the 
  // beams.

  if (_fixAzimuths &&
      !_fixAzimuth(beam_msg, missing_previous_beams))
    return;
  
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
  // or if we have been dropping and the either current elevation is
  // still less than the previous elevation or we haven't gotten back
  // to the first tilt again yet.

  if ((_prevElevation != -1 && 
       _prevElevation - beam_msg.getElevation() > _maxLegalElevDrop) ||
      (_beamDropping &&
       (beam_msg.getElevation() < _prevElevation ||
	tilt_num != 0)))
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
	    (long) (_radarParams->pulseRepFreq + 0.5),
	    (long) data_time.getYear(),
	    (long) data_time.getMonth(),
	    (long) data_time.getDay(),
	    (long) data_time.getHour(),
	    (long) data_time.getMin(),
	    (long) data_time.getSec());
  }

  count++;
  if (count == _summaryInterval)
  {
    count = 0;
  }

  return;
}


bool BeamWriter::_updateBeamData(const BinetRadarMsg &radar_msg,
				 const BinetBeamMsg &beam_msg,
				 DsRadarBeam &radar_beam) const
{
  static const string method_name = "BeamWriter::_updateBeamData()";

  // Now retrieve the actual beam data from the message.  We get
  // the beam data in float buffers.  These must then be scaled
  // so they can be put into the Ds message.

  Products *product_handler;

  switch (beam_msg.getDataFormat())
  {
  case BinetBeamMsg::DATA_SIMPLEPP :
    product_handler = new SimpleProducts(_debug);
    break;

  case BinetBeamMsg::DATA_SIMPLE16 :
  case BinetBeamMsg::DATA_DOW :
    product_handler = new NewSimpleProducts(_debug);
    break;

  case BinetBeamMsg::DATA_POLYPP :
    product_handler = new PolyProducts(_debug);
    break;

  case BinetBeamMsg::DATA_DUALPP :
    product_handler = new DualPrtProducts(_debug);
    break;

  case BinetBeamMsg::DATA_POL1 :
    product_handler = new DualPol1Products(_debug);
    break;

  case BinetBeamMsg::DATA_POL2 :
  case BinetBeamMsg::DATA_POL3 :
    product_handler = new DualPol3Products(_debug);
    break;

  case BinetBeamMsg::DATA_FULLPOL1 :
    product_handler = new DualPolFull1Products(_debug);
    break;

  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Unrecognized data format (" << beam_msg.getDataFormat()
	 << " found in beam header" << endl;
    cerr << "--- Skipping beam ---" << endl;

    return false;
  } /* endswitch - beam_msg.getDataFormat() */

  float *refl_buffer = new float[beam_msg.getNumGates()];
  float *coh_refl_buffer = new float[beam_msg.getNumGates()];
  float *vel_buffer = new float[beam_msg.getNumGates()];
  float *sw_buffer = new float[beam_msg.getNumGates()];
  float *ncp_buffer = new float[beam_msg.getNumGates()];
  float *power_buffer = new float[beam_msg.getNumGates()];

  if (!product_handler->fillProducts(radar_msg,
				     beam_msg,
				     refl_buffer,
				     coh_refl_buffer,
				     vel_buffer,
				     sw_buffer,
				     ncp_buffer,
				     power_buffer))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error loading beam data from Binet messages" << endl;

    delete refl_buffer;
    delete vel_buffer;
    delete sw_buffer;
    delete coh_refl_buffer;
    delete ncp_buffer;
    delete power_buffer;

    delete product_handler;

    return false;
  }

  delete product_handler;

  // Scale the beam data for the Dsr message.  The beam data in the
  // Dsr message is scaled and stored in a single byte array.  The
  // data is given gate-by-gate meaning that all of the field values
  // for the first gate are given first, then the field values for the
  // second gate, and so on.  The field values are put in this array
  // in the same order as they appear in the DsFieldParams initialized
  // above.
  //
  // Note that 0 is the missing data value for all of our fields because
  // we didn't explicitly set a value in our DsFieldParams structure.

  int beam_buffer_size = beam_msg.getNumGates() * NUM_OUTPUT_FIELDS;
  ui08 *beam_buffer = new ui08[beam_buffer_size];

  for (int gate = 0; gate < beam_msg.getNumGates(); ++gate)
  {
    // Fill in the scaled data values....

    // Fill in the refl value

    beam_buffer[(gate * NUM_OUTPUT_FIELDS) + REFL_FIELD_OFFSET] =
      _scaleValue(refl_buffer[gate], _reflScale, _reflBias);

    // Fill in the velocity value

    beam_buffer[(gate * NUM_OUTPUT_FIELDS) + VEL_FIELD_OFFSET] =
      _scaleValue(vel_buffer[gate], _velScale, _velBias);

    // Fill in the spectrum width value

    beam_buffer[(gate * NUM_OUTPUT_FIELDS) + SW_FIELD_OFFSET] =
      _scaleValue(sw_buffer[gate], _swScale, _swBias);

    // Fill in the coherent refl value

    beam_buffer[(gate * NUM_OUTPUT_FIELDS) + COH_REFL_FIELD_OFFSET] =
      _scaleValue(coh_refl_buffer[gate], _coherentReflScale, _coherentReflBias);

    // Fill in the NCP value

    beam_buffer[(gate * NUM_OUTPUT_FIELDS) + NCP_FIELD_OFFSET] =
      _scaleValue(ncp_buffer[gate], _ncpScale, _ncpBias);

    // Fill in the power value

    beam_buffer[(gate * NUM_OUTPUT_FIELDS) + POWER_FIELD_OFFSET] =
      _scaleValue(power_buffer[gate], _powerScale, _powerBias);

  } /* endfor - gate */

  // Reclaim the space

  delete refl_buffer;
  delete vel_buffer;
  delete sw_buffer;
  delete coh_refl_buffer;
  delete ncp_buffer;
  delete power_buffer;

  radar_beam.loadData(beam_buffer, beam_buffer_size);

  delete beam_buffer;

  return true;
}


bool BeamWriter::_fixElevation(BinetBeamMsg &beam_msg,
			       const bool missing_previous_beams)
{
  static const string method_name = "BeamWriter::_fixElevation()";

  // We are periodically getting beams for wild elevations from the
  // Binet processor on the Al Ain radar.  As a quick fix, first 
  // check to make sure the elevation in the beam message isn't
  // crazy.  If the elevation value is less than 0 or greater than
  // 90, we know right away it's bad.  If the elevation value is
  // in the valid range, then we will throw out the beam if the
  // elevation has changed too drastically from the last beam.
  // Since I've never seen two bad elevations in a row, accept the
  // beam if the previous elevation was bad.  This will hopefully
  // prevent us from getting into these states where we think every
  // new beam is bad because we got too far from the most recent
  // good beam.

  double elevation = beam_msg.getElevation();

  //  cerr << "In _fixElevation(): elevation = " << elevation
  //       << ", prev elev = " << _prevElevation << endl;

  if (_prevElevation == -1)
  {
    // Check to make sure that the first elevation we process is at
    // least within the defined scan strategy.  This is an attempt to
    // make sure we don't start with a bad elevation value and then
    // throw everything away because the good beams are too far away
    // from the bad elevation received when we first started up.

    if (_scanStrategy.getTiltNum(elevation) < 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Possible bad elevation received in Binet message: "
	   << elevation << endl;
      cerr << "Elevation not within scan strategy and no previous good message received yet" << endl;
      cerr << "Skipping message..." << endl;

      return false;
    }
  }
  else if (elevation < 0 || elevation >= 90.0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Bad elevation received in Binet message: " << elevation << endl;
    cerr << "Binet beam msg header:" << endl;
    beam_msg.print(cerr);

    if (_prevElevation == -1)
    {
      cerr << "Skipping message..." << endl;
      return false;
    }

    // Reset the elevation in this beam to the previous hopefully good
    // elevation and process the beam normally

    cerr << "Setting beam elevation to last good elevation: "
	 << _prevElevation << endl;

    beam_msg.setElevation(_prevElevation);

    // Indicate that the previous elevation was bad even though we
    // updated it with a presumably good elevation.  This is done so
    // that we don't get into a loop where we just keep updating the
    // elevation value because the elevation changed enough since the
    // last good elevation that we don't recognize the good elevations
    // anymore.

    _prevElevationBad = true;
  }
//  else if (!_prevElevationBad && !missing_previous_beams &&
//	   fabs(elevation - _prevElevation) > _maxElevDiff)
  else if (fabs(elevation - _prevElevation) > _maxElevDiff)
  {
    if (_prevElevationBad)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Possible bad elevatin received in Binet msg: el = "
	   << elevation << ", az = " << beam_msg.getAzimuth() << endl;
      cerr << "Can't fix because previous elevation bad also" << endl;

      _prevElevationBad = false;
    }
    else if (missing_previous_beams)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Possible bad elevatin received in Binet msg: el = "
	   << elevation << ", az = " << beam_msg.getAzimuth() << endl;
      cerr << "Can't fix because missing previous beams" << endl;

      _prevElevationBad = false;
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Possible bad elevation received in Binet message: "
	   << elevation << endl;
      cerr << "Previous elevation was: " << _prevElevation << endl;
      cerr << "Binet beam msg header:" << endl;
      beam_msg.print(cerr);

      // Reset the elevation in this beam to the previous hopefully good
      // elevation and process the beam normally

      cerr << "Setting beam elevation to last good elevation: "
	   << _prevElevation << endl;

      beam_msg.setElevation(_prevElevation);

      // Indicate that the previous elevation was bad even though we
      // updated it with a presumably good elevation.  This is done so
      // that we don't get into a loop where we just keep updating the
      // elevation value because the elevation changed enough since the
      // last good elevation that we don't recognize the good elevations
      // anymore.

      _prevElevationBad = true;
    }
  }
  else
  {
    _prevElevationBad = false;
  }

  return true;
}


bool BeamWriter::_fixAzimuth(BinetBeamMsg &beam_msg,
			     const bool missing_previous_beams)
{
  static const string method_name = "BeamWriter::_fixAzimuth()";

  // Check for possible bad azimuth values and replace them with the
  // best value we can come up with.  If things have been going okay,
  // replace the azimuth the the previous azimuth value plus the difference
  // between the two previous azimuth values.  Otherwise, replace it 
  // with the previous azimuth value plus a predefined azimuth increment.

  double azimuth = beam_msg.getAzimuth();
  double check_azimuth = _prevAzimuth;
  if (check_azimuth > azimuth)
    check_azimuth -= 360.0;
  double azimuth_diff = azimuth - check_azimuth;

  if (_prevAzimuth != -1.0 &&
      !_prevAzimuthBad &&
      !missing_previous_beams &&
      (azimuth_diff > _maxAzimuthDiff || azimuth_diff < 0))
  {
    double new_azimuth = _prevAzimuth + _prevAzimuthDiff;
    if (new_azimuth >= 360.0)
      new_azimuth -= 360.0;

    cerr << "WARNING: " << method_name << endl;
    cerr << "Possible bad azimuth received in Binet message: "
	 << azimuth << endl;
    cerr << "Previous azimuth was: " << _prevAzimuth << endl;
    cerr << "Reseting current azimuth value to: " << new_azimuth << endl;
    cerr << "Binet beam msg header:" << endl;
    beam_msg.print(cerr);

    beam_msg.setAzimuth(new_azimuth);

    _prevAzimuthBad = true;
  }
  else if (azimuth < 0 || azimuth >= 360.0)
  {
    double new_azimuth = _prevAzimuth + _prevAzimuthDiff;
    if (new_azimuth >= 360.0)
      new_azimuth -= 360.0;

    cerr << "WARNING: " << method_name << endl;
    cerr << "Bad azimuth received in Binet message: " << azimuth << endl;
    cerr << "Reseting current azimuth value to: " << new_azimuth << endl;
    cerr << "Binet beam msg header:" << endl;
    beam_msg.print(cerr);

    beam_msg.setAzimuth(new_azimuth);

    _prevAzimuthBad = true;
  }
  else
  {
    _prevAzimuthBad = false;
  }
  
  if (_prevAzimuthBad || _prevAzimuth < 0.0 || azimuth_diff > _maxAzimuthDiff)
    _prevAzimuthDiff = _azimuthIncrement;
  else
    _prevAzimuthDiff = azimuth_diff;

  _prevAzimuth = azimuth;

  return true;
}


bool BeamWriter::_putStartEndFlags(const int volume_num,
				   const int tilt_num,
				   const time_t data_time) const
{
  bool new_tilt = false;

  if (tilt_num != _prevTiltNum)
  {
    // Set the new_tilt flag so we know whether to send out the params
    // message down below.

    new_tilt = true;

    // Write the end of tilt flag if this isn't the first
    // beam we've processed.

    if (_prevTiltNum >= 0)
    {
      if (_debug || _printSummaryFlag)
      {
//	cerr << endl;
	cerr << "-------------------> end of tilt "
	     << _prevTiltNum << endl;
      }

      _radarQueue->putEndOfTilt(_prevTiltNum);
    }
  }

  // Check for this being the end of the volume

  if (volume_num != _prevVolNum)
  {
    if (_debug || _printSummaryFlag)
    {
//	cerr << endl;
      cerr << "-------------------> end of volume "
	   << _prevVolNum << endl;
    }

    _radarQueue->putEndOfVolume(_prevVolNum);

    // Output the beginning of volume flag.  We do this whenever the
    // volume number changes, even on the first volume read.

    if (_debug || _printSummaryFlag)
    {
//	cerr << endl;
      cerr << "-------------------> start of volume "
	   << volume_num << endl;
    }

    _radarQueue->putStartOfVolume(volume_num, data_time);
  }

  // Finally, output the beginning of tilt flag

  if (tilt_num != _prevTiltNum)
  {
    if (_debug || _printSummaryFlag)
    {
//      cerr << endl;
      cerr << "-------------------> start of tilt "
	   << tilt_num << endl;
    }

    _radarQueue->putStartOfTilt(tilt_num, data_time);
  }

  return new_tilt;
}
