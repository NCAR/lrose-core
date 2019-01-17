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
//   $Date: 2016/03/06 23:53:40 $
//   $Id: EolBeamWriter.cc,v 1.2 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EolBeamWriter : Class of objects that write beam data in the Dsr format
 *                 using radar messages in the old EOL format.
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

#include "EolBeamWriter.hh"

#include "DualPol1Products.hh"
#include "DualPol3Products.hh"
#include "DualPolFull1Products.hh"
#include "DualPrtProducts.hh"
#include "NewSimpleProducts.hh"
#include "PolyProducts.hh"
#include "SimpleProducts.hh"

using namespace std;


EolBeamWriter::EolBeamWriter() :
  BeamWriter(),
  _eovStrategy(0),
  _medianFilter(0),
  _currRadarMsg(0),
  _currBeamMsg(0)
{
}


EolBeamWriter::~EolBeamWriter() 
{
  delete _eovStrategy;
  delete _medianFilter;
  delete _currRadarMsg;
  delete _currBeamMsg;
}


bool EolBeamWriter::processMsg(HiqMsg *msg)
{ 
  static const string method_name = "EolBeamWriter::processMsg()";
  
  // We can only process EOL_BEAM_MSG and EOL_RADAR_MSG messages

  if (msg->getMsgType() != HiqMsg::EOL_BEAM_MSG &&
      msg->getMsgType() != HiqMsg::EOL_RADAR_MSG)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid message type received from radar processor" << endl;

    delete msg;
    return false;
  }
  
  // Process the received message.  Give the message to the median filter,
  // which takes control of the pointer, then process any messages that are
  // ready.

  _medianFilter->addMsg(msg);
  
  HiqMsg *next_msg;
  
  while ((next_msg = _medianFilter->getNextMsg()) != 0)
  {
    if (next_msg->getMsgType() == HiqMsg::EOL_BEAM_MSG)
    {
      PMU_auto_register("Processing beam data");

      delete _currBeamMsg;
      _currBeamMsg = (EolBeamMsg *)next_msg;

      if (_currRadarMsg == 0)
      {
	if (_debug)
	  cerr << "Got beam message but no params received -- skipping beam" << endl;
      }
      else
      {
	if (_debug)
	  cerr << "Processing beam message" << endl;
      
	_updateParams(*_currRadarMsg, *_currBeamMsg);

	_writeBeam(*_currRadarMsg, *_currBeamMsg);
      }
    }
    else if (next_msg->getMsgType() == HiqMsg::EOL_RADAR_MSG)
    {
      PMU_auto_register("Reading radar information");

      if (_debug)
	cerr << "Processing radar message" << endl;
      
      delete _currRadarMsg;
      _currRadarMsg = (EolRadarMsg *)next_msg;

      _updateParams(*_currRadarMsg, *_currBeamMsg);
      
      if (_debug)
	cerr << "Parameters read" << endl;
    }
    
    // Don't delete the next_msg pointer down here because it is
    // copied to either _currRadarMsg, _currBeamMsg or _currArcBeamMsg
    // and is then deleted next time through.
  } /* endwhile - next_msg */

  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

bool EolBeamWriter::_init(DsRadarQueue *radar_queue,
			  const ScanStrategy &scan_strategy,
			  EOVStrategy *end_of_volume_strategy,
			  const bool debug)
{ 
  static const string method_name = "EolBeamWriter::_init()";
  
  _debug = debug;
  
  // Copy the incoming objects

  _radarQueue = radar_queue;
  _scanStrategy = scan_strategy;
  _eovStrategy = end_of_volume_strategy;

  return true;
}


bool EolBeamWriter::_updateBeamData(const EolRadarMsg &radar_msg,
				    const EolBeamMsg &beam_msg,
				    DsRadarBeam &radar_beam) const
{
  static const string method_name = "EolBeamWriter::_updateBeamData()";

  // Now retrieve the actual beam data from the message.  We get
  // the beam data in float buffers.  These must then be scaled
  // so they can be put into the Ds message.

  Products *product_handler;

  switch (beam_msg.getDataFormat())
  {
  case EolBeamMsg::DATA_SIMPLEPP :
    product_handler = new SimpleProducts(_debug);
    break;

  case EolBeamMsg::DATA_SIMPLE16 :
  case EolBeamMsg::DATA_DOW :
    product_handler = new NewSimpleProducts(_debug);
    break;

  case EolBeamMsg::DATA_POLYPP :
    product_handler = new PolyProducts(_debug);
    break;

  case EolBeamMsg::DATA_DUALPP :
    product_handler = new DualPrtProducts(_debug);
    break;

  case EolBeamMsg::DATA_POL1 :
    product_handler = new DualPol1Products(_debug);
    break;

  case EolBeamMsg::DATA_POL2 :
  case EolBeamMsg::DATA_POL3 :
    product_handler = new DualPol3Products(_debug);
    break;

  case EolBeamMsg::DATA_FULLPOL1 :
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

  if (!product_handler->fillProducts(radar_msg.getRadarConstant(),
				     radar_msg.getXmitPulsewidth(),
				     beam_msg.getRcvrPulseWidth(),
				     radar_msg.getPeakPower(),
				     radar_msg.getNoisePower(),
				     radar_msg.getVertNoisePower(),
				     beam_msg.getHorizXmitPower(),
				     radar_msg.getFrequency(),
				     beam_msg.getPrt(),
				     beam_msg.getPrt2(),
				     radar_msg.getDataSysSat(),
				     radar_msg.getPhaseOffset(),
				     beam_msg.getHits(),
				     radar_msg.getReceiverGain(),
				     radar_msg.getVertRcvrGain(),
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


void EolBeamWriter::_updateParams(const EolRadarMsg &radar_msg,
				  const EolBeamMsg &beam_msg)
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
