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
#include "OpsInfo.hh"
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

const double OutputFmq::_missingDbl = -9999.0;

// Constructor

OutputFmq::OutputFmq(const string &prog_name,
		     const Params &params,
                     const OpsInfo &opsInfo) :
  _progName(prog_name),
  _params(params),
  _opsInfo(opsInfo)
  
{
  
  isOK = TRUE;
  _nFields = 0;
  _nGates = 1000;
  _nSamples = 64;
  _prf = 999;
  _pulseWidth = 999;

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

  _dbBias = -320.0;
  _dbScale = 0.01;
  
  _velBias = -320.0;
  _velScale = 0.01;

  _widthBias = -0.01;
  _widthScale = 0.001;

  _angleBias = -640.0;
  _angleScale = 0.02;

  _flagBias = -1000;

}

// destructor

OutputFmq::~OutputFmq()
  
{

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeParams(double start_range, double gate_spacing)
  
{

  // compute nyquist and unambig range
  
  double nyquistVel = ((_opsInfo.getRadarWavelengthCm() / 100.0) * _prf) / 4.0;
  double unambigRange = (3.0e8 / (2.0 * _prf)) / 1000.0; 
  
  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;

  if (_params.output_snr) {
    DsFieldParams* fparams =
      new DsFieldParams("SNR", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_power_dbm) {
    DsFieldParams* fparams =
      new DsFieldParams("DBM", "dBm", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_dbz) {
    DsFieldParams* fparams =
      new DsFieldParams("DBZ", "dBZ", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_velocity) {
    //  _velScale = (nyquistVel * 2.0) / 65000.0;
    //  _velBias = -nyquistVel - _velScale * 2.0;
    DsFieldParams* fparams =
      new DsFieldParams("VEL", "m/s", _velScale, _velBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_width) {
    DsFieldParams* fparams =
      new DsFieldParams("SPW", "m/s", _widthScale, _widthBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_aiq) {
    DsFieldParams* fparams =
      new DsFieldParams("AIQ", "deg", _angleScale, _angleBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_niq) {
    DsFieldParams* fparams =
      new DsFieldParams("NIQ", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_mean_i) {
    DsFieldParams* fparams =
      new DsFieldParams("MeanI", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_mean_q) {
    DsFieldParams* fparams =
      new DsFieldParams("MeanQ", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_aiq_vpol) {
    DsFieldParams* fparams =
      new DsFieldParams("AIQ_VPOL", "deg",
                        _angleScale, _angleBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_niq_vpol) {
    DsFieldParams* fparams =
      new DsFieldParams("NIQ_VPOL", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_mean_i_vpol) {
    DsFieldParams* fparams =
      new DsFieldParams("MeanI_VPOL", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_mean_q_vpol) {
    DsFieldParams* fparams =
      new DsFieldParams("MeanQ_VPOL", "dB", _dbScale, _dbBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  // first get params from the parameter file
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = _nGates;
  rp.samplesPerBeam = _nSamples;
  rp.scanType = _params.scan_type_id;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  if (_opsInfo.getPolarizationType() == "H") {
    rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  } else if (_opsInfo.getPolarizationType() == "V") {
    rp.polarization = DS_POLARIZATION_VERT_TYPE;
  } else if (_opsInfo.getPolarizationType() == "Dual_alt" ||
             _opsInfo.getPolarizationType() == "Dual_simul") {
    rp.polarization = DS_POLARIZATION_DUAL_TYPE;
  } else {
    rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  } 
  
  rp.radarConstant = _params.radar.radar_constant;
  rp.altitude = _params.radar.altitude;
  rp.latitude = _params.radar.latitude;
  rp.longitude = _params.radar.longitude;

  rp.gateSpacing = gate_spacing;
  rp.startRange = start_range;

  rp.horizBeamWidth = _params.radar.horiz_beam_width;
  rp.vertBeamWidth = _params.radar.vert_beam_width;
  
  rp.pulseWidth = _pulseWidth;
  rp.pulseRepFreq = _prf;
  rp.wavelength = _opsInfo.getRadarWavelengthCm();
  
  rp.xmitPeakPower = _params.radar.xmit_peak_pwr;
  rp.receiverMds = _params.radar.receiver_mds;
  rp.receiverGain = _params.radar.receiver_gain;
  rp.antennaGain = _params.radar.antenna_gain;
  rp.systemGain = _params.radar.system_gain;

  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.radarName = _opsInfo.getSiteName();
  rp.scanTypeName = _params.scan_type_name;
  
  // write the message
  
  if (_rQueue.putDsMsg(msg,
                       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
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

int OutputFmq::writeBeam(const Beam *beam, int volNum, int tiltNum)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::writeBeam, az: " << beam->getAz() << endl;
  }

  int iret = 0;
  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  int ndata = _nGates * _nFields;
  ui16 *data = new ui16[ndata];
  memset(data, 0, ndata * sizeof(ui16));
  
  // params
  
  dsBeam.dataTime = beam->getTime();
  dsBeam.volumeNum = volNum;
  dsBeam.tiltNum = tiltNum;
  dsBeam.elevation = beam->getEl();
  dsBeam.targetElev = beam->getTargetEl();
  dsBeam.azimuth = beam->getAz();
  
  // data
  
  int fieldNum = 0;

  if (_params.output_snr) {
    _loadDoubleField(beam->getSnr(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_power_dbm) {
    _loadDoubleField(beam->getPowerDbm(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_dbz) {
    _loadDoubleField(beam->getDbz(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_velocity) {
    _loadDoubleField(beam->getVel(), data + fieldNum,
		     _velScale, _velBias);
    fieldNum++;
  }
  
  if (_params.output_width) {
    _loadDoubleField(beam->getWidth(), data + fieldNum,
		     _widthScale, _widthBias);
    fieldNum++;
  }

  if (_params.output_aiq) {
    _loadDoubleField(beam->getAiq(), data + fieldNum,
		     _angleScale, _angleBias);
    fieldNum++;
  }

  if (_params.output_niq) {
    _loadDoubleField(beam->getNiq(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_mean_i) {
    _loadDoubleField(beam->getMeanI(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_mean_q) {
    _loadDoubleField(beam->getMeanQ(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_aiq_vpol) {
    _loadDoubleField(beam->getAiqVpol(), data + fieldNum,
		     _angleScale, _angleBias);
    fieldNum++;
  }
  
  if (_params.output_niq_vpol) {
    _loadDoubleField(beam->getNiqVpol(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_mean_i_vpol) {
    _loadDoubleField(beam->getMeanIVpol(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  if (_params.output_mean_q_vpol) {
    _loadDoubleField(beam->getMeanQVpol(), data + fieldNum,
		     _dbScale, _dbBias);
    fieldNum++;
  }

  // load the data into the message

  dsBeam.loadData(data, ndata * sizeof(ui16), sizeof(ui16));
  delete[] data;
   
  // write the message
  
  if (_rQueue.putDsMsg(_msg, DsRadarMsg::RADAR_BEAM)) {
    iret = -1;
  }

  if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }
  
  if (iret) {
    cerr << "ERROR - OutputFmq::writeBeams" << endl;
    cerr << "  Cannot put radar beam to queue" << endl;
  }

  return iret;

}

////////////////////////////////////////
// put volume flags

void OutputFmq::putStartOfVolume(int volNum, time_t time)
{
  _rQueue.putStartOfVolume(volNum, time);
}

void OutputFmq::putEndOfVolume(int volNum, time_t time)
{
  _rQueue.putEndOfVolume(volNum, time);
}

void OutputFmq::putStartOfTilt(int tiltNum, time_t time)
{
  _rQueue.putStartOfTilt(tiltNum, time);
}

void OutputFmq::putEndOfTilt(int tiltNum, time_t time)
{
  _rQueue.putEndOfTilt(tiltNum, time);
}

////////////////////////////////////////
// load up data for a double field

void OutputFmq::_loadDoubleField(const double *in, ui16 *out,
				 double scale, double bias)
				 
{

  if (in == NULL) {
    return;
  }

  for (int i = 0; i < _nGates; i++, out += _nFields, in++) {
    if (*in != _missingDbl) {
      int val = (int) ((*in - bias) / scale + 0.5);
      if (val < 1) {
        val = 1;
      }
      if (val > 65535) {
        val = 65535;
      }
      *out = val;
    }
  }
  
}

////////////////////////////////////////
// load up data for an integer field

void OutputFmq::_loadIntField(const int *in, ui16 *out)
  
{

  if (in == NULL) {
    return;
  }
  
  for (int i = 0; i < _nGates; i++, out += _nFields, in++) {
    if (*in != 0) {
      int val = *in - _flagBias;
      if (val < 0) {
	val = 0;
      }
      if (val > 65535) {
	val = 65535;
      }
      *out = val;
    }
  }

}
