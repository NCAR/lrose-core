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
// May 2006
//
///////////////////////////////////////////////////////////////
//
// OutputFmq puts the data out at FMQ.
//
////////////////////////////////////////////////////////////////

#define _in_OutputFmq_cc
#include "OutputFmq.hh"
#undef _in_OutputFmq_cc

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

  _snrBias = -320.0;
  _snrScale = 0.01;
  
  _dbmBias = -320.0;
  _dbmScale = 0.01;
  
  _dbzBias = -320.0;
  _dbzScale = 0.01;

  _velBias = -320.0;
  _velScale = 0.01;

  _widthBias = -0.01;
  _widthScale = 0.001;

  _percentBias = -100.0;
  _percentScale = 0.01;

  _interestBias = -1.0;
  _interestScale = 0.0001;

  _angleBias = -640.0;
  _angleScale = 0.02;

  _kdpBias = -32.0;
  _kdpScale = 0.001;

  _zdrBias = -32.0;
  _zdrScale = 0.001;

  _ldrBias = -32.0;
  _ldrScale = 0.001;

  _rhohvBias = -1.0;
  _rhohvScale = 0.0001;
  
  _cmdBias = -0.01;
  _cmdScale = 0.001;
  
  _tdbzBias = -0.1;
  _tdbzScale = 0.1;
  
  _spinBias = -0.1;
  _spinScale = 0.01;
  
  _sdveBias = -0.01;
  _sdveScale = 0.0001;
  
  _fracBias = -0.01;
  _fracScale = 0.0001;
  
  _testBias = -320.0;
  _testScale = 0.01;
  
  _flagBias = -1000;

  _prevAz = 0;

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

  double nyquistVel = ((_opsInfo.getWavelengthCm() / 100.0) * _prf) / 4.0;
  double unambigRange = (3.0e8 / (2.0 * _prf)) / 1000.0; 

  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;

  if (_params.output_fields.snr) {
    _addField("SNR", "dB", _snrScale, _snrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbm) {
    _addField("DBM", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbz) {
    _addField("DBZ", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.vel) {
    _addField("VEL", "m/s", _velScale, _velBias, fp);
    _nFields++;
  }

  if (_params.output_fields.width) {
    _addField("WIDTH", "m/s", _widthScale, _widthBias, fp);
    _nFields++;
  }

  if (_params.output_fields.clut) {
    _addField("CLUT", "dB", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbzf) {
    _addField("DBZF", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.velf) {
    _addField("VELF", "m/s", _velScale, _velBias, fp);
    _nFields++;
  }

  if (_params.output_fields.widthf) {
    _addField("WIDTHF", "m/s", _widthScale, _widthBias, fp);
    _nFields++;
   }
  
  if (_params.output_fields.zdr) {
    _addField("ZDR", "dB", _zdrScale, _zdrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.zdrm) {
    _addField("ZDRM", "dB", _zdrScale, _zdrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.ldrh) {
    _addField("LDRH", "dB", _ldrScale, _ldrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.ldrv) {
    _addField("LDRV", "dB", _ldrScale, _ldrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.rhohv) {
    _addField("RHOHV", "none", _rhohvScale, _rhohvBias, fp);
    _nFields++;
  }

  if (_params.output_fields.phidp) {
    _addField("PHIDP", "deg", _angleScale, _angleBias, fp);
    _nFields++;
  }

  if (_params.output_fields.kdp) {
    _addField("KDP", "deg/km", _kdpScale, _kdpBias, fp);
    _nFields++;
  }

  if (_params.output_fields.snrhc) {
    _addField("SNRHC", "dB", _snrScale, _snrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.snrhx) {
    _addField("SNRHX", "dB", _snrScale, _snrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.snrvc) {
    _addField("SNRVC", "dB", _snrScale, _snrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.snrvx) {
    _addField("SNRVX", "dB", _snrScale, _snrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbmhc) {
    _addField("DBMHC", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbmhx) {
    _addField("DBMHX", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbmvc) {
    _addField("DBMVC", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbmvx) {
    _addField("DBMVX", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbzhc) {
    _addField("DBZHC", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbzhx) {
    _addField("DBZHX", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbzvc) {
    _addField("DBZVC", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.dbzvx) {
    _addField("DBZVX", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.sz_trip_flag) {
    _addField("TRIP_FLAG", "none", 1.0, _flagBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.sz_leakage) {
    _addField("LEAKAGE", "none", _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.sz_dbzt) {
    _addField("SZ_DBZT", "dbz", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.sz_zinfill) {
    _addField("SZ_ZINFILL", "none", 1.0, _flagBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.sz_itexture) {
    _addField("SZ_ZTEXTURE", "none", _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.sz_dbzi) {
    _addField("SZ_DBZI", "dBZ", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.sz_veli) {
    _addField("SZ_VELI", "m/s", _velScale, _velBias, fp);
    _nFields++;
  }

  if (_params.output_fields.sz_widthi) {
    _addField("SZ_WIDTHI", "m/s", _widthScale, _widthBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.cmd) {
    _addField("CMD", "none", _cmdScale, _cmdBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_flag) {
    _addField("CMD_FLAG", "none", 1.0, _flagBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_dbz_diff_sq) {
    _addField("CMD_DBZ_DIFF_SQ", "dbz2",  _tdbzScale, _tdbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_spin_change) {
    _addField("CMD_SPIN_CHANGE", "none",  _percentScale, _percentBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_tdbz) {
    _addField("CMD_TDBZ", "dbzSq", _tdbzScale, _tdbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_sqrt_tdbz) {
    _addField("CMD_SQRT_TDBZ", "dbz", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_spin) {
    _addField("CMD_SPIN", "%", _spinScale, _spinBias, fp);
    _nFields++;
  }

  if (_params.output_fields.interest_min_tdbz_spin) {
    _addField("INTEREST_MIN_TDBZ_SPIN", "%",
              _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.interest_max_tdbz_spin) {
    _addField("INTEREST_MAX_TDBZ_SPIN", "%",
              _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_vel_sdev) {
    _addField("CMD_SDVE", "m/s", _sdveScale, _sdveBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_power_ratio) {
    _addField("CMD_POWER_RATIO", "", _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_dbz_narrow) {
    _addField("CMD_DBZ_NARROW", "dbz", _dbzScale, _dbzBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_ratio_narrow) {
    _addField("CMD_RATIO_NARROW", "dB", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_pr_narrow) {
    _addField("CMD_PR_NARROW", "", _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_ratio_wide) {
    _addField("CMD_RATIO_WIDE", "dB", _dbmScale, _dbmBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_pr_wide) {
    _addField("CMD_PR_WIDE", "", _interestScale, _interestBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_clut2wx_sep) {
    _addField("CMD_CLUT2WX_SEP", "fraction", _fracScale, _fracBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.cmd_clut2wx_ratio) {
    _addField("CMD_CLUT2WX_RATIO", "dB", _dbmScale, _dbmBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.cmd_clut_width) {
    _addField("CMD_CLUT_WIDTH", "m/s", _widthScale, _widthBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.interest_max_clut_width_sep) {
    _addField("INTEREST_MAX_CLUT_WIDTH_SEP", "",
              _interestScale, _interestBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.cmd_wx2noise_ratio) {
    _addField("CMD_WX2NOISE_RATIO", "dB", _dbmScale, _dbmBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.cmd_zdr_sdev) {
    _addField("CMD_ZDR_SDEV", "dB", _zdrScale, _zdrBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_rhohv_sdev) {
    _addField("CMD_RHOHV_SDEV", "none", _rhohvScale, _rhohvBias, fp);
    _nFields++;
  }

  if (_params.output_fields.cmd_phidp_sdev) {
    _addField("CMD_PHIDP_SDEV", "none", _angleScale, _angleBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.cpa) {
    _addField("CPA", "", _interestScale, _interestBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.aiq) {
    _addField("AIQ", "deg", _angleScale, _angleBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.niq) {
    _addField("NIQ", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.meani) {
    _addField("MEANI", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.meanq) {
    _addField("MEANQ", "dBm", _dbmScale, _dbmBias, fp);
    _nFields++;
  }
  
  if (_params.output_fields.test) {
    _addField("TEST", "none", _testScale, _testBias, fp);
    _nFields++;
  }
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
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
  rp.wavelength = _opsInfo.getWavelengthCm();
  
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

  double deltaAz = fabs(beam->getAz() - _prevAz);
  if (deltaAz > 180) {
    deltaAz -= 360;
  }

  if (_params.debug) {
    if (_prevAz != 0.0 && fabs(deltaAz) > 1.5) {
      cerr << "WARNING - skipping beams from "
	   << _prevAz << " to " << beam->getAz() << endl;
    }
  }

  _prevAz = beam->getAz();

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
  
  // Load up data on a gate-by-gate basis.
  // There are multiple fields for each gate

  ui16 *dp = data;
  for (int igate = 0; igate < _nGates; igate++) {

    const Fields &fields = beam->getFields()[igate];
  
    if (_params.output_fields.snr) {
      *dp = _convertDouble(fields.snr, _snrScale, _snrBias);
      dp++;
    }

    if (_params.output_fields.dbm) {
      *dp = _convertDouble(fields.dbm, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.dbz) {
      *dp = _convertDouble(fields.dbz, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.vel) {
      *dp = _convertDouble(fields.vel, _velScale, _velBias);
      dp++;
    }

    if (_params.output_fields.width) {
      *dp = _convertDouble(fields.width, _widthScale, _widthBias);
      dp++;
    }

    if (_params.output_fields.clut) {
      *dp = _convertDouble(fields.clut, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.dbzf) {
      *dp = _convertDouble(fields.dbzf, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.velf) {
      *dp = _convertDouble(fields.velf, _velScale, _velBias);
      dp++;
    }

    if (_params.output_fields.widthf) {
      *dp = _convertDouble(fields.widthf, _widthScale, _widthBias);
      dp++;
    }

    if (_params.output_fields.zdr) {
      *dp = _convertDouble(fields.zdr, _zdrScale, _zdrBias);
      dp++;
    }

    if (_params.output_fields.zdrm) {
      *dp = _convertDouble(fields.zdrm, _zdrScale, _zdrBias);
      dp++;
    }

    if (_params.output_fields.ldrh) {
      *dp = _convertDouble(fields.ldrh, _ldrScale, _ldrBias);
      dp++;
    }

    if (_params.output_fields.ldrv) {
      *dp = _convertDouble(fields.ldrv, _ldrScale, _ldrBias);
      dp++;
    }

    if (_params.output_fields.rhohv) {
      *dp = _convertDouble(fields.rhohv, _rhohvScale, _rhohvBias);
      dp++;
    }

    if (_params.output_fields.phidp) {
      *dp = _convertDouble(fields.phidp, _angleScale, _angleBias);
      dp++;
    }

    if (_params.output_fields.kdp) {
      *dp = _convertDouble(fields.kdp, _kdpScale, _kdpBias);
      dp++;
    }

    if (_params.output_fields.snrhc) {
      *dp = _convertDouble(fields.snrhc, _snrScale, _snrBias);
      dp++;
    }

    if (_params.output_fields.snrhx) {
      *dp = _convertDouble(fields.snrhx, _snrScale, _snrBias);
      dp++;
    }

    if (_params.output_fields.snrvc) {
      *dp = _convertDouble(fields.snrvc, _snrScale, _snrBias);
      dp++;
    }

    if (_params.output_fields.snrvx) {
      *dp = _convertDouble(fields.snrvx, _snrScale, _snrBias);
      dp++;
    }

    if (_params.output_fields.dbmhc) {
      *dp = _convertDouble(fields.dbmhc, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.dbmhx) {
      *dp = _convertDouble(fields.dbmhx, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.dbmvc) {
      *dp = _convertDouble(fields.dbmvc, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.dbmvx) {
      *dp = _convertDouble(fields.dbmvx, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.dbzhc) {
      *dp = _convertDouble(fields.dbzhc, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.dbzhx) {
      *dp = _convertDouble(fields.dbzhx, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.dbzvc) {
      *dp = _convertDouble(fields.dbzvc, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.dbzvx) {
      *dp = _convertDouble(fields.dbzvx, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.sz_trip_flag) {
      *dp = _convertInt(fields.sz_trip_flag, _flagBias);
      dp++;
    }
  
    if (_params.output_fields.sz_leakage) {
      *dp = _convertDouble(fields.sz_leakage, _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.sz_dbzt) {
      *dp = _convertDouble(fields.sz_dbzt, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.sz_zinfill) {
      *dp = _convertInt(fields.sz_zinfill, _flagBias);
      dp++;
    }

    if (_params.output_fields.sz_itexture) {
      *dp = _convertDouble(fields.sz_itexture, _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.sz_dbzi) {
      *dp = _convertDouble(fields.sz_dbzi, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.sz_veli) {
      *dp = _convertDouble(fields.sz_veli, _velScale, _velBias);
      dp++;
    }

    if (_params.output_fields.sz_widthi) {
      *dp = _convertDouble(fields.sz_widthi, _widthScale, _widthBias);
      dp++;
    }

    if (_params.output_fields.cmd) {
      *dp = _convertDouble(fields.cmd, _cmdScale, _cmdBias);
      dp++;
    }
  
    if (_params.output_fields.cmd_flag) {
      *dp = _convertInt(fields.cmd_flag, _flagBias);
      dp++;
    }

    if (_params.output_fields.cmd_dbz_diff_sq) {
      *dp = _convertDouble(fields.cmd_dbz_diff_sq, _tdbzScale, _tdbzBias);
      dp++;
    }

    if (_params.output_fields.cmd_spin_change) {
      *dp = _convertDouble(fields.cmd_spin_change, _percentScale, _percentBias);
      dp++;
    }

    if (_params.output_fields.cmd_tdbz) {
      *dp = _convertDouble(fields.cmd_tdbz, _tdbzScale, _tdbzBias);
      dp++;
    }

    if (_params.output_fields.cmd_sqrt_tdbz) {
      *dp = _convertDouble(fields.cmd_sqrt_tdbz, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.cmd_spin) {
      *dp = _convertDouble(fields.cmd_spin, _spinScale, _spinBias);
      dp++;
    }

    if (_params.output_fields.interest_min_tdbz_spin) {
      *dp = _convertDouble(fields.interest_min_tdbz_spin,
                           _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.interest_max_tdbz_spin) {
      *dp = _convertDouble(fields.interest_max_tdbz_spin,
                           _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.cmd_vel_sdev) {
      *dp = _convertDouble(fields.cmd_vel_sdev, _sdveScale, _sdveBias);
      dp++;
    }
  
    if (_params.output_fields.cmd_power_ratio) {
      *dp = _convertDouble(fields.cmd_power_ratio,
                           _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.cmd_dbz_narrow) {
      *dp = _convertDouble(fields.cmd_dbz_narrow, _dbzScale, _dbzBias);
      dp++;
    }

    if (_params.output_fields.cmd_ratio_narrow) {
      *dp = _convertDouble(fields.cmd_ratio_narrow, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.cmd_pr_narrow) {
      *dp = _convertDouble(fields.cmd_pr_narrow,
                           _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.cmd_ratio_wide) {
      *dp = _convertDouble(fields.cmd_ratio_wide, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.cmd_pr_wide) {
      *dp = _convertDouble(fields.cmd_pr_wide,
                           _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.cmd_clut2wx_sep) {
      *dp = _convertDouble(fields.cmd_clut2wx_sep, _fracScale, _fracBias);
      dp++;
    }

    if (_params.output_fields.cmd_clut2wx_ratio) {
      *dp = _convertDouble(fields.cmd_clut2wx_ratio, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.cmd_clut_width) {
      *dp = _convertDouble(fields.cmd_clut_width, _widthScale, _widthBias);
      dp++;
    }

    if (_params.output_fields.interest_max_clut_width_sep) {
      *dp = _convertDouble(fields.interest_max_clut_width_sep,
                           _interestScale, _interestBias);
      dp++;
    }

    if (_params.output_fields.cmd_wx2noise_ratio) {
      *dp = _convertDouble(fields.cmd_wx2noise_ratio, _dbmScale, _dbmBias);
      dp++;
    }

    if (_params.output_fields.cmd_zdr_sdev) {
      *dp = _convertDouble(fields.cmd_zdr_sdev, _zdrScale, _zdrBias);
      dp++;
    }

    if (_params.output_fields.cmd_rhohv_sdev) {
      *dp = _convertDouble(fields.cmd_rhohv_sdev, _rhohvScale, _rhohvBias);
      dp++;
    }

    if (_params.output_fields.cmd_phidp_sdev) {
      *dp = _convertDouble(fields.cmd_phidp_sdev, _angleScale, _angleBias);
      dp++;
    }

    if (_params.output_fields.cpa) {
      *dp = _convertDouble(fields.cpa, _interestScale, _interestBias);
      dp++;
    }
    
    if (_params.output_fields.aiq) {
      *dp = _convertDouble(fields.aiq, _angleScale, _angleBias);
      dp++;
    }
    
    if (_params.output_fields.niq) {
      *dp = _convertDouble(fields.niq, _dbmScale, _dbmBias);
      dp++;
    }
    
    if (_params.output_fields.meani) {
      *dp = _convertDouble(fields.meani, _dbmScale, _dbmBias);
      dp++;
    }
    
    if (_params.output_fields.meanq) {
      *dp = _convertDouble(fields.meanq, _dbmScale, _dbmBias);
      dp++;
    }
  
    if (_params.output_fields.test) {
      *dp = _convertDouble(fields.test, _testScale, _testBias);
      dp++;
    }

  } // igate

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

