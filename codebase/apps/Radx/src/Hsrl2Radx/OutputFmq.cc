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
    cerr << "  Cannot open output fmq, URL: " << _params.output_fmq_url << endl;
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
    DsFieldParams* fparams =
      new DsFieldParams(field->getName().c_str(), field->getUnits().c_str(),
                        1.0, 0.0, sizeof(fl32));
    fp.push_back(fparams);
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
  
  double pulseWidthUs =
    ((_params.gate_spacing_km * 1000.0) / Radx::LIGHT_SPEED) * 1.0e6;
  rp.pulseWidth = pulseWidthUs;

  // rp.pulseRepFreq = 1.0 / ray->getPrt();
  // rp.prt = ray->getPrt();

  // rp.wavelength = 538.0e-9 * 100.0; // cm
  rp.wavelength = 1064.0e-9 * 100.0; // cm

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

  // add georeference first, if applicable
  
  int contents = 0;
  if (ray->getGeoreference() != NULL) {

    const RadxGeoref *radxGeoref = ray->getGeoreference();
    ds_iwrf_platform_georef_t igeo;
    
    igeo.altitude_msl_km = radxGeoref->getAltitudeKmMsl();
    igeo.altitude_agl_km = radxGeoref->getAltitudeKmAgl();
    igeo.ew_velocity_mps = radxGeoref->getEwVelocity();
    igeo.ns_velocity_mps = radxGeoref->getNsVelocity();
    igeo.vert_velocity_mps = radxGeoref->getVertVelocity();
    igeo.heading_deg = radxGeoref->getHeading();
    igeo.roll_deg = radxGeoref->getRoll();
    igeo.pitch_deg = radxGeoref->getPitch();
    igeo.drift_angle_deg = radxGeoref->getDrift();
    igeo.rotation_angle_deg = radxGeoref->getRotation();
    igeo.tilt_angle_deg = radxGeoref->getTilt();
    igeo.ew_horiz_wind_mps = radxGeoref->getEwWind();
    igeo.ns_horiz_wind_mps = radxGeoref->getNsWind();
    igeo.vert_wind_mps = radxGeoref->getVertWind();
    igeo.heading_rate_dps = radxGeoref->getHeadingRate();
    igeo.pitch_rate_dps = radxGeoref->getHeadingRate();
    igeo.drive_angle_1_deg = radxGeoref->getDriveAngle1();
    igeo.drive_angle_2_deg = radxGeoref->getDriveAngle2();
    igeo.longitude = radxGeoref->getLongitude();
    igeo.latitude = radxGeoref->getLatitude();
    igeo.track_deg = radxGeoref->getTrack();
    igeo.roll_rate_dps = radxGeoref->getRollRate();
    
    DsPlatformGeoref &dsGeoref = _msg.getPlatformGeoref();
    dsGeoref.setGeoref(igeo);
    contents |= DsRadarMsg::PLATFORM_GEOREF;

  }
  
  // params
  
  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  dsBeam.dataTime = ray->getTimeSecs();
  dsBeam.nanoSecs = ray->getNanoSecs();
  dsBeam.volumeNum = -1;
  dsBeam.tiltNum = -1;
  dsBeam.elevation = ray->getElevationDeg();
  dsBeam.azimuth = ray->getAzimuthDeg();
  dsBeam.targetElev = ray->getFixedAngleDeg();
  dsBeam.targetAz = -9999.0;
  dsBeam.antennaTransition = ray->getAntennaTransition();
  dsBeam.scanMode = DS_RADAR_VERTICAL_POINTING_MODE;
  dsBeam.beamIsIndexed = false;
  dsBeam.angularResolution = -9999.0;
  dsBeam.nSamples = ray->getNSamples();
  dsBeam.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  dsBeam.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  // Load up data on a gate-by-gate basis.
  // There are multiple fields for each gate

  vector<const Radx::fl32 *> fldPtrs;
  for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
    const RadxField *field = ray->getField(ifield);
    const Radx::fl32 *fptr = field->getDataFl32();
    fldPtrs.push_back(fptr);
  } // ifield

  // set data array
  
  size_t nGates = ray->getNGates();
  size_t nFields = ray->getNFields();
  size_t nData = nGates * nFields;
  fl32 *data = new fl32[nData];
  memset(data, 0, nData * sizeof(fl32));

  fl32 *dp = data;
  for (size_t igate = 0; igate < nGates; igate++) {
    for (size_t ifield = 0; ifield < fldPtrs.size(); ifield++) {
      *dp = fldPtrs[ifield][igate];
      dp++;
    } // ifield
  } // igate
  
  // load the data into the message

  dsBeam.loadData(data, nData * sizeof(fl32), sizeof(fl32));
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

