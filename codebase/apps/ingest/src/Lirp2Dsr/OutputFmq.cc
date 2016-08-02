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
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/os_config.h>
#include <cstdio>
#include <string.h>
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
  _nSamples = 64;
  _prf = 999;
  _pulseWidth = 999;

  _doWrite = TRUE;

  if (_params.print_summary) {
    _doWrite = FALSE;
  }
  
  // initialize the output queue
  
  if (_doWrite) {
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
  }

  // set the scale and bias values

  _snrBias = -320.0;
  _snrScale = 0.01;
  
  _dbzBias = -320.0;
  _dbzScale = 0.01;

  _velBias = -320.0;
  _velScale = 0.01;

  _widthBias = -0.01;
  _widthScale = 0.001;

  _interestBias = -1.0;
  _interestScale = 0.0001;

  _angleBias = -640.0;
  _angleScale = 0.02;

  _kdpBias = -32.0;
  _kdpScale = 0.001;

  _zdrBias = -32.0;
  _zdrScale = 0.001;

  _rhohvBias = -1.0;
  _rhohvScale = 0.0001;
  
  _recBias = -0.01;
  _recScale = 0.001;
  
  _tdbzBias = -0.1;
  _tdbzScale = 0.1;
  
  _spinBias = -0.01;
  _spinScale = 0.01;
  
  _sdveBias = -0.01;
  _sdveScale = 0.0001;
  
  _fracBias = -0.01;
  _fracScale = 0.0001;
  
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

  double nyquistVel = ((_params.radar.wavelength / 100.0) * _prf) / 4.0;
  double unambigRange = (3.0e8 / (2.0 * _prf)) / 1000.0; 

  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;

  if (_params.output_snr) {
    DsFieldParams* fparams =
      new DsFieldParams("SNR", "dB", _snrScale, _snrBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_dbz_total) {
    DsFieldParams* fparams =
      new DsFieldParams("DBZT", "dBZ", _dbzScale, _dbzBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_dbz) {
    DsFieldParams* fparams =
      new DsFieldParams("DBZ", "dBZ", _dbzScale, _dbzBias, sizeof(ui16));
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

  if (_params.output_filtered) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("CLUT", "dB", _snrScale, _snrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("DBZF", "dBZ", _dbzScale, _dbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("VELF", "m/s", _velScale, _velBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("SPWF", "m/s", _widthScale, _widthBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }

  if (_params.output_infilled) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("DBZI", "dBZ", _dbzScale, _dbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("VELI", "m/s", _velScale, _velBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("SPWI", "m/s", _widthScale, _widthBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }

  if (_params.output_flags) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("FLAGS", "none", 1.0, _flagBias);
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("CENSOR", "none", 1.0, _flagBias);
      fp.push_back(fparams);
      _nFields++;
    }
  }

  if (_params.output_trip_flag) {
    DsFieldParams* fparams =
      new DsFieldParams("TRIP_FLAG", "none", 1.0, _flagBias);
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_infill_flags) {
    DsFieldParams* fparams =
      new DsFieldParams("ZINFILL", "none", 1.0, _flagBias);
    fp.push_back(fparams);
    _nFields++;
  }

  if (_params.output_censoring_debug) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("LEAKAGE", "none",
			  _interestScale, _interestBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("VTEXTURE", "none",
			  _interestScale, _interestBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("VSPIN", "none",
			  _interestScale, _interestBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("VCENSOR", "none", 1.0, _flagBias);
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("FCENSOR", "none",
			  _interestScale, _interestBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }

  if (_params.output_infill_debug) {
    DsFieldParams* fparams =
      new DsFieldParams("ZTEXTURE", "none",
			_interestScale, _interestBias, sizeof(ui16));
    fp.push_back(fparams);
    _nFields++;
  }
  
  if (_params.output_dual_pol_fields) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("PHIDP", "deg", _angleScale, _angleBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("KDP", "deg/km", _kdpScale, _kdpBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("ZDR", "dB", _zdrScale, _zdrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("RHOHV", "none", _rhohvScale, _rhohvBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }

  if (_params.output_rec) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("CMD", "none",
			  _recScale, _recBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("CMDFLAG", "none",
			  _recScale, _recBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }
  
  if (_params.output_rec_debug) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("ClutDbzNarrow", "dBZ",
                          _dbzScale, _dbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("TDBZ", "dbzSq", _tdbzScale, _tdbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("SqrtTDBZ", "dbz", _dbzScale, _dbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("SPIN", "%", _spinScale, _spinBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("WtSPIN", "%", _dbzScale, _dbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("SDVE", "m/s", _sdveScale, _sdveBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    if (_params.output_dual_pol_fields) {
      {
	DsFieldParams* fparams =
	  new DsFieldParams("SDZDR", "dB", _zdrScale, _zdrBias, sizeof(ui16));
	fp.push_back(fparams);
	_nFields++;
      }
      {
	DsFieldParams* fparams =
	  new DsFieldParams("SDRHOHV", "none",
			    _rhohvScale, _rhohvBias, sizeof(ui16));
	fp.push_back(fparams);
	_nFields++;
      }
    } // output_dual_pol_fields
  } // output_rec_debug

  if (_params.output_noise_fields) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("DBZN", "dBZ", _dbzScale, _dbzBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("SNRN", "dB", _snrScale, _snrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("NOISE", "dB", _snrScale, _snrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }
  
  if (_params.output_clutter_fields) {
    {
      DsFieldParams* fparams =
	new DsFieldParams("ClutRatioNarrow", "dB",
                          _snrScale, _snrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("ClutRatioWide", "dB",
                          _snrScale, _snrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("ClutWxPeakRatio", "dB",
                          _snrScale, _snrBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("ClutWxPeakSep", "fraction",
                          _fracScale, _fracBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
    {
      DsFieldParams* fparams =
	new DsFieldParams("PeakSepSdev", "fraction",
                          _fracScale, _fracBias, sizeof(ui16));
      fp.push_back(fparams);
      _nFields++;
    }
  }
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  // first get params from the parameter file
  
  rp.radarId = _params.radar.radar_id;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = _nGates;
  rp.samplesPerBeam = _nSamples;
  rp.scanType = _params.scan_type_id;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  switch (_params.radar.polarization) {
  case Params::HORIZONTAL:
    rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
    break;
  case Params::VERTICAL:
    rp.polarization = DS_POLARIZATION_VERT_TYPE;
    break;
  case Params::RIGHT_CIRCULAR:
    rp.polarization = DS_POLARIZATION_RIGHT_CIRC_TYPE;
    break;
  case Params::ELLIPTICAL:
    rp.polarization = DS_POLARIZATION_ELLIPTICAL_TYPE;
    break;
  case Params::LEFT_CIRCULAR:
    rp.polarization = DS_POLARIZATION_LEFT_CIRC_TYPE;
    break;
  case Params::DUAL:
    rp.polarization = DS_POLARIZATION_DUAL_TYPE;
    break;
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
  rp.wavelength = _params.radar.wavelength;
  
  rp.xmitPeakPower = _params.radar.xmit_peak_pwr;
  rp.receiverMds = _params.radar.receiver_mds;
  rp.receiverGain = _params.radar.receiver_gain;
  rp.antennaGain = _params.radar.antenna_gain;
  rp.systemGain = _params.radar.system_gain;

  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.radarName = _params.radar.radar_name;
  rp.scanTypeName = _params.scan_type_name;
  
  // write the message
  
  if (_doWrite) {
    if (_rQueue.putDsMsg(msg,
			 DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
      cerr << "ERROR - OutputFmq::writeParams" << endl;
      cerr << "  Cannot put params to queue" << endl;
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeBeam(const Beam *beam, int volNum)

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
  dsBeam.tiltNum = -1;
  dsBeam.elevation = beam->getEl();
  dsBeam.targetElev = beam->getEl();
  dsBeam.azimuth = beam->getAz();
  
  // data
  
  int fieldNum = 0;

  if (_params.output_snr) {
    _loadDoubleField(beam->getSnr(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;
  }

  if (_params.output_dbz_total) {
    _loadDoubleField(beam->getDbzt(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;
  }

  if (_params.output_dbz) {
    _loadDoubleField(beam->getDbz(), data + fieldNum,
		     _dbzScale, _dbzBias);
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

  if (_params.output_filtered) {

    _loadDoubleField(beam->getClut(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;

    _loadDoubleField(beam->getDbzf(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;

    _loadDoubleField(beam->getVelf(), data + fieldNum,
		     _velScale, _velBias);
    fieldNum++;

    _loadDoubleField(beam->getWidthf(), data + fieldNum,
		     _widthScale, _widthBias);
    fieldNum++;

  }

  if (_params.output_infilled) {

    _loadDoubleField(beam->getDbzi(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;

    _loadDoubleField(beam->getVeli(), data + fieldNum,
		     _velScale, _velBias);
    fieldNum++;

    _loadDoubleField(beam->getWidthi(), data + fieldNum,
		     _widthScale, _widthBias);
    fieldNum++;
  
  }

  if (_params.output_flags) {

    _loadIntField(beam->getFlags(), data + fieldNum);
    fieldNum++;

    _loadIntField(beam->getCensor(), data + fieldNum);
    fieldNum++;

  }

  if (_params.output_trip_flag) {
    _loadIntField(beam->getTripFlag(), data + fieldNum);
    fieldNum++;
  }

  if (_params.output_infill_flags) {
    _loadIntField(beam->getZinfill(), data + fieldNum);
    fieldNum++;
  }

  if (_params.output_censoring_debug) {

    _loadDoubleField(beam->getLeakage(), data + fieldNum,
		     _interestScale, _interestBias);
    fieldNum++;

    _loadDoubleField(beam->getVtexture(), data + fieldNum,
		     _interestScale, _interestBias);
    fieldNum++;

    _loadDoubleField(beam->getVspin(), data + fieldNum,
		     _interestScale, _interestBias);
    fieldNum++;

    _loadIntField(beam->getVcensor(), data + fieldNum);
    fieldNum++;
    
    _loadDoubleField(beam->getFcensor(), data + fieldNum,
		     _interestScale, _interestBias);
    fieldNum++;

  }

  if (_params.output_infill_debug) {
    _loadDoubleField(beam->getZtexture(), data + fieldNum,
		     _interestScale, _interestBias);
    fieldNum++;
  }
  
  if (_params.output_dual_pol_fields) {

    _loadDoubleField(beam->getPhidp(), data + fieldNum,
		     _angleScale, _angleBias);
    fieldNum++;

    _loadDoubleField(beam->getKdp(), data + fieldNum,
		     _kdpScale, _kdpBias);
    fieldNum++;

    _loadDoubleField(beam->getZdr(), data + fieldNum,
		     _zdrScale, _zdrBias);
    fieldNum++;

    _loadDoubleField(beam->getRhohv(), data + fieldNum,
		     _rhohvScale, _rhohvBias);
    fieldNum++;

  }

  if (_params.output_rec) {
    _loadDoubleField(beam->getRec(), data + fieldNum,
		     _recScale, _recBias);
    fieldNum++;
    _loadDoubleField(beam->getRecFlag(), data + fieldNum,
		     _recScale, _recBias);
    fieldNum++;
  }
  
  if (_params.output_rec_debug) {

    _loadDoubleField(beam->getRecClutDbzNarrow(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;

    _loadDoubleField(beam->getRecDbzTexture(), data + fieldNum,
		     _tdbzScale, _tdbzBias);
    fieldNum++;
    
    _loadDoubleField(beam->getRecSqrtTexture(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;
    
    _loadDoubleField(beam->getRecDbzSpin(), data + fieldNum,
		     _spinScale, _spinBias);
    fieldNum++;

    _loadDoubleField(beam->getRecWtSpin(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;

    _loadDoubleField(beam->getRecVelSdev(), data + fieldNum,
		     _sdveScale, _sdveBias);
    fieldNum++;

    if (_params.output_dual_pol_fields) {

      _loadDoubleField(beam->getRecZdrSdev(), data + fieldNum,
		       _zdrScale, _zdrBias);
      fieldNum++;

      _loadDoubleField(beam->getRecRhohvSdev(), data + fieldNum,
		       _rhohvScale, _rhohvBias);
      fieldNum++;
      
    } // if (_params.output_dual_pol_fields)

  } // if (_params.output_rec_debug)
  
  if (_params.output_noise_fields) {
    _loadDoubleField(beam->getDbzn(), data + fieldNum,
		     _dbzScale, _dbzBias);
    fieldNum++;
    _loadDoubleField(beam->getSnr(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;
    _loadDoubleField(beam->getNoiseDbm(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;
  }

  if (_params.output_clutter_fields) {
    _loadDoubleField(beam->getRecClutRatioNarrow(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;
    _loadDoubleField(beam->getRecClutRatioWide(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;
    _loadDoubleField(beam->getRecClutWxPeakRatio(), data + fieldNum,
		     _snrScale, _snrBias);
    fieldNum++;
    _loadDoubleField(beam->getRecClutWxPeakSep(), data + fieldNum,
		     _fracScale, _fracBias);
    fieldNum++;
    _loadDoubleField(beam->getRecClutWxPeakSepSdev(), data + fieldNum,
		     _fracScale, _fracBias);
    fieldNum++;

  }

  // load the data into the message

  dsBeam.loadData(data, ndata * sizeof(ui16), sizeof(ui16));
  delete[] data;
  
  // write the message
  
  if (_doWrite) {
    if (_rQueue.putDsMsg(_msg, DsRadarMsg::RADAR_BEAM)) {
      iret = -1;
    }
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
  if (_doWrite) {
    _rQueue.putStartOfVolume(volNum, time);
  }
}

void OutputFmq::putEndOfVolume(int volNum, time_t time)
{
  if (_doWrite) {
    _rQueue.putEndOfVolume(volNum, time);
  }
}

////////////////////////////////////////
// load up data for a double field

void OutputFmq::_loadDoubleField(const double *in, ui16 *out,
				 double scale, double bias)
				 
{

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
