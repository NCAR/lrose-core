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
// OutputFmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// OutputFmq puts the data out at FMQ.
//
////////////////////////////////////////////////////////////////

#include "OutputFmq.hh"
#include "Fields.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/uusleep.h>
#include <toolsa/os_config.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <math.h>
using namespace std;

// Constructor

OutputFmq::OutputFmq(const string &prog_name,
		     const Params &params) :
  _progName(prog_name),
  _params(params)
  
{
  
  isOK = TRUE;
  _nFields = 0;
  _nGates = 1000;
  _prtSecs = 0.001;

  // initialize the output queue
  
  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::OutputFmq" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    isOK = FALSE;
    return;
  }
  if (_params.output_fmq_compress) {
    _rQueue.setCompressionMethod(TA_COMPRESSION_ZLIB);
  }
  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  // set the scale and bias values

  _powerScale = 1.0;
  _powerBias = -120.0;
  _dbzScale = 0.5;
  _dbzBias = -40.0;
  _velScale = 0.5;
  _velBias = -64.0;
  _widthScale = 0.1;
  _widthBias = 0.0;

}

// destructor

OutputFmq::~OutputFmq()
  
{

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeParams(const volume_header_t &volHdr)

{

  // create message
  
  DsRadarMsg msg;

  // get number of supported fields

  _nFields = 0;
  for (int i = 0; i < volHdr.nMoments; i++) {
    if (Fields::isSupported(volHdr.momData[i])) {
      _nFields++;
    }
  }

  // compute nyquist and unambig range

  _nFields = volHdr.nMoments;
  _nGates = volHdr.nBins;

  double prf = (double) volHdr.prf;
  double wavelength = (double) volHdr.wavelengthCmOver100 / 10000.0;
  double nyquistVel = (wavelength * prf) / 4.0;
  double unambigRange = (3.0e8 / (2.0 * prf)) / 1000.0; 

  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  // first get params from the parameter file
  
  rp.radarId = _params.radar.radar_id;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = _nGates;
  rp.samplesPerBeam = _params.samples_per_beam;
  rp.scanType = _params.scan_type_id;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  switch (volHdr.polType) {
  case POLARIZATION_H:
    rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
    break;
  case POLARIZATION_V:
    rp.polarization = DS_POLARIZATION_VERT_TYPE;
    break;
  default:
    rp.polarization = DS_POLARIZATION_RIGHT_CIRC_TYPE;
    break;
  }
  
  rp.radarConstant = _params.radar.radar_constant;
  rp.altitude = _params.radar.altitude;
  rp.latitude = _params.radar.latitude;
  rp.longitude = _params.radar.longitude;

  rp.gateSpacing = volHdr.binSpacingCm / 100000.0;
  rp.startRange = volHdr.rangeFirstBinCm / 100000.0 +
    rp.gateSpacing / 2.0;

  rp.horizBeamWidth = _params.radar.horiz_beam_width;
  rp.vertBeamWidth = _params.radar.vert_beam_width;
  
  rp.pulseWidth = _params.radar.pulse_width;
  rp.pulseRepFreq = prf;
  rp.wavelength = wavelength * 100.0;
  
  rp.xmitPeakPower = _params.radar.xmit_peak_pwr;
  rp.receiverMds = _params.radar.receiver_mds;
  rp.receiverGain = _params.radar.receiver_gain;
  rp.antennaGain = _params.radar.antenna_gain;
  rp.systemGain = _params.radar.system_gain;
  
  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.radarName = volHdr.siteNameLong;
  rp.scanTypeName = _params.scan_type_name;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  for (int i = 0; i < volHdr.nMoments; i++) {
    if (Fields::isSupported(volHdr.momData[i])) {
      string name, units;
      double scale, bias;
      Fields::getNameUnitsScaleBias(volHdr.momData[i], nyquistVel,
				    name, units, scale, bias);
      DsFieldParams* fparams =
	new DsFieldParams(name.c_str(), units.c_str(), scale, bias);
      fp.push_back(fparams);
    }
  }
  // write the message
  
  if (_rQueue.putDsMsg(msg,
		       DsRadarMsg::RADAR_PARAMS |
		       DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - OutputFmq::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeBeam(const Beam &iBeam, int volNum)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iBeam.printBeamSummary(cerr);
  }

  int iret = 0;
  DsRadarBeam &oBeam = _msg.getRadarBeam();
  int ndata = _nGates * _nFields;
  ui08 *data = new ui08[ndata];
  memset(data, 0, ndata * sizeof(ui08));
  
  // params
  
  oBeam.dataTime = iBeam.getTime();
  oBeam.volumeNum = volNum;
  oBeam.tiltNum = -1;
  oBeam.elevation = iBeam.getEl();
  oBeam.targetElev = iBeam.getEl();
  oBeam.azimuth = iBeam.getAz();

  // data

  int fieldNum = 0;
  for (int i = 0; i < iBeam.getNFields(); i++) {

    if (Fields::isSupported(iBeam.getVolHdr().momData[i])) {

      // convert to 8-bit
      
      MemBuf fBuf;
      int fieldId = iBeam.getVolHdr().momData[i];
      Fields::convertTo8Bit(fieldId,
			    iBeam.getNyquistVel(),
			    iBeam.getFieldBuf(i),
			    fBuf);

      // copy to output data buffer on gate-by-gate basis

      ui08 *in = (ui08 *) fBuf.getPtr();
      ui08 *out = data + fieldNum;
      for (int i = 0; i < _nGates; i++, in++, out += _nFields) {
	*out = *in;
      }

      fieldNum++;

    }

  }

  // load the data into the message

  oBeam.loadData(data, ndata);
  delete[] data;
  
  // write the message
  
  if (_rQueue.putDsMsg(_msg, DsRadarMsg::RADAR_BEAM)) {
    iret = -1;
  }

  if (_params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }
  
  if (iret) {
    cerr << "ERROR - OutputFmq::writeBeams" << endl;
    cerr << "  Cannot put radar beams to queue" << endl;
  }

  return iret;

}

////////////////////////////////////////
// write volume flags

int OutputFmq::writeStartOfVolume(int volNum, time_t time)
{
  if (_rQueue.putStartOfVolume(volNum, time)) {
    return -1;
  }
  return 0;
    
}

int OutputFmq::writeEndOfVolume(int volNum, time_t time)
{
  if (_rQueue.putEndOfVolume(volNum, time)) {
    return -1;
  }
  return 0;
}

