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
// PointingCpCompute.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
///////////////////////////////////////////////////////////////
//
// PointingCpCompute analyses time series data from pointing ops
//
////////////////////////////////////////////////////////////////

#include "PointingCpCompute.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>

using namespace std;

// Constructor

PointingCpCompute::PointingCpCompute(int argc, char **argv)
  
{

  isOK = true;
  _paramsPath = NULL;
  _reader = NULL;
  _nPulsesH = 0;
  _nPulsesV = 0;

  _sumAzXH = 0.0;
  _sumAzYH = 0.0;
  _sumElXH = 0.0;
  _sumElYH = 0.0;

  _sumAzXV = 0.0;
  _sumAzYV = 0.0;
  _sumElXV = 0.0;
  _sumElYV = 0.0;

  _meanAzH = -9999;
  _meanElH = -9999;
  _startTimeH = 0;
  _endTimeH = 0;
  
  _meanAzV = -9999;
  _meanElV = -9999;
  _startTimeV = 0;
  _endTimeV = 0;
  
  _gateSpacingWarningPrinted = false;
  _startRangeWarningPrinted = false;

  // set programe name
  
  _progName = "PointingCpCompute";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // gate geometry

  if (_params.analysis_end_range < _params.analysis_start_range) {
    cerr << "ERROR - range geometry for analysis" << endl;
    cerr << "  Analysis end range must exceed start range" << endl;
    cerr << "  analysis_start_range: " << _params.analysis_start_range;
    cerr << "  analysis_end_range: " << _params.analysis_end_range;
    isOK = false;
    return;
  }

  _analysisStartGate =
    (int) (_params.analysis_start_range / _params.data_gate_spacing + 0.5);
  if (_analysisStartGate < 0) _analysisStartGate = 0;
  _analysisEndGate =
    (int) (_params.analysis_end_range / _params.data_gate_spacing + 0.5);
  _analysisNGates = _analysisEndGate - _analysisStartGate + 1;

  if (_params.debug) {
    cerr << "=======================================================" << endl;
    cerr << "Running: " << _progName << endl;
    cerr << "  analysis_start_range: " << _params.analysis_start_range << endl;
    cerr << "  analysis_end_range: " << _params.analysis_end_range << endl;
    cerr << "  analysis_start_range: " << _params.analysis_start_range << endl;
    cerr << "  data_start_range: " << _params.data_start_range << endl;
    cerr << "  data_gate_spacing: " << _params.data_gate_spacing << endl;
    cerr << "  _analysisStartGate: " << _analysisStartGate << endl;
    cerr << "  _analysisEndGate: " << _analysisEndGate << endl;
    cerr << "  _analysisNGates: " << _analysisNGates << endl;
    cerr << "  min_snr: " << _params.min_snr << endl;
    cerr << "  max_snr: " << _params.max_snr << endl;
    cerr << "  min_cpa: " << _params.min_cpa << endl;
    cerr << "  min_valid_ratio: " << _params.min_valid_ratio << endl;
    cerr << "  max_valid_ratio: " << _params.max_valid_ratio << endl;
  }

  // calibration

  string errStr;
  if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
    cerr << "ERROR - Calibration::_readCalFromFile" << endl;
    cerr << "  Cannot decode cal file: "
	 << _params.cal_xml_file_path << endl;
    isOK = false;
    return;
  }
  
  // set up gate data vectors

  GateData hGate, vGate;
  for (int ii = 0; ii < _analysisNGates; ii++) {
    _gatesH.push_back(hGate);
    _gatesV.push_back(vGate);
  }

  // times

  DateTime startV(_params.v_data_start_time.year,
                  _params.v_data_start_time.month,
                  _params.v_data_start_time.day,
                  _params.v_data_start_time.hour,
                  _params.v_data_start_time.min,
                  _params.v_data_start_time.sec);
  _startSecsVData = startV.utime();

  DateTime endV(_params.v_data_end_time.year,
                _params.v_data_end_time.month,
                _params.v_data_end_time.day,
                _params.v_data_end_time.hour,
                _params.v_data_end_time.min,
                _params.v_data_end_time.sec);
  _endSecsVData = endV.utime();

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

PointingCpCompute::~PointingCpCompute()

{

  // close readers
  
  if (_reader) {
    delete _reader;
  }

  // clean up memory
  
  _gatesH.clear();
  _gatesV.clear();
  
}

//////////////////////////////////////////////////
// Run

int PointingCpCompute::Run ()
{

  // create reader

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  if (_params.input_mode == Params::TS_FILE_INPUT) {
    _reader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  } else {
    _reader = new IwrfTsReaderFmq(_params.input_fmq_name, iwrfDebug);
  }

  // accumulate pulse data

  int nPulses = 0;
  while (nPulses < 1000000) {
    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    _inputPath = _reader->getPathInUse();
    if (pulse == NULL) {
      break;
    }
    _processPulse(pulse);
    delete pulse;
  }

  // compute cross-polar power results
  
  _computeCp();

  // write results to file if needed

  if (_params.write_output_files) {
    _writeResults();
  }

  return 0;

}

/////////////////////
// process a pulse

void PointingCpCompute::_processPulse(const IwrfTsPulse *pulse)

{

  // check the gate spacing?
  
  const IwrfTsInfo &info = _reader->getOpsInfo();
  double gateSpacing = info.get_proc_gate_spacing_km();
  double startRange = info.get_proc_start_range_km();
  double gateSpacingError = gateSpacing - _params.data_gate_spacing;
  double startRangeError = startRange - _params.data_start_range;
  
  if (fabs(gateSpacingError) > 0.001) {
    if (!_gateSpacingWarningPrinted) {
      cerr << "WARNING - found incorrect gate spacing" << endl;
      cerr << "  Expecting: " << _params.data_gate_spacing << endl;
      cerr << "  Got: " << gateSpacing << endl;
      cerr << "  Only one warning will be printed" << endl;
      _gateSpacingWarningPrinted = true;
    }
    return;
  }
  
  if (fabs(startRangeError) > 0.001) {
    if (!_startRangeWarningPrinted) {
      cerr << "WARNING - found incorrect start range" << endl;
      cerr << "  Expecting: " << _params.data_start_range << endl;
      cerr << "  Got: " << startRange << endl;
      cerr << "  Only one warning will be printed" << endl;
      _startRangeWarningPrinted = true;
    }
    return;
  }
  
  // pulse count
  
  double pulseTime = pulse->getFTime();
  
  // is this a horizontal or vertical pulse?

  bool isHoriz = true;
  if (_params.hv_flag_set) {
    if (!pulse->get_hv_flag()) {
      isHoriz = false;
    }
  } else if (_params.set_hv_from_file_path) {
    if (_inputPath.find("horiz_") != string::npos) {
      isHoriz = true;
    } else {
      isHoriz = false;
    }
  } else {
    // use time
    if (pulseTime >= _startSecsVData && pulseTime <= _endSecsVData) {
      isHoriz = false;
    }
  }

  // check HV flag - temporary - FIX LATER

   if (isHoriz) {
    if (!pulse->isHoriz()) return;
   } else {
     if (pulse->isHoriz()) return;
   }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    if (isHoriz) {
      cerr << "h";
    } else {
      cerr << "v";
    }
  }
      
  // az/el

  double azRad = pulse->getAz() * DEG_TO_RAD;
  double elRad = pulse->getEl() * DEG_TO_RAD;

  if (isHoriz) {

    _nPulsesH++;
    _sumAzXH += cos(azRad);
    _sumAzYH += sin(azRad);
    _sumElXH += cos(elRad);
    _sumElYH += sin(elRad);

    if (_startTimeH == 0) {
      _startTimeH = pulseTime;
    }
    _endTimeH = pulseTime;
    
  } else {

    _nPulsesV++;
    _sumAzXV += cos(azRad);
    _sumAzYV += sin(azRad);
    _sumElXV += cos(elRad);
    _sumElYV += sin(elRad);

    if (_startTimeV == 0) {
      _startTimeV = pulseTime;
    }
    _endTimeV = pulseTime;
    
  }

  // loop through the gates
  
  int endGate = _analysisEndGate;
  if (endGate > pulse->getNGates() - 1) {
    endGate = pulse->getNGates() - 1;
  }

  const fl32 *iq0 = pulse->getIq0();
  const fl32 *iq1 = pulse->getIq1();

  for (int igate = _analysisStartGate, jgate = 0;
       igate <= endGate; igate++, jgate++) {

    int loci = igate * 2;
    int locq = loci + 1;
    double i_co = 0, q_co = 0;
    double i_xx = 0, q_xx = 0;

    if (iq0) {
      i_co = iq0[loci];
      q_co = iq0[locq];
    }
    if (iq1) {
      i_xx = iq1[loci];
      q_xx = iq1[locq];
    }

    if (isHoriz) {
      
      _gatesH[jgate].addCo(i_co, q_co);
      _gatesV[jgate].addXx(i_xx, q_xx);
      
    } else {

      _gatesV[jgate].addCo(i_co, q_co);
      _gatesH[jgate].addXx(i_xx, q_xx);

    }
    
  } // igate

}

//////////////////////////////
// compute the CP quantities

void PointingCpCompute::_computeCp()

{

  // compute mean pointing angles for H and V

  double azxH = _sumAzXH / _nPulsesH;
  double azyH = _sumAzYH / _nPulsesH;
  double elxH = _sumElXH / _nPulsesH;
  double elyH = _sumElYH / _nPulsesH;
  _meanAzH = atan2(azyH, azxH) * RAD_TO_DEG;
  _meanElH = atan2(elyH, elxH) * RAD_TO_DEG;
  
  double azxV = _sumAzXV / _nPulsesV;
  double azyV = _sumAzYV / _nPulsesV;
  double elxV = _sumElXV / _nPulsesV;
  double elyV = _sumElYV / _nPulsesV;
  _meanAzV = atan2(azyV, azxV) * RAD_TO_DEG;
  _meanElV = atan2(elyV, elxV) * RAD_TO_DEG;

  if (_params.debug) {
    cerr << "Start time H: " << DateTime::strm((time_t)_startTimeH) << endl;
    cerr << "End   time H: " << DateTime::strm((time_t)_endTimeH) << endl;
    cerr << "Start time V: " << DateTime::strm((time_t)_startTimeV) << endl;
    cerr << "End   time V: " << DateTime::strm((time_t)_endTimeV) << endl;
    cerr << "El, az H: " << _meanElH << ", " << _meanAzH << endl;
    cerr << "El, az V: " << _meanElV << ", " << _meanAzV << endl;
    cerr << "N samples H: " << _nPulsesH << endl;
    cerr << "N samples V: " << _nPulsesV << endl;
    if (_params.write_results_to_stdout &&
	_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Writing results to stdout" << endl;
      cerr << "  col  1: gateNum" << endl;
      cerr << "  col  2: ratio_xx_dB" << endl;
      cerr << "  col  3: count_h" << endl;
      cerr << "  col  4: count_v" << endl;
      cerr << "  col  5: mean_power_co_h" << endl;
      cerr << "  col  6: mean_power_co_v" << endl;
      cerr << "  col  7: mean_power_xx_h" << endl;
      cerr << "  col  8: mean_power_xx_v" << endl;
      cerr << "  col  9: mean_dbm_co_h" << endl;
      cerr << "  col 10: mean_dbm_co_v" << endl;
      cerr << "  col 11: mean_dbm_xx_h" << endl;
      cerr << "  col 12: mean_dbm_xx_v" << endl;
      cerr << "  col 13: sdev_dbm_co_h" << endl;
      cerr << "  col 14: sdev_dbm_co_v" << endl;
      cerr << "  col 15: sdev_dbm_xx_h" << endl;
      cerr << "  col 16: sdev_dbm_xx_v" << endl;
      cerr << "  col 17: cpa_co_h" << endl;
      cerr << "  col 18: cpa_co_v" << endl;
      cerr << "  col 19: cpa_xx_h" << endl;
      cerr << "  col 20: cpa_xx_v" << endl;
    }
  }

  double sum_power_xx_h = 0.0, sum_power_xx_v = 0.0;
  double sum_norm_xx_h = 0.0, sum_norm_xx_v = 0.0;
  _nGatesUsed = 0;

  for (int ii = 0; ii < _analysisNGates; ii++) {

    if (_gatesH[ii].count_xx < 1000 || 
	_gatesV[ii].count_xx < 1000) {
      continue;
    }

    // compute the moments
    
    _gatesH[ii].computeDerived();
    _gatesV[ii].computeDerived();

    // check CPA

    if (_gatesH[ii].cpa_xx < _params.min_cpa ||
	_gatesV[ii].cpa_xx < _params.min_cpa) {
      continue;
    }

    // accumulate

    double noisePowerHx = pow(10.0, _calib.getNoiseDbmHx() / 10.0);
    double noisePowerVx = pow(10.0, _calib.getNoiseDbmVx() / 10.0);
    double powerHx = _gatesH[ii].mean_power_xx - noisePowerHx;
    double powerVx = _gatesV[ii].mean_power_xx - noisePowerVx;
    if (!_params.subtract_noise_power) {
      powerHx = _gatesH[ii].mean_power_xx;
      powerVx = _gatesV[ii].mean_power_xx;
    }

    if (powerHx < 0 || powerVx < 0) {
      continue;
    }

    double snrHx = 10.0 * log10(powerHx / noisePowerHx);
    double snrVx = 10.0 * log10(powerVx / noisePowerVx);

    if (snrHx < _params.min_snr || snrHx > _params.max_snr ||
 	snrVx < _params.min_snr || snrVx > _params.max_snr) {
      continue;
    }

    double ratio = powerVx / powerHx;
    double ratioDb = 10.0 * log10(ratio);

    if (ratio < _params.min_valid_ratio ||
	ratio > _params.max_valid_ratio) {
      continue;
    }

    sum_power_xx_h += powerHx;
    sum_power_xx_v += powerVx;
    _nGatesUsed++;

    double meanPower = (powerHx + powerVx) / 2.0;
    sum_norm_xx_h += powerHx / meanPower;
    sum_norm_xx_v += powerVx / meanPower;
    
    // print them out

    if (_params.write_results_to_stdout) {
      fprintf(stdout,
	      "%4d %8.3f %5d %5d "
	      "%8.3e %8.3e %8.3e %8.3e "
	      "%8.3f %8.3f %8.3f %8.3f "
	      "%8.3f %8.3f %8.3f %8.3f "
	      "%8.3f %8.3f %8.3f %8.3f "
	      "\n",
	      ii, ratioDb,
	      _gatesH[ii].count_xx,
	      _gatesV[ii].count_xx,
	      _gatesH[ii].mean_power_co,
	      _gatesV[ii].mean_power_co,
	      _gatesH[ii].mean_power_xx,
	      _gatesV[ii].mean_power_xx,
	      _gatesH[ii].mean_dbm_co,
	      _gatesV[ii].mean_dbm_co,
	      _gatesH[ii].mean_dbm_xx,
	      _gatesV[ii].mean_dbm_xx,
	      _gatesH[ii].sdev_dbm_co,
	      _gatesV[ii].sdev_dbm_co,
	      _gatesH[ii].sdev_dbm_xx,
	      _gatesV[ii].sdev_dbm_xx,
	      _gatesH[ii].cpa_co,
	      _gatesV[ii].cpa_co,
	      _gatesH[ii].cpa_xx,
	      _gatesV[ii].cpa_xx);
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      
      cerr << "======= ANALYSIS GATE NUM: " << ii << " =======" << endl;
      cerr << "------- H Channel moments ---------" << endl;
      cerr << "  mean_power_co: " << _gatesH[ii].mean_power_co << endl;
      cerr << "  mean_dbm_co: " << _gatesH[ii].mean_dbm_co << endl;
      cerr << "  sdev_dbm_co: " << _gatesH[ii].sdev_dbm_co << endl;
      cerr << "  mean_power_xx: " << _gatesH[ii].mean_power_xx << endl;
      cerr << "  mean_dbm_xx: " << _gatesH[ii].mean_dbm_xx << endl;
      cerr << "  sdev_dbm_xx: " << _gatesH[ii].sdev_dbm_xx << endl;
      cerr << "  cpa_co: " << _gatesH[ii].cpa_co << endl;
      cerr << "  cpa_xx: " << _gatesH[ii].cpa_xx << endl;
      
      cerr << "------- V Channel moments ---------" << endl;
      cerr << "  mean_power_co: " << _gatesV[ii].mean_power_co << endl;
      cerr << "  mean_dbm_co: " << _gatesV[ii].mean_dbm_co << endl;
      cerr << "  sdev_dbm_co: " << _gatesV[ii].sdev_dbm_co << endl;
      cerr << "  mean_power_xx: " << _gatesV[ii].mean_power_xx << endl;
      cerr << "  mean_dbm_xx: " << _gatesV[ii].mean_dbm_xx << endl;
      cerr << "  sdev_dbm_xx: " << _gatesV[ii].sdev_dbm_xx << endl;
      cerr << "  cpa_co: " << _gatesV[ii].cpa_co << endl;
      cerr << "  cpa_xx: " << _gatesV[ii].cpa_xx << endl;

    }
    
  } // ii
  
  _cpPowerRatio = sum_power_xx_v / sum_power_xx_h;
  _cpPowerRatioDb = 10.0 * log10(_cpPowerRatio);

  _cpNormRatio = sum_norm_xx_v / sum_norm_xx_h;
  _cpNormRatioDb = 10.0 * log10(_cpNormRatio);

  if (_params.debug) {
    cerr << "*********************************************" << endl;
    cerr << "***** n gates used: " << _nGatesUsed << endl;
    cerr << "***** cpPowerRatio: " << _cpPowerRatio << endl;
    cerr << "***** cpPowerRatioDb: " << _cpPowerRatioDb << endl;
    cerr << "***** cpNormRatio: " << _cpNormRatio << endl;
    cerr << "***** cpNormRatioDb: " << _cpNormRatioDb << endl;
    cerr << "*********************************************" << endl;
  }

}

///////////////////////////////
// write out results to files

int PointingCpCompute::_writeResults()

{

  // create the directory for the output files, if needed

  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - PointingCpCompute::_writeResults";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output file path

  time_t calTime = (time_t) _endTimeH;
  if (_endTimeV > _endTimeH) calTime = (time_t) _endTimeV;
  DateTime ctime(calTime);
  char outPath[1024];
  sprintf(outPath, "%s/point_zdr_cal_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());

  if (_params.debug) {
    cerr << "-->> Writing results file: " << outPath << endl;
  }
  
  // open file
  
  FILE *out;
  if ((out = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - PointingCpCompute::_writeFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "========================================\n");
  fprintf(out, "Stationary pointing ZDR calibration\n");
  fprintf(out, "  Time: %s\n", DateTime::strm(calTime).c_str());
  fprintf(out, "  min CPA: %g\n", _params.min_cpa);
  fprintf(out, "  min snr (dB): %g\n", _params.min_snr);
  fprintf(out, "  max snr (dB): %g\n", _params.max_snr);
  fprintf(out, "  min valid_ratio (dB): %g\n", _params.min_valid_ratio);
  fprintf(out, "  max valid_ratio (dB): %g\n", _params.max_valid_ratio);
  fprintf(out, "  start range (km): %g\n", _params.analysis_start_range);
  fprintf(out, "  end range (km): %g\n", _params.analysis_end_range);
  fprintf(out, "  end range (km): %g\n", _params.analysis_end_range);
  fprintf(out, "  gate spacing (km): %g\n", _params.data_gate_spacing);
  fprintf(out, "----------------------------------------\n");
  fprintf(out, "  start time H: %s\n",
	  DateTime::strm((time_t)_startTimeH).c_str());
  fprintf(out, "  end   time H: %s\n",
	  DateTime::strm((time_t)_endTimeH).c_str());
  fprintf(out, "  start time V: %s\n",
	  DateTime::strm((time_t)_startTimeV).c_str());
  fprintf(out, "  end   time V: %s\n",
	  DateTime::strm((time_t)_endTimeV).c_str());
  fprintf(out, "  elev, az H: %8.2f deg, %8.2f deg\n",
	  _meanElH, _meanAzH);
  fprintf(out, "  elev, az V: %8.2f deg, %8.2f deg\n",
	  _meanElV, _meanAzV);
  fprintf(out, "  n samples H: %d\n", _nPulsesH);
  fprintf(out, "  n samples V: %d\n", _nPulsesV);
  fprintf(out, "----------------------------------------\n");
  fprintf(out, "  n gates used: %d\n", _nGatesUsed);
  fprintf(out, "  cpPowerRatio, dB: %10.3f %10.3f\n",
	  _cpPowerRatio, _cpPowerRatioDb);
  fprintf(out, "  cpNormRatio,  dB: %10.3f %10.3f\n",
	  _cpNormRatio, _cpNormRatioDb);
  fprintf(out, "========================================\n");

  return 0;

}

///////////////////////////////
// write out results to files

int PointingCpCompute::_writeGlobalResults()

{

#ifdef JUNK
  
  // compute output file path

  time_t startTime = (time_t) _startTime;
  DateTime ctime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s/vert_zdr_global_cal_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "-->> Writing global results file: " << outPath << endl;
  }

  // open file
  
  FILE *out;
  if ((out = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - PointingCpCompute::_writeFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "========================================\n");
  fprintf(out, "Vertical-pointing ZDR calibration - global\n");
  fprintf(out, "Time: %s\n", DateTime::strm(startTime).c_str());
  fprintf(out, "n samples   : %d\n", _params.n_samples);
  fprintf(out, "min snr (dB): %g\n", _params.min_snr);
  fprintf(out, "max snr (dB): %g\n", _params.max_snr);
  fprintf(out, "========================================\n");
  fprintf(out, "%10s%10s%10s%10s%10s%10s%10s%10s\n",
          "Range", "npts", "snr", "dBZ", "Hc", "Hx", "Vc", "Vx");
  for (int ii = 0; ii < (int) _intervals.size(); ii++) {
    const RangeStats &interval = *(_intervals[ii]);
    fprintf(out,
            "%10.2f%10d%10.3f%10.3f%10.1f%10.3f%10.3f%10.3f\n",
            interval.getMeanRange(),
            interval.getGlobalNValid(),
            interval.getGlobalMean().snr,
            interval.getGlobalMean().dbz,
            interval.getGlobalMean().dbmhc,
            interval.getGlobalMean().dbmhx,
            interval.getGlobalMean().dbmvc,
            interval.getGlobalMean().dbmvx);
  }
  
  // close file

  fclose(out);

#endif

  return 0;

}

