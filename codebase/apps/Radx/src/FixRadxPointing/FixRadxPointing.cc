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
// FixRadxPointing.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#include "FixRadxPointing.hh"
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

FixRadxPointing::FixRadxPointing(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "FixRadxPointing";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

}

// destructor

FixRadxPointing::~FixRadxPointing()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int FixRadxPointing::Run()
{

  // set up rotation matrices

  _headingDeg = _params.platform_heading_deg;
  if (_params.georef_mode == Params::GEOREF_TILT_VECTOR) {
    _computeRollAndPitch();
  } else {
    _rollDeg = _params.platform_roll_deg;
    _pitchDeg = _params.platform_pitch_deg;
  }

  // compute the rotation matrix

  _computeRotationMatrix();

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printTest();
  }

  // run

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }

}

//////////////////////////////////////////////////
// Run in filelist mode

int FixRadxPointing::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int FixRadxPointing::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - FixRadxPointing::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - FixRadxPointing::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int FixRadxPointing::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int FixRadxPointing::_processFile(const string &filePath)
{

  if (_params.debug) {
    cerr << "INFO - FixRadxPointing::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  RadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - FixRadxPointing::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // fix each ray in turn

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    _fixRay(*rays[iray]);
  }

  // adjust the sweep details if the fixed angles have changed

  if (_params.correct_fixed_angles) {
    vol.loadSweepInfoFromRays();
  }

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - FixRadxPointing::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void FixRadxPointing::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// set up write

void FixRadxPointing::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  if (_params.output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  if (_params.output_native_byte_order) {
    file.setWriteNativeByteOrder(true);
  } else {
    file.setWriteNativeByteOrder(false);
  }

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_UF:
      file.setFileFormat(RadxFile::FILE_FORMAT_UF);
      break;
    case Params::OUTPUT_FORMAT_DORADE:
      file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
      break;
    case Params::OUTPUT_FORMAT_FORAY:
      file.setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
      break;
    case Params::OUTPUT_FORMAT_NEXRAD:
      file.setFileFormat(RadxFile::FILE_FORMAT_NEXRAD_AR2);
      break;
    case Params::OUTPUT_FORMAT_MDV_RADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_MDV_RADIAL);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  switch (_params.netcdf_style) {
    case Params::NETCDF4_CLASSIC:
      file.setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case Params::NC64BIT:
      file.setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case Params::NETCDF4:
      file.setNcFormat(RadxFile::NETCDF4);
      break;
    default:
      file.setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (_params.write_individual_sweeps) {
    file.setWriteIndividualSweeps(true);
  }

}

//////////////////////////////////////////////////
// write out the volume

int FixRadxPointing::_writeVol(const RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - FixRadxPointing::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - FixRadxPointing::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - FixRadxPointing::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// compute roll and pitch from tilt

void FixRadxPointing::_computeRollAndPitch()
{

  // compute tilt vector

  double platTiltAmountDeg = _params.platform_tilt_amount_deg;
  double platTiltDirnDeg = _params.platform_tilt_direction_deg;
  
  double platTiltAmountRad = platTiltAmountDeg * Rotate3d::Deg2Rad;
  double platTiltDirnRad = platTiltDirnDeg * Rotate3d::Deg2Rad;
  
  Rotate3d::Vector vTilt(sin(platTiltAmountRad) * sin(platTiltDirnRad),
                         sin(platTiltAmountRad) * cos(platTiltDirnRad),
                         cos(platTiltAmountRad));

  // if the tilt direction is relative to true north, rotate the tilt
  // vector by the vehicle heading, so that it is relative to the
  // platform longitudinal axis

  if (_params.tilt_direction_is_relative_to_north) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====================" << endl;
      cerr << "vTilt rel to N:" << endl;
      Rotate3d::print(vTilt, cerr);
      Rotate3d::printElAz(vTilt, cerr);
      cerr << endl;
    }
    Rotate3d::Matrix rot2Plat = Rotate3d::createRotDegAboutZ(_headingDeg);
    vTilt = Rotate3d::mult(rot2Plat, vTilt);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====================" << endl;
    cerr << "vTilt rel to platform long axis:" << endl;
    Rotate3d::print(vTilt, cerr);
    Rotate3d::printElAz(vTilt, cerr);
    cerr << endl;
  }

  // compute basis vectors relative to horizontal frame aligned
  // with platform long axis
  
  Rotate3d::Vector b3 = Rotate3d::normalize(vTilt);
  Rotate3d::Vector b2Hat =
    Rotate3d::Vector(0.0, 1.0, -1.0 * (vTilt.yy / vTilt.zz));
  Rotate3d::Vector b2 = Rotate3d::normalize(b2Hat);
  Rotate3d::Vector b1 = Rotate3d::cross(b2, b3);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {

    cerr << "====================" << endl;
    cerr << "b1 vector:" << endl;
    Rotate3d::print(b1, cerr);
    Rotate3d::printElAz(b1, cerr);
    
    cerr << "====================" << endl;
    cerr << "b2 vector:" << endl;
    Rotate3d::print(b2, cerr);
    Rotate3d::printElAz(b2, cerr);
    
    cerr << "====================" << endl;
    cerr << "b3 vector:" << endl;
    Rotate3d::print(b3, cerr);
    Rotate3d::printElAz(b3, cerr);

  }
    
  // compute pitch
  
  Rotate3d::Vector ey(0.0, 1.0, 0.0);
  double cosPitch = Rotate3d::dot(b2, ey);
  double pitchDeg = acos(cosPitch) * Rotate3d::Rad2Deg;
  if (b2.zz < 0) pitchDeg *= -1.0;
  
  _pitchDeg = pitchDeg;

  // compute roll
  // find vector in b1/b3 plane
  
  // Rotate3d::Vector hrHat(1.0, -b2.xx / b2.yy, 0);
  // Rotate3d::Vector hr = Rotate3d::normalize(hrHat);
  // double cosRoll = Rotate3d::dot(hr, b1);
  // double rollDeg = acos(cosRoll) * Rotate3d::Rad2Deg;
  // if (b1.zz > 0) rollDeg *= -1.0;
  
  Rotate3d::Vector ex(1.0, 0.0, 0.0);
  double cosRoll = Rotate3d::dot(b1, ex);
  double rollDeg = acos(cosRoll) * Rotate3d::Rad2Deg;
  if (b1.zz > 0) rollDeg *= -1.0;

  _rollDeg = rollDeg;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====================" << endl;
    cerr << "pitch deg: " << _pitchDeg << endl;
    cerr << "roll deg: " << _rollDeg << endl;
  }
  
}

//////////////////////////////////////////////////
// set up rotation matrix from pitch and roll

void FixRadxPointing::_computeRotationMatrix()
{
  
  Rotate3d::Matrix rotRoll = Rotate3d::createRotDegAboutY(_rollDeg);
  Rotate3d::Matrix rotPitch = Rotate3d::createRotDegAboutX(_pitchDeg);
  Rotate3d::Matrix rotHead = Rotate3d::createRotDegAboutZ(-_headingDeg);

  Rotate3d::Matrix rotPitchRoll = Rotate3d::mult(rotPitch, rotRoll);
  _rotMatrix = Rotate3d::mult(rotHead, rotPitchRoll);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====================" << endl;
    cerr << "rotation matrix:" << endl;
    Rotate3d::print(_rotMatrix, cerr);
  }

}

//////////////////////////////////////////////////
// print test values

void FixRadxPointing::_printTest()
{

  cerr << "=============================================" << endl;
  cerr << "Test table" << endl;

  double elDeg = 0.0;
  for (double azDeg = 0.0; azDeg <= 360.0; azDeg += 10.0) {
    
    // compute vector coords from el and az
    
    double sinEl, cosEl;
    double sinAz, cosAz;
    EG_sincos(elDeg * Rotate3d::Deg2Rad, &sinEl, &cosEl);
    EG_sincos(azDeg * Rotate3d::Deg2Rad, &sinAz, &cosAz);
    double xx = sinAz * cosEl;
    double yy = cosAz * cosEl;
    double zz = sinEl;
    
    // create vector
    
    Rotate3d::Vector measured(xx, yy, zz);
    
    // rotate the vector
    
    Rotate3d::Vector corrected = Rotate3d::mult(_rotMatrix, measured);
    
    double r = Rotate3d::norm(corrected);
    double elCorrDeg = asin(corrected.zz / r) * Rotate3d::Rad2Deg;
    double azCorrDeg = atan2(corrected.xx, corrected.yy) * Rotate3d::Rad2Deg;
    if (azCorrDeg < 0) {
      azCorrDeg += 360.0;
    }
    
    fprintf(stderr, "platEl, platAz, fixEl, fixAz: %10.2f %10.2f %10.2f %10.2f\n" ,
            elDeg, azDeg, elCorrDeg, azCorrDeg);
    
  } // azDeg

}

//////////////////////////////////////////////////
// fix a ray

void FixRadxPointing::_fixRay(RadxRay &ray)
{

  if (_params.correct_fixed_angles) {
    _fixRayFixedAngle(ray);
  }
  
  if (_params.correct_elevation_angles) {
    _fixRayElevation(ray);
  }
  
  if (_params.correct_azimuth_angles) {
    _fixRayAzimuth(ray);
  }

  if (_params.correct_for_platform_orientation) {
    _fixRayOrientation(ray);
  }

}

//////////////////////////////////////////////////
// fix a ray's fixed angle

void FixRadxPointing::_fixRayFixedAngle(RadxRay &ray)
{

  double fixedDeg = ray.getFixedAngleDeg();
  double fixedCorr = fixedDeg + _params.fixed_angle_offset;
  if (ray.getSweepMode() == Radx::SWEEP_MODE_RHI) {
    if (fixedCorr < 0) {
      fixedCorr += 360.0;
    } else if (fixedCorr >= 360.0) {
      fixedCorr -= 360.0;
    }
  }
  ray.setFixedAngleDeg(fixedCorr);

}

//////////////////////////////////////////////////
// fix a ray's elevation

void FixRadxPointing::_fixRayElevation(RadxRay &ray)
{

  double elDeg = ray.getElevationDeg();
  double elCorr = elDeg + _params.elevation_angle_offset;
  ray.setElevationDeg(elCorr);

}

//////////////////////////////////////////////////
// fix a ray's azimuth

void FixRadxPointing::_fixRayAzimuth(RadxRay &ray)
{
  
  double azDeg = ray.getAzimuthDeg();
  
  double azCorr = azDeg + _params.azimuth_angle_offset;
  if (azCorr < 0) {
    azCorr += 360.0;
  } else if (azCorr >= 360.0) {
    azCorr -= 360.0;
  }

  ray.setAzimuthDeg(azCorr);

}

//////////////////////////////////////////////////
// fix a ray's orienation

void FixRadxPointing::_fixRayOrientation(RadxRay &ray)
{

  double elDeg = ray.getElevationDeg();
  double azDeg = ray.getAzimuthDeg();

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Fixing ray:" << endl;
    cerr << "  measured  el, az: " << elDeg << ", " << azDeg << endl;
  }

  // compute vector coords from el and az

  double sinEl, cosEl;
  double sinAz, cosAz;
  EG_sincos(elDeg * Rotate3d::Deg2Rad, &sinEl, &cosEl);
  EG_sincos(azDeg * Rotate3d::Deg2Rad, &sinAz, &cosAz);
  double xx = sinAz * cosEl;
  double yy = cosAz * cosEl;
  double zz = sinEl;

  // create vector

  Rotate3d::Vector measured(xx, yy, zz);

  // rotate the vector

  Rotate3d::Vector corrected = Rotate3d::mult(_rotMatrix, measured);
  
  double r = Rotate3d::norm(corrected);
  double elCorrDeg = asin(corrected.zz / r) * Rotate3d::Rad2Deg;
  double azCorrDeg = atan2(corrected.xx, corrected.yy) * Rotate3d::Rad2Deg;
  if (azCorrDeg < 0) {
    azCorrDeg += 360.0;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "  corrected el, az: " << elCorrDeg << ", " << azCorrDeg << endl;
  }

  ray.setElevationDeg(elCorrDeg);
  ray.setAzimuthDeg(azCorrDeg);

}

