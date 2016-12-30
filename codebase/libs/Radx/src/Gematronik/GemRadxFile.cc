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
// GemRadxFile.cc
//
// Reading Gematronik file data for radar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2014
//
///////////////////////////////////////////////////////////////

#include <Radx/GemRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxReadDir.hh>
#include <Radx/RadxArray.hh>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <algorithm>
#include "GemInputField.hh"
using namespace std;

int GemRadxFile::_volumeNumber = 0;

//////////////
// Constructor

GemRadxFile::GemRadxFile() : RadxFile()
  
{
  
  _file = NULL;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

GemRadxFile::~GemRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void GemRadxFile::clear()
  
{

  clearErrStr();
  _clearRays();
  _clearFields();
  
}

void GemRadxFile::_clearFields()

{
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();

}


void GemRadxFile::_clearRays()
{
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int GemRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  // Writing GamicHdf5 files is not supported
  // therefore write in CF Radial format instead

  if (_debug) {
    cerr << "WARNING - GemRadxFile::writeToDir" << endl;
    cerr << "  Writing Gematronik format files not supported" << endl;
    cerr << "  Will write CfRadial file instead" << endl;
  }
  
  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  // set return values

  _errStr = ncfFile.getErrStr();
  _dirInUse = ncfFile.getDirInUse();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int GemRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  // Writing Gem files is not supported
  // therefore write in CF Radial format instead

  if (_debug) {
    cerr << "WARNING - GemRadxFile::writeToPath" << endl;
    cerr << "  Writing Gematronik format files not supported" << endl;
    cerr << "  Will write CfRadial file instead" << endl;
  }

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// Check if specified file is Gem format
// Returns true if supported, false otherwise

bool GemRadxFile::isSupported(const string &path)

{
  
  if (isGematronik(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a Gem file
// Returns true on success, false on failure

bool GemRadxFile::isGematronik(const string &path)
  
{

  _close();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - GemRadxFile::isGem");
    return false;
  }
  
  // read ID of first block

  char id[32];
  if (fread(id, 1, 32, _file) != 32) {
    _close();
    return false;
  }
  _close();

  if (strncmp(id, "<volume", 7) == 0) {
    return true;
  } 
  if (strncmp(id, "<volfile", 8) == 0) {
    return true;
  } 
  
  return false;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int GemRadxFile::readFromPath(const string &path,
                              RadxVol &vol)
  
{
  
  // initialize for reading

  _initForRead(path, vol);
  _volumeNumber++;

  // read in fields
  
  if (_readFields(path)) {
    _addErrStr("ERROR - GemRadxFile::readFromPath");
    return -1;
  }

  // compute number of sweeps

  if (_computeNSweeps()) {
    _addErrStr("ERROR - GemRadxFile::readFromPath");
    _addErrStr("  Number of sweeps varies from file to file");
    return -1;
  }

  // load the ray data for the read volume

  if (_loadRays(path)) {
    _addErrStr("ERROR - GemRadxFile::readFromPath");
    _addErrStr("  reading fields for path: ", path);
    _clearFields();
    return -1;
  }

  // load the metadata for the read volume

  if (_loadMetaData(path)) {
    _addErrStr("ERROR - GemRadxFile::readFromPath");
    _addErrStr("  reading fields for path: ", path);
    _clearFields();
    return -1;
  }

  // set format as read

  _fileFormat = FILE_FORMAT_GEM_XML;

  // clean up

  _clearFields();

  return 0;

}

//////////////////////////////////////
// open file for reading
// Returns 0 on success, -1 on failure

int GemRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - GemRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close file if open

void GemRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////////////
// get list of field paths for the volume for the specified path

void GemRadxFile::_getFieldPaths(const string &primaryPath,
                                 vector<string> &fileNames,
                                 vector<string> &filePaths,
                                 vector<string> &fieldNames)
  
{
  
  // init

  fileNames.clear();
  filePaths.clear();
  fieldNames.clear();

  // decompose the path to get the date/time prefix for the primary path
  
  RadxPath rpath(primaryPath);
  const string &dir = rpath.getDirectory();
  const string &fileName = rpath.getFile();
  const string &ext = rpath.getExt();
  string prefix(fileName.substr(0, 16));
  
  // load up array of file names that match the prefix
  
  RadxReadDir rdir;
  if (rdir.open(dir.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      string dName(dp->d_name);
      
      // exclude dir entries beginning with '.'
      
      if (dName[0] == '.') {
	continue;
      }

      // make sure we have .vol files
      
      if (dName.find(ext) == string::npos) {
	continue;
      }

      string dStr(dName.substr(0, 16));
      
      if (dStr == prefix) {
        // get field name from file name
        size_t pos = dName.find('.', 16);
        if (pos != string::npos) {
          fileNames.push_back(dName);
        } // if (pos ...
      } // if (dStr ...
      
    } // dp
    
    rdir.close();

  } // if (rdir ...

  // sort the file names

  sort(fileNames.begin(), fileNames.end());

  // load up the paths and field names

  for (size_t ii = 0; ii < fileNames.size(); ii++) {

    const string &fileName = fileNames[ii];

    size_t pos = fileName.find('.', 16);
    string fieldName = fileName.substr(16, pos - 16);
    fieldNames.push_back(fieldName);
    
    string dPath(dir);
    dPath += RadxPath::RADX_PATH_DELIM;
    dPath += fileName;
    filePaths.push_back(dPath);

  } // ii
  
}

/////////////////////////////////////////////////////
/// set the file time from the path
/// returns 0 on success, -1 on failure

int GemRadxFile::setTimeFromPath(const string &filePath,
                                 time_t &fileTime)
  
{

  // get the file name part of the path

  RadxPath rpath(filePath);
  string fileName = rpath.getFile();
  
  // find first digit in entry name - if no digits, return now
  
  const char *start = NULL;
  for (size_t ii = 0; ii < fileName.size(); ii++) {
    if (isdigit(fileName[ii])) {
      start = fileName.c_str() + ii;
      break;
    }
  }
  if (!start) return -1;
  const char *end = start + strlen(start);
  
  // iteratively try getting the date and time from the string
  // moving along by one character at a time
  
  while (start < end - 6) {
    int year, month, day, hour, min, sec;
    if (sscanf(start, "%4d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      RadxTime ftime(year, month, day, hour, min, sec);
      fileTime = ftime.utime();
      return 0;
    }
    start++;
  }
  
  return -1;
  
}

////////////////////////////////////////////////////////////
// Read in all fields for specified path.
// Returns 0 on success, -1 on failure

int GemRadxFile::_readFields(const string &path)
  
{
  
  // is this a Gem file?

  if (!isGematronik(path)) {
    _addErrStr("ERROR - GemRadxFile::_readFields");
    _addErrStr("  Not a gem file: ", path);
    return -1;
  }

  // get the list of all files, one field in each, that match this time

  vector<string> fileNames;
  vector<string> filePaths;
  vector<string> fieldNames;
  _getFieldPaths(path, fileNames, filePaths, fieldNames);

  if (filePaths.size() < 1) {
    _addErrStr("ERROR - GemRadxFile::_readFields");
    _addErrStr("  No field files found, path: ", path);
    return -1;
  }

  // set file time

  time_t fileTime;
  if (setTimeFromPath(filePaths[0], fileTime)) {
    _addErrStr("ERROR - GemRadxFile::_readFields");
    _addErrStr("  Cannot get time from file: ", fileNames[0]);
    return -1;
  }
  _fileTime.set(fileTime);
      
  // load vector of input fields

  _clearFields();
  for (size_t ii = 0; ii < fileNames.size(); ii++) {

    const string &fieldName = fieldNames[ii];

    // determine units

    string units;
    string standardName;
    string longName;
    if (fieldName.find("dBuZv") != string::npos) {
      units = "dBZ";
      standardName = "equivalent_reflectivity_factor";
      longName = "unfiltered_reflectivity_from_vertical_polarization";
    } else if (fieldName.find("dBuZ") != string::npos) {
      units = "dBZ";
      standardName = "equivalent_reflectivity_factor";
      longName = "unfiltered_reflectivity_from_horizontal_polarization";
    } else if (fieldName.find("dBZv") != string::npos) {
      units = "dBZ";
      standardName = "equivalent_reflectivity_factor";
      longName = "reflectivity_from_vertical_polarization";
    } else if (fieldName.find("dBZ") != string::npos) {
      units = "dBZ";
      standardName = "equivalent_reflectivity_factor";
      longName = "reflectivity_from_horizontal_polarization";
    } else if (fieldName.find("DOPu") != string::npos) {
      units = "";
      standardName = "degree_of_polarization";
      longName = "unfiltered_degree_of_polarization";
    } else if (fieldName.find("DOP") != string::npos) {
      units = "";
      standardName = "degree_of_polarization";
      longName = "degree_of_polarization";
    } else if (fieldName.find("uKDPu") != string::npos) {
      units = "degrees/km";
      standardName = "specific_differential_phase_hv";
      longName = "unfiltered_specific_differential_phase_shift_derived_by_signal_processor(GDRX)";
    } else if (fieldName.find("uKDP") != string::npos) {
      units = "degrees/km";
      standardName = "specific_differential_phase_hv";
      longName = "specific_differential_phase_shift_derived_by_signal_processor(GDRX)";
    } else if (fieldName.find("KDP") != string::npos) {
      units = "degrees/km";
      standardName = "specific_differential_phase_hv";
      longName = "specific_differential_phase_shift_derived_by_Rainbow_from_PhiDP";
    } else if (fieldName.find("uPhiDPu") != string::npos) {
      units = "degrees";
      standardName = "differential_phase_hv";
      longName = "unfiltered_differential_phase_shift_derived_by_signal_processor_(GDRX)";
    } else if (fieldName.find("uPhiDP") != string::npos) {
      units = "degrees";
      standardName = "differential_phase_hv";
      longName = "differential_phase_shift_derived_by_signal_processor_(GDRX)";
    } else if (fieldName.find("PhiDP") != string::npos) {
      units = "degrees";
      standardName = "differential_phase_hv";
      longName = "differential_phase_shift_filtered_by_Rainbow";
    } else if (fieldName.find("RhoHVu") != string::npos) {
      units = "";
      standardName = "cross_correlation_hv";
      longName = "unfiltered_co-polar_cross_correlation_coefficient";
    } else if (fieldName.find("Rho") != string::npos) {
      units = "";
      standardName = "cross_correlation_hv";
      longName = "co-polar_cross_correlation_coefficient";
    } else if (fieldName.find("SQIvu") != string::npos) {
      units = "";
      standardName = "normalized_coherent_power";
      longName = "unfiltered_signal_quality_index_from_vertical_polarization";
    } else if (fieldName.find("SQIv") != string::npos) {
      units = "";
      standardName = "normalized_coherent_power";
      longName = "signal_quality_index_from_vertical_polarization";
    } else if (fieldName.find("SQIu") != string::npos) {
      units = "";
      standardName = "normalized_coherent_power";
      longName = "unfiltered_signal_quality_index_from_horizontal_polarization";
    } else if (fieldName.find("SQI") != string::npos) {
      units = "";
      standardName = "normalized_coherent_power";
      longName = "signal_quality_index_from_horizontal_polarization";
    } else if (fieldName.find("ZDRu") != string::npos) {
      units = "db";
      standardName = "log_differential_reflecivity_hv";
      longName = "unfiltered_differential_reflectivity";
    } else if (fieldName.find("ZDR") != string::npos) {
      units = "db";
      standardName = "log_differential_reflecivity_hv";
      longName = "differential_reflectivity";
    } else if (fieldName.find("LDR") != string::npos) {
      units = "db";
      standardName = "log_linear_depolarization_ratio";
      longName = "linear_depolarization_ratio";
    } else if (fieldName.find("Vvu") != string::npos) {
      units = "m/s";
      standardName = "radial_velocity_of_scatterers_away_from_instrument";
      longName = "unfiltered_radial_velocity_from_vertical_polarization";
    } else if (fieldName.find("Vu") != string::npos) {
      units = "m/s";
      standardName = "radial_velocity_of_scatterers_away_from_instrument";
      longName = "unfiltered_radial_velocity_from_horizontal_polarization";
    } else if (fieldName.find("Vv") != string::npos) {
      units = "m/s";
      standardName = "radial_velocity_of_scatterers_away_from_instrument";
      longName = "radial_velocity_from_vertical_polarization";
    } else if (fieldName.find("V") != string::npos) {
      units = "m/s";
      standardName = "radial_velocity_of_scatterers_away_from_instrument";
      longName = "radial_velocity_from_horizontal_polarization";
    } else if (fieldName.find("Wvu") != string::npos) {
      units = "m/s";
      standardName = "doppler_spectrum_width";
      longName = "unfiltered_spectral_width_from_vertical_polarization";
    } else if (fieldName.find("Wv") != string::npos) {
      units = "m/s";
      standardName = "doppler_spectrum_width";
      longName = "spectral_width_from_vertical_polarization";
    } else if (fieldName.find("Wu") != string::npos) {
      units = "m/s";
      standardName = "doppler_spectrum_width";
      longName = "unfiltered_spectral_width_from_horizontal_polarization";
    } else if (fieldName.find("W") != string::npos) {
      units = "m/s";
      standardName = "doppler_spectrum_width";
      longName = "spectral_width_from_horizontal_polarization";
    } else if (fieldName.find("SNRvu") != string::npos) {
      units = "dB";
      standardName = "signal_to_noise_ratio";
      longName = "unfiltered_signal_to_noise_ratio_from_vertical_polarization";
    } else if (fieldName.find("SNRu") != string::npos) {
      units = "dB";
      standardName = "signal_to_noise_ratio";
      longName = "unfiltered_signal_to_noise_ratio_from_horizontal_polarization";
    } else if (fieldName.find("SNRv") != string::npos) {
      units = "dB";
      standardName = "signal_to_noise_ratio";
      longName = "signal_to_noise_ratio_from_vertical_polarization";
    } else if (fieldName.find("SNR") != string::npos) {
      units = "dB";
      standardName = "signal_to_noise_ratio";
      longName = "signal_to_noise_ratio_from_horizontal_polarization";
    } else if (fieldName.find("CCORv") != string::npos) {
      units = "dB";
      standardName = "clutter_correction_ratio";
      longName = "clutter_correction_ratio_from_vertical_polarization";
    } else if (fieldName.find("CCOR") != string::npos) {
      units = "dB";
      standardName = "clutter_correction_ratio";
      longName = "clutter_correction_ratio_from_horizontal_polarization";
    }

    // create input field object
    
    GemInputField *field = new GemInputField(fileNames[ii],
                                             filePaths[ii],
                                             _fileTime.utime(),
                                             fieldName,
                                             standardName,
                                             longName,
                                             units,
                                             _debug,
                                             _verbose);

    // read in the data
    
    if (field->read()) {
      _addErrStr("ERROR - GemRadxFile::_readFields");
      _addErrStr("  Cannot read in field, path: ", filePaths[ii]);
      delete field;
    } else {
      _fields.push_back(field);
    }

    // add to paths used on read
    
    _readPaths.push_back(filePaths[ii]);

  } // ii

  if (_fields.size() == 0) {
    _addErrStr("ERROR - GemRadxFile::_readFields");
    _addErrStr("  No fields read in");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load up the ray data

int GemRadxFile::_loadRays(const string &path)
  
{

  time_t volEndTime = _fields[0]->getVolTime();

  // loop through sweeps

  int prevSweepDuration = 0;
  
  for (int isweep = 0; isweep < _nSweeps; isweep++) {
    
    const GemSweep &sweep = *(_fields[0]->getSweeps()[isweep]);
    double antennaSpeed = sweep.getAntennaSpeed();

    // times
    
    time_t sweepStartTime = sweep.getStartTime();
    time_t sweepEndTime = volEndTime;
    if (isweep == _nSweeps - 1) {
      if (prevSweepDuration > 0) {
        sweepEndTime = sweepStartTime + prevSweepDuration;
      }
    } else {
      const GemSweep &nextSweep = *(_fields[0]->getSweeps()[isweep+1]);
      sweepEndTime = nextSweep.getStartTime();
    }

    // compute sweep parameters
    
    if (_setSweepGeom(isweep) == 0) {
      if (_loadSweep(isweep, sweepStartTime, sweepEndTime, antennaSpeed)) {
        _addErrStr("ERROR - GemRadxFile::_loadRays");
        _addErrInt("  Cannot load rays for sweep: ", isweep);
        return -1;
      }
    }

    prevSweepDuration = sweepEndTime - sweepStartTime;
    
  } // isweep
  
  return 0;

}

/////////////////////////////////////////////////////////
// load up the ray data

int GemRadxFile::_loadSweep(int sweepNum,
                            time_t startTime,
                            time_t endTime,
                            double antennaSpeed)
  
{
  
  if (_debug) {
    cerr << "===>>   sweep num: " << sweepNum << endl;
    cerr << "          antennaSpeed: " << antennaSpeed << endl;
    cerr << "          start time: " << RadxTime::strm(startTime) << endl;
  }

  const GemSweep &sweepField0 = *(_fields[0]->getSweeps()[sweepNum]);
  const vector<double> &angles = sweepField0.getAngles();
  double prevAngle = angles[0];

  double deltaTime = ((double) endTime - (double) startTime) / (_nAngles + 1.0);

  // loop through the beams

  time_t lastTime = 0;
  double dSecs = 0.0;

  for (int iangle = 0; iangle < _nAngles; iangle++) {
    
    // create new ray
    
    RadxRay *ray = new RadxRay;
    
    ray->setVolumeNumber(_volumeNumber);
    ray->setSweepNumber(sweepNum);
    ray->setCalibIndex(0);

    // compute change in azimuth

    double angle = angles[iangle];
    double absDeltaAngle = fabs(angle - prevAngle);
    if (absDeltaAngle > 180) {
      absDeltaAngle = fabs(absDeltaAngle - 360);
    }
    prevAngle = angle;

    // set angles

    if (_fields[0]->getIsRhi()) {
      if (angle > 180.0) {
        ray->setElevationDeg(angle - 360.0);
      } else {
        ray->setElevationDeg(angle);
      }
      ray->setAzimuthDeg(sweepField0.getFixedAngle());
    } else {
      if (angle < 0) {
        ray->setAzimuthDeg(angle + 360.0);
      } else {
        ray->setAzimuthDeg(angle);
      }
      ray->setElevationDeg(sweepField0.getFixedAngle());
    }
    ray->setFixedAngleDeg(sweepField0.getFixedAngle());
    
    ray->setTrueScanRateDegPerSec(antennaSpeed);
    ray->setTargetScanRateDegPerSec(antennaSpeed);
    ray->setAntennaTransition(0);
    ray->setNSamples(sweepField0.getNSamples());

    ray->setIsIndexed(sweepField0.getIsIndexed());
    ray->setAngleResDeg(sweepField0.getAngleRes());
    
    // time
    // compute change in time from antenna rate if available
    // otherwiste use fixed delta time
    
    if (antennaSpeed > 0) {
      dSecs += (absDeltaAngle / antennaSpeed);
    } else {
      dSecs = iangle * deltaTime;
    }

    int iSecs = (int) dSecs;
    // round to nearest millisec, since we are estimating anyway
    double fracSecs = ((int) ((dSecs - iSecs) * 1000.0 + 0.5)) / 1000.0;
    int nanoSecs = fracSecs * 1.0e9;
    time_t raySecs = startTime + iSecs;
    ray->setTime(raySecs, nanoSecs);
    lastTime = raySecs;

    // metadata
    
    ray->setRangeGeom(sweepField0.getStartRange(), sweepField0.getGateSpacing());
    ray->setNGates(_nGates);

    if (_fields[0]->getIsRhi()) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else if (_fields[0]->getIsSector()) {
      ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
    } else {
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    }

    // polarization mode
    if (sweepField0.getIsDualPol()) {
      ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    } else {
      ray->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
    }
    
    // prf mode - assume fixed
    if (sweepField0.getIsStaggered()) {
      ray->setPrtSec(1.0 / sweepField0.getHighPrf());
      ray->setPrtMode(Radx::PRT_MODE_DUAL);
      ray->setPrtRatio(sweepField0.getHighPrf() / sweepField0.getLowPrf());
    } else {
      ray->setPrtSec(1.0 / sweepField0.getHighPrf());
      ray->setPrtMode(Radx::PRT_MODE_FIXED);
    }

    // follow mode
    ray->setFollowMode(Radx::FOLLOW_MODE_NONE);
    
    // position
    
    ray->setPulseWidthUsec(sweepField0.getPulseWidthUs());
    ray->setNyquistMps(sweepField0.getNyquist());

    // ray->setUnambigRangeKm(_unambigRange);

    // compose data

    // some fields are 2 bytes, map all fields into 2 bytes

    // int nBytes = _nFields * _nGates * _outputByteWidth;
    // ui16 *beamData = new ui16[nBytes];

    for (int ifld = 0; ifld < (int) _fields.size(); ifld++) {

      const GemInputField &field = *_fields[ifld];
      const GemSweep &sweep = *(field.getSweeps()[sweepNum]);

      double samplingRatio =
        (double) sweep.getNSamples() / (double) sweepField0.getNSamples();
      
      // create field

      RadxField *rfield = new RadxField(field.getFieldName(), field.getUnits());
      rfield->setStandardName(field.getStandardName());
      rfield->setLongName(field.getLongName());
      rfield->setRangeGeom(sweep.getStartRange(), sweep.getGateSpacing());
      rfield->setSamplingRatio(samplingRatio);

      int byteWidth = sweep.getDataByteWidth();
      double minVal = sweep.getMinValue();
      double maxVal = sweep.getMaxValue();
      double dataRange = maxVal - minVal;
      double scale = dataRange / 254.0;
      if (byteWidth > 1) {
        scale = dataRange / 65534.0;
      }
      double offset = (minVal + maxVal) / 2.0;

      if (byteWidth == 1) {

        // get input data - unsigned
        
        const Radx::ui08 *uData = sweep.getFieldData();
        const Radx::ui08 *ud = uData + (iangle * _nGates);

        // convert to signed

        RadxArray<Radx::si08> sd_;
        Radx::si08 *sd = sd_.alloc(_nGates);
        for (int igate = 0; igate < _nGates; igate++) {
          sd[igate] = ud[igate] - 128;
        }
        rfield->setTypeSi08(-128, scale, offset);
        rfield->addDataSi08(_nGates, sd);

      } else {

        // byte width 2
        // get input data - unsigned
        
        const Radx::ui16 *uData = (Radx::ui16 *) sweep.getFieldData();
        const Radx::ui16 *ud = uData + (iangle * _nGates);

        // convert to signed

        RadxArray<Radx::si16> sd_;
        Radx::si16 *sd = sd_.alloc(_nGates);
        for (int igate = 0; igate < _nGates; igate++) {
          sd[igate] = ud[igate] - 32768;
        }
        rfield->setTypeSi16(-32768, scale, offset);
        rfield->addDataSi16(_nGates, sd);

      } // inputByteWidth

      // add field to ray

      ray->addField(rfield);
      
    } // ifld

    // check for all data in all fields missing, if requested
    
    if (_readRemoveRaysAllMissing) {
      if (ray->checkDataAllMissing()) {
        delete ray;
        return 0;
      }
    }
    
    if (_readSetMaxRange) {
      ray->setMaxRangeKm(_readMaxRangeKm);
    }
    
    // add ray to vector
    
    _rays.push_back(ray);
    
  } // iangle

  if (_debug) {
    cerr << "          end   time: " << RadxTime::strm(lastTime) << endl;
  }
    
  return 0;

}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int GemRadxFile::_loadMetaData(const string &path)
  
{

  _readVol->setOrigFormat("GEMATRONIK");

  // volume number from time of day

  _readVol->setVolumeNumber(_volumeNumber);
  
  // set meta-data

  const GemInputField &field0 = *_fields[0];
  const vector<GemSweep *> &sweeps0 = field0.getSweeps();
  const GemSweep &sweepField0 = *sweeps0[0];

  // info strings
  
  _readVol->setSiteName(field0.getRadarName());
  _readVol->setInstrumentName(field0.getRadarName());
  _readVol->setSource(field0.getRadarType());
  _readVol->setTitle("GEMATRONIK RADAR DATA");
  _readVol->setComment("");
  _readVol->setHistory("Data converted by Radx::GemRadxFile");
  _readVol->setStatusXml(field0.getXmlStr());

  // instrument properties

  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z);

  // location

  _readVol->setLatitudeDeg(field0.getRadarLat());
  _readVol->setLongitudeDeg(field0.getRadarLon());
  _readVol->setAltitudeKm(field0.getRadarAlt() / 1000.0);

  // properties

  _readVol->setWavelengthM(field0.getRadarWavelength());
  _readVol->setRadarBeamWidthDegH(field0.getRadarBeamwidth());
  _readVol->setRadarBeamWidthDegV(field0.getRadarBeamwidth());

  // calibration

  RadxRcalib *calib = new RadxRcalib;
  calib->setCalibTime(_fileTime.utime());
  calib->setPulseWidthUsec(sweepField0.getPulseWidthUs());
  calib->setXmitPowerDbmH(10.0 * log10(sweepField0.getXmitPeakPowerKw() * 1.0e6));
  calib->setXmitPowerDbmV(10.0 * log10(sweepField0.getXmitPeakPowerKw() * 1.0e6));
  calib->setRadarConstantH(sweepField0.getRadarConstH());
  calib->setRadarConstantV(sweepField0.getRadarConstV());
  calib->setBaseDbz1kmHc(sweepField0.getNoisePowerDbzH());
  calib->setBaseDbz1kmVc(sweepField0.getNoisePowerDbzV());
  _readVol->addCalib(calib);
  
  _readVol->setTargetScanRateDegPerSec(sweepField0.getAntennaSpeed());
  
  // add rays to volume
  // responsibility for management of ray memory passes from 
  // this object to the volume

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  // memory allocation for rays has passed to _readVol,
  // so free up pointer array
  
  _rays.clear();

  // set max range

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // remove rays with all missing data, if requested
  
  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // load the sweep info from rays
  
  _readVol->loadSweepInfoFromRays();

  // Intermediate frequency

  for (int isweep = 0; isweep < _nSweeps; isweep++) {
    const vector<RadxSweep *> &sweeps = _readVol->getSweeps();
    if (isweep > (int) sweeps.size() - 1) {
      break;
    }
    RadxSweep *sweep = sweeps[isweep];
    const GemSweep &gsweep = *(_fields[0]->getSweeps()[isweep]);
    double intermedFreqHz = gsweep.getIfMhz();
    if (intermedFreqHz != Radx::missingMetaDouble) {
      sweep->setIntermedFreqHz(intermedFreqHz);
    }
  }

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - GemRadxFile::_loadMetaData");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - GemRadxFile::_loadMetaData");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  if (_readFixedAngleLimitsSet) {
    _readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle);
  } else if (_readSweepNumLimitsSet) {
    _readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum);
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();
  
  // load up volume info from rays

  _readVol->loadVolumeInfoFromRays();

  return 0;
  
}

/////////////////////////////////////////////////////////
// print object

void GemRadxFile::print(ostream &out) const
  
{
  
  out << "=========== GemRadxFile summary ===========" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print data in nantive gem format
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int GemRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  clear();

  // read in fields
  
  if (_readFields(path)) {
    _addErrStr("ERROR - GemRadxFile::printNative");
    return -1;
  }
  
  out << "============== Gematronik XML =================" << endl;

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    const GemInputField &field = *_fields[ii];
    out << "====>> Field: " << field.getFieldName() << endl;
    out << "====>> Units: " << field.getUnits() << endl;
    out << "====>> Path: " << field.getFilePath() << endl;
    out << "===XML===XML===XML===XML===XML===XML===XML===XML===XML" << endl;
    out << field.getXmlStr() << endl;
    out << "===XML===XML===XML===XML===XML===XML===XML===XML===XML" << endl;
  }
  
  out << "===============================================" << endl;
  
  // free up

  _clearFields();
  
  return 0;

}

/////////////////////////////////////////////////////////
// print field data

void GemRadxFile::_printFieldData(ostream &out, int nGates, const double *data) const
  
{

  out << "============== Field data =================" << endl;
  if (nGates == 0) {
    out << "========= currently no data =========" << endl;
  } else {
    int printed = 0;
    int count = 1;
    double prevVal = data[0];
    for (int ii = 1; ii < nGates; ii++) {
      double dval = data[ii];
      if (dval != prevVal) {
        _printPacked(out, count, prevVal);
        printed++;
        if (printed > 6) {
          out << endl;
          printed = 0;
        }
        prevVal = dval;
        count = 1;
      } else {
        count++;
      }
    } // ii
    _printPacked(out, count, prevVal);
    out << endl;
  }
  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void GemRadxFile::_printPacked(ostream &out, int count, double val) const

{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == Radx::missingFl64) {
    out << "MISS ";
  } else {
    if (fabs(val) > 0.01) {
      sprintf(outstr, "%.3f ", val);
      out << outstr;
    } else if (val == 0.0) {
      out << "0.0 ";
    } else {
      sprintf(outstr, "%.3e ", val);
      out << outstr;
    }
  }
}

//////////////////////////////////////////////////
// compute number of sweeps
// returns 0 on success, -1 on failure

int GemRadxFile::_computeNSweeps()
  
{
  _nSweeps = _fields[0]->getNSweeps();
  for (int ii = 1; ii < (int) _fields.size(); ii++) {
    if (_fields[ii]->getNSweeps() != _nSweeps) {
      return -1;
    }
  }
  return 0;
}

//////////////////////////////////////////////////
// set number of angles in sweep
// returns 0 on success, -1 on failure

int GemRadxFile::_setSweepGeom(int sweepNum)
  
{

  const GemSweep &field0 = *(_fields[0]->getSweeps()[sweepNum]);
  _nAngles = field0.getNAngles();
  _nGates = field0.getNGates();
  
  for (int ii = 1; ii < (int) _fields.size(); ii++) {
    const GemSweep &field = *(_fields[ii]->getSweeps()[sweepNum]);
    if (_nAngles < 1) {
      _addErrStr("ERROR - GemRadxFile::_setSweepGeom");
      _addErrInt("  nAngles not positive: ", _nAngles);
      return -1;
    }
    if (_nAngles != field.getNAngles()) {
      _addErrStr("ERROR - GemRadxFile::_setSweepGeom");
      _addErrStr("  nAngles not constant across fields");
      return -1;
    }
    if (_nGates != field.getNGates()) {
      _addErrStr("ERROR - GemRadxFile::_setSweepGeom");
      _addErrStr("  nGates not constant across fields");
      return -1;
    }
  }

  return 0;

}


