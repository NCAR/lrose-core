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
// TsConvert.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2011
//
///////////////////////////////////////////////////////////////
//
// TsConvert reads IWRF data from files, converts the packing type
// and writes the converted files to a specified location
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include "TsConvert.hh"

using namespace std;

// Constructor

TsConvert::TsConvert(int argc, char **argv)
  
{

  isOK = true;
  _prevGeorefPktSeqNum = -1;
  
  // set programe name
  
  _progName = "TsConvert";

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
  
  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

TsConvert::~TsConvert()

{
  
  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsConvert::Run ()
{
  
  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running TsConvert - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running TsConvert - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running TsConvert - debug mode" << endl;
  }

  // loop through the input files

  int iret = 0;
  for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
    if (_processFile(_args.inputFileList[ii])) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////////////////////////////
// Process one file

int TsConvert::_processFile(const string &inputPath)
  
{

  if (_params.debug) {
    cerr << "Reading input file: " << inputPath << endl;
  }

  // set up a vector with a single file entry

  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
  IwrfTsReaderFile reader(fileList, iwrfDebug);
  
  // read in all pulses

  IwrfTsPulse *pulse = reader.getNextPulse();
  while (pulse != NULL) {
    
    // open output file as needed
    
    if (_openOutputFile(inputPath, *pulse)) {
      cerr << "ERROR - TsConvert::_processFile" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      return -1;
    }

    // write pulse info to file

    if (_handlePulse(reader, *pulse)) {
      _closeOutputFile();
      return -1;
    }
    
    // delete pulse to free memory

    delete pulse;

    // read next one

    pulse = reader.getNextPulse();

  } // while

  // close output file

  _closeOutputFile();
  
  return 0;
  
}

/////////////////////////////
// handle a pulse

int TsConvert::_handlePulse(IwrfTsReaderFile &reader,
                            IwrfTsPulse &pulse)

{

  // compute HCR metadata if requested

  if (_params.compute_hcr_rotation_and_tilt) {
    _computeHcrRotAndTilt(reader, pulse);
  }

  if (_params.compute_hcr_elevation_and_azimuth) {
    _computeHcrElAndAz(pulse);
  }

  // write ops info to file, if info has changed since last write
  
  const IwrfTsInfo &info = pulse.getTsInfo();
  if (info.writeMetaQueueToFile(_out, true)) {
    cerr << "TsConvert::_handlePulse" << endl;
    return -1;
  }
  
  // reformat pulse packing as needed

  if (_params.modify_packing) {
    _reformatIqPacking(pulse);
  }

  // write pulse to file
  
  if (pulse.writeToFile(_out)) {
    cerr << "TsConvert::_handlePulse" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////
// open output file, if needed

int TsConvert::_openOutputFile(const string &inputPath,
                               const IwrfTsPulse &pulse)

{

  if (_out != NULL) {
    return 0;
  }

  // get time from pulse

  DateTime ptime(pulse.getTime());

  // make the output subdir

  char subdir[4000];
  sprintf(subdir, "%s%s%.4d%.2d%.2d",
          _params.output_dir,
          PATH_DELIM,
          ptime.getYear(), ptime.getMonth(), ptime.getDay());
  
  if (ta_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - TsConvert" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute name from input path

  Path inPath(inputPath);
  string outputPath(subdir);
  outputPath += PATH_DELIM;
  outputPath += inPath.getFile();

  // open file

  if ((_out = fopen(outputPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsConvert" << endl;
    cerr << "  Cannot open output file: " << outputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Writing to output file: " << outputPath << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close output file
//
// Returns 0 if file already open, -1 if not

void TsConvert::_closeOutputFile()

{
  
  // close out old file
  
  if (_out != NULL) {
    fclose(_out);
    _out = NULL;
  }

}
  
/////////////////////////////
// reformat IQ packing

void TsConvert::_reformatIqPacking(IwrfTsPulse &pulse)

{

  if (_params.output_packing == Params::PACKING_FL32) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_FL32);
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_DBM_PHASE_SI16);
  } else if (_params.output_packing == Params::PACKING_SIGMET_FL16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_SIGMET_FL16);
  }

}

/////////////////////////////////
// compute HCR rotation and tilt

void TsConvert::_computeHcrRotAndTilt(IwrfTsReaderFile &reader,
                                      IwrfTsPulse &pulse)

{

  IwrfTsInfo &info = reader.getOpsInfo();
  if (!info.isPlatformGeorefActive()) {
    return;
  }
  iwrf_platform_georef_t georef = info.getPlatformGeoref();
  
  double rotMotorAngle = georef.drive_angle_1_deg;
  double tiltMotorAngle = georef.drive_angle_2_deg;
  
  georef.rotation_angle_deg = rotMotorAngle;
  georef.tilt_deg = ((2.0 * tiltMotorAngle) // x2 for reflection
                     * cos(rotMotorAngle * DEG_TO_RAD));

  info.setPlatformGeoref(georef, false);
  pulse.setPlatformGeoref(georef);

}

/////////////////////////////////////
// compute HCR elevation and azimuth

void TsConvert::_computeHcrElAndAz(IwrfTsPulse &pulse)
  
{
  
  if (!pulse.getGeorefActive()) {
    return;
  }
  
  const iwrf_platform_georef_t &georef = pulse.getPlatformGeoref();

  double rollCorr = 0.0;
  double pitchCorr = 0.0;
  double headingCorr = 0.0;
  double driftCorr = 0.0;
  double rotationCorr = 0.0;
  double tiltCorr = 0.0;
  
  double R = (georef.roll_deg + rollCorr) * DEG_TO_RAD;
  double P = (georef.pitch_deg + pitchCorr) * DEG_TO_RAD;
  double H = (georef.heading_deg + headingCorr) * DEG_TO_RAD;
  double D = (georef.drift_angle_deg + driftCorr) * DEG_TO_RAD;
  double T = H + D;
  
  double sinP = sin(P);
  double cosP = cos(P);
  double sinD = sin(D);
  double cosD = cos(D);
  
  double theta_a = 
    (georef.rotation_angle_deg + rotationCorr) * DEG_TO_RAD;
  double tau_a =
    (georef.tilt_deg + tiltCorr) * DEG_TO_RAD;
  double sin_tau_a = sin(tau_a);
  double cos_tau_a = cos(tau_a);
  double sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
  double cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */
  
  double xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
                  + cosD * sin_theta_rc * cos_tau_a
                  -sinD * cosP * sin_tau_a);
  
  double ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
                  + sinD * sin_theta_rc * cos_tau_a
                  + cosP * cosD * sin_tau_a);
  
  double zsubt = (cosP * cos_tau_a * cos_theta_rc
                  + sinP * sin_tau_a);
  
  // georef.rotation_angle_deg = atan2(xsubt, zsubt) * RAD_TO_DEG;
  // georef.tilt_deg = asin(ysubt) * RAD_TO_DEG;

  double lambda_t = atan2(xsubt, ysubt);
  double azimuthRad = fmod(lambda_t + T, M_PI * 2.0);
  double elevationRad = asin(zsubt);
  
  double elevationDeg = elevationRad * RAD_TO_DEG;
  double azimuthDeg = azimuthRad * RAD_TO_DEG;

  pulse.set_elevation(elevationDeg);
  pulse.set_azimuth(azimuthDeg);

}

