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

#include "Calibration.hh"

#include <radar/IwrfTsInfo.hh>
#include <radar/RadarCalib.hh>
#include <cmath>
#include <toolsa/uusleep.h>
#include <dsserver/DmapAccess.hh>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

OutputFmq::OutputFmq(const string &prog_name,
		     const Params &params) :
  _progName(prog_name),
  _params(params)
 
{
  
  constructorOK = TRUE;
  _nFields = 0;
  _nBeamsWritten = 0;
  _rQueue = NULL;

  // initialize the output queue

  if (_openFmq()) {
    constructorOK = FALSE;
    return;
  }

  // initialize the busy mutex

  pthread_mutex_init(&_busy, NULL);

}

// destructor

OutputFmq::~OutputFmq()
  
{

  if (_rQueue != NULL) {
    delete _rQueue;
  }

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeParams(const Beam &beam)

{

  // get a lock on the busy mutex

  pthread_mutex_lock(&_busy);

  // initialize 

  const IwrfTsInfo &opsInfo = beam.getOpsInfo();
  const IwrfCalib &calib = beam.getCalib();
  iwrf_xmit_rcv_mode_t xmitRcvMode = beam.getXmitRcvMode();
                           
  int nGatesOut = beam.getNGatesOut();
  int nSamples = beam.getNSamplesEffective();
  double pulseWidthUs = beam.getPulseWidth() * 1.0e6;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::writeParams, nGates: " << nGatesOut << endl;
  }
  
  // compute nyquist and unambig range

  double nyquistVel = beam.getNyquist();
  double unambigRange = beam.getMaxRange();

  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &field = _params._output_fields[ii];

    if (field.write_unfiltered) {
      _addField(field.name, field.units, field.scale, field.bias, fp);
      _nFields++;
    }
    
    if (field.write_filtered) {
      string filteredName = field.name;
      filteredName += "_F";
      _addField(filteredName, field.units, field.scale, field.bias, fp);
      _nFields++;
    }
    
  } // ii
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = nGatesOut;
  rp.samplesPerBeam = nSamples;
  rp.scanType = opsInfo.get_scan_mode();
  rp.scanMode = beam.getScanMode();
  rp.followMode = beam.getFollowMode();

  switch(xmitRcvMode) {
    case IWRF_SINGLE_POL:
      rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
      break;
    case IWRF_ALT_HV_CO_ONLY:
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_HV_ALT;
      break;
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_HV_SIM;
      break;
    case IWRF_H_ONLY_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_H_XMIT;
      break;
    case IWRF_V_ONLY_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_V_XMIT;
      break;
    default: {
      rp.polarization = DS_POLARIZATION_DUAL_TYPE;
    }
  }

  if (beam.getIsStagPrt()) {
    if (beam.getStagM() == 2) {
      rp.prfMode = DS_RADAR_PRF_MODE_STAGGERED_2_3;
    } else if (beam.getStagM() == 3) {
      rp.prfMode = DS_RADAR_PRF_MODE_STAGGERED_3_4;
    } else if (beam.getStagM() == 4) {
      rp.prfMode = DS_RADAR_PRF_MODE_STAGGERED_4_5;
    }
  }
  
  rp.radarConstant = calib.getRadarConstH();

  rp.altitude = opsInfo.get_radar_altitude_m() / 1000.0;
  rp.latitude = opsInfo.get_radar_latitude_deg();
  rp.longitude = opsInfo.get_radar_longitude_deg();

  // override if georefs active

  if (beam.getGeorefActive()) {
    const iwrf_platform_georef_t &georef = beam.getGeoref();
    rp.latitude = georef.latitude;
    rp.longitude = georef.longitude;
    rp.altitude = georef.altitude_msl_km;
  }

  rp.gateSpacing = opsInfo.get_proc_gate_spacing_km();
  rp.startRange = opsInfo.get_proc_start_range_km();

  rp.horizBeamWidth = calib.getBeamWidthDegH();
  rp.vertBeamWidth = calib.getBeamWidthDegV();
  
  rp.pulseWidth = pulseWidthUs;
  rp.pulseRepFreq = 1.0 / beam.getPrt();
  rp.prt = beam.getPrt();
  rp.prt2 = beam.getPrtLong();
  rp.wavelength = opsInfo.get_radar_wavelength_cm();
  
  rp.xmitPeakPower = pow(10.0, calib.getXmitPowerDbmH() / 10.0);
  rp.receiverGain = calib.getReceiverGainDbHc();
  rp.receiverMds = calib.getNoiseDbmHc() - rp.receiverGain;
  rp.antennaGain = calib.getAntGainDbH();
  rp.systemGain =  rp.receiverGain + rp.antennaGain;

  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.measXmitPowerDbmH = beam.getMeasXmitPowerDbmH();
  rp.measXmitPowerDbmV = beam.getMeasXmitPowerDbmV();

  rp.radarName = opsInfo.get_radar_name();
  rp.scanTypeName = opsInfo.get_scan_segment_name();
  
  // write the message
  
  if (_rQueue->putDsMsg(msg,
                       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - OutputFmq::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      pthread_mutex_unlock(&_busy);
      return -1;
    }
  }
  
  pthread_mutex_unlock(&_busy);
  return 0;

}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeCalib(const Beam &beam)

{

  // get a lock on the busy mutex

  pthread_mutex_lock(&_busy);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::writeCalib" << endl;
  }
  
  // create DsRadar message
  
  DsRadarMsg msg;

  // set calib in message to the beam calibration
  
  DsRadarCalib &calib = msg.getRadarCalib();
  IwrfCalib icalib = beam.getCalib();
  RadarCalib::copyIwrfToDsRadar(icalib, calib);

  // write the message
  
  if (_rQueue->putDsMsg(msg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - OutputFmq::writeCalib" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      pthread_mutex_unlock(&_busy);
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    string xml;
    beam.getCalib().convert2Xml(xml);
    cerr << "Writing out calibration:" << endl;
    cerr << xml;
  }
  
  pthread_mutex_unlock(&_busy);
  return 0;

}

////////////////////////////////////////
// Write status XML to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeStatusXml(const Beam &beam)

{

  // get a lock on the busy mutex

  pthread_mutex_lock(&_busy);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::writeStatusXml" << endl;
  }
  
  // create DsRadar message
  
  DsRadarMsg msg;
  msg.setStatusXml(beam.getStatusXml());
  
  // write the message
  
  if (_rQueue->putDsMsg(msg, DsRadarMsg::STATUS_XML)) {
    cerr << "ERROR - OutputFmq::writeCalib" << endl;
    cerr << "  Cannot put status XML to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      pthread_mutex_unlock(&_busy);
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Writing out status xml: " << endl;
    cerr << beam.getStatusXml() << endl;
  }
  
  pthread_mutex_unlock(&_busy);
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int OutputFmq::writeBeam(const Beam &beam)

{

  // get a lock on the busy mutex

  pthread_mutex_lock(&_busy);
  
  bool printBeamDebug = (_params.debug >= Params::DEBUG_VERBOSE);
  if (_params.debug &&
      ((_nBeamsWritten % _params.beam_count_for_debug_print) == 0)) {
    printBeamDebug = true;
  }
  if (printBeamDebug) {
    DateTime beamTime(beam.getTimeSecs(), true, beam.getNanoSecs() / 1.0e9);
    time_t now = time(NULL);
    int lateSecs = now - beam.getTimeSecs();
    fprintf(stderr,
            "-->> OutputFmq::writeBeam %s (late %d) - %s, vol: %.3d, "
            "sweep: %.3d, "
            "az: %6.3f, el %5.3f, nSamples: %3d\n",
            beamTime.asString(3).c_str(), lateSecs, 
            iwrf_scan_mode_to_short_str(beam.getScanMode()).c_str(), 
            beam.getVolNum(), beam.getSweepNum(),
            beam.getAz(), beam.getEl(),
            beam.getNSamplesEffective());
  }

  // put georeference if applicable

  int contents = 0;
  if (beam.getGeorefActive()) {
    DsPlatformGeoref &georef = _msg.getPlatformGeoref();
    ds_iwrf_platform_georef_t dsGeoref;
    // copy struct from iwrf to DsRadar
    // these are identical, stored in different libs
    // TODO - bring DsRadar into libs/radar
    memcpy(&dsGeoref, &beam.getGeoref(), sizeof(dsGeoref));
    georef.setGeoref(dsGeoref);
    contents |= DsRadarMsg::PLATFORM_GEOREF;
  }
    
  int nGatesOut = beam.getNGatesOut();
  
  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  int ndata = nGatesOut * _nFields;
  ui16 *data = new ui16[ndata];
  memset(data, 0, ndata * sizeof(ui16));
  
  // params
  
  dsBeam.dataTime = beam.getTimeSecs();
  double dtime = beam.getDoubleTime();
  double partialSecs = fmod(dtime, 1.0);
  dsBeam.nanoSecs = (int) (partialSecs * 1.0e9 + 0.5);
  dsBeam.volumeNum = beam.getVolNum();
  dsBeam.tiltNum = beam.getSweepNum();
  dsBeam.elevation = beam.getEl();
  dsBeam.azimuth = beam.getAz();
  dsBeam.targetElev = beam.getTargetEl();
  dsBeam.targetAz = beam.getTargetAz();
  dsBeam.antennaTransition = beam.getAntennaTransition();
  dsBeam.scanMode = beam.getScanMode();
  dsBeam.beamIsIndexed = beam.getBeamIsIndexed();
  dsBeam.angularResolution = beam.getAngularResolution();
  dsBeam.nSamples = beam.getNSamplesEffective();
  dsBeam.measXmitPowerDbmH = beam.getMeasXmitPowerDbmH();
  dsBeam.measXmitPowerDbmV = beam.getMeasXmitPowerDbmV();

  // load up vector of field offsets

  vector<int> offsets;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    int offset = _findFieldOffset(field.id);
    offsets.push_back(offset);
  }

  // Load up data on a gate-by-gate basis.
  // There are multiple fields for each gate

  const MomentsFields *fieldsArray = beam.getFields();
  const MomentsFields *fieldsFArray = beam.getFieldsF();
  ui16 *dp = data;
  for (int igate = 0; igate < nGatesOut; igate++) {

    const MomentsFields &fields = fieldsArray[igate];
    const MomentsFields &fieldsF = fieldsFArray[igate];

    const double *start = &fields.start;
    const double *startF = &fieldsF.start;
  
    for (int ii = 0; ii < _params.output_fields_n; ii++) {
      
      const Params::output_field_t &field = _params._output_fields[ii];
      int offset = offsets[ii];
      
      if (field.write_unfiltered) {
        double val = *(start + offset);
        *dp = _convertDouble(val, field.scale, field.bias);
        dp++;
      }
      
      if (field.write_filtered) {
        double val = *(startF + offset);
        *dp = _convertDouble(val, field.scale, field.bias);
        dp++;
      }
      
    } // ii

  } // igate

  // load the data into the message

  dsBeam.loadData(data, ndata * sizeof(ui16), sizeof(ui16));
  delete[] data;
  
  if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }
  
  // write the message
  
  contents |= DsRadarMsg::RADAR_BEAM;
  if (_rQueue->putDsMsg(_msg, contents)) {
    cerr << "ERROR - OutputFmq::writeBeams" << endl;
    cerr << "  Cannot put radar beam to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      pthread_mutex_unlock(&_busy);
      return -1;
    }
  }

  _nBeamsWritten++;
  pthread_mutex_unlock(&_busy);
  return 0;

}

////////////////////////////////////////
// put volume flags

void OutputFmq::putStartOfVolume(int volNum, time_t time)
{
  pthread_mutex_lock(&_busy);
  _rQueue->putStartOfVolume(volNum, time);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putEndOfVolume(int volNum, time_t time)
{
  pthread_mutex_lock(&_busy);
  _rQueue->putEndOfVolume(volNum, time);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putStartOfTilt(int tiltNum, time_t time)
{
  pthread_mutex_lock(&_busy);
  _rQueue->putStartOfTilt(tiltNum, time);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putEndOfTilt(int tiltNum, time_t time)
{
  pthread_mutex_lock(&_busy);
  _rQueue->putEndOfTilt(tiltNum, time);
  pthread_mutex_unlock(&_busy);
}

void OutputFmq::putNewScanType(int scanType, time_t time)
{
  pthread_mutex_lock(&_busy);
  _rQueue->putNewScanType(scanType, time);
  pthread_mutex_unlock(&_busy);
}

////////////////////////////////////////
// open the FMQ

int OutputFmq::_openFmq()
  
{

  if (_rQueue != NULL) {
    delete _rQueue;
  }

  _rQueue = new DsRadarQueue();

  // initialize the output queue
  
  if (_rQueue->init(_params.output_fmq_url,
		    _progName.c_str(),
		    _params.debug >= Params::DEBUG_VERBOSE,
		    DsFmq::READ_WRITE, DsFmq::END,
		    _params.output_fmq_compress,
		    _params.output_fmq_nslots,
		    _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::OutputFmq" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _rQueue->setCompressionMethod(TA_COMPRESSION_ZLIB);
  }
  if (_params.write_blocking) {
    _rQueue->setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _rQueue->setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  _rQueue->setSingleWriter();

  return 0;

}

////////////////////////////////////////////////////////////////////
// get the offset for a given field ID
// This is the offset of the data element in the MomentsFields object
// realtive to the ncp field.

int OutputFmq::_findFieldOffset(Params::field_id_t fieldId)

{

  const double *start = &_flds.start;

  switch (fieldId) {
    
    // reflectivity
      
    case Params::DBZ:
      return (&_flds.dbz - start);
    case Params::DBZ_NO_ATMOS_ATTEN:
      return (&_flds.dbz_no_atmos_atten - start);
    case Params::DBZHC:
      return (&_flds.dbzhc - start);
    case Params::DBZVC:
      return (&_flds.dbzvc - start);
    case Params::DBZHX:
      return (&_flds.dbzhx - start);
    case Params::DBZVX:
      return (&_flds.dbzvx - start);

      // velocity

    case Params::VEL:
      return (&_flds.vel - start);
    case Params::VEL_ALT:
      return (&_flds.vel_alt - start);
    case Params::VEL_HV:
      return (&_flds.vel_hv - start);
    case Params::VEL_H_ONLY:
      return (&_flds.vel_h_only - start);
    case Params::VEL_V_ONLY:
      return (&_flds.vel_v_only - start);
    case Params::VEL_ALT_FOLD_INTERVAL:
      return (&_flds.vel_alt_fold_interval - start);
    case Params::VEL_ALT_FOLD_CONFIDENCE:
      return (&_flds.vel_alt_fold_confidence - start);
    case Params::VEL_CORRECTED:
      return (&_flds.vel_corrected - start);
      
      // STAGGERED PRT VEL
      
    case Params::VEL_PRT_SHORT:
      return (&_flds.vel_prt_short - start);
    case Params::VEL_PRT_LONG:
      return (&_flds.vel_prt_long - start);
    case Params::VEL_DIFF:
      return (&_flds.vel_diff - start);
    case Params::VEL_UNFOLD_INTERVAL:
      return (&_flds.vel_unfold_interval - start);

      // width

    case Params::WIDTH:
      return (&_flds.width - start);
    case Params::WIDTH_R0R1:
      return (&_flds.width_r0r1 - start);
    case Params::WIDTH_R1R2:
      return (&_flds.width_r1r2 - start);
    case Params::WIDTH_R1R3:
      return (&_flds.width_r1r3 - start);
    case Params::WIDTH_PPLS:
      return (&_flds.width_ppls - start);
    case Params::WIDTH_H_ONLY:
      return (&_flds.width_h_only - start);
    case Params::WIDTH_V_ONLY:
      return (&_flds.width_v_only - start);
    case Params::WIDTH_PRT_LONG:
      return (&_flds.width_prt_long - start);
    case Params::WIDTH_PRT_SHORT:
      return (&_flds.width_prt_short - start);

      // normalized coherent power
      
    case Params::NCP:
      return (&_flds.ncp - start);
    case Params::NCP_H_ONLY:
      return (&_flds.ncp_h_only - start);
    case Params::NCP_V_ONLY:
      return (&_flds.ncp_v_only - start);
    case Params::NCP_H_MINUS_V:
      return (&_flds.ncp_h_minus_v - start);
    case Params::NCP_TRIP1:
      return (&_flds.ncp_trip1 - start);
    case Params::NCP_TRIP2:
      return (&_flds.ncp_trip2 - start);
    case Params::NCP_TRIP3:
      return (&_flds.ncp_trip3 - start);
    case Params::NCP_TRIP4:
      return (&_flds.ncp_trip4 - start);
    case Params::NCP_PRT_SHORT:
      return (&_flds.ncp_prt_short - start);
    case Params::NCP_PRT_LONG:
      return (&_flds.ncp_prt_long - start);
    case Params::NCP_TRIP_FLAG:
      return (&_flds.ncp_trip_flag - start);

      // NOISE BIAS RELATIVE TO CALIBRATION

    case Params::NOISE_BIAS_DB_HC:
      return (&_flds.noise_bias_db_hc - start);
    case Params::NOISE_BIAS_DB_HX:
      return (&_flds.noise_bias_db_hx - start);
    case Params::NOISE_BIAS_DB_VC:
      return (&_flds.noise_bias_db_vc - start);
    case Params::NOISE_BIAS_DB_VX:
      return (&_flds.noise_bias_db_vx - start);

      // SIGNAL-TO-NOISE RATIO

    case Params::SNR:
      return (&_flds.snr - start);
    case Params::SNRHC:
      return (&_flds.snrhc - start);
    case Params::SNRHX:
      return (&_flds.snrhx - start);
    case Params::SNRVC:
      return (&_flds.snrvc - start);
    case Params::SNRVX:
      return (&_flds.snrvx - start);

      // UNCALIBRATED POWER
      
    case Params::DBM:
      return (&_flds.dbm - start);
    case Params::DBMHC:
      return (&_flds.dbmhc - start);
    case Params::DBMHX:
      return (&_flds.dbmhx - start);
    case Params::DBMVC:
      return (&_flds.dbmvc - start);
    case Params::DBMVX:
      return (&_flds.dbmvx - start);

      // NOISE SUBTRACTED POWER

    case Params::DBMHC_NS:
      return (&_flds.dbmhc_ns - start);
    case Params::DBMHX_NS:
      return (&_flds.dbmhx_ns - start);
    case Params::DBMVC_NS:
      return (&_flds.dbmvc_ns - start);
    case Params::DBMVX_NS:
      return (&_flds.dbmvx_ns - start);
      
      // DUAL POL

    case Params::ZDRM:
      return (&_flds.zdrm - start);
    case Params::ZDR:
      return (&_flds.zdr - start);
    case Params::ZDR_BIAS:
      return (&_flds.zdr_bias - start);
    case Params::LDR:
      return (&_flds.ldr - start);
    case Params::LDRHM:
      return (&_flds.ldrhm - start);
    case Params::LDRH:
      return (&_flds.ldrh - start);
    case Params::LDRVM:
      return (&_flds.ldrvm - start);
    case Params::LDRV:
      return (&_flds.ldrv - start);
    case Params::LDR_DIFF:
      return (&_flds.ldr_diff - start);
    case Params::LDR_MEAN:
      return (&_flds.ldr_mean - start);
    case Params::RHOHV:
      return (&_flds.rhohv - start);
    case Params::RHOHV_NNC:
      return (&_flds.rhohv_nnc - start);
    case Params::PHIDP0:
      return (&_flds.phidp0 - start);
    case Params::PHIDP:
      return (&_flds.phidp - start);
    case Params::PHIDP_COND:
      return (&_flds.phidp_cond - start);
    case Params::PHIDP_FILT:
      return (&_flds.phidp_filt - start);
    case Params::PHIDP_SDEV_4KDP:
      return (&_flds.phidp_sdev_4kdp - start);
    case Params::PHIDP_JITTER_4KDP:
      return (&_flds.phidp_jitter_4kdp - start);
    case Params::ZDR_SDEV_4KDP:
      return (&_flds.zdr_sdev_4kdp - start);
    case Params::KDP:
      return (&_flds.kdp - start);
    case Params::PSOB:
      return (&_flds.psob - start);
    case Params::KDP_HB:
      return (&_flds.kdp_hb - start);

      // co-cross correlation

    case Params::RHO_HC_VX:
      return (&_flds.rho_hcvx - start);
    case Params::RHO_VC_HX:
      return (&_flds.rho_vchx - start);
    case Params::RHO_VX_HX:
      return (&_flds.rho_vxhx - start);
    case Params::RHO_PHIDP:
      return (&_flds.rho_phidp - start);

      // cross polar ratio - CPR

    case Params::CPR_MAG:
      return (&_flds.cpr_mag - start);
    case Params::CPR_PHASE:
      return (&_flds.cpr_phase - start);
    case Params::CPR_LDR:
      return (&_flds.cpr_ldr - start);

      // CMD

    case Params::CPA:
      return (&_flds.cpa - start);
    case Params::TDBZ:
      return (&_flds.tdbz - start);
    case Params::SPIN:
      return (&_flds.spin - start);
    case Params::MAX_TDBZ_SPIN:
      return (&_flds.max_tdbz_spin - start);
    case Params::ZDR_SDEV:
      return (&_flds.zdr_sdev - start);
    case Params::PHIDP_SDEV:
      return (&_flds.phidp_sdev - start);
    case Params::DBZ_DIFF_SQ:
      return (&_flds.dbz_diff_sq - start);
    case Params::DBZ_SPIN_CHANGE:
      return (&_flds.dbz_spin_change - start);
    case Params::CMD:
      return (&_flds.cmd - start);
    case Params::CMD_FLAG:
      return (&_flds.cmd_flag - start);
    case Params::TDBZ_INTEREST:
      return (&_flds.tdbz_interest - start);
    case Params::SPIN_INTEREST:
      return (&_flds.spin_interest - start);
    case Params::CPA_INTEREST:
      return (&_flds.cpa_interest - start);
    case Params::ZDR_SDEV_INTEREST:
      return (&_flds.zdr_sdev_interest - start);
    case Params::PHIDP_SDEV_INTEREST:
      return (&_flds.phidp_sdev_interest - start);

      // CLUTTER FILTERED
      
    case Params::CLUT:
      return (&_flds.clut - start);
    case Params::CLUT_2_WX_RATIO:
      return (&_flds.clut_2_wx_ratio - start);
    case Params::SPECTRAL_NOISE:
      return (&_flds.spectral_noise - start);
    case Params::SPECTRAL_SNR:
      return (&_flds.spectral_snr - start);
      
      // REFRACT

    case Params::AIQ_HC:
      return (&_flds.aiq_hc - start);
    case Params::NIQ_HC:
      return (&_flds.niq_hc - start);
    case Params::AIQ_VC:
      return (&_flds.aiq_vc - start);
    case Params::NIQ_VC:
      return (&_flds.niq_vc - start);

      // SZ8-64

    case Params::SZ_TRIP_FLAG:
      return (&_flds.sz_trip_flag - start);
    case Params::SZ_LEAKAGE:
      return (&_flds.sz_leakage - start);

      // censoring flag

    case Params::CENSORING_FLAG:
      return (&_flds.censoring_flag - start);

      // covariances

    case Params::LAG0_HC_DB:
      return (&_flds.lag0_hc_db - start);
    case Params::LAG0_HX_DB:
      return (&_flds.lag0_hx_db - start);
    case Params::LAG0_VC_DB:
      return (&_flds.lag0_vc_db - start);
    case Params::LAG0_VX_DB:
      return (&_flds.lag0_vx_db - start);
    case Params::LAG0_HC_SHORT_DB:
      return (&_flds.lag0_hc_short - start);

    case Params::LAG0_VC_SHORT_DB:
      return (&_flds.lag0_vc_short - start);
    case Params::LAG0_HC_LONG_DB:
      return (&_flds.lag0_hc_long - start);
    case Params::LAG0_VC_LONG_DB:
      return (&_flds.lag0_vc_long - start);
    case Params::LAG0_VCHX_DB:
      return (&_flds.lag0_vchx_db - start);
    case Params::LAG0_VCHX_PHASE:
      return (&_flds.lag0_vchx_phase - start);

    case Params::LAG0_HCVX_DB:
      return (&_flds.lag0_hcvx_db - start);
    case Params::LAG0_HCVX_PHASE:
      return (&_flds.lag0_hcvx_phase - start);

    case Params::LAG1_HC_DB:
      return (&_flds.lag1_hc_db - start);
    case Params::LAG1_HC_PHASE:
      return (&_flds.lag1_hc_phase - start);
    case Params::LAG1_VC_DB:
      return (&_flds.lag1_vc_db - start);
    case Params::LAG1_VC_PHASE:
      return (&_flds.lag1_vc_phase - start);

    case Params::LAG1_HC_LONG_DB:
      return (&_flds.lag1_hc_long_db - start);
    case Params::LAG1_HC_LONG_PHASE:
      return (&_flds.lag1_hc_long_phase - start);
    case Params::LAG1_VC_LONG_DB:
      return (&_flds.lag1_vc_long_db - start);
    case Params::LAG1_VC_LONG_PHASE:
      return (&_flds.lag1_vc_long_phase - start);

    case Params::LAG1_HC_SHORT_DB:
      return (&_flds.lag1_hc_short_db - start);
    case Params::LAG1_HC_SHORT_PHASE:
      return (&_flds.lag1_hc_short_phase - start);
    case Params::LAG1_VC_SHORT_DB:
      return (&_flds.lag1_vc_short_db - start);
    case Params::LAG1_VC_SHORT_PHASE:
      return (&_flds.lag1_vc_short_phase - start);

    case Params::LAG1_HC_LONG_TO_SHORT_PHASE:
      return (&_flds.lag1_hc_long_to_short_phase - start);
    case Params::LAG1_VC_LONG_TO_SHORT_DB:
      return (&_flds.lag1_vc_long_to_short_db - start);
    case Params::LAG1_VC_LONG_TO_SHORT_PHASE:
      return (&_flds.lag1_vc_long_to_short_phase - start);
    case Params::LAG1_HC_LONG_TO_SHORT_DB:
      return (&_flds.lag1_hc_long_to_short_db - start);

    case Params::LAG1_HC_SHORT_TO_LONG_PHASE:
      return (&_flds.lag1_hc_short_to_long_phase - start);
    case Params::LAG1_VC_SHORT_TO_LONG_DB:
      return (&_flds.lag1_vc_short_to_long_db - start);
    case Params::LAG1_VC_SHORT_TO_LONG_PHASE:
      return (&_flds.lag1_vc_short_to_long_phase - start);
    case Params::LAG1_HC_SHORT_TO_LONG_DB:
      return (&_flds.lag1_hc_short_to_long_db - start);

    case Params::LAG1_HCVC_DB:
      return (&_flds.lag1_hcvc_db - start);
    case Params::LAG1_HCVC_PHASE:
      return (&_flds.lag1_hcvc_phase - start);
    case Params::LAG1_VCHC_DB:
      return (&_flds.lag1_vchc_db - start);
    case Params::LAG1_VCHC_PHASE:
      return (&_flds.lag1_vchc_phase - start);
    case Params::LAG1_VXHX_DB:
      return (&_flds.lag1_vxhx_db - start);
    case Params::LAG1_VXHX_PHASE:
      return (&_flds.lag1_vxhx_phase - start);

    case Params::LAG2_HC_DB:
      return (&_flds.lag2_hc_db - start);
    case Params::LAG2_HC_PHASE:
      return (&_flds.lag2_hc_phase - start);
    case Params::LAG2_VC_DB:
      return (&_flds.lag2_vc_db - start);
    case Params::LAG2_VC_PHASE:
      return (&_flds.lag2_vc_phase - start);

    case Params::LAG3_HC_DB:
      return (&_flds.lag3_hc_db - start);
    case Params::LAG3_HC_PHASE:
      return (&_flds.lag3_hc_phase - start);
    case Params::LAG3_VC_DB:
      return (&_flds.lag3_vc_db - start);
    case Params::LAG3_VC_PHASE:
      return (&_flds.lag3_vc_phase - start);

    case Params::RVVHH0_DB:
      return (&_flds.rvvhh0_db - start);
    case Params::RVVHH0_PHASE:
      return (&_flds.rvvhh0_phase - start);

    case Params::RVVHH0_LONG_DB:
      return (&_flds.rvvhh0_long_db - start);
    case Params::RVVHH0_LONG_PHASE:
      return (&_flds.rvvhh0_long_phase - start);
    case Params::RVVHH0_SHORT_DB:
      return (&_flds.rvvhh0_short_db - start);
    case Params::RVVHH0_SHORT_PHASE:
      return (&_flds.rvvhh0_short_phase - start);

      // sdev of V-V covariance

    case Params::SDEV_VV:
      return (&_flds.sdev_vv - start);

      // attenuation correction

    case Params::DBZ_ATTEN_CORRECTION:
      return (&_flds.dbz_atten_correction - start);
    case Params::ZDR_ATTEN_CORRECTION:
      return (&_flds.zdr_atten_correction - start);
    case Params::DBZ_ATTEN_CORRECTED:
      return (&_flds.dbz_atten_corrected - start);
    case Params::ZDR_ATTEN_CORRECTED:
      return (&_flds.zdr_atten_corrected - start);

      // DEBUG

    case Params::PRT:
      return (&_flds.prt - start);
    case Params::NUM_PULSES:
      return (&_flds.num_pulses - start);
    case Params::TEST:
      return (&_flds.test - start);
    case Params::TEST2:
      return (&_flds.test2 - start);
    case Params::TEST3:
      return (&_flds.test3 - start);
    case Params::TEST4:
      return (&_flds.test4 - start);
    case Params::TEST5:
      return (&_flds.test5 - start);
  }

  return 0;

}

