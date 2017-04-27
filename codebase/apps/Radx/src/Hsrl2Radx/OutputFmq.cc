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
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////
//
// OutputFmq puts the data out in DsRadar format to an FMQ
//
////////////////////////////////////////////////////////////////

#include "OutputFmq.hh"
#include <cmath>
#include <dsserver/DmapAccess.hh>
#include <toolsa/DateTime.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxGeoref.hh>
using namespace std;

// Constructor

OutputFmq::OutputFmq(const string &prog_name,
		     const Params &params) :
        _progName(prog_name),
        _params(params)
        
{
  
  constructorOK = TRUE;
  _nFields = 0;
  _nRaysWritten = 0;
  _rQueue = NULL;

  // initialize the output queue
  
  if (_openFmq()) {
    constructorOK = FALSE;
    return;
  }

}

// destructor

OutputFmq::~OutputFmq()
  
{

  if (_rQueue != NULL) {
    delete _rQueue;
  }

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

  return 0;

}

////////////////////////////////////////
// Write params to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeParams(const RadxRay *ray)
  
{
  
  // initialize 

  int nGatesOut = ray->getNGates();
  int nSamples = ray->getNSamples();
  double pulseWidthUs =
    ((_params.gate_spacing_km * 1000.0) / Radx::LIGHT_SPEED) * 1.0e6;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::writeParams, nGates: " << nGatesOut << endl;
  }
  
  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;
  for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
    const RadxField *field = ray->getField(ifield);
    _addField(field->getName(), field->getUnits(),
              1.0, 0.0, fp);
    _nFields++;
  } // ifield
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = nGatesOut;
  rp.samplesPerBeam = nSamples;
  rp.scanType = 0;
  rp.scanMode = 0;
  rp.followMode = 0;
  rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  
  rp.radarConstant = 0;

  rp.altitude = _params.instrument_altitude_meters / 1000.0;
  rp.latitude = _params.instrument_latitude_deg;
  rp.longitude = _params.instrument_longitude_deg;
  
  // override if georefs active

  if (ray->getGeoreference() != NULL) {
    const RadxGeoref *georef = ray->getGeoreference();
    rp.latitude = georef->getLatitude();
    rp.longitude = georef->getLongitude();
    rp.altitude = georef->getAltitudeKmMsl();
  }

  rp.gateSpacing = ray->getGateSpacingKm();
  rp.startRange = ray->getStartRangeKm();

  // rp.horizBeamWidth = calib.getBeamWidthDegH();
  // rp.vertBeamWidth = calib.getBeamWidthDegV();
  
  rp.pulseWidth = pulseWidthUs;
  // rp.pulseRepFreq = 1.0 / ray->getPrt();
  // rp.prt = ray->getPrt();
  // rp.wavelength = opsInfo.get_radar_wavelength_cm();
  
  rp.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  rp.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  rp.radarName = "HSRL";
  rp.scanTypeName = "VertPointing";
  
  // write the message
  
  if (_rQueue->putDsMsg(msg,
                        DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - OutputFmq::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write status XML to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeStatusXml(const RadxRay *ray)

{

  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> OutputFmq::writeStatusXml" << endl;
  }
  
  // create DsRadar message
  
  DsRadarMsg msg;
  // msg.setStatusXml(ray->getStatusXml());
  
  // write the message
  
  if (_rQueue->putDsMsg(msg, DsRadarMsg::STATUS_XML)) {
    cerr << "ERROR - OutputFmq::writeCalib" << endl;
    cerr << "  Cannot put status XML to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Writing out status xml: " << endl;
    // cerr << ray->getStatusXml() << endl;
  }
  
  return 0;

}

////////////////////////////////////////
// Write ray data to queue
// Returns 0 on success, -1 on failure

int OutputFmq::writeRay(const RadxRay *ray)

{

  // put georeference if applicable

  int contents = 0;
  if (ray->getGeoreference() != NULL) {

    // DsPlatformGeoref &dsGeoref = _msg.getPlatformGeoref();
    // const RadxGeoref *radxGeoref = ray->getGeoreference();

    // copy struct from iwrf to DsRadar
    // these are identical, stored in different libs
    // TODO - COPY the georef data across

    // memcpy(&dsGeoref, *ray->getGeoref(), sizeof(dsGeoref));
    // georef.setGeoref(dsGeoref);
    // contents |= DsRadarMsg::PLATFORM_GEOREF;

  }
    
  int nGatesOut = ray->getNGates();
  int nFields = ray->getNFields();

  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  int ndata = nGatesOut * nFields;
  fl32 *data = new fl32[ndata];
  memset(data, 0, ndata * sizeof(fl32));
  
  // params
  
  dsBeam.dataTime = ray->getTimeSecs();
  double dtime = ray->getDoubleTime();
  double partialSecs = fmod(dtime, 1.0);
  dsBeam.nanoSecs = (int) (partialSecs * 1.0e9 + 0.5);
  dsBeam.volumeNum = ray->getVolNum();
  dsBeam.tiltNum = ray->getSweepNum();
  dsBeam.elevation = ray->getEl();
  dsBeam.azimuth = ray->getAz();
  dsBeam.targetElev = ray->getTargetEl();
  dsBeam.targetAz = ray->getTargetAz();
  dsBeam.antennaTransition = ray->getAntennaTransition();
  dsBeam.scanMode = ray->getScanMode();
  dsBeam.rayIsIndexed = beam.getRayIsIndexed();
  dsBeam.angularResolution = ray->getAngularResolution();
  dsBeam.nSamples = ray->getNSamplesEffective();
  dsBeam.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  dsBeam.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  // load up vector of field offsets

  vector<int> offsets;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    int offset = _findFieldOffset(field.id);
    offsets.push_back(offset);
  }

  // Load up data on a gate-by-gate basis.
  // There are multiple fields for each gate

  const MomentsFields *fieldsArray = ray->getFields();
  const MomentsFields *fieldsFArray = ray->getFieldsF();
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
  
  // write the message
  
  contents |= DsRadarMsg::RADAR_BEAM;
  if (_rQueue->putDsMsg(_msg, contents)) {
    cerr << "ERROR - OutputFmq::writeBeams" << endl;
    cerr << "  Cannot put radar beam to queue" << endl;
    // reopen the queue
    if (_openFmq()) {
      return -1;
    }
  }

  _nRaysWritten++;
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

