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
/////////////////////////////////////////////////////////////
// ForayNcRadxFile.cc
//
// Write methods for ForayNcRadxFile object
//
// NetCDF file data for radar radial data in FORAY format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/ForayNcRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// A separate file will be written out for each sweep.
//
// If addDaySubDir is true, a  subdir will be
// created with the name dir/yyyymmdd/
//
// If addYearSubDir is true, a subdir will be
// created with the name dir/yyyy/
//
// If both addDaySubDir and addYearSubDir are true,
// the subdir will be dir/yyyy/yyyymmdd/
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getDirInUse() for dir written
// Use getPathInUse() for path written

int ForayNcRadxFile::writeToDir(const RadxVol &vol,
                                const string &dir,
                                bool addDaySubDir,
                                bool addYearSubDir)
  
{

  // clear array of write paths
  
  _writePaths.clear();
  _writeDataTimes.clear();
  _volStartTimeSecs = vol.getStartTimeSecs();

  // write one file for each sweep

  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    
    const RadxSweep &sweep = *sweeps[ii];
    
    // construct new volume just for this sweep
    
    RadxVol *sweepVol = new RadxVol(vol, sweep.getSweepNumber());
    
    // write the sweep file

    if (_writeSweepToDir(*sweepVol, sweep.getSweepNumber(),
                         dir, addDaySubDir, addYearSubDir)) {
      delete sweepVol;
      return -1;
    }

    // clean up

    delete sweepVol;

  } // ii

  return 0;

}
  
// write single sweep to dir

int ForayNcRadxFile::_writeSweepToDir(RadxVol &vol,
                                      int sweepNum,
                                      const string &dir,
                                      bool addDaySubDir,
                                      bool addYearSubDir)
  
{
  
  clearErrStr();
  _dirInUse = dir;

  if (_debug) {
    cerr << "DEBUG - ForayNcRadxFile::_writeSweepToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }
  
  int nSweeps = vol.getSweeps().size();
  if (nSweeps != 1) {
    _addErrStr("ERROR - ForayNcRadxFile::_writeSweepToDir");
    _addErrStr("  No sweeps found");
    return -1;
  }

  // Dorade always written using the start time

  RadxTime ftime(vol.getStartTimeSecs());
  int millisecs = (int) (vol.getStartNanoSecs() / 1.0e6 + 0.5);

  string outDir(dir);
  if (addYearSubDir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_SEPARATOR, ftime.getYear());
    outDir += yearStr;
  }
  if (addDaySubDir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_SEPARATOR,
            ftime.getYear(), ftime.getMonth(), ftime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (makeDirRecurse(outDir)) {
    _addErrStr("ERROR - ForayNcRadxFile::_writeSweepToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute path
  
  const RadxSweep &sweep = *vol.getSweeps()[0];
  string scanType = Radx::sweepModeToShortStr(sweep.getSweepMode());
  double fixedAngle = sweep.getFixedAngleDeg();
  int volNum = vol.getVolumeNumber();
  string outName =
    _computeFileName(volNum, sweepNum, fixedAngle,
                     vol.getInstrumentName(), scanType,
                     ftime.getYear(), ftime.getMonth(), ftime.getDay(),
                     ftime.getHour(), ftime.getMin(), ftime.getSec(),
                     millisecs);
  string outPath(outDir);
  outPath += PATH_SEPARATOR;
  outPath += outName;
  
  int iret = _writeSweepToPath(vol, outPath);

  if (iret) {
    _addErrStr("ERROR - ForayNcRadxFile::_writeSweepToDir");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int ForayNcRadxFile::writeToPath(const RadxVol &vol,
                                 const string &path)
  
{
  
  // clear array of write paths

  _writePaths.clear();
  _writeDataTimes.clear();
  _volStartTimeSecs = vol.getStartTimeSecs();

  // If more than 1 sweep, just write the first one

  int nSweeps = vol.getSweeps().size();
  if (nSweeps < 1) {
    _addErrStr("ERROR - ForayNcRadxFile::writeToPath");
    _addErrStr("  No sweeps found");
    _addErrStr("  Path: ", path);
    return -1;
  }

  // copy the sweep

  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  const RadxSweep &sweep = *sweeps[0];
  RadxVol *sweepVol = new RadxVol(vol, sweep.getSweepNumber());

  if (_writeSweepToPath(*sweepVol, path)) {
    delete sweepVol;
    return -1;
  }
  
  delete sweepVol;

  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int ForayNcRadxFile::_writeSweepToPath(RadxVol &vol,
                                       const string &path)
  
{

  // check we have valid data

  if (vol.getNSweeps() == 0 || vol.getNRays() == 0) {
    _addErrStr("ERROR - ForayNcRadxFile::_writeSweepToPath");
    _addErrStr("  No rays in file: ", path);
    return -1;
  }

  // initialize

  clearErrStr();
  _writeVol = &vol;
  _pathInUse = path;
  vol.setPathInUse(path);

  // open the output Nc file
  
  _tmpPath = tmpPathFromFilePath(path, "");
  
  if (_debug) {
    cerr << "DEBUG - ForayNcRadxFile::_writeSweepToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path is: " << _tmpPath << endl;
    cerr << "  Writing fields ..." << endl;
  }

  if (_file.openWrite(_tmpPath, _getFileFormat(_ncFormat))) {
    _addErrStr("ERROR - ForayNcRadxFile::_writeSweepToPath");
    _addErrStr("  Cannot open tmp Nc file: ", _tmpPath);
    return -1;
  }

  // ensure that we have a constant number of gates
  // shorter rays are padded out with missing values

  _writeVol->setNGatesConstant();

  // load the fields from the rays, to make them contiguous

  _writeVol->loadFieldsFromRays();

  // add attributes, dimensions and variables

  if (_addGlobalAttributes()) {
    return _closeOnError("_addGlobalAttributes");
  }
  if (_addDimensions()) {
    return _closeOnError("_addDimensions");
  }
  if (_addTimeVariables()) {
    return _closeOnError("_addTimeVariables");
  }
  if (_addFieldNamesVariable()) {
    return _closeOnError("_addFieldNamesVariable");
  }
  if (_addScalarVariables()) {
    return _closeOnError("_addScalarVariables");
  }
  if (_addNumSystemsVariables()) {
    return _closeOnError("_addNumSystemsVariables");
  }
  if (_addTimeArrayVariables()) {
    return _closeOnError("_addTimeArrayVariables");
  }
  if (_addDataFieldVariables()) {
    return _closeOnError("_addDataFieldVariables");
  }

  // write variables

  if (_writeFieldNamesVariable()) {
    return _closeOnError("_writeFieldNamesVariable");
  }

  if (_writeScalarVariables()) {
    return _closeOnError("_writeScalarVariables");
  }

  if (_writeRadarVariables()) {
    return _closeOnError("_writeRadarVariables");
  }

  if (_writeVol->getNRcalibs() > 0) {
    if (_writeCalibDataPresent(1)) {
      return _closeOnError("_writeCalibDataPresent");
    }
    const RadxRcalib &calib = *_writeVol->getRcalibs()[0];
    if (_writeCalibVariables(calib)) {
      return _closeOnError("_writeCalibVariables");
    }
  } else {
    if (_writeCalibDataPresent(0)) {
      return _closeOnError("_writeCalibDataPresent");
    }
  }

  if (_writeTimeOffsetVariable()) {
    return _closeOnError("_writeTimeOffsetVariable");
  }
  if (_writeAngleVariables()) {
    return _closeOnError("_writeAngleVariables");
  }
  if (_writeClipRangeVariable()) {
    return _closeOnError("_writeClipRangeVariable");
  }
  if (_writeDataFieldVariables()) {
    return _closeOnError("_writeDataFieldVariables");
  }

  // close output file

  _file.close();

  // rename the tmp to final output file path
  
  if (rename(_tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - ForayNcRadxFile::_writeSweepToPath");
    _addErrStr("  Cannot rename tmp file: ", _tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - ForayNcRadxFile::_writeSweepToPath" << endl;
    cerr << "  Renamed tmp path: " << _tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());
  
  return 0;

  if (_debug) {
    cerr << "DEBUG - ForayNcRadxFile::_writeSweepToPath" << endl;
    cerr << "  Renamed tmp path: " << _tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  // add to array of write paths

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());

  return 0;

}

//////////////////
// close on error

int ForayNcRadxFile::_closeOnError(const string &caller)
{
  _addErrStr("ERROR - ForayNcRadxFile::" + caller);
  _addErrStr(_file.getErrStr());
  _file.close();
  unlink(_tmpPath.c_str());
  return -1;
}

////////////////////////////////////////////////////////////
// compute and return dorade file name

string ForayNcRadxFile::_computeFileName(int volNum,
                                         int sweepNum,
                                         double fixedAngle,
                                         string instrumentName,
                                         string scanType,
                                         int year, int month, int day,
                                         int hour, int min, int sec,
                                         int millisecs)
  
{

  // make sure instrument name is reasonable length

  if (instrumentName.size() > 8) {
    instrumentName.resize(8);
  }

  // replaces spaces in strings with underscores

  for (size_t ii = 0; ii < instrumentName.size(); ii++) {
    if (isspace(instrumentName[ii])) {
      instrumentName[ii] = '_';
    }
  }
  for (size_t ii = 0; ii < scanType.size(); ii++) {
    if (isspace(scanType[ii] == ' ')) {
      scanType[ii] = '_';
    }
  }

  char outName[BUFSIZ];
  sprintf(outName,
          "ncswp_%s_%04d%02d%02d_%02d%02d%02d.%03d_v%03d_s%03d_%05.1f_%s_.nc",
          instrumentName.c_str(),
          year, month, day,
          hour, min, sec,
          millisecs,
          volNum,
          sweepNum,
          fixedAngle,
          scanType.c_str());

  return outName;

}
  
///////////////////////////////////////////////////////////////
// addGlobalAttributes()
//

int ForayNcRadxFile::_addGlobalAttributes()
{

  const RadxSweep *sweep = _writeVol->getSweeps()[0];
  const RadxRay *ray0 = _writeVol->getRays()[0];
  
  if (_verbose) {
    cerr << "ForayNcRadxFile::_addGlobalAttributes()" << endl;
  }

  // Add required global attributes

  string conventions = "NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor";
  if (_file.addGlobAttr("Conventions" , conventions)) {
    return -1;
  }
  
  if (_file.addGlobAttr("Instrument_Name",  _writeVol->getInstrumentName())) {
    return -1;
  }

  if (_writeVol->getPlatformType() == Radx::PLATFORM_TYPE_FIXED) {
    if (_file.addGlobAttr("Instrument_Type", "Ground")) {
      return -1;
    }
  } else {
    if (_file.addGlobAttr("Instrument_Type", 
                          Radx::platformTypeToStr(_writeVol->getPlatformType()))) {
      return -1;
    }
  }
  
  string scanModeStr("unknown");
  if (sweep != NULL) {
    scanModeStr = Radx::sweepModeToShortStr(sweep->getSweepMode());
  }
  if (_file.addGlobAttr("Scan_Mode", scanModeStr)) {
    return -1;
  }

  RadxTime startTime(_writeVol->getStartTimeSecs());
  char startTimeStr[1024];
  sprintf(startTimeStr, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
          startTime.getYear(), startTime.getMonth(), startTime.getDay(),
          startTime.getHour(), startTime.getMin(), startTime.getSec());
  if (_file.addGlobAttr("Volume_Start_Time", startTimeStr)) {
    return -1;
  }

  if (_file.addGlobAttr("Year", startTime.getYear())) {
    return -1;
  }
  if (_file.addGlobAttr("Month", startTime.getMonth())) {
    return -1;
  }
  if (_file.addGlobAttr("Day", startTime.getDay())) {
    return -1;
  }
  if (_file.addGlobAttr("Hour", startTime.getHour())) {
    return -1;
  }
  if (_file.addGlobAttr("Minute", startTime.getMin())) {
    return -1;
  }
  if (_file.addGlobAttr("Second", startTime.getSec())) {
    return -1;
  }

  if (_file.addGlobAttr("Volume_Number", _writeVol->getVolumeNumber())) {
    return -1;
  }

  if (_file.addGlobAttr("Scan_Number", sweep->getSweepNumber())) {
    return -1;
  }

  if (_file.addGlobAttr("Num_Samples", (int) ray0->getNSamples())) {
    return -1;
  }

  if (_file.addGlobAttr("Project_Name",  _writeVol->getSiteName())) {
    return -1;
  }

  RadxTime now(time(NULL));
  char nowStr[1024];
  sprintf(nowStr, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
          now.getYear(), now.getMonth(), now.getDay(),
          now.getHour(), now.getMin(), now.getSec());
  if (_file.addGlobAttr("Production_Date", nowStr)) {
    return -1;
  }

  if (_file.addGlobAttr("Producer_Name", _writeVol->getInstitution())) {
    return -1;
  }

  if (_file.addGlobAttr("Software", "Radx ForayNcRadxFile class")) {
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// addDimensions()
//
//  Add Nc3Dims to the NetCDF file. We loop through the
//  GridInfo objects and record the dimensions of the
//  x and y coordinates. Then we loop through the VlevelInfo
//  objects and record the dimensions of the vertical coordinates

int ForayNcRadxFile::_addDimensions()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_addDimensions()" << endl;
  }

  // add time dimension - unlimited??

  if (_file.addDim(_TimeDim, TIME, -1)) {
    // if (_file.addDim(_TimeDim, TIME, _writeVol->getRays().size())) {
    return -1;
  }

  // add number of gates dimension

  if (_file.addDim(_maxCellsDim, "maxCells", _writeVol->getMaxNGates())) {
    return -1;
  }

  // add number of system dimension

  if (_file.addDim(_numSystemsDim, "numSystems", 1)) {
    return -1;
  }

  // add number of fields dimension

  if (_file.addDim(_fieldsDim, "fields", _writeVol->getNFields())) {
    return -1;
  }

  // add string length dims

  if (_file.addDim(_short_stringDim, "short_string", SHORT_STRING_LEN)) {
    return -1;
  }
  if (_file.addDim(_long_stringDim, "long_string", LONG_STRING_LEN)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// add time variables

int ForayNcRadxFile::_addTimeVariables()
{

  int iret = 0;
  if (_verbose) {
    cerr << "ForayNcRadxFile::_addTimeVariables()" << endl;
  }

  iret |= _addTimeVar(_volume_start_timeVar, "volume_start_time",
                      "Unix Date/Time value for volume start time",
                      "seconds since 1970-01-01 00:00 UTC");

  iret |= _addTimeVar(_base_timeVar, "base_time",
                      "Unix Date/Time value for first record",
                      "seconds since 1970-01-01 00:00 UTC");

  if (iret) {
    _addErrStr("ERROR - ForayNcRadxFile::_addTimeVariables");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add field names variable

int ForayNcRadxFile::_addFieldNamesVariable()
{
  
  _field_namesVar = _file.getNc3File()->add_var("field_names",  nc3Char,
                                               _fieldsDim, _short_stringDim);

  if (_field_namesVar == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_addFieldNamesVariable");
    _addErrStr("  Cannot add var field_names");
    _addErrStr("  Type: ", _file.ncTypeToStr(nc3Char));
    _addErrStr("  Dim0: ", _fieldsDim->name());
    _addErrStr("  Dim1: ", _short_stringDim->name());
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// add scalar variables

int ForayNcRadxFile::_addScalarVariables()
{

  int iret = 0;
  if (_verbose) {
    cerr << "ForayNcRadxFile::_addScalarVariables()" << endl;
  }

  iret |= _addVar(_Fixed_AngleVar, nc3Float, "Fixed_Angle", 
                  "Targeted fixed angle for this scan", "degrees");
  
  iret |= _addVar(_Range_to_First_CellVar, nc3Float, "Range_to_First_Cell", 
                  "Range to the center of the first cell", "meters");
  
  iret |= _addVar(_Cell_Spacing_MethodVar, nc3Int, "Cell_Spacing_Method", 
                  "Technique for recording cell spacing: 0 = by vector, 1 = by segment", 
                  "");
  iret |= _addVar(_Cell_SpacingVar, nc3Float, "Cell_Spacing", 
                  "Distance between cells", "meters");
  
  iret |= _addVar(_Nyquist_VelocityVar, nc3Float, "Nyquist_Velocity", 
                  "Effective unambigous velocity", "meters/second");
  
  iret |= _addVar(_Unambiguous_RangeVar, nc3Float, "Unambiguous_Range", 
                  "Effective unambigous range", "meters");
  
  iret |= _addVar(_LatitudeVar, nc3Double, "Latitude", 
                  "Latitude of the instrument", "degrees");
  
  double range[2];
  range[0] = -90.0;
  range[1] = 90.0;
  _LatitudeVar->add_att("valid_range", 2, range);

  iret |= _addVar(_LongitudeVar, nc3Double, "Longitude", 
                  "Longitude of the instrument", "degrees");
  
  range[0] = -360.0;
  range[1] = 360.0;
  _LongitudeVar->add_att("valid_range", 2, range);

  iret |= _addVar(_AltitudeVar, nc3Double, "Altitude", 
                  "Altitude in meters (asl) of the instrument", "meters");
  
  range[0] = -10000.0;
  range[1] = 90000.0;
  _AltitudeVar->add_att("valid_range", 2, range);

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variables with numSystems dimension

int ForayNcRadxFile::_addNumSystemsVariables()
{
  
  if (_verbose) {
    cerr << "ForayNcRadxFile::_addNumSystemsVariables()" << endl;
  }
  
  int iret = 0;

  iret |= _addVar(_Radar_ConstantVar, _numSystemsDim, nc3Float,
                  "Radar_Constant",
                  "Radar Constant",
                  "mm6/(m3.mW.km-2)");

  iret |= _addVar(_rcvr_gainVar, _numSystemsDim, nc3Float,
                  "rcvr_gain",
                  "Receiver Gain",
                  "dB");

  iret |= _addVar(_ant_gainVar, _numSystemsDim, nc3Float,
                  "ant_gain",
                  "Antenna Gain",
                  "dB");

  iret |= _addVar(_sys_gainVar, _numSystemsDim, nc3Float,
                  "sys_gain",
                  "System Gain",
                  "dB");

  iret |= _addVar(_bm_widthVar, _numSystemsDim, nc3Float,
                  "bm_width",
                  "Beam Width",
                  "degrees");

  iret |= _addVar(_pulse_widthVar, _numSystemsDim, nc3Float,
                  "pulse_width",
                  "Pulse Width",
                  "seconds");

  iret |= _addVar(_band_widthVar, _numSystemsDim, nc3Float,
                  "band_width",
                  "Band Width",
                  "hertz");

  iret |= _addVar(_peak_pwrVar, _numSystemsDim, nc3Float,
                  "peak_pwr",
                  "Peak Power",
                  "watts");

  iret |= _addVar(_xmtr_pwrVar, _numSystemsDim, nc3Float,
                  "xmtr_pwr",
                  "Transmitter Power",
                  "dBM");

  iret |= _addVar(_noise_pwrVar, _numSystemsDim, nc3Float,
                  "noise_pwr",
                  "Noise Power",
                  "dBM");

  iret |= _addVar(_tst_pls_pwrVar, _numSystemsDim, nc3Float,
                  "tst_pls_pwr",
                  "Test Pulse Power",
                  "dBM");

  iret |= _addVar(_tst_pls_rng0Var, _numSystemsDim, nc3Float,
                  "tst_pls_rng0",
                  "Range to start of test pulse",
                  "meters");

  iret |= _addVar(_tst_pls_rng1Var, _numSystemsDim, nc3Float,
                  "tst_pls_rng1",
                  "Range to end of test pulse",
                  "meters");

  iret |= _addVar(_WavelengthVar, _numSystemsDim, nc3Float,
                  "Wavelength",
                  "System wavelength",
                  "meters");

  iret |= _addVar(_PRFVar, _numSystemsDim, nc3Float,
                  "PRF",
                  "System pulse repetition frequency",
                  "pulses/sec");

  iret |= _addVar(_calibration_data_presentVar, _numSystemsDim, nc3Int,
                  "calibration_data_present",
                  "Used as bool; 0 = calibration variables used, "
                  "1 = calibration variables not used",
                  "");

  iret |= _addVar(_ant_gain_h_dbVar, _numSystemsDim, nc3Float,
                  "ant_gain_h_db",
                  "Antenna gain H in db",
                  "dB");

  iret |= _addVar(_ant_gain_v_dbVar, _numSystemsDim, nc3Float,
                  "ant_gain_v_db",
                  "Antenna gain V in db",
                  "dB");

  iret |= _addVar(_xmit_power_h_dbmVar, _numSystemsDim, nc3Float,
                  "xmit_power_h_dbm",
                  "Peak transmit H power in dBm",
                  "dBm");

  iret |= _addVar(_xmit_power_v_dbmVar, _numSystemsDim, nc3Float,
                  "xmit_power_v_dbm",
                  "Peak transmit V power in dBm",
                  "dBm");

  iret |= _addVar(_two_way_waveguide_loss_h_dbVar, _numSystemsDim, nc3Float,
                  "two_way_waveguide_loss_h_db",
                  "two way H waveguide loss in dB",
                  "dB");

  iret |= _addVar(_two_way_waveguide_loss_v_dbVar, _numSystemsDim, nc3Float,
                  "two_way_waveguide_loss_v_db",
                  "two way V waveguide loss in dB",
                  "dB");

  iret |= _addVar(_two_way_radome_loss_h_dbVar, _numSystemsDim, nc3Float,
                  "two_way_radome_loss_h_db",
                  "two way H radome loss in dB",
                  "dB");

  iret |= _addVar(_two_way_radome_loss_v_dbVar, _numSystemsDim, nc3Float,
                  "two_way_radome_loss_v_db",
                  "two way V radome loss in dB",
                  "dB");

  iret |= _addVar(_receiver_mismatch_loss_dbVar, _numSystemsDim, nc3Float,
                  "receiver_mismatch_loss_db",
                  "Receiver mismatch loss in dB",
                  "dB");

  iret |= _addVar(_radar_constant_hVar, _numSystemsDim, nc3Float,
                  "radar_constant_h",
                  "Radar constant H",
                  "mm6/(m3.mW.km-2)");

  iret |= _addVar(_radar_constant_vVar, _numSystemsDim, nc3Float,
                  "radar_constant_v",
                  "Radar constant V",
                  "mm6/(m3.mW.km-2)");

  iret |= _addVar(_noise_hc_dbmVar, _numSystemsDim, nc3Float,
                  "noise_hc_dbm",
                  "calibrated moise value H co-polar",
                  "dBm");

  iret |= _addVar(_noise_vc_dbmVar, _numSystemsDim, nc3Float,
                  "noise_vc_dbm",
                  "calibrated moise value V co-polar",
                  "dBm");

  iret |= _addVar(_noise_hx_dbmVar, _numSystemsDim, nc3Float,
                  "noise_hx_dbm",
                  "calibrated moise value H cross-polar",
                  "dBm");

  iret |= _addVar(_noise_vx_dbmVar, _numSystemsDim, nc3Float,
                  "noise_vx_dbm",
                  "calibrated moise value V cross-polar",
                  "dBm");

  iret |= _addVar(_receiver_gain_hc_dbVar, _numSystemsDim, nc3Float,
                  "receiver_gain_hc_db",
                  "Receiver gain H co-polar",
                  "dB");

  iret |= _addVar(_receiver_gain_vc_dbVar, _numSystemsDim, nc3Float,
                  "receiver_gain_vc_db",
                  "Receiver gain V co-polar",
                  "dB");

  iret |= _addVar(_receiver_gain_hx_dbVar, _numSystemsDim, nc3Float,
                  "receiver_gain_hx_db",
                  "Receiver gain H cross-polar",
                  "dB");

  iret |= _addVar(_receiver_gain_vx_dbVar, _numSystemsDim, nc3Float,
                  "receiver_gain_vx_db",
                  "Receiver gain V cross-polar",
                  "dB");

  iret |= _addVar(_base_1km_hc_dbzVar, _numSystemsDim, nc3Float,
                  "base_1km_hc_dbz",
                  "Base reflectivity at 1km, H co-polar",
                  "dBz");

  iret |= _addVar(_base_1km_vc_dbzVar, _numSystemsDim, nc3Float,
                  "base_1km_vc_dbz",
                  "Base reflectivity at 1km, V co-polar",
                  "dBz");

  iret |= _addVar(_base_1km_hx_dbzVar, _numSystemsDim, nc3Float,
                  "base_1km_hx_dbz",
                  "Base reflectivity at 1km, H cross-polar",
                  "dBz");

  iret |= _addVar(_base_1km_vx_dbzVar, _numSystemsDim, nc3Float,
                  "base_1km_vx_dbz",
                  "Base reflectivity at 1km, V cross-polar",
                  "dBz");

  iret |= _addVar(_sun_power_hc_dbmVar, _numSystemsDim, nc3Float,
                  "sun_power_hc_dbm",
                  "Sun power H co-polar",
                  "dBm");

  iret |= _addVar(_sun_power_vc_dbmVar, _numSystemsDim, nc3Float,
                  "sun_power_vc_dbm",
                  "Sun power V co-polar",
                  "dBm");

  iret |= _addVar(_sun_power_hx_dbmVar, _numSystemsDim, nc3Float,
                  "sun_power_hx_dbm",
                  "Sun power H cross-polar",
                  "dBm");

  iret |= _addVar(_sun_power_vx_dbmVar, _numSystemsDim, nc3Float,
                  "sun_power_vx_dbm",
                  "Sun power V cross-polar",
                  "dBm");

  iret |= _addVar(_noise_source_power_h_dbmVar, _numSystemsDim, nc3Float,
                  "noise_source_power_h_dbm",
                  "Noise source power H in dBm",
                  "dBm");

  iret |= _addVar(_noise_source_power_v_dbmVar, _numSystemsDim, nc3Float,
                  "noise_source_power_v_dbm",
                  "Noise source power V in dBm",
                  "dBm");

  iret |= _addVar(_power_measure_loss_h_dbVar, _numSystemsDim, nc3Float,
                  "power_measure_loss_h_db",
                  "power measurement loss H in dB",
                  "dB");

  iret |= _addVar(_power_measure_loss_v_dbVar, _numSystemsDim, nc3Float,
                  "power_measure_loss_v_db",
                  "power measurement loss V in dB",
                  "dB");

  iret |= _addVar(_coupler_forward_loss_h_dbVar, _numSystemsDim, nc3Float,
                  "coupler_forward_loss_h_db",
                  "Directional coupler forward loss H in dB",
                  "dB");

  iret |= _addVar(_coupler_forward_loss_v_dbVar, _numSystemsDim, nc3Float,
                  "coupler_forward_loss_v_db",
                  "Directional coupler forward loss V in dB",
                  "dB");

  iret |= _addVar(_zdr_correction_dbVar, _numSystemsDim, nc3Float,
                  "zdr_correction_db",
                  "zdr correction in dB",
                  "dB");

  iret |= _addVar(_ldr_correction_h_dbVar, _numSystemsDim, nc3Float,
                  "ldr_correction_h_db",
                  "ldr correction H in dB",
                  "dB");

  iret |= _addVar(_ldr_correction_v_dbVar, _numSystemsDim, nc3Float,
                  "ldr_correction_v_db",
                  "ldr correction V in dB",
                  "dB");

  iret |= _addVar(_system_phidp_degVar, _numSystemsDim, nc3Float,
                  "system_phidp_deg",
                  "system phidp, degrees",
                  "degrees");

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variables with Time dimension

int ForayNcRadxFile::_addTimeArrayVariables()
{
  
  if (_verbose) {
    cerr << "ForayNcRadxFile::_addTimeArrayVariables()" << endl;
  }
  
  int iret = 0;
  
  iret |= _addTimeOffsetVar(_time_offsetVar, _TimeDim,
                            "time_offset",
                            "time offset of the current record from base_time",
                            "seconds");
  
  iret |= _addVar(_AzimuthVar, _TimeDim, nc3Float,
                  "Azimuth",
                  "Earth relative azimuth of the ray",
                  "degrees");

  double range[2] = { -360.0, 360.0 };
  _AzimuthVar->add_att("valid_range", 2, range);
  _AzimuthVar->add_att("comment", "Degrees clockwise from true North");

  iret |= _addVar(_ElevationVar, _TimeDim, nc3Float,
                  "Elevation",
                  "Earth relative elevation of the ray",
                  "degrees");

  _ElevationVar->add_att("valid_range", 2, range);
  _ElevationVar->add_att("comment", "Degrees from earth tangent towards zenith");
  
  iret |= _addVar(_clip_rangeVar, _TimeDim, nc3Float,
                  "clip_range",
                  "Range of last useful cell",
                  "meters");

  return iret;

}

//////////////////////////////////////////////
// add data field variables

int ForayNcRadxFile::_addDataFieldVariables()
{
  
  if (_verbose) {
    cerr << "ForayNcRadxFile::_addDataFieldVariables()" << endl;
  }
  
  const RadxSweep *sweep = _writeVol->getSweeps()[0];
  const RadxRay *ray0 = _writeVol->getRays()[0];
  
  _dataFieldVars.clear();

  for (size_t ifield = 0; ifield < _writeVol->getNFields(); ifield++) {
    
    RadxField &field = *_writeVol->getFields()[ifield];
    field.convertToSi16();
    
    Nc3Var *var = _file.getNc3File()->add_var(field.getName().c_str(),
                                            nc3Short, _TimeDim, _maxCellsDim);
    if (var == NULL) {
      _addErrStr("ERROR - ForayNcRadxFile::_addDataFieldVariables");
      _addErrStr("  Cannot add data field var, name: ", field.getName());
      _addErrStr("  Type: ", _file.ncTypeToStr(nc3Short));
      _addErrStr("  Dim0: ", _TimeDim->name());
      _addErrStr("  Dim1: ", _maxCellsDim->name());
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

    if (field.getLongName().size() > 0) {
      if (_file.addAttr(var, LONG_NAME, field.getLongName().c_str())) {
        return -1;
      }
    }
    
    if (field.getUnits().size() > 0) {
      if (_file.addAttr(var, UNITS, field.getUnits().c_str())) {
        return -1;
      }
    }
    
    if (_file.addAttr(var, SCALE_FACTOR, (float) field.getScale())) {
      return -1;
    }

    if (_file.addAttr(var, ADD_OFFSET, (float) field.getOffset())) {
      return -1;
    }
    
    if (_file.addAttr(var, FILL_VALUE, (short) field.getMissingSi16())) {
      return -1;
    }
    
    if (_file.addAttr(var, MISSING_VALUE, (short) field.getMissingSi16())) {
      return -1;
    }
    
    if (_file.addAttr(var, "polarization",
                      Radx::polarizationModeToStr(sweep->getPolarizationMode()))) {
      return -1;
    }

    const vector<double> &freqs = _writeVol->getFrequencyHz();
    if (freqs.size() > 0) {
      if (_file.addAttr(var, "Frequencies_GHz",
                        freqs[0] / 1.0e9)) {
        return -1;
      }
    }

    if (_file.addAttr(var, "InterPulsePeriods_secs",
                      ray0->getPrtSec())) {
      return -1;
    }

    if (_file.addAttr(var, "num_segments", 1)) {
      return -1;
    }

    if (_file.addAttr(var, "cells_in_segment",
                      (int) ray0->getNGates())) {
      return -1;
    }

    if (_file.addAttr(var, "meters_between_cells",
                      ray0->getGateSpacingKm() * 1000.0)) {
      return -1;
    }

    if (_file.addAttr(var, "meters_to_first_cell",
                      ray0->getStartRangeKm() * 1000.0)) {
      return -1;
    }

    // set compression
    
    _setCompression(var);
    
    // save
    
    _dataFieldVars.push_back(var);
    
  } // ifield
    
  return 0;

}

//////////////////////////////////////////////
// add time variable

int ForayNcRadxFile::_addTimeVar(Nc3Var* &var,
                                 const string &name,
                                 const string &longName,
                                 const string &units)

{
  
  var = _file.getNc3File()->add_var(name.c_str(), nc3Int);
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_addTimeVar");
    _addErrStr("  Cannot add time var, name: ", name);
    _addErrStr("  Type: ", _file.ncTypeToStr(nc3Int));
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (longName.length() > 0) {
    if (_file.addAttr(var, LONG_NAME, longName)) {
      return -1;
    }
  }
  
  if (units.length() > 0) {
    if (_file.addAttr(var, UNITS, units)) {
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////
// add variable

int ForayNcRadxFile::_addVar(Nc3Var* &var,
                             Nc3Type ncType,
                             const string &name,
                             const string &longName,
                             const string &units)
  
{
  
  var = _file.getNc3File()->add_var(name.c_str(), ncType);
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_addVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", _file.ncTypeToStr(ncType));
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (longName.length() > 0) {
    if (_file.addAttr(var, LONG_NAME, longName)) {
      return -1;
    }
  }
  
  if (units.length() > 0) {
    if (_file.addAttr(var, UNITS, units)) {
      return -1;
    }
  }
  
  switch (ncType) {
    case nc3Double:
      if (_file.addAttr(var, FILL_VALUE, missingDouble)) {
        return -1;
      }
      if (_file.addAttr(var, MISSING_VALUE, missingDouble)) {
        return -1;
      }
      break;
    case nc3Float:
      if (_file.addAttr(var, FILL_VALUE, missingFloat)) {
        return -1;
      }
      if (_file.addAttr(var, MISSING_VALUE, missingFloat)) {
        return -1;
      }
      break;
    case nc3Int:
    default:
      if (_file.addAttr(var, FILL_VALUE, missingInt)) {
        return -1;
      }
      if (_file.addAttr(var, MISSING_VALUE, missingInt)) {
        return -1;
      }
  }
  
  return 0;
  
}

int ForayNcRadxFile::_addVar(Nc3Var* &var,
                             Nc3Dim *dim,
                             Nc3Type ncType,
                             const string &name,
                             const string &longName,
                             const string &units)
  
{
  
  var = _file.getNc3File()->add_var(name.c_str(), ncType, dim);
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_addVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", _file.ncTypeToStr(ncType));
    _addErrStr("  Dim: ", dim->name());
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (longName.length() > 0) {
    if (_file.addAttr(var, LONG_NAME, longName)) {
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }
  
  if (units.length() > 0) {
    if (_file.addAttr(var, UNITS, units)) {
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }
  
  switch (ncType) {
    case nc3Double:
      if (_file.addAttr(var, FILL_VALUE, missingDouble)) {
        _addErrStr(_file.getErrStr());
        return -1;
      }
      if (_file.addAttr(var, MISSING_VALUE, missingDouble)) {
        _addErrStr(_file.getErrStr());
        return -1;
      }
      break;
    case nc3Float:
      if (_file.addAttr(var, FILL_VALUE, missingFloat)) {
        _addErrStr(_file.getErrStr());
        return -1;
      }
      if (_file.addAttr(var, MISSING_VALUE, missingFloat)) {
        _addErrStr(_file.getErrStr());
        return -1;
      }
      break;
    case nc3Int:
    default:
      if (_file.addAttr(var, FILL_VALUE, (int) -32768)) {
        _addErrStr(_file.getErrStr());
        return -1;
      }
      if (_file.addAttr(var, MISSING_VALUE, (int) -32768)) {
        _addErrStr(_file.getErrStr());
        return -1;
      }
  }
  
  return 0;
  
}

int ForayNcRadxFile::_addTimeOffsetVar(Nc3Var* &var,
                                       Nc3Dim *dim,
                                       const string &name,
                                       const string &longName,
                                       const string &units)

{
  
  var = _file.getNc3File()->add_var(name.c_str(), nc3Double, dim);
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_addTimeOffsetVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", _file.ncTypeToStr(nc3Double));
    _addErrStr("  Dim: ", dim->name());
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  if (longName.length() > 0) {
    if (_file.addAttr(var, LONG_NAME, longName)) {
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }

  if (units.length() > 0) {
    if (_file.addAttr(var, UNITS, units)) {
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }
  
  if (_file.addAttr(var, FILL_VALUE, 0.0)) {
      _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_file.addAttr(var, MISSING_VALUE, 0.0)) {
      _addErrStr(_file.getErrStr());
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////
// write field names variables

int ForayNcRadxFile::_writeFieldNamesVariable()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeFieldNamesVariable()" << endl;
  }
  
  size_t nFields = _writeVol->getNFields();
  ShortString_t *strings = new ShortString_t[nFields];
  
  for (size_t ii = 0; ii < nFields; ii++) {
    const RadxField &field = *_writeVol->getFields()[ii];
    memset(strings[ii], 0, sizeof(ShortString_t));
    strncpy(strings[ii], field.getName().c_str(),
            sizeof(ShortString_t) - 1);
  }

  if (_file.writeStringVar(_field_namesVar, strings)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  delete[] strings;
  return 0;

}

////////////////////////////////////////////////
// write scalar variables

int ForayNcRadxFile::_writeScalarVariables()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeScalarVariables()" << endl;
  }

  const RadxSweep *sweep = _writeVol->getSweeps()[0];
  const RadxRay *ray0 = _writeVol->getRays()[0];

  // start time
  
  if (_file.writeVar(_volume_start_timeVar, (int) _volStartTimeSecs)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // base time
  
  if (_file.writeVar(_base_timeVar,
                     (int) _writeVol->getStartTimeSecs())) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // fixed angle

  if (_file.writeVar(_Fixed_AngleVar,
                     (float) sweep->getFixedAngleDeg())) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // start range

  if (_file.writeVar(_Range_to_First_CellVar,
                     (float) (ray0->getStartRangeKm() * 1.0e3))) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // gate spacing

  if (_file.writeVar(_Cell_Spacing_MethodVar, 1)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  if (_file.writeVar(_Cell_SpacingVar,
                     (float) (ray0->getGateSpacingKm() * 1.0e3))) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // nyquist, unambig range

  if (_file.writeVar(_Nyquist_VelocityVar,
                     (float) ray0->getNyquistMps())) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_file.writeVar(_Unambiguous_RangeVar,
                     (float) (ray0->getUnambigRangeKm() * 1.0e3))) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // position

  if (_file.writeVar(_LatitudeVar, _writeVol->getLatitudeDeg())) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_file.writeVar(_LongitudeVar, _writeVol->getLongitudeDeg())) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_file.writeVar(_AltitudeVar, _writeVol->getAltitudeKm() * 1.0e3)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// write radar variables

int ForayNcRadxFile::_writeRadarVariables()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeRadarVariables()" << endl;
  }

  float Radar_Constant = missingFloat;
  float rcvr_gain = missingFloat;
  float ant_gain = missingFloat;
  float sys_gain = missingFloat;
  float bm_width = missingFloat;
  float pulse_width = missingFloat;
  float band_width = missingFloat;
  float peak_pwr = missingFloat;
  float xmtr_pwr = missingFloat;
  float noise_pwr = missingFloat;
  float tst_pls_pwr = missingFloat;
  float tst_pls_rng0 = missingFloat;
  float tst_pls_rng1 = missingFloat;
  float Wavelength = missingFloat;
  float PRF = missingFloat;

  const RadxRay *ray0 = _writeVol->getRays()[0];
  double prtSec = ray0->getPrtSec();
  double prf = 1.0 / prtSec;
  double pulseWidthSec = ray0->getPulseWidthUsec() / 1.0e6;
  double dutyCycle = 1.0 / (prf * pulseWidthSec);
  
  ant_gain =  (float) _writeVol->getRadarAntennaGainDbH();
  bm_width = (float) _writeVol->getRadarBeamWidthDegH();
  pulse_width = (float) pulseWidthSec;
  band_width = (float) _writeVol->getRadarReceiverBandwidthMhz();
  Wavelength = (float) _writeVol->getWavelengthM();
  PRF = (float) prf;

  if (_writeVol->getNRcalibs() > 0) {
    const RadxRcalib &calib = *_writeVol->getRcalibs()[0];
    double peakPowerDbm = calib.getXmitPowerDbmH();
    double peakPowerWatts = pow(10.0, peakPowerDbm / 10.0) / 1.0e3;
    double meanPowerWatts = peakPowerWatts * dutyCycle;
    double meanPowerDbm = 10.0 * log10(meanPowerWatts * 1.0e3);
    
    Radar_Constant = (float) calib.getRadarConstantH();
    rcvr_gain = (float) calib.getReceiverGainDbHc();
    noise_pwr = 
      (float) (calib.getNoiseDbmHc() - calib.getReceiverGainDbHc());
    tst_pls_pwr = (float) calib.getTestPowerDbmH();
    peak_pwr = (float) peakPowerWatts;
    xmtr_pwr = (float) meanPowerDbm;

  }
  
  int iret = 0;
  
  iret |= _writeNumSystemsVar(_Radar_ConstantVar, Radar_Constant);
  iret |= _writeNumSystemsVar(_rcvr_gainVar, rcvr_gain);
  iret |= _writeNumSystemsVar(_ant_gainVar, ant_gain);
  iret |= _writeNumSystemsVar(_sys_gainVar, sys_gain);
  iret |= _writeNumSystemsVar(_bm_widthVar, bm_width);
  iret |= _writeNumSystemsVar(_pulse_widthVar, pulse_width);
  iret |= _writeNumSystemsVar(_band_widthVar, band_width);
  iret |= _writeNumSystemsVar(_peak_pwrVar, peak_pwr);
  iret |= _writeNumSystemsVar(_xmtr_pwrVar, xmtr_pwr);
  iret |= _writeNumSystemsVar(_noise_pwrVar, noise_pwr);
  iret |= _writeNumSystemsVar(_tst_pls_pwrVar, tst_pls_pwr);
  iret |= _writeNumSystemsVar(_tst_pls_rng0Var, tst_pls_rng0);
  iret |= _writeNumSystemsVar(_tst_pls_rng1Var, tst_pls_rng1);
  iret |= _writeNumSystemsVar(_WavelengthVar, Wavelength);
  iret |= _writeNumSystemsVar(_PRFVar, PRF);

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write calibration variables

int ForayNcRadxFile::_writeCalibVariables(const RadxRcalib &calib)
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeCalibVariables()" << endl;
  }

  int iret = 0;
  
  iret |= _writeNumSystemsVar(_ant_gain_h_dbVar,
                              (float) calib.getAntennaGainDbH());
  iret |= _writeNumSystemsVar(_ant_gain_v_dbVar,
                              (float) calib.getAntennaGainDbV());
  iret |= _writeNumSystemsVar(_xmit_power_h_dbmVar,
                              (float) calib.getXmitPowerDbmH());
  iret |= _writeNumSystemsVar(_xmit_power_v_dbmVar,
                              (float) calib.getXmitPowerDbmV());
  iret |= _writeNumSystemsVar(_two_way_waveguide_loss_h_dbVar,
                              (float) calib.getTwoWayWaveguideLossDbH());
  iret |= _writeNumSystemsVar(_two_way_waveguide_loss_v_dbVar,
                              (float) calib.getTwoWayWaveguideLossDbV());
  iret |= _writeNumSystemsVar(_two_way_radome_loss_h_dbVar,
                              (float) calib.getTwoWayRadomeLossDbH());
  iret |= _writeNumSystemsVar(_two_way_radome_loss_v_dbVar,
                              (float) calib.getTwoWayRadomeLossDbV());
  iret |= _writeNumSystemsVar(_receiver_mismatch_loss_dbVar,
                              (float) calib.getReceiverMismatchLossDb());
  iret |= _writeNumSystemsVar(_radar_constant_hVar,
                              (float) calib.getRadarConstantH());
  iret |= _writeNumSystemsVar(_radar_constant_vVar,
                              (float) calib.getRadarConstantV());
  iret |= _writeNumSystemsVar(_noise_hc_dbmVar,
                              (float) calib.getNoiseDbmHc());
  iret |= _writeNumSystemsVar(_noise_vc_dbmVar,
                              (float) calib.getNoiseDbmVc());
  iret |= _writeNumSystemsVar(_noise_hx_dbmVar,
                              (float) calib.getNoiseDbmHx());
  iret |= _writeNumSystemsVar(_noise_vx_dbmVar,
                              (float) calib.getNoiseDbmVx());
  iret |= _writeNumSystemsVar(_receiver_gain_hc_dbVar,
                              (float) calib.getReceiverGainDbHc());
  iret |= _writeNumSystemsVar(_receiver_gain_vc_dbVar,
                              (float) calib.getReceiverGainDbVc());
  iret |= _writeNumSystemsVar(_receiver_gain_hx_dbVar,
                              (float) calib.getReceiverGainDbHx());
  iret |= _writeNumSystemsVar(_receiver_gain_vx_dbVar,
                              (float) calib.getReceiverGainDbVx());
  iret |= _writeNumSystemsVar(_base_1km_hc_dbzVar,
                              (float) calib.getBaseDbz1kmHc());
  iret |= _writeNumSystemsVar(_base_1km_vc_dbzVar,
                              (float) calib.getBaseDbz1kmVc());
  iret |= _writeNumSystemsVar(_base_1km_hx_dbzVar,
                              (float) calib.getBaseDbz1kmHx());
  iret |= _writeNumSystemsVar(_base_1km_vx_dbzVar,
                              (float) calib.getBaseDbz1kmVx());
  iret |= _writeNumSystemsVar(_sun_power_hc_dbmVar,
                              (float) calib.getSunPowerDbmHc());
  iret |= _writeNumSystemsVar(_sun_power_vc_dbmVar,
                              (float) calib.getSunPowerDbmVc());
  iret |= _writeNumSystemsVar(_sun_power_hx_dbmVar,
                              (float) calib.getSunPowerDbmHx());
  iret |= _writeNumSystemsVar(_sun_power_vx_dbmVar,
                              (float) calib.getSunPowerDbmVx());
  iret |= _writeNumSystemsVar(_noise_source_power_h_dbmVar,
                              (float) calib.getNoiseSourcePowerDbmH());
  iret |= _writeNumSystemsVar(_noise_source_power_v_dbmVar,
                              (float) calib.getNoiseSourcePowerDbmV());
  iret |= _writeNumSystemsVar(_power_measure_loss_h_dbVar,
                              (float) calib.getPowerMeasLossDbH());
  iret |= _writeNumSystemsVar(_power_measure_loss_v_dbVar,
                              (float) calib.getPowerMeasLossDbV());
  iret |= _writeNumSystemsVar(_coupler_forward_loss_h_dbVar,
                              (float) calib.getCouplerForwardLossDbH());
  iret |= _writeNumSystemsVar(_coupler_forward_loss_v_dbVar,
                              (float) calib.getCouplerForwardLossDbV());
  iret |= _writeNumSystemsVar(_zdr_correction_dbVar,
                              (float) calib.getZdrCorrectionDb());
  iret |= _writeNumSystemsVar(_ldr_correction_h_dbVar,
                              (float) calib.getLdrCorrectionDbH());
  iret |= _writeNumSystemsVar(_ldr_correction_v_dbVar,
                              (float) calib.getLdrCorrectionDbV());
  iret |= _writeNumSystemsVar(_system_phidp_degVar,
                              (float) calib.getSystemPhidpDeg());

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write time offset variable
//
// This is time since the start of the sweep.

int ForayNcRadxFile::_writeTimeOffsetVariable()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeTimeOffsetVariable()" << endl;
  }

  size_t nTimes = _writeVol->getNRays();
  double *timeOffset = new double[nTimes];
  int startTimeSec = _writeVol->getStartTimeSecs();
  
  for (size_t ii = 0; ii < nTimes; ii++) {

    const RadxRay &ray = *_writeVol->getRays()[ii];
    int idiff = ray.getTimeSecs() - startTimeSec;
    timeOffset[ii] = (double) idiff + ray.getNanoSecs() / 1.0e9;

  } // ii

  int iret = _file.writeVar(_time_offsetVar, _TimeDim, nTimes, timeOffset);

  delete[] timeOffset;

  if (iret) {
    _addErrStr(_file.getErrStr());
    return -1;
  } else {
    return 0;
  }

}
  
////////////////////////////////////////////////
// write angle variables

int ForayNcRadxFile::_writeAngleVariables()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeAngleVariables()" << endl;
  }

  size_t nTimes = _writeVol->getNRays();
  float *el = new float[nTimes];
  float *az = new float[nTimes];
  
  for (size_t ii = 0; ii < nTimes; ii++) {

    const RadxRay &ray = *_writeVol->getRays()[ii];
    el[ii] = (float) ray.getElevationDeg();
    az[ii] = (float) ray.getAzimuthDeg();
    
  } // ii

  int iret = 0;
  iret |= _file.writeVar(_AzimuthVar, _TimeDim, nTimes, az);
  iret |= _file.writeVar(_ElevationVar, _TimeDim, nTimes, el);

  delete[] el;
  delete[] az;

  if (iret) {
    _addErrStr(_file.getErrStr());
    return -1;
  } else {
    return 0;
  }

}
  
////////////////////////////////////////////////
// write clip range variable
//
// this is the max usable range

int ForayNcRadxFile::_writeClipRangeVariable()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeClipRangeVariable()" << endl;
  }
  
  size_t nTimes = _writeVol->getNRays();
  float *clipRange = new float[nTimes];
  
  for (size_t ii = 0; ii < nTimes; ii++) {

    const RadxRay &ray = *_writeVol->getRays()[ii];
    int nGates = ray.getNGates();
    double maxRangeKm =
      ray.getStartRangeKm() + nGates * ray.getGateSpacingKm();
    clipRange[ii] = maxRangeKm * 1000.0; // meters

  } // ii

  int iret = _file.writeVar(_clip_rangeVar, _TimeDim, nTimes, clipRange);

  delete[] clipRange;

  if (iret) {
    _addErrStr(_file.getErrStr());
    return -1;
  } else {
    return 0;
  }

}
  
////////////////////////////////////////////////
// write field data variables

int ForayNcRadxFile::_writeDataFieldVariables()
{

  if (_verbose) {
    cerr << "ForayNcRadxFile::_writeDataFieldVariables()" << endl;
  }

  int iret = 0;
  for (size_t ifield = 0; ifield < _writeVol->getNFields(); ifield++) {
    RadxField &field = *_writeVol->getFields()[ifield];
    Nc3Var *var = _dataFieldVars[ifield];
    iret |= !var->put((short *) field.getData(),
                      _writeVol->getNRays(), _writeVol->getMaxNGates());
  } // ifield
    
  if (iret) {
    _addErrStr(_file.getErrStr());
    return -1;
  } else {
    return 0;
  }

}
  
////////////////////////////////////////////////
// write numSystems variable

int ForayNcRadxFile::_writeNumSystemsVar(Nc3Var *var, float val)
{
  if (_file.writeVar(var, _numSystemsDim, &val)) {
    _addErrStr(_file.getErrStr());
    return -1;
  } else {
    return 0;
  }

}

int ForayNcRadxFile::_writeCalibDataPresent(int val)
{
  int vals[1];
  vals[0] = val;
  if (_file.writeVar(_calibration_data_presentVar,
                     _numSystemsDim, vals)) {
    return -1;
  } else {
    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////
// Set output compression for variable

int ForayNcRadxFile::_setCompression(Nc3Var *var)  
{
  
  if (_ncFormat == NETCDF_CLASSIC || _ncFormat == NETCDF_OFFSET_64BIT) {
    // cannot compress
    return 0;
  }

  if (var == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_setCompression");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  if (!_writeCompressed) {
    return 0;
  }

  int fileId = _file.getNc3File()->id();
  int varId = var->id();
  int shuffle = 0;
  
  if (nc_def_var_deflate(fileId, varId, shuffle,
                         _writeCompressed, _compressionLevel)!= NC_NOERR) {
    cerr << "WARNING NcfRadxFile::_setCompression" << endl;
    cerr << "  Cannot compress field: " << var->name() << endl;
    cerr << "  Will be written uncompressed instead" << endl;
  }

  return 0;

}

