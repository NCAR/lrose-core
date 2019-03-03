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
 * BeamWriter : Class of objects that write beam data in the Dsr format.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/DateTime.hh>

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
  _radarParamsInitialized(false),
  _fieldParamsInitialized(false),
  _debug(false),
  _printSummaryFlag(false),
  _summaryInterval(0),
  _eovStrategy(0),
  _prevVolNum(-1),
  _prevTiltNum(-1),
  _prevElevation(-1.0),
  _prevAzimuth(-1.0),
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


void BeamWriter::updateParams(const HiqRadarMsg &radar_msg,
			      const HiqBeamMsg &beam_msg)

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


bool BeamWriter::_init(DsRadarQueue *radar_queue,
		       const ScanStrategy &scan_strategy,
		       EOVStrategy *end_of_volume_strategy,
		       const bool debug)
{ 
  static const string method_name = "BeamWriter::init()";
  
  _debug = debug;
  
  // Copy the incoming objects

  _radarQueue = radar_queue;
  _scanStrategy = scan_strategy;
  _eovStrategy = end_of_volume_strategy;

  return true;
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


bool BeamWriter::_updateBeamData(const HiqRadarMsg &radar_msg,
				 const HiqBeamMsg &beam_msg,
				 DsRadarBeam &radar_beam) const
{
  static const string method_name = "BeamWriter::_updateBeamData()";

  // Now retrieve the actual beam data from the message.  We get
  // the beam data in float buffers.  These must then be scaled
  // so they can be put into the Ds message.

  Products *product_handler;

  switch (beam_msg.getDataFormat())
  {
  case HiqBeamMsg::DATA_SIMPLEPP :
    product_handler = new SimpleProducts(_debug);
    break;

  case HiqBeamMsg::DATA_SIMPLE16 :
  case HiqBeamMsg::DATA_DOW :
    product_handler = new NewSimpleProducts(_debug);
    break;

  case HiqBeamMsg::DATA_POLYPP :
    product_handler = new PolyProducts(_debug);
    break;

  case HiqBeamMsg::DATA_DUALPP :
    product_handler = new DualPrtProducts(_debug);
    break;

  case HiqBeamMsg::DATA_POL1 :
    product_handler = new DualPol1Products(_debug);
    break;

  case HiqBeamMsg::DATA_POL2 :
  case HiqBeamMsg::DATA_POL3 :
    product_handler = new DualPol3Products(_debug);
    break;

  case HiqBeamMsg::DATA_FULLPOL1 :
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
    cerr << "Error loading beam data from HiQ messages" << endl;

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


bool BeamWriter::_putStartEndFlags(const int volume_num,
				   const time_t data_time) const
{
  bool new_volume = false;

  // Check for this being the end of the volume

  if (volume_num != _prevVolNum)
  {
    new_volume = true;
    
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

  return new_volume;
}
