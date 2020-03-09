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
/*********************************************************************
 * ArcBeamWriter : Class of objects that write beam data in the Dsr format
 *                 using radar messages in the new ARC format.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

#include "ArcBeamWriter.hh"

#include "DualPol1Products.hh"
#include "DualPol3Products.hh"
#include "DualPolFull1Products.hh"
#include "DualPrtProducts.hh"
#include "NewSimpleProducts.hh"
#include "PolyProducts.hh"
#include "SimpleProducts.hh"

using namespace std;


ArcBeamWriter::ArcBeamWriter(const bool get_beam_time_from_header) :
  BeamWriter(),
  _objectInitialized(false),
  _getBeamTimeFromHeader(get_beam_time_from_header),
  _lastParamsUpdate(0),
  _prevBeamNum(-1)
{
}

ArcBeamWriter::~ArcBeamWriter() 
{
}


bool ArcBeamWriter::init(DsRadarQueue *radar_queue,
			 const bool debug)
{ 
  static const string method_name = "ArcBeamWriter::init()";
  
  _debug = debug;
  
  // Copy the incoming objects

  _radarQueue = radar_queue;

  _objectInitialized = true;
  
  return true;
}


bool ArcBeamWriter::processMsg(HiqMsg *msg)
{ 
  static const string method_name = "ArcBeamWriter::processMsg()";
  
  // We can only process ARC_BEAM_MSG messages

  if (msg->getMsgType() != HiqMsg::ARC_BEAM_MSG)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid message type received from radar processor" << endl;

    delete msg;
    return false;
  }
  
  // Process the received message.

  PMU_auto_register("Processing beam data");

  if (_debug)
    cerr << "Processing beam message" << endl;
    
  _updateParams(*(ArcBeamMsg *)msg);

  _writeBeam(*(ArcBeamMsg *)msg);

  delete msg;
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

bool ArcBeamWriter::_updateBeamData(const ArcBeamMsg &beam_msg,
				    DsRadarBeam &radar_beam) const
{
  static const string method_name = "ArcBeamWriter::_updateBeamData()";

  // Now retrieve the actual beam data from the message.  We get
  // the beam data in float buffers.  These must then be scaled
  // so they can be put into the Ds message.

  Products *product_handler;

  switch (beam_msg.getDataFormat())
  {
  case ArcBeamMsg::DATA_SIMPLEPP :
    product_handler = new SimpleProducts(_debug);
    break;

  case ArcBeamMsg::DATA_SIMPLE16 :
  case ArcBeamMsg::DATA_DOW :
    product_handler = new NewSimpleProducts(_debug);
    break;

  case ArcBeamMsg::DATA_POLYPP :
    product_handler = new PolyProducts(_debug);
    break;

  case ArcBeamMsg::DATA_DUALPP :
    product_handler = new DualPrtProducts(_debug);
    break;

  case ArcBeamMsg::DATA_POL1 :
    product_handler = new DualPol1Products(_debug);
    break;

  case ArcBeamMsg::DATA_POL2 :
  case ArcBeamMsg::DATA_POL3 :
    product_handler = new DualPol3Products(_debug);
    break;

  case ArcBeamMsg::DATA_FULLPOL1 :
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

  if (!product_handler->fillProducts(beam_msg.getRadarConstant(),
				     beam_msg.getXmitPulsewidth(),
				     beam_msg.getRcvrPulseWidth(),
				     beam_msg.getPeakPower(),
				     beam_msg.getNoisePower(),
				     beam_msg.getVertNoisePower(),
				     beam_msg.getHorizXmitPower(),
				     beam_msg.getFrequency(),
				     beam_msg.getPrt(),
				     beam_msg.getPrt2(),
				     beam_msg.getDataSysSat(),
				     beam_msg.getPhaseOffset(),
				     beam_msg.getHits(),
				     beam_msg.getReceiverGain(),
				     beam_msg.getVertRcvrGain(),
				     beam_msg.getNumGates(),
				     beam_msg.getAbp(),
				     refl_buffer,
				     coh_refl_buffer,
				     vel_buffer,
				     sw_buffer,
				     ncp_buffer,
				     power_buffer))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error loading beam data from HiQ messages" << endl;

    delete[] refl_buffer;
    delete[] vel_buffer;
    delete[] sw_buffer;
    delete[] coh_refl_buffer;
    delete[] ncp_buffer;
    delete[] power_buffer;

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

  delete[] refl_buffer;
  delete[] vel_buffer;
  delete[] sw_buffer;
  delete[] coh_refl_buffer;
  delete[] ncp_buffer;
  delete[] power_buffer;

  radar_beam.loadData(beam_buffer, beam_buffer_size);

  delete[] beam_buffer;

  return true;
}


void ArcBeamWriter::_updateParams(const ArcBeamMsg &beam_msg)
{
//  _radarParams->radarId = 
//  _radarParams->radarType = 
//  _radarParams->numFields = 
  _radarParams->numGates = beam_msg.getNumGates();
  _radarParams->samplesPerBeam = beam_msg.getSamplesPerBeam();
  _radarParams->scanType = beam_msg.getScanType();
//  _radarParams->scanMode =
  _radarParams->polarization = beam_msg.getPolarization();
  _radarParams->radarConstant = beam_msg.getRadarConstant();
  
  if(!_overrideLocation && !_useDynamicLocation)
  {
    _radarParams->altitude  = beam_msg.getRadarAltitude();
    _radarParams->latitude  = beam_msg.getRadarLatitude();
    _radarParams->longitude = beam_msg.getRadarLongitude();
  }
  
  if(_useDynamicLocation) {
      loadOriginOverride();
  }

  _radarParams->gateSpacing = beam_msg.getGateSpacing() / 1000.0;  // km
  _radarParams->startRange = beam_msg.getStartRange() / 1000.0;    // km
  _radarParams->horizBeamWidth = beam_msg.getHorizBeamWidth();    // degrees
  _radarParams->vertBeamWidth = beam_msg.getVertBeamWidth();      // degrees
  _radarParams->pulseWidth = beam_msg.getRcvrPulseWidth();         // micro-sec
  _radarParams->pulseRepFreq = beam_msg.getPrf();                  // (/s)
  _radarParams->wavelength = beam_msg.getWavelength();            // cm
  _radarParams->xmitPeakPower = beam_msg.getPeakPower();          // watts
  _radarParams->receiverMds = beam_msg.getNoisePower();           // dBm
  _radarParams->receiverGain = beam_msg.getReceiverGain();        // dB
  _radarParams->antennaGain = beam_msg.getAntennaGain();          // dB
  _radarParams->systemGain = beam_msg.getAntennaGain();           // dB
  _radarParams->unambigVelocity =
    beam_msg.getPrf() * beam_msg.getWavelength() / 400.0;         // m/s 
  _radarParams->unambigRange = beam_msg.getUnambiguousRange();     // km
//  _radarParams->measXmitPowerDbmH =
//  _radarParams->measXmitPowerDbmV =
  _radarParams->radarName = beam_msg.getRadarName();
//  _radarParams->scanTypeName = 
}


void ArcBeamWriter::_writeBeam(ArcBeamMsg &beam_msg)
{
  static const string method_name = "ArcBeamWriter::_writeBeam()";
  // Make sure everything is initialized
  
  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "init() MUST be called before calling _writeBeam()" << endl;
    
    return;
  }
  
  if (!_radarParamsInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "initRadarParams() MUST be called before calling _writeBeam()" << endl;

    return;
  }
  
  if (!_fieldParamsInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "initFieldParams() MUST be called before calling writeBeam()" << endl;

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

  int curr_beam_num = beam_msg.getBeamNum();
  // bool missing_previous_beams;

  if (_prevBeamNum >= 0 &&
      curr_beam_num != _prevBeamNum + 1)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Missing beams" << endl;
    cerr << "   current beam number = " << curr_beam_num << endl;
    cerr << "   previous beam number = " << _prevBeamNum << endl;

    // missing_previous_beams = true;
  }
  else
  {
    // missing_previous_beams = false;
  }

  _prevBeamNum = curr_beam_num;

  // Get the beam's data time.

  time_t data_time;

  if (_getBeamTimeFromHeader)
    data_time = beam_msg.getDataTime();
  else
    data_time = time((time_t *)0);

  // Get the volume and tilt numbers.  This needs to be immediately before
  // outputting start/end of volume so that we don't exit after updating
  // the volume number but before outputting the flags.

  int volume_num = beam_msg.getVolumeNumber();
  int tilt_num = beam_msg.getTiltNumber();
  
  // If we get here, we are going to output a beam.  Now check for
  // end of tilt and/or volume so we can send those flags along with
  // the beam.  We only want to send these flags with a valid beam
  // so we aren't flooding the output queue with start/end of tilt
  // flags during the beam drop between volumes.

  bool new_vol = _putStartEndFlags(volume_num, tilt_num, data_time);
   
  // Set previous tilt and volume since we don't use these below
  // this point.

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

  if (new_vol || time_since_update > 5)
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
  if(_use_dynamic_heading) {
       _radarBeam->azimuth = beam_msg.getAzimuth() + _heading_correction;
  } else {
	_radarBeam->azimuth = beam_msg.getAzimuth();
  }
  _radarBeam->elevation  = beam_msg.getElevation();
  _radarBeam->targetElev = beam_msg.getElevation();
  _radarBeam->volumeNum  = volume_num;
  _radarBeam->tiltNum    = beam_msg.getTiltNumber();

  // Update the beam data

  if (!_updateBeamData(beam_msg, *_radarBeam))
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
