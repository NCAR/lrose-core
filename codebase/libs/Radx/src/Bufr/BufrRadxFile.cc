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
// BufrRadxFile.cc
//
// BufrRadxFile object
//
// Intermediate class between BUFR file and Radx Volume structure.
// This class maps the BUFR information into a Radx Volume and Radx
// file structure.
//
// Mike Dixon and Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, 
// August 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/BufrRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxReadDir.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <float.h>
using namespace std;

//////////////
// Constructor

BufrRadxFile::BufrRadxFile() : RadxFile()
  
{

  //_debug = true;
  //_ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

BufrRadxFile::~BufrRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void BufrRadxFile::clear()
  
{

  clearErrStr();

  //_file.close();
  /*
  _timeDim = NULL;
  _rangeDim = NULL;

  _timeVar = NULL;
  _rangeVar = NULL;
  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _heightVar = NULL;

  _frequencyVar = NULL;
  _prfVar = NULL;
  _beamwidthHVar = NULL;
  _beamwidthVVar = NULL;
  _antennaDiameterVar = NULL;
  _pulsePeriodVar = NULL;
  _transmitPowerVar = NULL;
  _sweepVar = NULL;
  _azimuthVar = NULL;
  _elevationVar = NULL;

  _dTimes.clear();
  _nTimesInFile = 0;
  _rayTimesIncrease = true;
  _refTimeSecsFile = 0;

  _rangeKm.clear();
  _nRangeInFile = 0;
  _gateSpacingIsConstant = true;

  _latitudeDeg = 0;
  _longitudeDeg = 0;
  _heightKm = 0;

  _frequencyGhz = Radx::missingMetaDouble;
  _prfHz = Radx::missingMetaDouble;
  _beamwidthHDeg = Radx::missingMetaDouble;
  _beamwidthVDeg = Radx::missingMetaDouble;
  _antennaDiameterM = Radx::missingMetaDouble;
  _pulsePeriodUs = Radx::missingMetaDouble;
  _transmitPowerW = Radx::missingMetaDouble;  

  _azimuths.clear();
  _elevations.clear();
  
  _British_National_Grid_Reference_attr.clear();
  _Conventions_attr.clear();
  _operator_attr.clear();
  _radar_attr.clear();
  _references_attr.clear();
  _scan_datetime_attr.clear();
  _scantype_attr.clear();

  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();

  _statusXml.clear(); // global attributes
  _siteName.clear();
  _scanName.clear();
  _instrumentName.clear();
  */

  _scanId = 0;
  _volumeNumber = 0;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;
  
}

/*
void BufrRadxFile::_clearFields()

{
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();

}
*/
/////////////////////////////////////////////////////////
// Check if specified file is Bufr format
// Returns true if supported, false otherwise

bool BufrRadxFile::isSupported(const string &path)

{
  
  if (isBufr(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a Bufr file
// Returns true on success, false on failure

bool BufrRadxFile::isBufr(const string &path)
  
{

  clear();
  if (_debug) {
    cerr << "DEBUG - inside isBufr file" << endl;
  }
  // open file
  BufrFile aFile;
  aFile.setDebug(_debug);
  aFile.setVerbose(_verbose);

  if (aFile.openRead(path)) {
    if (_debug) {
      cerr << "DEBUG openRead failed" << endl;
    }
    if (_verbose) {
      cerr << "DEBUG - not Bufr file" << endl;
      //cerr << _file.getErrStr() << endl;
    }
  
    return false;
  }

  // read Section 0
  aFile.readSection0();
  aFile.close();
  if (_debug) {
    cerr << "DEBUG - it's all good! we have a Bufr file " << endl;
  }
  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int BufrRadxFile::writeToDir(const RadxVol &vol,
                                const string &dir,
                                bool addDaySubDir,
                                bool addYearSubDir)
  
{

  // Writing Bufr files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - BufrRadxFile::writeToDir" << endl;
  cerr << "  Writing Bufr raw format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;
  
  // set up NcfRadxFile object

  // NcfRadxFile ncfFile;
  // ncfFile.copyWriteDirectives(*this);

  // perform write

  //  int iret = ncfFile.writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  // set return values

  // _errStr = ncfFile.getErrStr();
  // _dirInUse = ncfFile.getDirInUse();
  // _pathInUse = ncfFile.getPathInUse();
  // vol.setPathInUse(_pathInUse);

  // return iret;
  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int BufrRadxFile::writeToPath(const RadxVol &vol,
                                 const string &path)
  
{

  // Writing Bufr files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - BufrRadxFile::writeToPath" << endl;
  cerr << "  Writing Bufr raw format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  // NcfRadxFile ncfFile;
  // ncfFile.copyWriteDirectives(*this);

  // perform write

  //  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  // _errStr = ncfFile.getErrStr();
  // _pathInUse = ncfFile.getPathInUse();
  // vol.setPathInUse(_pathInUse);

  // return iret;
  return 0;
}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

time_t  BufrRadxFile::getTimeFromString(const char *dateTime)
{
  struct tm tm;
  int year, month, day, hour, min, sec;
  if (strptime(dateTime, "%Y%m%dT%H%M%S", &tm) != NULL) {
    day = tm.tm_mday;
    month = tm.tm_mon + 1;
    year = tm.tm_year + 1900;
    hour = tm.tm_hour;
    min = tm.tm_min;
    sec = tm.tm_sec;

    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
      return -1;
    }
    if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
      return -1;
    }
    RadxTime theTime;
    time_t t;
    t = theTime.set(year, month, day, hour, min, sec);

    return t;
    //return 0;
  }
  return 0;
 //-------

 /*

  RadxPath rpath(path);
  const string &fileName = rpath.getFile();
  
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
    char cc;
    if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
               &year, &month, &day, &cc, &hour, &min, &sec) == 7) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      rtime.set(year, month, day, hour, min, sec);
      return 0;
    }
    start++;
  }
  
  return -1;
 */

}

/////////////////////////////////////////////////////////
// print summary after read

void BufrRadxFile::print(ostream &out) const
  
{
  
  out << "=============== BufrRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId: " << _scanId << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  refTimeSecsFile: " << RadxTime::strm(_refTimeSecsFile) << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  latitude: " << _latitudeDeg << endl;
  out << "  longitude: " << _longitudeDeg << endl;
  out << "  height: " << _heightKm << endl;
  out << "  frequencyGhz: " << _frequencyGhz << endl;
  
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int BufrRadxFile::printNative(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{

  _addErrStr("ERROR - BufrRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

/////////////////////////////////////////////////////
/// set the file time from the path
/// returns 0 on success, -1 on failure

int BufrRadxFile::setTimeFromPath(const string &filePath,
                                 time_t &fileTime)  
{
  /*
  getTimeFromString(filePath, RadxTime &fTime);
  
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
      //     rtime.set(year, month, day, hour, min, sec);  from getTimeFromString method
      RadxTime ftime(year, month, day, hour, min, sec);
      fileTime = ftime.utime();
      return 0;
    }
    start++;
  }
  */
  return -1;

}

void BufrRadxFile::lookupFieldName(string fieldName, string &units, 
string &standardName, string &longName) {
    if (fieldName.find("TH") != string::npos) {
      units = "dBz";
      standardName = "horizontal_reflectivity";
      longName = "horizontal_reflectivity";
    } else if (fieldName.find("CM") != string::npos) {
      units = "m/s";
      standardName = "standard_deviation_of_velocity";
      longName = "standard_deviation_of_velocity";
      //continue; // TODO:  skip this for now, it may be poorly formatted file
    } else if (fieldName.find("KDP") != string::npos) {
      units = "deg/km";
      standardName = "specific_differential_phase";
      longName = "specific_differential_phase";
    } else if (fieldName.find("PHIDP") != string::npos) {
      units = "degrees";
      standardName = "differential_phase_hv";
      longName = "differential_phase_shift";
    } else if (fieldName.find("RHOHV") != string::npos) {
      units = "";
      standardName = "cross_correlation_hv";
      longName = "cross_correlation_coefficient";
    } else if (fieldName.find("TDR") != string::npos) {
      units = "db";
      standardName = "log_differential_reflecivity_hv";
      longName = "differential_reflectivity";
    } else if (fieldName.find("TV") != string::npos) {
      units = "dBz";
      standardName = "vertical reflectivity";
      longName = "vertical reflectivity";
    } else if (fieldName.find("VRAD") != string::npos) {
      units = "m/s";
      standardName = "radial_velocity";
      longName = "radial_velocity";
    } else if (fieldName.find("WRAD") != string::npos) {
      units = "m/s";
      standardName = "width";
      longName = "width";
    } else {
      _addErrStr("ERROR - BufrRadxFile::_readFields");
      _addErrStr("  Unrecognized field: ", fieldName);
      throw _errStr.c_str();
    }
}

////////////////////////////////////////////////////////////
// Read in all fields for specified path.
// Returns 0 on success, -1 on failure

int BufrRadxFile::_readFields(const string &path)
  
{
  // get the list of all files, one field in each, that match this time

  vector<string> fileNames;
  vector<string> filePaths;
  vector<string> fieldNames;
  _getFieldPaths(path, fileNames, filePaths, fieldNames);

  if (filePaths.size() < 1) {
    _addErrStr("ERROR - BufrRadxFile::_readFields");
    _addErrStr("  No field files found, path: ", path);
    return -1;
  }

  // set file time

  time_t fileTime;
  fileTime = getTimeFromString(fileNameSuffix.c_str());
  if (fileTime == 0) {
  // if (setTimeFromPath(fileNameSuffix, fileTime)) {
    _addErrStr("ERROR - BufrRadxFile::_readFields");
    _addErrStr("  Cannot get time from file: ", fileNames[0]);
    return -1;
  }

  _fileTime.set(fileTime);
      
  // load vector of input fields
  //_clearFields();
  for (size_t ii = 0; ii < fileNames.size(); ii++) {
    string fieldName = fieldNames[ii];
    string units;
    string standardName;
    string longName;
    lookupFieldName(fieldName, units, standardName, longName);

    // read in the data
    try {
      if (_debug)  cerr << "reading field " << fieldName << endl;
      _file.readThatField(fileNames[ii], filePaths[ii], _fileTime.utime(),
          fieldName, standardName, longName, units, _debug, _verbose);
      // add to paths used on read  
      _readPaths.push_back(filePaths[ii]);
      if (_debug) cerr << "  .. accumulating field info " << endl;
      if (ii == 0) {
	_accumulateFieldFirstTime(fieldName, units, standardName, longName);
      } else {
        _accumulateField(fieldName, units, standardName, longName);
      }
    } catch (const char *msg) {
      // report error message and move to the next field
      _addErrStr("ERROR - BufrRadxFile::_readFields");
      _addErrStr("  Cannot read in field, path: ", filePaths[ii]);
      _addErrStr(msg);
      cout << _errStr << endl;
      //return -1;
      //delete field;
    }
  } // ii

  if (_readPaths.size() == 0) {
    _addErrStr("ERROR - BufrRadxFile::_readFields");
    _addErrStr("  No fields read in");
    return -1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////
// get list of field paths for the volume for the specified path

void BufrRadxFile::_getFieldPaths(const string &primaryPath,
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
  // example fileName:  RMA1_0117_02_TH_20170430T070516Z.BUFR
  //                    01234567890123456789012345678901
  //                              1         2         3
  // AH! this won't work, because the embedded field name has variable length.
  // fixed length for the prefix is ok, but use string.find 
  // to locate the suffix.
  int prefix_start = 0;
  int prefix_len = 12;
  int suffix_start = 16;
  int suffix_len = 16;
  string prefix(fileName.substr(prefix_start, prefix_len));
  size_t pos = fileName.find('.', prefix_len);
  if (pos != string::npos) {
    suffix_start = pos - suffix_len;
  } else {
    suffix_start = fileName.length() - suffix_len;
  }
  string suffix(fileName.substr(suffix_start, suffix_len));
  // set the class variable for the suffix
  fileNameSuffix = suffix;

  
  // load up array of file names that match the prefix and suffix
  
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

      string dStr(dName.substr(prefix_start, prefix_len));
      // to get the suffix, we need to find the end of the file name,
      // then subtract the suffix length, to get the start of the suffix
      // really, strip the extension off, then use the end of the string or string length
      // size_t pos = fileName.find('.', prefix_len);
	  // just find the suffix
      size_t suffix_start2 = dName.rfind(suffix);
      
      if ((dStr == prefix) && (suffix_start2 != string::npos)) {
        // get field name from file name
	//    size_t pos = dName.find('_', prefix_len);
        //if (pos != string::npos) {
          fileNames.push_back(dName);
	  // +/- 1 to skip the '_' characters
          int fieldNameLength = suffix_start2 - prefix_len - 2;
          string fieldName = dName.substr(prefix_len+1, fieldNameLength);
          fieldNames.push_back(fieldName);
          string dPath(dir);
          dPath += RadxPath::RADX_PATH_DELIM;
          dPath += dName;
          filePaths.push_back(dPath);
          
	  //} // if (pos ...
      } // if (dStr ...
      
    } // dp
    
    rdir.close();

  } // if (rdir ...
  /*
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
  */
}


/*
void BufrRadxFile::readThatField() {

 // TODO: here ... I need to somehow read each file;
  // each BufrFile == a field in one or more sweeps.
  // each BufrFile/field then needs to be accumulated in
  // the RadxVol structure.

  // maybe move all of this read stuff to separate function,
  // that _readFields can call???
  // push the BufrProducts/fields into a vector,
  // then when finished reading all the files for the same timestamp,
  // load the RadxVol with the BufrProducts/fields.

  // clear tmp rays

  _nTimesInFile = 0;
  _raysToRead.clear();
  _raysValid.clear();
  _nRangeInFile = 0;

  try {
    _file.openRead(path);
    _file.readSection0();
    _file.readSection1();
    _file.readDataDescriptors();
    _file.readData();  // TODO:  need to send vol object for streaming;
                       // or need to completely fill _file.currentProduct
                       // with data and load RadxVol in batch mode.
                       // choose the route that is easier for someone
                       // else to understand because the software will be
                       // open source, the easier it is to understand, 
                       // the better.
		       
  } catch (const char *msg) {
    //printf("Exception: %s\n", msg);
    _addErrStr(msg);
    return -1;
  }

  if (_debug) {
    cerr << " finished reading  " << endl; 
  }

  // close file

  _file.close();

}
*/
////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int BufrRadxFile::readFromPath(const string &path,
                                  RadxVol &vol)
  
{

  _initForRead(path, vol);  // TODO: may not need this; it causes a seg fault
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }

  string errStr("ERROR - BufrRadxFile::readFromPath");
  

  // read in fields from individual files;
  // accumulate the field data in the volume structure
  //---------  cut pasted from GemRadxFile readFromPath
  // read in fields
  
  // clear tmp rays

  _nTimesInFile = 0;
  _raysToRead.clear();
  _raysValid.clear();
  _nRangeInFile = 0;

  try {
    if (_readFields(path)) {
      return -1;
    }
  } catch (const char *msg) {
    printf("EXCEPTION - %s\n", msg);
  }

  // ----- end cut ----
  // TODO: here ... I need to somehow read each file;
  // each BufrFile == a field in one or more sweeps.
  // each BufrFile/field then needs to be accumulated in
  // the RadxVol structure.

  // maybe move all of this read stuff to separate function,
  // that _readFields can call???
  // push the BufrProducts/fields into a vector,
  // then when finished reading all the files for the same timestamp,
  // load the RadxVol with the BufrProducts/fields.

  /*
  // clear tmp rays

  _nTimesInFile = 0;
  _raysToRead.clear();
  _raysValid.clear();
  _nRangeInFile = 0;
  */

  // At this point, all the related BUFR files have been 
  // read and their data is sitting in the _fields list,
  // waiting to be loaded into a RadxVol.  
 
  // load the RadxVol with the data from the Bufr file
  // also, perform sanity checks, for example,
  // the number of sweeps is the same for all fields, etc. 

  _qualityCheckRays();

  // load the data into the read volume
  if (_debug) {
    cerr << "before _loadReadVolume() " << endl; 
  }

  if (_loadReadVolume()) {
    return -1;
  }
  if (_debug) {
    cerr << "after _loadReadVolume() " << endl;
  }
  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();
  
  // set format as read
  // TODO:
  //_fileFormat = FILE_FORMAT_BUFR;

  // clean up

  _clearRayVariables();
  _dTimes.clear();
  
  return 0;
}

void BufrRadxFile::_accumulateFieldFirstTime(string fieldName, string units, string standardName, string longName) {

  //_file = file;
  // all of these functions work on the current BufrFile 
  // pointed to by _file class variable.
  try {
    // if 1st time through, establish the dimensions
    // read required dimensions
    _nTimesInFile = _file.getTimeDimension();
    _nRangeInFile = _file.getRangeDimension();

    // _readGlobalAttributes();
    _year_attr = _file.hdr_year;
    _month_attr = _file.hdr_month;
    _day_attr = _file.hdr_day;
    if ((_year_attr != _fileTime.getYear()) ||
       (_month_attr != _fileTime.getMonth()) ||
       (_day_attr != _fileTime.getDay()))
    throw "Time in file name does not match time in file";

    _siteName = _file.typeOfStationId;
    _instrumentName = _file.stationId;

    _setRangeVariable();
  
    // read lat/lon/alt 
    _setPositionVariables();
 
    // create the rays array
    _raysToRead.clear();

    // get the number of sweeps
    size_t nSweeps = _file.getNumberOfSweeps();

    // for each sweep
    for (size_t sn=0; sn<nSweeps; sn++) {
      RadxSweep *sweep = new RadxSweep();
      // TODO: Ok How are the field variables added to the sweep???
      sweep->setSweepNumber(sn);
      // read time variable
      _getRayTimes(sn);
      // get ray variables
      if (_debug) {
	cerr << " fetching ray  variable " << endl; 
      }
      _getRayVariables(sn);  // fills in _azimuths & _elevations
      if (_readMetadataOnly) {
	// read field variables
	_addFieldVariables(sn, fieldName, units, standardName, longName, true);
      } else {
	// create the rays to be read in, filling out the metadata
	_createRays(sn);  // stuffs rays into _raysToRead vector
	// add field variables to file rays
	_addFieldVariables(sn, fieldName, units, standardName, longName, false);
      }
      _sweeps.push_back(sweep);
    } // end for each sweep
  } catch (const char *msg) {
    printf("Exception: %s\n", msg);
    exit(1);
  }
}

void BufrRadxFile::_errorMessage(string location, string msg, int foundValue, 
int expectedValue) {
  _addErrStr(location); // "ERROR - BufrRadxFile::_accumulateField");
  _addErrInt(msg, foundValue); // "Number of sweeps incompatible: found ", nSweeps);
  _addErrInt(" expected ", expectedValue); // _sweeps.size());
}

void BufrRadxFile::_errorMessage(string location, string msg, string foundValue, 
string expectedValue) {
  _addErrStr(location); // "ERROR - BufrRadxFile::_accumulateField");
  _addErrStr(msg, foundValue); // "Number of sweeps incompatible: found ", nSweeps);
  _addErrStr(" expected ", expectedValue); // _sweeps.size());
}

// call _accumulateFieldFirstTime prior to calling
// this method.  This method assumes all the 
// arrays (accumulators) have been created.
// This method only verifies that the dimensions
// in the next input file have the same dimensions
// and then the data are added to the accumulators.
void BufrRadxFile::_accumulateField(string fieldName, string units, string standardName, string longName) {

  //_file = file;
  // all of these functions work on the current BufrFile 
  // pointed to by _file class variable.
  
  // if 2nd time through, or more, then verify dimensions are compatible
  // verify required dimensions
  const char *location = "ERROR - BufrRadxFile::_accumulateField";
  if (_file.getTimeDimension() != _nTimesInFile) {
    throw ;
    _errorMessage(location, "Time dimension incompatible, found ", _file.getTimeDimension(),
		 _nTimesInFile);
    throw "incompatible";
  }
  if (_file.getRangeDimension() != _nRangeInFile) {
    _errorMessage(location, "Range dimension incompatible, found ", _file.getRangeDimension(),
		 _nRangeInFile);
    throw "incompatible";
  }
  //_readGlobalAttributes();
  if ((_year_attr != _file.hdr_year) || (_month_attr != _file.hdr_month) ||
      (_day_attr != _file.hdr_day)) {
    _errorMessage(location, "Date is incompatible, found ", _file.hdr_year, _year_attr);
    _errorMessage(location, "Date is incompatible, found ", _file.hdr_month, _month_attr);
    _errorMessage(location, "Date is incompatible, found ", _file.hdr_day, _day_attr);
    throw "incompatible";
  }
  if ((_siteName != _file.typeOfStationId) || 
      (_instrumentName != _file.stationId)) {
    _errorMessage(location, "Global data are incompatible, found ", _file.typeOfStationId, _siteName);
    _errorMessage(location, "Global data are incompatible, found ", _file.stationId, _instrumentName);
    throw "incompatible";
  }
  _verifyRangeVariable();
  
  // verify lat/lon/alt 
  _verifyPositionVariables();

  // get the number of sweeps
  size_t nSweeps = _file.getNumberOfSweeps();
  if (nSweeps != _sweeps.size()) {
    _addErrStr("ERROR - BufrRadxFile::_accumulateField");
    _addErrInt("Number of sweeps incompatible: found ", nSweeps);
    _addErrInt(" expected ",  _sweeps.size());
    throw _errString.c_str();
  }
  // TODO: consider putting this loop into a separate method
  // since it seems similar in 1st and 2nd times
  try {
    // for each sweep
    for (size_t sn=0; sn<nSweeps; sn++) {
      //RadxSweep *sweep = _sweeps.at(sn); //new RadxSweep();
      // TODO: Ok How are the field variables added to the sweep???
      //sweep->setSweepNumber(sn);
      // read time variable
      // TODO: compare the ray times; make sure they are compatible
      //_getRayTimes(sn);
      // get ray variables
      if (_debug) {
	cerr << " fetching ray  variable " << endl; 
      }
      //_getRayVariables(sn);  // fills in _azimuths & _elevations
      // TODO: compare the ray az and el; make sure they are compatible
      if (_readMetadataOnly) {
	// read field variables
	_addFieldVariables(sn, fieldName, units, standardName, longName, true);
      } else {
	// add field variables to file rays
	_addFieldVariables(sn, fieldName, units, standardName, longName, false);
      }
      //_sweeps.push_back(sweep);
    } // end for each sweep
  } catch (const char *msg) {
    printf("Exception: %s\n", msg);
    exit(1);
  }
}

  //  call after all files are read
void BufrRadxFile::_qualityCheckRays() {

  // add file rays to main rays
  
  _raysValid.clear();

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    RadxRay *ray = _raysToRead[ii];
    
    // check if we should keep this ray or discard it
    
    bool keep = true;
    if (_readRemoveRaysAllMissing) {
      if (ray->checkDataAllMissing()) {
        keep = false;
      }
    }

    // add to main vector if we are keeping it

    if (keep) {
      _raysValid.push_back(ray);
    } else {
      delete ray;
    }

  }
  
  _raysToRead.clear();
  
}

///////////////////////////////////
// read in the dimensions

int BufrRadxFile::_readDimensions()

{
  // read required dimensions
  _nTimesInFile = _file.getTimeDimension();
  _nRangeInFile = _file.getRangeDimension();
  return 0;
}

///////////////////////////////////
// read the global attributes

int BufrRadxFile::_readGlobalAttributes()

{
  /*
   _file.readGlobAttr("scan_number", _scan_number_attr);
   _file.readGlobAttr("file_number", _file_number_attr);

   _file.readGlobAttr("scantype", _scantype_attr);
   _file.readGlobAttr("experiment_id", _experiment_id_attr);
   _file.readGlobAttr("operator", _operator_attr);
   _file.readGlobAttr("scan_velocity", _scan_velocity_attr);
   _file.readGlobAttr("min_range", _min_range_attr);
   _file.readGlobAttr("max_range", _max_range_attr);
   _file.readGlobAttr("min_angle", _min_angle_attr);
   _file.readGlobAttr("max_angle", _max_angle_attr);
   _file.readGlobAttr("scan_angle", _scan_angle_attr);
   _file.readGlobAttr("scan_datetime", _scan_datetime_attr);
   _file.readGlobAttr("extra_attenuation", _extra_attenuation_attr);
   _file.readGlobAttr("ADC_bits_per_sample", _ADC_bits_per_sample_attr);
   _file.readGlobAttr("samples_per_pulse", _samples_per_pulse_attr);
   _file.readGlobAttr("pulses_per_daq_cycle", _pulses_per_daq_cycle_attr);
   _file.readGlobAttr("ADC_channels", _ADC_channels_attr);
   _file.readGlobAttr("delay_clocks", _delay_clocks_attr);
   _file.readGlobAttr("pulses_per_ray", _pulses_per_ray_attr);
   _file.readGlobAttr("radar_constant", _radar_constant_attr);
   _file.readGlobAttr("receiver_gain", _receiver_gain_attr);
   _file.readGlobAttr("cable_losses", _cable_losses_attr);
  */
   _year_attr = _file.hdr_year;
   _month_attr = _file.hdr_month;
   _day_attr = _file.hdr_day;
   // _file.readGlobAttr("institution", _institution);
 
   _siteName = _file.typeOfStationId;
   _instrumentName = _file.stationId;
   //_platformType = // WMO block number
     // WMO station number
   
  return 0;

}

/*
///////////////////////////////////
// read the times

int BufrRadxFile::_readTimes()

{

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", "time");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 1) {
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }

  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att("units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

#ifdef NOTNOW

  // check if this is a time variable, using udunits
  
  ut_unit *udUnit = ut_parse(_udunits.getSystem(), units.c_str(), UT_ASCII);
  if (udUnit == NULL) {
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  Cannot parse time units: ", units);
    _addErrInt("  udunits status: ", ut_get_status());
    return -1;
  }
    
  if (ut_are_convertible(udUnit, _udunits.getEpoch()) == 0) {
    // not a time variable
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  Time does not have convertible units: ", units);
    _addErrInt("  udunits status: ", ut_get_status());
    ut_free(udUnit);
    return -1;
  }
  
  // get ref time as unix time using udunits
  
  cv_converter *conv = ut_get_converter(udUnit, _udunits.getEpoch());
  double refTimeDouble = cv_convert_double(conv, 0);
  _refTimeSecsFile = (time_t) refTimeDouble;

#endif
  
  // parse the time units reference time

  RadxTime stime(units);
  _refTimeSecsFile = stime.utime();
  
  // set the time array
  
  RadxArray<double> dtimes_;
  double *dtimes = dtimes_.alloc(_nTimesInFile);
  if (_timeVar->get(dtimes, _nTimesInFile) == 0) {
    _addErrStr("ERROR - BufrRadxFile::_readTimes");
    _addErrStr("  Cannot read times variable");
    return -1;
  }
  _dTimes.clear();
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    _dTimes.push_back(dtimes[ii]);
  }

  return 0;

}
*/

int BufrRadxFile::_getRayTimes(int sweepNumber)

{
  _dTimes.clear();
  double rayTimeSeconds;
  double increment;
  double endTimeSeconds;
  double startTimeSeconds;
  endTimeSeconds = _file.getEndTimeForSweep(sweepNumber);
  startTimeSeconds = _file.getStartTimeForSweep(sweepNumber);
  increment = (endTimeSeconds - startTimeSeconds) / _file.getTimeDimension();
  rayTimeSeconds = startTimeSeconds;
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    _dTimes.push_back(rayTimeSeconds);
    rayTimeSeconds += increment;
  }

  return 0;

}

///////////////////////////////////
// read the range variable

int BufrRadxFile::_setRangeVariable()
{
  // set units

  double kmPerUnit = 0.001; // default - units in km

  double rangeBinSizeKm;
  rangeBinSizeKm = _file.getRangeBinSizeMeters() * kmPerUnit;
  double rangeBinOffsetKm;
  rangeBinOffsetKm = _file.getRangeBinOffsetMeters() * kmPerUnit; 

  // set range vector

  _rangeKm.clear();
  _nRangeInFile = _file.getNBinsAlongTheRadial(); //_rangeVar->num_vals();
  //  RadxArray<double> rangeVals_;
  //double *rangeVals = rangeVals_.alloc(_nRangeInFile);
  for (size_t ii = 0; ii < _nRangeInFile; ii++) {
    _rangeKm.push_back(rangeBinOffsetKm + ii*rangeBinSizeKm);
  }

  // set the geometry from the range vector
  
  _remap.computeRangeLookup(_rangeKm);
  _gateSpacingIsConstant = _remap.getGateSpacingIsConstant();
  _geom.setRangeGeom(_remap.getStartRangeKm(), _remap.getGateSpacingKm());

  return 0;
}

int BufrRadxFile::_verifyRangeVariable()
{
  // set units
  /*
  double kmPerUnit = 0.001; // default - units in km
  
  double rangeBinSizeKm;
  if (rangeBinSizeKm != _file.getRangeBinSizeMeters() * kmPerUnit)
    throw "Range bin size is incompatible";
  double rangeBinOffsetKm;
  if (rangeBinOffsetKm != _file.getRangeBinOffsetMeters() * kmPerUnit)
    throw "Range bin offset is incompatible";
  */
  // verify range vector

  if (_nRangeInFile != _file.getNBinsAlongTheRadial())
    throw "Number of bins along the radial is incompatible";

  return 0;
}


///////////////////////////////////
// read the position variables

int BufrRadxFile::_setPositionVariables()
{

  // find latitude, longitude, height

  int iret = 0;
  _latitudeDeg = _file.latitude;
  if ((_latitudeDeg < -90) || (_latitudeDeg > 90)) {
    _addErrStr("ERROR - BufrRadxFile::_setPositionVariables");
    char temp[1024];
    sprintf(temp, "%g", _latitudeDeg);
    _addErrStr("  Latitude outside boundaries (-90 to 90): ", temp);
    iret = -1;
  }

  _longitudeDeg = _file.longitude;
  if ((_longitudeDeg < -180) || (_longitudeDeg > 180)) {
    _addErrStr("ERROR - BufrRadxFile::_setPositionVariables");
    char temp[1024];
    sprintf(temp, "%g", _longitudeDeg);
    _addErrStr("  Longitude outside boundaries (-180 to 180): ", temp);
    iret = -1;
  }

  _heightKm = _file.height / 1000.0;  // convert to Km
  if (_heightKm < 0) {
    _addErrStr("ERROR - BufrRadxFile::_setPositionVariables");
    _addErrStr("  Cannot read height");
    iret = -1;
  }
// TODO: need to figure out units for height
/*
  Nc3Att* unitsAtt = _heightVar->get_att("units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units == "m") {
      _heightKm /= 1000.0;
    }
    delete unitsAtt;
  }
*/
  return iret;

}

int BufrRadxFile::_verifyPositionVariables()

{

  // find latitude, longitude, height

  int iret = 0;
  if (_latitudeDeg != _file.latitude) {
    _addErrStr("ERROR - BufrRadxFile::_verifyPositionVariables");
    char temp[1024];
    sprintf(temp, "%g", _latitudeDeg);
    _addErrStr("  Latitude incompatible: ", temp);
    iret = -1;
  }

  if(_longitudeDeg != _file.longitude) {
    _addErrStr("ERROR - BufrRadxFile::_verifyPositionVariables");
    char temp[1024];
    sprintf(temp, "%g", _longitudeDeg);
    _addErrStr("  Longitude incompatible: ", temp);
    iret = -1;
  }

  if (_heightKm != _file.height / 1000.0) {  // convert to Km
    _addErrStr("ERROR - BufrRadxFile::_verifyPositionVariables");
    char temp[1024];
    sprintf(temp, "%g", _file.height);
    _addErrStr("  Height incompatible, found height (Km): ", temp);
    iret = -1;
  }
// TODO: need to figure out units for height
/*
  Nc3Att* unitsAtt = _heightVar->get_att("units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units == "m") {
      _heightKm /= 1000.0;
    }
    delete unitsAtt;
  }
*/
  return iret;

}

/*
///////////////////////////////////
// read scalar variables

int BufrRadxFile::_readScalarVariables()

{

  // 

  int iret = 0;
  if (_file.readDoubleVar(_beamwidthHVar, "beamwidthH", _beamwidthHDeg , 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read beam width H");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_beamwidthVVar, "beamwidthV", _beamwidthVDeg , 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read beam width V");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_frequencyVar, "frequency", _frequencyGhz, 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read frequency");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_prfVar, "prf", _prfHz, 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read prf");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_antennaDiameterVar, "antenna_diameter", _antennaDiameterM, 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read antenna diameter");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_transmitPowerVar, "transmit_power", _transmitPowerW, 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read transmit power");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_pulsePeriodVar, "pulse_period", _pulsePeriodUs, 0, true)) {
    _addErrStr("ERROR - BufrRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read pulse period");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  return iret;

}
*/

///////////////////////////////////
// clear the ray variables

void BufrRadxFile::_clearRayVariables()

{
  _azimuths.clear();
  _elevations.clear();
}

///////////////////////////////////
// read in ray variables

int BufrRadxFile::_getRayVariables(int sweepNumber)
{

  _clearRayVariables();
  int iret = 0;

  // azimuth is calculated from "Antenna beam azimuth" ... 
  //   "Antenna beam azimuth" + "Number of azimuths" - 1 

  // get the starting offset 
  double startingAzimuth = _file.getStartingAzimuthForSweep(sweepNumber);
// TODO:  **** testing only ****
  startingAzimuth = 0.0;  // keep this! we don't offset by Azimuth
// TODO: ***** end testing only ***
  // get the number of azimuths
  int nAzimuths = _file.getNAzimuthsForSweep(sweepNumber);
 
  // fill in the vector
  double value;
  double increment;
  increment = 360.0/nAzimuths;
  value = startingAzimuth;
  for (int i=0; i<nAzimuths; i++) {
    _azimuths.push_back(value);
    value += increment;
    if (value >= 360)
      value = value - 360.0;
  }
  /*
  _readRayVar(_azimuthVar, "azimuth", _azimuths);
  if ((int) _azimuths.size() != _timeDim->size()) {
    _addErrStr("ERROR - Azimuth_current variable required");
    iret = -1;
  }
  */ 
  // elevation is the same for all rays in the sweep
  //_readRayVar(_elevationVar, "elevation", _elevations);
  //if ((int) _elevations.size() != _timeDim->size()) {
  //  _addErrStr("ERROR - Elevation_current variable required");
  //  iret = -1;
  //}
  _elevations.push_back(_file.getElevationForSweep(sweepNumber));
  if (iret) {
    _addErrStr("ERROR - BufrRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int BufrRadxFile::_createRays(int sweepNumber)

{
  // remember, _nTimesInFile is set to nAzimuths for this sweep
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {  // TODO: nTimesInFile??

    // new ray
    
    RadxRay *ray = new RadxRay;
    // rayInfo.ray = ray;

    ray->copyRangeGeom(_geom);
    
    // set time
    
    double rayTimeDouble = _dTimes[ii]; 
    time_t rayUtimeSecs = (time_t) rayTimeDouble; // _refTimeSecsFile + (time_t) rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
    ray->setTime(rayUtimeSecs, rayNanoSecs);
    
    // sweep info
    ray->setSweepNumber(sweepNumber);
    ray->setAzimuthDeg(_azimuths[ii]);
    ray->setElevationDeg(_elevations.back());
    //ray->setPrtSec(1.0/_prfHz);
    //ray->setTargetScanRateDegPerSec(_scan_velocity_attr);
    //ray->setNSamples(_pulses_per_ray_attr);

    // add to ray vector for reading

    _raysToRead.push_back(ray);

  } // ii

  return 0;

}

////////////////////////////////////////////
// read the field variables
// TODO:  I need 
int BufrRadxFile::_addFieldVariables(int sweepNumber,
				      string name, string units,
				      string standardName, string longName,
                                      bool metaOnly)
{
    int iret = 0;
    bool isDiscrete = false;
    // TODO: verify Type of Product???
    //string name = _file.getTypeOfProductForSweep(sweepNumber);; // "TH";
    //string units = "m/s";
    // string standardName = name;
    //string longName = name;
    bool fieldFolds = false;
    float foldLimitLower = 0.0;
    float foldLimitUpper = 0.0;
    //switch (var->type()) {
    //  case nc3Double: {
        if (_addFl64FieldToRays(sweepNumber, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
    //     break;
    //   }
    //   case nc3Float: {
    //     if (_addFl32FieldToRays(var, name, units, standardName, longName,
    //                             isDiscrete, fieldFolds,
    //                             foldLimitLower, foldLimitUpper)) {
    //       iret = -1;
    //     }
    //     break;
    //   }
    //   default: {
    //     iret = -1;
    //     // will not reach here because of earlier check on type
    //   }

    // } // switch
    
  /*
    if (metaOnly) {
      bool fieldAlreadyAdded = false;
      for (size_t ii = 0; ii < _readVol->getNFields(); ii++) {
        if (_readVol->getField(ii)->getName() == name) {
          fieldAlreadyAdded = true;
          break;
        }
      }
      if (!fieldAlreadyAdded) {
        RadxField *field = new RadxField(name, units);
        field->setLongName(longName);
        field->setStandardName(standardName);
        if (fieldFolds &&
            foldLimitLower != Radx::missingMetaFloat &&
            foldLimitUpper != Radx::missingMetaFloat) {
          field->setFieldFolds(foldLimitLower, foldLimitUpper);
        }
        _readVol->addField(field);
      }
      continue;
    }
  */

    if (iret) {
      _addErrStr("ERROR - BufrRadxFile::_addFieldVariables");
      _addErrStr("  cannot add field name: ", name);
      //_addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

    //} // ivar

  return 0;

}
/*
///////////////////////////////////
// read a ray variable - double

int BufrRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                 vector<double> &vals, bool required)

{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - BufrRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  double *data = new double[_nTimesInFile];
  double *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - BufrRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - integer

int BufrRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                 vector<int> &vals, bool required)

{

  vals.clear();

  // get var
  
  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - BufrRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int *data = new int[_nTimesInFile];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - BufrRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* BufrRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - BufrRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - BufrRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - BufrRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim->name());
      _addErrStr("  should be: ", "time");
    }
    return NULL;
  }

  return var;

}
*/
//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Add the single field to each ray of a sweep
// Returns 0 on success, -1 on failure

int BufrRadxFile::_addFl64FieldToRays(int sweepNumber,    // Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         bool isDiscrete,
                                         bool fieldFolds,
                                         float foldLimitLower,
                                         float foldLimitUpper)
  
{

  // get data from array

  Radx::fl64 *data; //  = new Radx::fl64[_nTimesInFile * _nRangeInFile];
  //int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  //if (iret) {
  //  delete[] data;
  //  return -1;
  //}
  data = _file.getDataForSweep(sweepNumber);

  // set missing value

  Radx::fl64 missingVal = -DBL_MAX; // Radx::missingFl64;
  // Nc3Att *missingValueAtt = var->get_att("missing_value");
  // if (missingValueAtt != NULL) {
  //   missingVal = missingValueAtt->as_double(0);
  //   delete missingValueAtt;
  // }

  // look for fill value, if not present, then use missing value

   Radx::fl64 fillVal = missingVal;
  // Nc3Att *fillValueAtt = var->get_att("_FillValue");
  // if (fillValueAtt != NULL) {
  //   fillVal = fillValueAtt->as_double(0);
  //   delete fillValueAtt;
  // }

  // replace any NaN's with fill value
  for (size_t jj = 0; jj < _nTimesInFile * _nRangeInFile; jj++) {
    if (std::isnan(data[jj]))
      data[jj] = fillVal;
  }

  // load field on rays

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    
    int nGates = _nRangeInFile;
    int startIndex = ii * _nRangeInFile;

    cout << "adding ray " << ii << endl;    

    // the rays for this sweep start at sweepNumber*_nTimesInFile
    int rayIdx;
    rayIdx = (sweepNumber * _nTimesInFile) + ii;
    bool pleaseCopyData = true;
    RadxField *field =
      _raysToRead[rayIdx]->addField(name, units, nGates,
				missingVal,
				data + startIndex,
				pleaseCopyData);

    field->setMissingFl64(missingVal);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

/*
//////////////////////////////////////////////////////////////
// Add fl32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int BufrRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         bool isDiscrete,
                                         bool fieldFolds,
                                         float foldLimitLower,
                                         float foldLimitUpper)
  
{

  // get data from array

  Radx::fl32 *data = new Radx::fl32[_nTimesInFile * _nRangeInFile];
  int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_float(0);
    delete missingValueAtt;
  }

  // look for fill value, if not present, then use missing value

  Radx::fl32 fillVal = missingVal;
  Nc3Att *fillValueAtt = var->get_att("_FillValue");
  if (fillValueAtt != NULL) {
    fillVal = fillValueAtt->as_float(0);
    delete fillValueAtt;
  }

  // replace any NaN's with fill value
  for (size_t jj = 0; jj < _nTimesInFile * _nRangeInFile; jj++) {
    if (std::isnan(data[jj]))
      data[jj] = fillVal;
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    int nGates = _nRangeInFile;
    int startIndex = ii * _nRangeInFile;

    RadxField *field =
      _raysToRead[ii]->addField(name, units, nGates,
				missingVal,
				data + startIndex,
				true);
    
    field->setMissingFl32(missingVal);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

*/
/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int BufrRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("bufr");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyGhz * 1.0e9);

  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanName);
  _readVol->setScanId(_scanId);
  _readVol->setInstrumentName(_instrumentName);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_heightKm);

  _readVol->copyRangeGeom(_geom);

  _readVol->setRadarBeamWidthDegH(_beamwidthHDeg);
  _readVol->setRadarBeamWidthDegV(_beamwidthVDeg);

  // add calibration 
  RadxRcalib *cal = new RadxRcalib;
  cal->setPulseWidthUsec(_pulsePeriodUs);
  cal->setRadarConstantH(_radar_constant_attr);
  cal->setRadarConstantV(_radar_constant_attr);
  cal->setReceiverGainDbHc(_receiver_gain_attr);
  cal->setReceiverGainDbHx(_receiver_gain_attr);
  cal->setReceiverGainDbVc(_receiver_gain_attr);
  cal->setReceiverGainDbVx(_receiver_gain_attr);
  cal->setPowerMeasLossDbH(_cable_losses_attr);
  cal->setPowerMeasLossDbV(_cable_losses_attr);
  double transmitPowerDbm = 10.0 * log10(_transmitPowerW * 1000.0);
  cal->setXmitPowerDbmH(transmitPowerDbm);
  cal->setXmitPowerDbmV(transmitPowerDbm);
  _readVol->addCalib(cal); 

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  if (_raysValid.size() <= 0) 
    cerr << "Warning: there are no valid rays" << endl;
  for (int ii = 0; ii < (int) _raysValid.size(); ii++) {
    _raysValid[ii]->setVolumeNumber(_volumeNumber);
  }

  // add rays to vol - they will be freed by vol
  // TODO: the rays are striped in the vector, with a stride = # of fields
  for (size_t ii = 0; ii < _raysValid.size(); ii++) {
    _readVol->addRay(_raysValid[ii]);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _raysValid.clear();

  // set sweep mode from the ray angles

  Radx::SweepMode_t sweepMode = _readVol->getPredomSweepModeFromAngles();
  vector<RadxRay *> &rays = _readVol->getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    rays[ii]->setSweepMode(sweepMode);
  }
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - BufrRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - BufrRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void BufrRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumAngle = 0.0;
    double count = 0.0;

    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      if (ray.getSweepMode() == Radx::SWEEP_MODE_RHI) {
	sumAngle += ray.getAzimuthDeg();
      } else {
	sumAngle += ray.getElevationDeg();
      }
      count++;
    }

    double meanAngle = sumAngle / count;
    double fixedAngle = ((int) (meanAngle * 100.0 + 0.5)) / 100.0;

    sweep.setFixedAngleDeg(fixedAngle);
      
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      RadxRay &ray = *_readVol->getRays()[iray];
      ray.setFixedAngleDeg(fixedAngle);
    }

  } // isweep

  _readVol->loadFixedAnglesFromSweepsToRays();

}

