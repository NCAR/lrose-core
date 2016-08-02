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
//
// main for MomentsCompute
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// MomentsCompute reads pulse data, forms the pulses into beams
// and computes the moments.
//
////////////////////////////////////////////////////////////////

#include <new>
#include <rvp8_rap/TsReader.hh>
#include <Fmq/DsRadarQueue.hh>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Beam.hh"
#include "MomentsEngine.hh"
using namespace std;

// scale and bias for output

static double _snrBias = -320.0;
static double _snrScale = 0.01;

static double _dbmBias = -320.0;
static double _dbmScale = 0.01;

static double _dbzBias = -320.0;
static double _dbzScale = 0.01;

static double _velBias = -320.0;
static double _velScale = 0.01;

static double _widthBias = -0.01;
static double _widthScale = 0.001;

static double _angleBias = -640.0;
static double _angleScale = 0.02;

static double _kdpBias = -32.0;
static double _kdpScale = 0.001;

static double _zdrBias = -32.0;
static double _zdrScale = 0.001;

static double _ldrBias = -32.0;
static double _ldrScale = 0.001;

static double _rhohvBias = -1.0;
static double _rhohvScale = 0.0001;
  
static double _missingDbl = -9999.0;

static int _nFields = 0;
static int _nGates = 0;
static DsRadarQueue _rQueue;
static DsRadarMsg _msg;

// functions

static int writeParams(const Params &params, const Beam &beam);

static int writeBeam(const Beam &beam, int volNum, int tiltNum);

static void _addField(const string &name,
                      const string &units,
                      double scale,
                      double bias,
                      vector<DsFieldParams*> &fp);

static ui16 _convertDouble(double val, double scale, double bias);

// main

int main(int argc, char **argv)

{

  string progName = "MomentsEngine";

  // get command line args

  Args args;
  if (args.parse(argc, argv)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with command line args" << endl;
    return -1;
  }
 
  // create Moments compute

  Params params;
  MomentsEngine engine(params);
  if (!engine.isOK) {
    return(-1);
  }
  if (args.debugExtraVerbose) {
    engine.setDebugExtraVerbose();
  } else if (args.debugVerbose) {
    engine.setDebugVerbose();
  } else if (args.debug) {
    engine.setDebug();
  }

  // initialize the output queue
  
  string url = "/tmp/MomentsCompute_fmq";
  if (_rQueue.init(url.c_str(), "MomentsCompute", false,
                   DsFmq::READ_WRITE, DsFmq::END,
                   false, 7200, 50000000)) {
    cerr << "ERROR - MomentsCompute" << endl;
    cerr << "  Cannot open fmq, URL: " << url << endl;
    return -1;
  }

  // set up time series reader

  TsReader reader;
  const TsInfo opsInfo;

  // loop through the input files

  int iret = 0;
  time_t latestBeamTime = 0;
  int beamCount = 0;

  for (int ii = 0; ii < (int) args.inputFileList.size(); ii++) {
    
    const char *filePath = args.inputFileList[ii].c_str();
    if (args.debug) {
      cerr << "Opening file: " << filePath << endl;
    }
    
    if (reader.openTsFile(filePath)) {
      cerr << "ERROR - MomentsCompute" << endl;
      cerr << "  Cannot open file: " << filePath << endl;
      iret = -1;
      continue;
    }

    const TsInfo &info = reader.getInfo();

    while (true) {

      TsPulse pulse(info.getClockMhz());
      if (reader.readPulse(pulse)) {
        break;
      }

      // process this pulse

      _nGates = pulse.getNGates();
      float *chan0 = ((float *) pulse.getIqChan0());
      float *chan1 = NULL;
      if (pulse.getIqChan1() != NULL) {
        chan1 = ((float *) pulse.getIqChan1());
      }
      if (engine.processPulse(chan0, chan1,
                              _nGates,
                              pulse.getPrt(),
                              pulse.getTime(),
                              pulse.getEl(),
                              pulse.getAz(),
                              pulse.getSeqNum(),
                              pulse.isHoriz())) {
        iret = -1;
      }
      
      const Beam *beam = engine.getNewBeam();
      if (beam != NULL) {
        latestBeamTime = (time_t) beam->getTime();
        if (beamCount == 0) {
          writeParams(params, *beam);
        }
        writeBeam(*beam, 0, 0);
        beamCount++;
        delete beam;
      }
      
    } // while
    
    reader.closeTsFile();

  } // ii

  // put and end of volume flag

  _rQueue.putEndOfVolume(latestBeamTime, time(NULL));

  return (iret);
  
}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

static int writeParams(const Params &params, const Beam &beam)

{

  // compute nyquist and unambig range

  double nyquistVel =
    ((params.radar.wavelength_cm / 100.0) * beam.getPrf()) / 4.0;
  double unambigRange = (3.0e8 / (2.0 * beam.getPrf())) / 1000.0; 

  // create message
  
  DsRadarMsg msg;
  
  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;

  _addField("SNR", "dB", _snrScale, _snrBias, fp);
  _nFields++;
  
  _addField("DBM", "dBm", _dbmScale, _dbmBias, fp);
  _nFields++;
  
  _addField("DBZ", "dBZ", _dbzScale, _dbzBias, fp);
  _nFields++;

  _addField("VEL", "m/s", _velScale, _velBias, fp);
  _nFields++;

  _addField("WIDTH", "m/s", _widthScale, _widthBias, fp);
  _nFields++;

  _addField("CLUT", "dB", _dbmScale, _dbmBias, fp);
  _nFields++;

  _addField("DBZF", "dBZ", _dbzScale, _dbzBias, fp);
  _nFields++;

  _addField("VELF", "m/s", _velScale, _velBias, fp);
  _nFields++;

  _addField("WIDTHF", "m/s", _widthScale, _widthBias, fp);
  _nFields++;

  _addField("ZDR", "dB", _zdrScale, _zdrBias, fp);
  _nFields++;

  _addField("ZDRM", "dB", _zdrScale, _zdrBias, fp);
  _nFields++;

  _addField("LDRH", "dB", _ldrScale, _ldrBias, fp);
  _nFields++;

  _addField("LDRV", "dB", _ldrScale, _ldrBias, fp);
  _nFields++;

  _addField("RHOHV", "none", _rhohvScale, _rhohvBias, fp);
  _nFields++;

  _addField("PHIDP", "deg", _angleScale, _angleBias, fp);
  _nFields++;

  _addField("KDP", "deg/km", _kdpScale, _kdpBias, fp);
  _nFields++;

  _addField("SNRHC", "dB", _snrScale, _snrBias, fp);
  _nFields++;

  _addField("SNRHX", "dB", _snrScale, _snrBias, fp);
  _nFields++;

  _addField("SNRVC", "dB", _snrScale, _snrBias, fp);
  _nFields++;

  _addField("SNRVX", "dB", _snrScale, _snrBias, fp);
  _nFields++;

  _addField("DBMHC", "dBm", _dbmScale, _dbmBias, fp);
  _nFields++;

  _addField("DBMHX", "dBm", _dbmScale, _dbmBias, fp);
  _nFields++;

  _addField("DBMVC", "dBm", _dbmScale, _dbmBias, fp);
  _nFields++;

  _addField("DBMVX", "dBm", _dbmScale, _dbmBias, fp);
  _nFields++;
  
  // Set radar parameters
  
  DsRadarParams &rp = msg.getRadarParams();
  
  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = _nFields;
  rp.numGates = beam.getNGatesOut();
  rp.samplesPerBeam = beam.getNSamples();
  rp.scanType = 0;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;

  switch (params.radar.xmit_rcv_mode) {
    case Params::SP:
    case Params::SP_STAGGERED_2_3:
    case Params::SP_STAGGERED_3_4:
    case Params::SP_STAGGERED_4_5:
      rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
      break;
    case Params::DP_ALT_HV_CO_ONLY:
    case Params::DP_ALT_HV_CO_CROSS:
    case Params::DP_ALT_HV_FIXED_HV:
    case Params::DP_SIM_HV_FIXED_HV:
    case Params::DP_SIM_HV_SWITCHED_HV:
    case Params::DP_H_ONLY_FIXED_HV:
      rp.polarization = DS_POLARIZATION_DUAL_TYPE;
      break;
    default:
      rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  }

  rp.radarConstant = params.hc_receiver.radar_constant;
  rp.altitude = 1.742;
  rp.latitude = 39.9321;
  rp.longitude = -105.182;

  rp.gateSpacing = beam.getGateSpacing();
  rp.startRange = beam.getStartRange();

  rp.horizBeamWidth = params.radar.horiz_beam_width;
  rp.vertBeamWidth = params.radar.vert_beam_width;
  
  rp.pulseWidth = params.radar.pulse_width;
  rp.pulseRepFreq = beam.getPrf();
  rp.wavelength = params.radar.wavelength_cm;
  rp.xmitPeakPower = params.radar.xmit_peak_pwr;
  rp.receiverMds = -114;
  rp.receiverGain = 35;
  rp.antennaGain = 45;
  rp.systemGain = 45;

  rp.unambigVelocity = nyquistVel;
  rp.unambigRange = unambigRange;
  
  rp.radarName = "unknown";
  rp.scanTypeName = "unknown";
  
  // write the message
  
  if (_rQueue.putDsMsg(msg,
                       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - Cannot put params to queue" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

static int writeBeam(const Beam &beam, int volNum, int tiltNum)

{

  int iret = 0;
  DsRadarBeam &dsBeam = _msg.getRadarBeam();
  int ndata = beam.getNGatesOut() * _nFields;
  ui16 *data = new ui16[ndata];
  memset(data, 0, ndata * sizeof(ui16));
  
  // params
  
  dsBeam.dataTime = (time_t) beam.getTime();
  dsBeam.volumeNum = volNum;
  dsBeam.tiltNum = tiltNum;
  dsBeam.elevation = beam.getEl();
  dsBeam.targetElev = beam.getEl();
  dsBeam.azimuth = beam.getAz();
  
  // Load up data on a gate-by-gate basis.
  // There are multiple fields for each gate

  ui16 *dp = data;
  for (int igate = 0; igate < beam.getNGatesOut(); igate++) {

    const Fields &fields = beam.getFields()[igate];

    // fields.print(cerr);
  
    *dp = _convertDouble(fields.snr, _snrScale, _snrBias);
    dp++;

    *dp = _convertDouble(fields.dbm, _dbmScale, _dbmBias);
    dp++;

    *dp = _convertDouble(fields.dbz, _dbzScale, _dbzBias);
    dp++;

    *dp = _convertDouble(fields.vel, _velScale, _velBias);
    dp++;

    *dp = _convertDouble(fields.width, _widthScale, _widthBias);
    dp++;

    *dp = _convertDouble(fields.clut, _dbmScale, _dbmBias);
    dp++;

    *dp = _convertDouble(fields.dbzf, _dbzScale, _dbzBias);
    dp++;

    *dp = _convertDouble(fields.velf, _velScale, _velBias);
    dp++;

    *dp = _convertDouble(fields.widthf, _widthScale, _widthBias);
    dp++;

    *dp = _convertDouble(fields.zdr, _zdrScale, _zdrBias);
    dp++;

    *dp = _convertDouble(fields.zdrm, _zdrScale, _zdrBias);
    dp++;

    *dp = _convertDouble(fields.ldrh, _ldrScale, _ldrBias);
    dp++;

    *dp = _convertDouble(fields.ldrv, _ldrScale, _ldrBias);
    dp++;

    *dp = _convertDouble(fields.rhohv, _rhohvScale, _rhohvBias);
    dp++;

    *dp = _convertDouble(fields.phidp, _angleScale, _angleBias);
    dp++;

    *dp = _convertDouble(fields.kdp, _kdpScale, _kdpBias);
    dp++;

    *dp = _convertDouble(fields.snrhc, _snrScale, _snrBias);
    dp++;

    *dp = _convertDouble(fields.snrhx, _snrScale, _snrBias);
    dp++;

    *dp = _convertDouble(fields.snrvc, _snrScale, _snrBias);
    dp++;

    *dp = _convertDouble(fields.snrvx, _snrScale, _snrBias);
    dp++;

    *dp = _convertDouble(fields.dbmhc, _dbmScale, _dbmBias);
    dp++;

    *dp = _convertDouble(fields.dbmhx, _dbmScale, _dbmBias);
    dp++;

    *dp = _convertDouble(fields.dbmvc, _dbmScale, _dbmBias);
    dp++;

    *dp = _convertDouble(fields.dbmvx, _dbmScale, _dbmBias);
    dp++;

  } // igate

  // load the data into the message
  
  dsBeam.loadData(data, ndata * sizeof(ui16), sizeof(ui16));
  delete[] data;
  
  // write the message
  
  if (_rQueue.putDsMsg(_msg, DsRadarMsg::RADAR_BEAM)) {
    iret = -1;
  }
  
  if (iret) {
    cerr << "ERROR - OutputFmq::writeBeams" << endl;
    cerr << "  Cannot put radar beam to queue" << endl;
  }

  return iret;

}

// Add a field to the field params message.

static void _addField(const string &name,
                      const string &units,
                      double scale,
                      double bias,
                      vector<DsFieldParams*> &fp)
{
  DsFieldParams* fparams =
    new DsFieldParams(name.c_str(), units.c_str(),
                      scale, bias, sizeof(ui16));
  fp.push_back(fparams);
}

// convert double to ui16, applying scale and bias

static ui16 _convertDouble(double val, double scale, double bias) {
  if (val == _missingDbl) {
    return 0;
  }
  int ival = (int) ((val - bias) / scale + 0.5);
  if (ival < 1) {
    ival = 1;
  }
  if (ival > 65535) {
    ival = 65535;
  }
  return (ui16) ival;
}

