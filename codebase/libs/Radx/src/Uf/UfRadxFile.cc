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
// UfRadxFile.cc
//
// UfRadxFile object
//
// Universal Format for radar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/UfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxGeoref.hh>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
using namespace std;

//////////////
// Constructor

UfRadxFile::UfRadxFile() : RadxFile()
  
{
  
  _writeVol = NULL;
  _readVol = NULL;
  _file = NULL;
  clear();
  _createDefaultNameTable();

}

/////////////
// destructor

UfRadxFile::~UfRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void UfRadxFile::clear()
  
{

  clearErrStr();
  _close();
  
  _clearRays();

  _ufIsSwapped = false;
  _writeNativeByteOrder = false;
  _expandUfNames = false;
  _writeFileNameMode = FILENAME_WITH_START_TIME_ONLY;

  _latitudes.clear();
  _longitudes.clear();
  _altitudes.clear();
  
}

void UfRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

void UfRadxFile::_clearUfStructs()
{
  memset(&_manHdr, 0, sizeof(_manHdr));
  memset(&_optHdr, 0, sizeof(_optHdr));
  _optFound = false;
  memset(&_dataHdr, 0, sizeof(_dataHdr));
  _fieldInfo.clear();
  _fieldHdrs.clear();
  _fieldNames.clear();
  for (size_t ii = 0; ii < _fieldBuffers.size(); ii++) {
    delete _fieldBuffers[ii];
  }
  _fieldBuffers.clear();

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
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

int UfRadxFile::writeToDir(const RadxVol &vol,
                           const string &dir,
                           bool addDaySubDir,
                           bool addYearSubDir)
  
{
  
  clearErrStr();
  _writeVol = &vol;
  _dirInUse = dir;
  _writePaths.clear();
  _writeDataTimes.clear();

  if (_debug) {
    cerr << "DEBUG - UfRadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }

  RadxTime ftime(_writeVol->getStartTimeSecs());
  int millisecs = (int) (_writeVol->getStartNanoSecs() / 1.0e6 + 0.5);

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
    _addErrStr("ERROR - UfRadxFile::writeToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute path
  
  int nSweeps = _writeVol->getSweeps().size();
  string scanType = "SUR";
  double fixedAngle = 0.0;
  if (nSweeps > 0) {
    const RadxSweep &sweep = *_writeVol->getSweeps()[0];
    scanType = Radx::sweepModeToShortStr(sweep.getSweepMode());
    fixedAngle = sweep.getFixedAngleDeg();
  }
  int volNum = _writeVol->getVolumeNumber();
  string outName =
    computeFileName(volNum, nSweeps, fixedAngle,
                    _writeVol->getInstrumentName(), scanType,
                    ftime.getYear(), ftime.getMonth(), ftime.getDay(),
                    ftime.getHour(), ftime.getMin(), ftime.getSec(),
                    millisecs);
  string outPath(outDir);
  outPath += PATH_SEPARATOR;
  outPath += outName;
  
  int iret = writeToPath(*_writeVol, outPath);

  if (iret) {
    _addErrStr("ERROR - UfRadxFile::_writeToDir");
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

int UfRadxFile::writeToPath(const RadxVol &vol,
                            const string &path)
  
{

  clearErrStr();
  _writeVol = &vol;
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _writePaths.clear();
  _writeDataTimes.clear();

  // open the output Nc file

  string tmpPath(tmpPathFromFilePath(path, ""));

  if (_debug) {
    cerr << "DEBUG - UfRadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path: " << tmpPath << endl;
  }

  if (_openWrite(tmpPath)) {
    _addErrStr("ERROR - UfRadxFile::writeToPath");
    _addErrStr("  Cannot open tmp uf file: ", tmpPath);
    return -1;
  }

  for (size_t iray = 0; iray < vol.getNRays(); iray++) {
    
    // load record from ray

    if (_loadWriteRecordFromRay(vol, iray)) {
      _addErrStr("ERROR - UfRadxFile::writeToPath");
      _addErrStr("  Writing file: ", tmpPath);
      _close();
      remove(tmpPath.c_str());
      return -1;
    }

    // Write UF record to output file
    
    if (_writeRecord()) {
      _addErrStr("ERROR - UfRadxFile::writeToPath");
      _addErrStr("  Writing file: ", tmpPath);
      _close();
      remove(tmpPath.c_str());
      return -1;
    }

  } // iray      

  // close output file
  
  _close();
  
  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - UfRadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());
 
  return 0;

}

////////////////////////////////////////////////////////////
// compute and return uf file name

string UfRadxFile::computeFileName(int volNum,
                                   int nSweeps,
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

  if (nSweeps != 1) {
    fixedAngle = 0.0;
  }

  char outName[BUFSIZ];
  sprintf(outName, "%04d%02d%02d_%02d%02d%02d_%s_v%03d_%s.uf",
          year, month, day, hour, min, sec,
          instrumentName.c_str(), volNum, scanType.c_str());

  return outName;

}
  
/////////////////////////////////////////////////////////
// Check if specified file is UF format
// Returns true if supported, false otherwise

bool UfRadxFile::isSupported(const string &path)

{
  
  if (isUf(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a Uf file
// Returns true on success, false on failure

bool UfRadxFile::isUf(const string &path)
  
{

  _close();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - UfRadxFile::isUf");
    return false;
  }
  
  // read first 6 bytes

  char id[6];
  if (fread(id, 1, 6, _file) != 6) {
    _close();
    return false;
  }
  _close();

  if (id[4] == 'U' && id[5] == 'F') {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this file needs to be byte-swapped on this host
// Returns 0 on success, -1 on failure
// Use isSwapped() to get result after making this call.

int UfRadxFile::checkIsSwapped(const string &path)
  
{

  _ufIsSwapped = false;

  // stat file to get size

  struct stat fileStat;
  if (stat(path.c_str(), &fileStat)) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot stat file: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }

  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    return -1;
  }
  
  // read leading rec header - this is a 4-byte integer FORTRAN uses
  
  Radx::ui32 nbytesStart;
  if (fread(&nbytesStart, sizeof(Radx::ui32), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot read record length, file: ", path);
    _addErrStr("  ", strerror(errNum));
    _close();
    return -1;
  }

  // check against file size
  
  if (nbytesStart > fileStat.st_size - 2 * sizeof(Radx::ui32)) {
    // swapped
    _ufIsSwapped = true;
    _close();
    return 0;
  }

  // move to trailing record

  long offsetFromStart = nbytesStart + sizeof(Radx::ui32);
  if (fseek(_file, offsetFromStart, SEEK_SET)) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot seek to end record: ", path);
    _addErrInt("  offset: ", offsetFromStart);
    _addErrStr("  ", strerror(errNum));
    _close();
    return -1;
  }
    
  // read trailing rec header - this is a 4-byte integer FORTRAN uses
  
  Radx::ui32 nbytesEnd;
  if (fread(&nbytesEnd, sizeof(Radx::ui32), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot read record length, file: ", path);
    _addErrStr("  ", strerror(errNum));
    _close();
    return -1;
  }

  if (nbytesStart == nbytesEnd) {
    // not swapped
    _close();
    return 0;
  }

  // seems to be swapped

  Radx::ui32 nbytesSwapped = nbytesStart;
  ByteOrder::swap32(&nbytesSwapped, sizeof(Radx::ui32));

  // check against file size
  
  if (nbytesSwapped > fileStat.st_size - 2 * sizeof(Radx::ui32)) {
    // failure
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot decode file: ", path);
    _close();
    return -1;
  }

  // move to trailing record

  offsetFromStart = nbytesSwapped + sizeof(Radx::ui32);
  if (fseek(_file, offsetFromStart, SEEK_SET)) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot seek to end record: ", path);
    _addErrInt("  offset: ", offsetFromStart);
    _addErrStr("  ", strerror(errNum));
    _close();
    return -1;
  }
    
  // read trailing rec header - this is a 4-byte integer FORTRAN uses
  
  if (fread(&nbytesEnd, sizeof(Radx::ui32), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
    _addErrStr("  Cannot read record length, file: ", path);
    _addErrStr("  ", strerror(errNum));
    _close();
    return -1;
  }

  if (nbytesStart == nbytesEnd) {
    // swapped
    _ufIsSwapped = true;
    _close();
    return 0;
  }

  // failure

  _addErrStr("ERROR - UfRadxFile::checkIsSwapped");
  _addErrStr("  Cannot decode file: ", path);
  _close();
  return -1;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int UfRadxFile::readFromPath(const string &path,
                             RadxVol &vol)
  
{

  _initForRead(path, vol);
  RadxBuf buf;

  // is this a Uf file?
  
  if (!isUf(path)) {
    _addErrStr("ERROR - UfRadxFile::readFromPath");
    _addErrStr("  Not a uf file: ", path);
    return -1;
  }

  // check if data in this file is swapped - this sets _ufIsSwapped
  
  if (checkIsSwapped(path)) {
    _addErrStr("ERROR - UfRadxFile::readFromPath");
    _addErrStr("  Cannot check if swapped: ", path);
    return -1;
  }

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - UfRadxFile::readFromPath");
    return -1;
  }
  
  // read through the records in the file
  
  while (!feof(_file)) {
    
    // read rec header - this is a 4-byte integer FORTRAN uses
    
    Radx::ui32 nbytes;
    if (fread(&nbytes, sizeof(Radx::ui32), 1, _file) != 1) {
      continue;
    }
    if (_ufIsSwapped) {
      ByteOrder::swap32(&nbytes, sizeof(Radx::ui32), true);
    }
    if (nbytes < 8) {
      break;
    }

    // reasonableness check

    if (nbytes > 10000000) {
      _addErrStr("ERROR - UfRadxFile::readFromPath");
      _addErrInt("  Bad record length: ", nbytes);
      _close();
      return -1;
    }

    // read data record

    Radx::ui08 *record = (Radx::ui08 *) buf.reserve(nbytes);
    if (fread(record, sizeof(Radx::ui08), nbytes, _file) != nbytes) {
      break;
    }
    
    // read record trailer - this is a 4-byte integer FORTRAN uses
    
    Radx::ui32 nbytesTrailer;
    if (fread(&nbytesTrailer, sizeof(Radx::ui32), 1, _file) != 1) {
      break;
    }
    if (_ufIsSwapped) {
      ByteOrder::swap32(&nbytesTrailer, sizeof(Radx::ui32), true);
    }
      
    if (nbytesTrailer != nbytes) {
      _addErrStr("ERROR - UfRadxFile::readFromPath");
      _addErrStr("  Header record len differs from trailer len");
      _addErrInt("  Header  len: ", nbytes);
      _addErrInt("  Trailer len: ", nbytesTrailer);
      _close();
      return -1;
    }

    // load up record object
    
    if (_disassembleReadRecord(record, nbytes)) {
      _addErrStr("ERROR - UfRadxFile::readFromPath");
      _addErrStr("  cannot load UF record from raw data");
      _close();
      return -1;
    }
    
    // debug print
    
    if (_verbose) {
      _printRecord(cerr, true, true);
    }

    // handle this record

    if (_handleReadRecord()) {
      _addErrStr("ERROR - UfRadxFile::readFromPath");
      _addErrStr("  Cannot handle UF record");
      _close();
      return -1;
    }
    
  } // while
  
  if (_debug) {
    cerr << "End of file" << endl;
  }
  
  _close();

  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }

  // set the packing from the rays

  _readVol->setPackingFromRays();

  // add to paths used on read

  _readPaths.push_back(path);

  // set format as read

  _fileFormat = FILE_FORMAT_UF;

  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int UfRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// open netcdf file for writing
// create error object so we can handle errors
// Returns 0 on success, -1 on failure

int UfRadxFile::_openWrite(const string &path) 
{
  
  _close();
  _file = fopen(path.c_str(), "w");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_openWrite");
    _addErrStr("  Cannot open file for writing, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void UfRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

///////////////////////////////////////////////
// disassemble a raw UF record into structs
// Returns 0 on success, -1 on failure

int UfRadxFile::_disassembleReadRecord(const void *buf, int nBytes)

{

  _clearUfStructs();

  if (_verbose) {
    cerr << "======================================" << endl;
    cerr << "  sizeof(UF_mandatory_header_t): "
         << sizeof(UfData::mandatory_header_t) << endl;
    cerr << "  sizeof(UfData::optional_header_t) : "
         << sizeof(UfData::optional_header_t) << endl;
    cerr << "  sizeof(UfData::field_info_t)      : "
         << sizeof(UfData::field_info_t) << endl;
    cerr << "  sizeof(UfData::data_header_t)     : "
         << sizeof(UfData::data_header_t) << endl;
    cerr << "  sizeof(UfData::field_header_t)    : "
         << sizeof(UfData::field_header_t) << endl;
    cerr << "======================================" << endl;
  }

  const Radx::si16 *record = (const Radx::si16 *) buf;
  int nShorts = nBytes / sizeof(Radx::si16);
  int nExpected = sizeof(UfData::mandatory_header_t) / sizeof(Radx::si16);

  // mandatory header

  if (_verbose) {
    cerr << "  -->> reading mandatory header, size: "
         << sizeof(UfData::mandatory_header_t) << endl;
    cerr << "    record min nbytes: " << nExpected * sizeof(Radx::si16) << endl;
  }

  if (nShorts < nExpected) {
    cerr << "ERROR - UfRadxFile::_disassembleRecord" << endl;
    cerr << "  Record too short, found nShorts, nBytes = " << nShorts
         << ", " << nShorts * sizeof(Radx::si16) << endl;
    cerr << "  Expecting mandatory header, nBytes: "
         << nExpected * sizeof(Radx::si16) << endl;
    return -1;
  }
  
  memcpy(&_manHdr, record, sizeof(UfData::mandatory_header_t));
  if (_ufIsSwapped) UfData::swap(_manHdr, true);
  if (_verbose) UfData::print(cerr, _manHdr);

  // optional header

  if (_manHdr.optional_header_pos != _manHdr.data_header_pos) {
    
    int oheader_offset = (_manHdr.optional_header_pos - 1);
    nExpected = oheader_offset +
      sizeof(UfData::optional_header_t) / sizeof(Radx::si16);
    
    if (nShorts >= nExpected) {
      memcpy(&_optHdr, record + oheader_offset, sizeof(_optHdr));
      if (_ufIsSwapped) UfData::swap(_optHdr, true);
      if (_verbose) UfData::print(cerr, _optHdr);
      _optFound = true;
    }

  }

  // data header

  int dheader_offset = (_manHdr.data_header_pos - 1);
  nExpected = dheader_offset +
    sizeof(UfData::data_header_t) / sizeof(Radx::si16);

  if (_verbose) {
    cerr << "  -->> reading data header, size: "
         << sizeof(UfData::data_header_t) << endl;
    cerr << "    data offset bytes: "
         << dheader_offset * sizeof(Radx::si16) << endl;
    cerr << "    record min nbytes: " 
         << nExpected * sizeof(Radx::si16) << endl;
  }

  if (nShorts < nExpected) {
    cerr << "ERROR - UfRadxFile::_disassembleRecord" << endl;
    cerr << "  Record too short, found nShorts, nBytes = " << nShorts
         << ", " << nShorts * sizeof(Radx::si16) << endl;
    cerr << "  Data offset: " << dheader_offset << endl;
    cerr << "  Expecting nBytes: "
         << nExpected * sizeof(Radx::si16) << endl;
    return -1;
  }

  memcpy(&_dataHdr, record + dheader_offset, sizeof(_dataHdr));
  if (_ufIsSwapped) UfData::swap(_dataHdr, true);
  if (_verbose) UfData::print(cerr, _dataHdr);

  // fields

  for (int ifield = 0; ifield < _dataHdr.num_ray_fields; ifield++) {

    // field info

    UfData::field_info_t fInfo;
    int info_offset = dheader_offset +
      (sizeof(UfData::data_header_t) +
       ifield * sizeof(UfData::field_info_t)) / sizeof(Radx::si16);
    int nExpected = info_offset +
      sizeof(UfData::field_info_t) / sizeof(Radx::si16);
    
    if (_verbose) {
      cerr << "  -->> reading field info, size: "
           << sizeof(UfData::field_info_t) << endl;
      cerr << "    info offset bytes: "
           << info_offset * sizeof(Radx::si16) << endl;
      cerr << "    record min nbytes: "
           << nExpected * sizeof(Radx::si16) << endl;
    }
    
    if (nShorts < nExpected) {
      cerr << "ERROR - UfRadxFile::_disassembleRecord" << endl;
      cerr << "  Record too short, found nShorts, nBytes = " << nShorts
           << ", " << nShorts * sizeof(Radx::si16) << endl;
      cerr << "  Data offset: " << dheader_offset << endl;
      cerr << "  Field number: " << ifield << endl;
      cerr << "  Field info offset: " << info_offset << endl;
      cerr << "  Expecting nBytes: "
           << nExpected * sizeof(Radx::si16) << endl;
      return -1;
    }
    
    memcpy(&fInfo, record + info_offset, sizeof(fInfo));
    if (_ufIsSwapped) UfData::swap(fInfo, true);
    if (_verbose) UfData::print(cerr, ifield, fInfo);

    _fieldInfo.push_back(fInfo);
    _fieldNames.push_back(UfData::label(fInfo.field_name, 2));

  }

  for (size_t ifield = 0; ifield < _fieldInfo.size(); ifield++) {

    // field header

    UfData::field_header_t fhdr;
    int fld_hdr_offset = (_fieldInfo[ifield].field_pos - 1);
    int nExpected = fld_hdr_offset +
      sizeof(UfData::field_header_t) / sizeof(Radx::si16);

    if (_verbose) {
      cerr << "  -->> reading field header, size: "
           << sizeof(UfData::field_header_t) << endl;
      cerr << "    field header offset bytes: "
           << fld_hdr_offset * sizeof(Radx::si16) << endl;
      cerr << "    record min nbytes: "
           << nExpected * sizeof(Radx::si16) << endl;
    }
    
    if (nShorts < nExpected) {
      cerr << "ERROR - UfRadxFile::_disassembleRecord" << endl;
      cerr << "  Record too short, found nShorts, nBytes = " << nShorts
           << ", " << nShorts * sizeof(Radx::si16) << endl;
      cerr << "  Field number: " << ifield << endl;
      cerr << "  Field header offset: " << fld_hdr_offset << endl;
      cerr << "  Expecting nBytes: "
           << nExpected * sizeof(Radx::si16) << endl;
      return -1;
    }

    memcpy(&fhdr, record + fld_hdr_offset, sizeof(fhdr));
    if (_ufIsSwapped) UfData::swap(fhdr, _fieldNames[ifield], true);
    _fieldHdrs.push_back(fhdr);

    if (_verbose) {
      char fieldName[32];
      memset(fieldName, 0, sizeof(fieldName));
      memcpy(fieldName, _fieldInfo[ifield].field_name, 2);
      UfData::print(cerr, fieldName, ifield, fhdr);
    }

    // field data

    int data_offset = fhdr.data_pos - 1;
    nExpected = data_offset + fhdr.num_volumes;

    if (_verbose) {
      cerr << "  -->> reading field data, len: "
           << fhdr.num_volumes * sizeof(Radx::si16) << endl;
      cerr << "    field data offset bytes: "
           << data_offset * sizeof(Radx::si16) << endl;
      cerr << "    record min nbytes: "
           << nExpected * sizeof(Radx::si16) << endl;
    }
    
    if ((int) nShorts < nExpected) {
      cerr << "ERROR - UfRadxFile::_disassembleRecord" << endl;
      cerr << "  Record too short, found nShorts, nBytes = " << nShorts
           << ", " << nShorts * sizeof(Radx::si16) << endl;
      cerr << "  Field number: " << ifield << endl;
      cerr << "  Field data offset: " << data_offset << endl;
      cerr << "  Expecting nBytes: "
           << nExpected * sizeof(Radx::si16) << endl;
      return -1;
    }

    RadxBuf *buf = new RadxBuf;
    buf->load(record + data_offset, fhdr.num_volumes * sizeof(Radx::si16));
    if (_ufIsSwapped) {
      ByteOrder::swap16(buf->getPtr(), buf->getLen(), true);
    }
    _fieldBuffers.push_back(buf);
    
  } // ifield

  return 0;

}

///////////////////////////////////////////////////////
// handle the incoming UF record

int UfRadxFile::_handleReadRecord()
  
{

  // ignore ray if no fields

  int nFields = _fieldInfo.size();
  if (nFields < 1) {
    if (_debug) {
      cerr << "WARNING - ray with no fields" << endl;
      cerr << "  el, az: "
           << _manHdr.azimuth / 64.0 << ", "
           << _manHdr.elevation / 64.0 << endl;
    }
    return 0;
  }

  // create new ray
  
  RadxRay *ray = new RadxRay;

  // populate ray with data

  _volumeNumber = _manHdr.volume_scan_num;
  ray->setVolumeNumber(_volumeNumber);
  ray->setSweepNumber(_manHdr.sweep_num - 1);
  ray->setCalibIndex(0);

  const UfData::field_header_t &fhdr0 = _fieldHdrs[0];
  int nGates = fhdr0.num_volumes;
  double startRangeKm = fhdr0.start_range + fhdr0.start_center / 1000.0;
  double gateSpacingKm = fhdr0.volume_spacing / 1000.0;
  ray->setRangeGeom(startRangeKm, gateSpacingKm);

  // sweep mode

  UfData::sweep_mode_t ufSweepMode = (UfData::sweep_mode_t) _manHdr.sweep_mode;

  switch (ufSweepMode) {
    case UfData::SWEEP_CALIBRATION:
      ray->setSweepMode(Radx::SWEEP_MODE_CALIBRATION);
      break;
    case UfData::SWEEP_PPI:
      ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      break;
    case UfData::SWEEP_COPLANE:
      ray->setSweepMode(Radx::SWEEP_MODE_COPLANE);
      break;
    case UfData::SWEEP_RHI:
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
      break;
    case UfData::SWEEP_VERTICAL:
      ray->setSweepMode(Radx::SWEEP_MODE_VERTICAL_POINTING);
      break;
    case UfData::SWEEP_MANUAL:
      ray->setSweepMode(Radx::SWEEP_MODE_MANUAL_PPI);
      break;
    case UfData::SWEEP_IDLE:
      ray->setSweepMode(Radx::SWEEP_MODE_IDLE);
      break;
    case UfData::SWEEP_SURVEILLANCE:
    default:
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  }

  // polarization mode

  UfData::polarization_mode_t ufPolarizationMode =
    (UfData::polarization_mode_t) fhdr0.polarization;

  switch (ufPolarizationMode) {
    case UfData::POLARIZATION_HORIZONTAL:
      ray->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
      break;
    case UfData::POLARIZATION_VERTICAL:
      ray->setPolarizationMode(Radx::POL_MODE_VERTICAL);
      break;
    case UfData::POLARIZATION_CIRCULAR:
      ray->setPolarizationMode(Radx::POL_MODE_CIRCULAR);
      break;
    case UfData::POLARIZATION_ELLIPTICAL:
      ray->setPolarizationMode(Radx::POL_MODE_CIRCULAR);
      break;
    default:
      ray->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  }

  // prf mode - assume fixed

  ray->setPrtMode(Radx::PRT_MODE_FIXED);

  // follow mode

  switch (ufSweepMode) {
    case UfData::SWEEP_TARGET:
      ray->setFollowMode(Radx::FOLLOW_MODE_TARGET);
      break;
    default:
      ray->setFollowMode(Radx::FOLLOW_MODE_NONE);
  }

  // set time

  int year = _manHdr.year;
  if (year < 1900) {
    if (year < 70) {
      year += 2000;
    } else {
      year += 1900;
    }
  }

  RadxTime rtime(year, _manHdr.month, _manHdr.day,
                 _manHdr.hour, _manHdr.minute, _manHdr.second);
  ray->setTime(rtime.utime(), 0);

  // position

  ray->setAzimuthDeg(_manHdr.azimuth / 64.0);
  ray->setElevationDeg(_manHdr.elevation / 64.0);
  ray->setFixedAngleDeg(_manHdr.fixed_angle / 64.0);
  if (_manHdr.sweep_rate == UfData::NO_DATA) {
    ray->setTrueScanRateDegPerSec(Radx::missingMetaDouble);
  } else {
    ray->setTrueScanRateDegPerSec(_manHdr.sweep_rate / 64.0);
  }
  ray->setAntennaTransition(0);
  ray->setNSamples(fhdr0.num_samples);

  // prt, nyquist etc

  _prtSec = fhdr0.pulse_rep_time / 1.0e6;
  _wavelengthM = fhdr0.wavelength / 6400.0;
  _nyquist = _wavelengthM / (4.0 * _prtSec);
  _unambigRange = (_prtSec * Radx::LIGHT_SPEED) / 2000.0;

  _beamWidthH = fhdr0.horiz_beam_width / 64.0;
  _beamWidthV = fhdr0.vert_beam_width / 64.0;
  _receiverBandWidth = fhdr0.receiver_bandwidth;
  _pulseWidth = fhdr0.pulse_duration / 64.0;

  _receiverGain = Radx::missingFl64;
  _peakPower = Radx::missingFl64;
  _antennaGain = Radx::missingFl64;
  _noisePower = Radx::missingFl64;
  _nyquist = Radx::missingFl64;
  _dbz0 = Radx::missingFl64;
  
  for (int ii = 0; ii < nFields; ii++) {
    const UfData::field_header_t &fhdr = _fieldHdrs[ii];
    double scale = fhdr.scale;
    if (scale <= 0) { // missing
      scale = 100.0;
    }
    if (_fieldNames[ii][0] == 'V') {
      if (fhdr.word20.nyquist_vel == UfData::NO_DATA) {
        _nyquist = Radx::missingMetaDouble;
      } else {
        _nyquist = fhdr.word20.nyquist_vel / scale;
      }
    } else {
      if (fhdr.word20.dbz0 == UfData::NO_DATA) {
        _dbz0 = Radx::missingMetaDouble;
      } else {
        _dbz0 = fhdr.word20.dbz0;
      }
      if (fhdr.receiver_gain == UfData::NO_DATA) {
        _receiverGain = Radx::missingMetaDouble;
      } else {
        _receiverGain = fhdr.receiver_gain / scale;
      }
      if (fhdr.peak_power == UfData::NO_DATA) {
        _peakPower = Radx::missingMetaDouble;
      } else {
        _peakPower = fhdr.peak_power / scale;
      }
      if (fhdr.antenna_gain == UfData::NO_DATA) {
        _antennaGain = Radx::missingMetaDouble;
      } else {
        _antennaGain = fhdr.antenna_gain / scale;
      }
      if (fhdr.word21.noise_power == UfData::NO_DATA) {
        _noisePower = Radx::missingMetaDouble;
      } else {
        _noisePower = fhdr.word21.noise_power;
      }
    }
  } // ii

  if (_dbz0 > -9990) {
    _radarConstant = _dbz0 - _noisePower;
  } else {
    _radarConstant = Radx::missingFl64;
  }

  ray->setPulseWidthUsec(_pulseWidth);
  ray->setPrtSec(_prtSec);
  ray->setNyquistMps(_nyquist);
  ray->setUnambigRangeKm(_unambigRange);

  // get location, store in arrays
  
  double latDeg = _manHdr.lat_degrees + _manHdr.lat_minutes / 60.0 +
    _manHdr.lat_seconds / (3600.0 * 64.0);
  double lonDeg = _manHdr.lon_degrees + _manHdr.lon_minutes / 60.0 +
    _manHdr.lon_seconds / (3600.0 * 64.0);
  double htKm = _manHdr.antenna_height / 1000.0;

  _latitudes.push_back(latDeg);
  _longitudes.push_back(lonDeg);
  _altitudes.push_back(htKm);

  _radarName = Radx::makeString(_manHdr.radar_name, 8);
  if (_radarName.size() < 1) {
    _radarName = "unknown";
  }
  _siteName = Radx::makeString(_manHdr.site_name, 8);
  _genFacility = Radx::makeString(_manHdr.gen_facility, 8);

  if (_optFound) {
    _projName = Radx::makeString(_optHdr.project_name, 8);
    _tapeName = Radx::makeString(_optHdr.tape_name, 8);
  }

  // add fields

  for (int ifield = 0; ifield < nFields; ifield++) {

    const UfData::field_header_t &fhdr = _fieldHdrs[ifield];

    // get field name

    string ufName = _fieldNames[ifield];
    if (ufName.size() < 1) {
      continue;
    }

    // infer the units and long name from the field name
    
    string shortName, longName, units;
    for (size_t ii = 0; ii < _nameTable.size(); ii++) {
      if (ufName == _nameTable[ii].ufName) {
        shortName = _nameTable[ii].shortName;
        longName = _nameTable[ii].longName;
        units = _nameTable[ii].units;
        break;
      }
    }

    // create new field

    string fieldName = ufName;
    if (_expandUfNames) {
      fieldName = shortName;
    }
    if (!isFieldRequiredOnRead(fieldName)) {
      continue;
    }

    if (_readFieldNames.size() > 0) {
      bool needField = false;
      for (int kk = 0; kk < (int) _readFieldNames.size(); kk++) {
        if (_readFieldNames[kk] == fieldName) {
          needField = true;
          break;
        }
      }
      if (!needField) {
        continue;
      }
    }


    RadxField *field = new RadxField(fieldName, units);
    field->copyRangeGeom(*ray);
    field->setLongName(longName);

    // set the data for this field

    const Radx::si16 *data =
      (const Radx::si16 *) _fieldBuffers[ifield]->getPtr();

    double scale = 1.0 / fhdr.scale_factor;
    double offset = 0.0;
    field->setTypeSi16(_manHdr.missing_data_val, scale, offset);
    field->setDataSi16(nGates, data, true);

    field->setThresholdFieldName(Radx::makeString(fhdr.threshold_field, 2));
    field->setThresholdValue(fhdr.threshold_val);

    // add to ray

    ray->addField(field);

  } // ifield

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

  return 0;

}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int UfRadxFile::_loadReadVolume()
  
{

  int nRays = _rays.size();
  if (nRays < 1) {
    if (_debug) {
      cerr << "WARNING - UfRadxFile::_loadReadVolume" << endl;
      cerr << "  No rays" << endl;
    }
    return -1;
  }

  _readVol->clear();

  // is this platform moving?
  
  bool isMoving = false;
  double lat0 = _latitudes[0];
  double lon0 = _longitudes[0];
  double alt0 = _altitudes[0];
  for (size_t ii = 0; ii < _latitudes.size(); ii++) {
    if (fabs(_latitudes[ii] - lat0) > 0.00001 ||
        fabs(_longitudes[ii] - lon0) > 0.00001 ||
        fabs(_altitudes[ii] - alt0) > 0.00001) {
      isMoving = true;
      break;
    }
  }
  
  // if moving, set the georeference information on each ray
  
  if (isMoving) {
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      RadxGeoref ref;
      ref.setLatitude(_latitudes[ii]);
      ref.setLongitude(_longitudes[ii]);
      ref.setAltitudeKmMsl(_altitudes[ii]);
      _rays[ii]->setGeoref(ref);
    }
  }

  // set meta-data
 
  _readVol->setOrigFormat("UF");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  if (isMoving) {
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
  } else {
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  }

  double freqHz = Radx::LIGHT_SPEED / _wavelengthM;
  _readVol->addFrequencyHz(freqHz);

  _readVol->setRadarAntennaGainDbH(_antennaGain);
  _readVol->setRadarAntennaGainDbV(_antennaGain);

  _readVol->setRadarBeamWidthDegH(_beamWidthH);
  _readVol->setRadarBeamWidthDegV(_beamWidthV);
  _readVol->setRadarReceiverBandwidthMhz(_receiverBandWidth);

  _readVol->setStartTime(_rays[0]->getTimeSecs(),
                         _rays[0]->getNanoSecs());
  _readVol->setEndTime(_rays[nRays-1]->getTimeSecs(),
                       _rays[nRays-1]->getNanoSecs());

  _readVol->setTitle(_projName);
  _readVol->setSource(_genFacility);
  _readVol->setReferences(_tapeName);
  size_t nameStart = _pathInUse.find_last_of(PATH_SEPARATOR);
  string fileName = _pathInUse;
  if (nameStart != string::npos) {
    fileName = _pathInUse.substr(nameStart + 1);
  }
  _readVol->setHistory(fileName);
  _readVol->setSiteName(_siteName);
  _readVol->setInstrumentName(_radarName);

  _readVol->setLatitudeDeg(lat0);
  _readVol->setLongitudeDeg(lon0);
  _readVol->setAltitudeKm(alt0);

  // get pulse width from first ray

  double pulseWidthUs = _rays[0]->getPulseWidthUsec();

  // add rays

  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  // memory allocation for rays has passed to _readVol,
  // so free up pointer array

  _rays.clear();
  
  // add calibration information
  
  RadxRcalib *cal = new RadxRcalib;
  cal->setPulseWidthUsec(pulseWidthUs);
  cal->setReceiverGainDbHc(_receiverGain);
  cal->setXmitPowerDbmH(_peakPower);
  cal->setXmitPowerDbmV(_peakPower);
  cal->setNoiseDbmHc(_noisePower);
  cal->setNoiseDbmVc(_noisePower);
  cal->setBaseDbz1kmHc(_dbz0);
  cal->setBaseDbz1kmVc(_dbz0);
  cal->setRadarConstantH(_radarConstant);
  cal->setRadarConstantV(_radarConstant);
  _readVol->addCalib(cal);

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - UfRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - UfRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();

  // load up volume info from rays

  _readVol->loadVolumeInfoFromRays();

  return 0;
  
}

////////////////////////////////////////////////////////////
// create default name table
//
// Table is ordered so that most likely or suitable entries
// are closer to the top

void UfRadxFile::_createDefaultNameTable()
  
{

  _nameTable.clear();

  _nameTable.push_back
    (UfData::NameEntry("SNR", "dB", "SN", "signal to noise ratio"));
  _nameTable.push_back
    (UfData::NameEntry("SNR", "dB", "SR", "signal to noise ratio"));
  
  _nameTable.push_back
    (UfData::NameEntry("NCP", "", "NC", "normalized coherent power"));
  _nameTable.push_back
    (UfData::NameEntry("NCP", "", "NP", "normalized coherent power"));
  
  _nameTable.push_back
    (UfData::NameEntry("DBZ", "dBZ", "DZ", "reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("DBZ", "dBZ", "DB", "reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("DBZ", "dBZ", "ZH", "reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("DBZ", "dBZ", "RE", "reflectivity"));

  _nameTable.push_back
    (UfData::NameEntry("REF", "dBZ", "DZ", "reflectivity"));
  
  _nameTable.push_back
    (UfData::NameEntry("CDBZ", "dBZ", "CD", "corrected reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("CDBZ", "dBZ", "CZ", "corrected reflectivity"));
  
  _nameTable.push_back
    (UfData::NameEntry("DBMHC", "dBm", "DM", "power H co-polar"));
  _nameTable.push_back
    (UfData::NameEntry("DBMVC", "dBm", "DY", "power V co-polar"));
  _nameTable.push_back
    (UfData::NameEntry("DBMHX", "dBm", "DW", "power H cross-polar"));
  _nameTable.push_back
    (UfData::NameEntry("DBMVX", "dBm", "DX", "power V cross-polar"));

  _nameTable.push_back
    (UfData::NameEntry("DBMH", "dBm", "DM", "power H channel"));
  _nameTable.push_back
    (UfData::NameEntry("DBMV", "dBm", "DY", "power V channel"));

  _nameTable.push_back
    (UfData::NameEntry("DBM", "dBm", "DM", "power"));

  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VR", "radial velocity"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VE", "radial velocity"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VL", "radial velocity long PRT"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VS", "radial velocity short PRT"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VG", "radial velocity combined"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VT", "radial velocity combined"));
  
  _nameTable.push_back
    (UfData::NameEntry("WIDTH", "m/s", "SW", "spectrum width"));

  _nameTable.push_back
    (UfData::NameEntry("ZDR", "dB", "ZD", "differential reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("ZDR", "dB", "DR", "differential reflectivity"));

  // linear depolarization ratio

  _nameTable.push_back
    (UfData::NameEntry("LDR", "dB", "LD",
                       "Linear depolarization ratio, h-tx, v-rx"));
  _nameTable.push_back
    (UfData::NameEntry("LDRH", "dB", "LH",
                       "Linear depolarization ratio, v-rx"));
  _nameTable.push_back
    (UfData::NameEntry("LDRV", "dB", "LV",
                       "Linear depolarization ratio, v-tx, h-rx"));

  // differential phase

  _nameTable.push_back
    (UfData::NameEntry("PHIDP", "deg", "PH", "differential phase"));
  _nameTable.push_back
    (UfData::NameEntry("PHIDP", "deg", "DP", "differential phase"));

  // correlation HH to VV

  _nameTable.push_back
    (UfData::NameEntry("RHOHV", "", "RH", "correlation H-to-V"));
  _nameTable.push_back
    (UfData::NameEntry("RHOHV", "", "RO", "correlation H-to-"));
  _nameTable.push_back
    (UfData::NameEntry("RHOHV", "", "RX", "correlation H-to-"));
  _nameTable.push_back
    (UfData::NameEntry("CDBZ", "dBZ", "CD", "corrected reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("CDBZ", "dBZ", "CZ", "corrected reflectivity"));
  
  _nameTable.push_back
    (UfData::NameEntry("DBMHC", "dBm", "DM", "power H co-polar"));
  _nameTable.push_back
    (UfData::NameEntry("DBMVC", "dBm", "DY", "power V co-polar"));
  _nameTable.push_back
    (UfData::NameEntry("DBMHX", "dBm", "DW", "power H cross-polar"));
  _nameTable.push_back
    (UfData::NameEntry("DBMVX", "dBm", "DX", "power V cross-polar"));

  _nameTable.push_back
    (UfData::NameEntry("DBMH", "dBm", "DM", "power H channel"));
  _nameTable.push_back
    (UfData::NameEntry("DBMV", "dBm", "DY", "power V channel"));

  _nameTable.push_back
    (UfData::NameEntry("DBM", "dBm", "DM", "power"));

  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VR", "radial velocity"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VE", "radial velocity"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VL", "radial velocity long PRT"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VS", "radial velocity short PRT"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VG", "radial velocity combined"));
  _nameTable.push_back
    (UfData::NameEntry("VEL", "m/s", "VT", "radial velocity combined"));
  
  _nameTable.push_back
    (UfData::NameEntry("WIDTH", "m/s", "SW", "spectrum width"));

  _nameTable.push_back
    (UfData::NameEntry("ZDR", "dB", "ZD", "differential reflectivity"));
  _nameTable.push_back
    (UfData::NameEntry("ZDR", "dB", "DR", "differential reflectivity"));

  // linear depolarization ratio

  _nameTable.push_back
    (UfData::NameEntry("LDR", "dB", "LD",
                       "Linear depolarization ratio, h-tx, v-rx"));
  _nameTable.push_back
    (UfData::NameEntry("LDRH", "dB", "LH",
                       "Linear depolarization ratio, v-rx"));
  _nameTable.push_back
    (UfData::NameEntry("LDRV", "dB", "LV",
                       "Linear depolarization ratio, v-tx, h-rx"));

  // differential phase

  _nameTable.push_back
    (UfData::NameEntry("PHIDP", "deg", "PH", "differential phase"));
  _nameTable.push_back
    (UfData::NameEntry("PHIDP", "deg", "DP", "differential phase"));

  // correlation HH to VV

  _nameTable.push_back
    (UfData::NameEntry("RHOHV", "", "RH", "correlation H-to-V"));
  _nameTable.push_back
    (UfData::NameEntry("RHOHV", "", "RO", "correlation H-to-"));
  _nameTable.push_back
    (UfData::NameEntry("RHOHV", "", "RX", "correlation H-to-"));

  // specific diff propagation phase - KDP

  _nameTable.push_back
    (UfData::NameEntry("KDP", "deg/km", "KD",
                       "differential propagation of phase"));
  _nameTable.push_back
    (UfData::NameEntry("KDP", "deg/km", "KP",
                       "differential propagation of phase"));

  // refractivity

  _nameTable.push_back
    (UfData::NameEntry("NIQ", "dB", "NI", "NIQ for refractivity"));
  _nameTable.push_back
    (UfData::NameEntry("AIQ", "deg", "AI", "AIQ for refractivity"));

  // magnitude/angle of cross correlation, HH and VH

  _nameTable.push_back
    (UfData::NameEntry("CH", "", "CH", "mag correlation HH to VH"));
  _nameTable.push_back
    (UfData::NameEntry("AH", "deg", "AH", "angle correlation HH to VH"));
  
  // magnitude/angle magnitude of cross correlation, VV and HV
  
  _nameTable.push_back
    (UfData::NameEntry("CV", "", "CV", "mag correlation VV to HV"));
  _nameTable.push_back
    (UfData::NameEntry("AV", "deg", "AV", "angle correlatoin VV to HV"));

}

//////////////////////////////////////////////////////
// Load up the object from a ray in a RadxVol object

int UfRadxFile::_loadWriteRecordFromRay(const RadxVol &vol, int rayNumber)

{
  
  _clearUfStructs();

  // convert ray data to floats
  
  const RadxRay &ray = *vol.getRays()[rayNumber];
  RadxRay fl32Ray(ray);
  fl32Ray.convertToFl32();

  // get basic parameters

  int volNum = fl32Ray.getVolumeNumber();
  int sweepNum = fl32Ray.getSweepNumber();
  const RadxSweep *sweep = vol.getSweepByNumber(sweepNum);
  
  int numFields = fl32Ray.getNFields();
  int numGates = fl32Ray.getNGates();
  
  double elevation = fl32Ray.getElevationDeg();
  double azimuth = fl32Ray.getAzimuthDeg();
  double fixedAngle = elevation;
  Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  // Radx::PolarizationMode_t polMode = Radx::POL_MODE_HORIZONTAL;
  if (sweep != NULL) {
    fixedAngle = sweep->getFixedAngleDeg();
    sweepMode = sweep->getSweepMode();
    // polMode = sweep->getPolarizationMode();
  }

  // Fill UfData::MandatoryHeader data

  memcpy(_manHdr.uf_string, "UF", 2);

  // Compute the UF record length --number of 16 bit words:
  // recLen = (sizeof the various headers)/ 2 + dataLen
  // (We divide by 2 since we are looking for length in terms of a 2 byte
  // unit.

  int nWordsManHdr = sizeof(UfData::mandatory_header_t) / sizeof(Radx::si16);
  int nWordsOptHdr = sizeof(UfData::optional_header_t) / sizeof(Radx::si16);
  int nWordsDataHdr =
    (sizeof(UfData::data_header_t) +
     numFields * sizeof(UfData::field_info_t)) / sizeof(Radx::si16);
  int nWordsFieldHdr = sizeof(UfData::field_header_t) / sizeof(Radx::si16);
  int nWordsData = numGates;

  // data header position relative to the start 
  // of the UF buffer of 16bit words or shorts.
  // Note that the first short is position 1.
  
  // The UF structure is : Mandatory Header  
  //                       OptLocal Header
  //                       Local Header
  //                       Data Header
  //                       n * (Field Name, 
  //                            Data Position)
  //                       Field 1 Header, data
  //                       Field 2 Header, data
  //                       Field 3 Header, data ...

  int optHdrPos = 1 + nWordsManHdr;
  int dataHdrPos = optHdrPos + nWordsOptHdr;

  // field headers and data pos

  int pos = dataHdrPos + nWordsDataHdr;
  vector<int> fieldHdrPos;
  vector<int> fieldDataPos;
  for (int ii = 0; ii < numFields; ii++) {
    fieldHdrPos.push_back(pos);
    pos += nWordsFieldHdr;
    fieldDataPos.push_back(pos);
    pos += nWordsData;
  }

  // total record length

  int recLen =
    nWordsManHdr + nWordsOptHdr + nWordsDataHdr +
    numFields * (nWordsFieldHdr + nWordsData);

  // check to make sure the rec length is small enough to fit
  // into 16-bit signed int

  if (recLen > 32767) {
    int maxNumFields =
      (32767 - nWordsManHdr - nWordsOptHdr - nWordsDataHdr) /
      (nWordsFieldHdr + nWordsData);
    _addErrStr("ERROR - UfRadxFile::_loadWriteRecordFromRay");
    _addErrStr("  Too many fields to fit into max rec len of 32767");
    _addErrInt("  Trying to write nfields: ", numFields);
    _addErrInt("  num gates: ", numGates);
    _addErrInt("  Max num fields for this number of gates: ", maxNumFields);
    _addErrStr("  Reduce the number of fields or number of gates");
    return -1;
  }

  _manHdr.record_length = recLen;
  
  // optional header position
  // Note that the first word is in postion 1.

  _manHdr.optional_header_pos = optHdrPos;
  
  // local header position: since we dont have one, this
  // will give the position of the first 16 bit word of UfData::data_header_t
  // in the uf buffer. Note that the first word is in postion 1.;

  _manHdr.local_use_header_pos = dataHdrPos;
  
  // data header position
  
  _manHdr.data_header_pos = dataHdrPos;
  _manHdr.record_num = rayNumber;
  _manHdr.volume_scan_num = volNum;
  _manHdr.ray_num = rayNumber;
  _manHdr.ray_record_num = 1;

  // sweep numbers are 1-based. The UF processing by RSL lib reqires it.

  _manHdr.sweep_num = sweepNum + 1;

  // Copy radarName and siteName

  strncpy(_manHdr.radar_name, vol.getInstrumentName().c_str(), 8);
  strncpy(_manHdr.site_name, vol.getSiteName().c_str(), 8);

  // record latitude:
  // check for location override,
  // convert to degress, minutes, seconds

  double latitude = vol.getLatitudeDeg();
  double longitude = vol.getLongitudeDeg();
  double altitudeKm = vol.getAltitudeKm();
  const RadxGeoref *georef = fl32Ray.getGeoreference();
  if (georef) {
    latitude = georef->getLatitude();
    longitude = georef->getLongitude();
    altitudeKm = georef->getAltitudeKmMsl();
  }
  
  // record latitude--convert to degress, minutes, seconds
  
  double latDeg = trunc(latitude);
  double latMin = trunc((latitude - latDeg) * 60.0);
  double latSec = (latitude - latDeg - latMin / 60.0) * 3600.0;

  _manHdr.lat_degrees = (Radx::si16) floor(latDeg + 0.5);
  _manHdr.lat_minutes = (Radx::si16) floor(latMin + 0.5);
  _manHdr.lat_seconds = (Radx::si16) floor(latSec * 64.0 + 0.5);

  // record longitude--convert to degress, minutes, seconds

  double lonDeg = trunc(longitude);
  double lonMin = trunc((longitude - lonDeg) * 60.0);
  double lonSec = (longitude - lonDeg - lonMin / 60.0) * 3600.0;

  _manHdr.lon_degrees = (Radx::si16) floor(lonDeg + 0.5);
  _manHdr.lon_minutes = (Radx::si16) floor(lonMin + 0.5);
  _manHdr.lon_seconds = (Radx::si16) floor(lonSec * 64.0 + 0.5);
  
  // antennaAlt in meters.
  
  _manHdr.antenna_height = (Radx::si16)(altitudeKm * 1000 + .5);
  
  // Fill year,month, day, hour, min, seconds
  // in the UfData::mandatory_header_t struct.
  // (year is just the last two digits of the year.)
  
  RadxTime beamTime(ray.getTimeSecs());
  _manHdr.year = beamTime.getYear() - (beamTime.getYear()/100)*100;
  _manHdr.day = beamTime.getDay();
  _manHdr.month = beamTime.getMonth();
  _manHdr.hour  = beamTime.getHour();
  _manHdr.minute = beamTime.getMin();
  _manHdr.second = beamTime.getSec();

  memcpy(_manHdr.time_zone, "UT", 2);

  // azimuth * 64
  
  _manHdr.azimuth = (Radx::si16)(azimuth * 64 + .5);
  
  // elevation * 64

  _manHdr.elevation = (Radx::si16)(elevation * 64 + .5 );

  // sweep mode

  switch (sweepMode) {
    case Radx::SWEEP_MODE_SECTOR:
      _manHdr.sweep_mode = UfData::SWEEP_PPI;
      break;
    case Radx::SWEEP_MODE_COPLANE:
      _manHdr.sweep_mode = UfData::SWEEP_COPLANE;
      break;
    case Radx::SWEEP_MODE_RHI:
      _manHdr.sweep_mode = UfData::SWEEP_RHI;
      break;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      _manHdr.sweep_mode = UfData::SWEEP_VERTICAL;
      break;
    case Radx::SWEEP_MODE_IDLE:
      _manHdr.sweep_mode = UfData::SWEEP_IDLE;
      break;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      _manHdr.sweep_mode = UfData::SWEEP_SURVEILLANCE;
      break;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      _manHdr.sweep_mode = UfData::SWEEP_RHI;
      break;
    case Radx::SWEEP_MODE_SUNSCAN:
      _manHdr.sweep_mode = UfData::SWEEP_CALIBRATION;
      break;
    case Radx::SWEEP_MODE_POINTING:
      _manHdr.sweep_mode = UfData::SWEEP_MANUAL;
      break;
    case Radx::SWEEP_MODE_CALIBRATION:
      _manHdr.sweep_mode = UfData::SWEEP_CALIBRATION;
      break;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      _manHdr.sweep_mode = UfData::SWEEP_PPI;
      break;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      _manHdr.sweep_mode = UfData::SWEEP_RHI;
      break;
    default: 
      _manHdr.sweep_mode = UfData::SWEEP_SURVEILLANCE;
  }

  // fixed_angle is the target elevation (X 64).

  _manHdr.fixed_angle = (Radx::si16)(fixedAngle * 64 + .5 );

  // sweepRate(degrees/sec  * 64) 

  if (ray.getTrueScanRateDegPerSec() < -9990) {
    _manHdr.sweep_rate = UfData::NO_DATA;
  } else {
    _manHdr.sweep_rate =
      (Radx::si16)(ray.getTrueScanRateDegPerSec() * 64 + 0.5);
  }

  // Find file generation time(UTC):
  
  RadxTime currentTime(time(0));
  currentTime.set( currentTime.utime());
  _manHdr.gen_year = currentTime.getYear() - (currentTime.getYear()/100)*100;
  _manHdr.gen_month = currentTime.getMonth();
  _manHdr.gen_day = currentTime.getDay();
  
  // Facility

  strncpy(_manHdr.gen_facility, vol.getSource().c_str(), 8);

  // missing data value

  _manHdr.missing_data_val = UfData::NO_DATA;
  
  // Fill optional header
  
  strncpy(_optHdr.project_name, vol.getTitle().c_str(), 8);
  strncpy(_optHdr.tape_name, vol.getReferences().c_str(), 8);
  RadxTime volStartTime(vol.getStartTimeSecs());
  _optHdr.baseline_azimuth = -99 * 64;
  _optHdr.baseline_elevation = -99 * 64;
  _optHdr.hour = volStartTime.getHour();
  _optHdr.minute = volStartTime.getMin();
  _optHdr.second = volStartTime.getSec();

  // Fill UfData::data_header_t

  _dataHdr.num_ray_fields = numFields;
  _dataHdr.num_ray_records = 1;
  _dataHdr.num_record_fields = numFields;
  
  // Fill in field info
  
  _fieldInfo.clear();

  vector<string> ufNames;
  
  for (int ifield = 0; ifield < numFields; ifield++) {
    
    UfData::field_info_t info;
    memset(&info, 0, sizeof(info));
    
    const RadxField &field = *ray.getFields()[ifield];
    string shortName = field.getName();
    string ufName = UfData::label(shortName.c_str(), 2);
    for (size_t ii = 0; ii < _nameTable.size(); ii++) {
      if (_nameTable[ii].shortName == shortName) {
        ufName = _nameTable[ii].ufName;
        break;
      }
    }
    memcpy(info.field_name, ufName.c_str(), 2);
    info.field_pos = fieldHdrPos[ifield];
    _fieldInfo.push_back(info);

    ufNames.push_back(ufName);

  }

  // Create field headers and data
  
  _fieldHdrs.clear();
  _fieldBuffers.clear();

  // Fill UfData::field_header_t's and convert float data to shorts with 
  // appropriate scale factor.

  for (int ifield = 0; ifield < numFields; ifield++) {
    
    UfData::field_header_t fhdr;
    fhdr.data_pos = fieldDataPos[ifield];
    RadxField *field = fl32Ray.getFields()[ifield];
    field->computeMinAndMax();
    
    // set the scale factor to 100 except for fields which need
    // more precision" RHOHV, ZDR and NCP

    string ufName = ufNames[ifield];
    if (ufName == "RH" || ufName == "RO" || ufName == "RX" ||
        ufName == "NP" || ufName == "NC" || ufName == "CP") {
      fhdr.scale_factor = 10000;
    } else if (ufName == "ZD" || ufName == "DR") {
      fhdr.scale_factor = 1000;
    } else {
      fhdr.scale_factor = 100;
    }
    
#ifdef NOT_NOW
    // Compute scaling factor such that
    //   shortValue/scaleFactor = meteorlogical units.
    // Thus we need shortVal = fabs(floatVal) * scaleFactor < 32768
    // We check scaleFactor = 32000/(max(fabs(floatVal)))
    
    double maxVal = fabs(field->getMaxValue());
    double minVal = fabs(field->getMinValue());
    if (minVal > maxVal) {
      maxVal = minVal;
    }
    double scaleFactor = 32000.0 / maxVal;
    if (scaleFactor < 1000) {
      fhdr.scale_factor = 100;
    } else if (scaleFactor < 10000) {
      fhdr.scale_factor = 1000;
    } else {
      fhdr.scale_factor = 10000;
    }
#endif
      
    // range to first gate (km)
    // radarParams.startRange has units of km.

    double startRange = ray.getStartRangeKm();
    fhdr.start_range = (Radx::si16) (startRange + 0.5);

    // adjustment to center of first gate, in meters

    fhdr.start_center = (Radx::si16)
      ((startRange - fhdr.start_range) * 1000.0 + 0.5);

    // gate spacing (m) 
    // radarParams.gateSpacing has units of km.

    double gateSpacing = ray.getGateSpacingKm();
    fhdr.volume_spacing = (Radx::si16) (gateSpacing * 1000.0  + 0.5);
    
    fhdr.num_volumes = ray.getNGates();
    fhdr.volume_depth = UfData::NO_DATA;

    // beam width * 64

    double horizBeamWidth = vol.getRadarBeamWidthDegH();
    fhdr.horiz_beam_width = (Radx::si16) (horizBeamWidth * 64.0 + 0.5);
    double vertBeamWidth = vol.getRadarBeamWidthDegV();
    fhdr.vert_beam_width = (Radx::si16) (vertBeamWidth * 64.0 + 0.5);

    fhdr.receiver_bandwidth = (short) vol.getRadarReceiverBandwidthMhz();
    
    UfData::polarization_mode_t pmode = UfData::POLARIZATION_HORIZONTAL;
    switch (sweep->getPolarizationMode()) {
      case Radx::POL_MODE_VERTICAL:
        pmode = UfData::POLARIZATION_VERTICAL;
        break;
      case Radx::POL_MODE_CIRCULAR:
        pmode = UfData::POLARIZATION_CIRCULAR;
        break;
      default:
        pmode = UfData::POLARIZATION_HORIZONTAL;
    }
    fhdr.polarization = pmode;

    // wavelength in cm. * 64 
    // radarParams.wavelength has units of cm.

    if (vol.getFrequencyHz().size() > 0) {
      double wavelengthM = Radx::LIGHT_SPEED / vol.getFrequencyHz()[0];
      double wavelengthCm = wavelengthM * 100.0;
      fhdr.wavelength = (Radx::si16) floor(wavelengthCm * 64.0 + 0.5);
    }

    fhdr.num_samples = ray.getNSamples();
    
    memcpy(fhdr.threshold_field, field->getThresholdFieldName().c_str(), 2);
    fhdr.threshold_val = (short) field->getThresholdValue();
    
    // What is this?

    memcpy(fhdr.edit_code, "NO", 2);
    
    // pulseRep in micro seconds.

    double prtSec = ray.getPrtSec();
    fhdr.pulse_rep_time = (Radx::si16) (prtSec * 1000000.0 + 0.5);

    fhdr.volume_bits = UfData::NO_DATA;
    
    bool isVel = false;
    if (_fieldInfo[ifield].field_name[0] == 'V') {
      isVel = true;
    }

    if (isVel) {

      // nyquist velocity

      double nyquist = ray.getNyquistMps();
      fhdr.scale = 100;
      fhdr.word20.nyquist_vel = (Radx::si16) (fhdr.scale * nyquist + 0.5);

      // flag is set to 1 if velocity is bad

      fhdr.word21.fl_string[0] = 0;
      fhdr.word21.fl_string[1] = 0;

    } else {
      
      // Scale is used to scale noiserPower, receiverGain, peakPower
      // and antennaGain from floats to shorts. So first find
      // the max of the absolute value of the four varibales.
      // Scale is such that short/scale = float.
      
      double antennaGain = vol.getRadarAntennaGainDbH();
      double peakPowerDbm = ray.getMeasXmitPowerDbmH();
      double receiverGain = Radx::missingMetaDouble;
      double dbz0 = Radx::missingMetaDouble;
      double noisePower = Radx::missingMetaDouble;
      double radarConstant = Radx::missingMetaDouble;
      const vector<RadxRcalib *> &cals = vol.getRcalibs();
      for (size_t ii = 0; ii < cals.size(); ii++) {
        const RadxRcalib &cal = *cals[ii];
        receiverGain = fabs(cal.getReceiverGainDbHc());
        dbz0 = cal.getBaseDbz1kmHc();
        noisePower = cal.getNoiseDbmHc();
        radarConstant = cal.getRadarConstantH();
        if (peakPowerDbm <= 0) {
          peakPowerDbm = cal.getXmitPowerDbmH();
        }
        if (antennaGain <= 0) {
          antennaGain = cal.getAntennaGainDbH();
        }
      }
      // if dbz0 is not available, compute it if possible

      if (dbz0 < -9990) {
        if (noisePower > -9990 && radarConstant > -9990) {
          dbz0 = noisePower + fabs(radarConstant);
        }
      } else if (radarConstant < -9990) {
        if (noisePower > -9990 && receiverGain > -9990) {
          radarConstant = dbz0 - noisePower;
        }
      }

#ifdef NOT_NOW      
      double maxx = fabs(antennaGain);
      if (maxx < fabs(peakPowerDbm)) {
        maxx = fabs(peakPowerDbm);
      }
      if (maxx < fabs(receiverGain)) {
        maxx = fabs(receiverGain);
      }
      if (maxx < fabs(noisePower)) {
        maxx = fabs(noisePower);
      }
      fhdr.scale = (Radx::si16) (32000.0 / maxx + 0.5);
#endif
      
      // set scale to 100 since many users seem to expect this
      
      fhdr.scale = 100;
      
      // dbz0 is such that dBZ = (dbz0 + data)/scale + 20 * log(range in km).
      
      fhdr.word20.dbz0 = (Radx::si16) dbz0;
      
      // nosiePower dB(mW)* scale
      
      if (noisePower< -9990) {
        fhdr.word21.noise_power = UfData::NO_DATA;
      } else {
        fhdr.word21.noise_power = (Radx::si16) (noisePower * fhdr.scale + 0.5);
      }
      
      // receiverGain(dB) * scale
      //  radarParams.receiverGain has units dB.
      
      if (receiverGain < -9990) {
        fhdr.receiver_gain = UfData::NO_DATA;
      } else {
        fhdr.receiver_gain = (Radx::si16)(receiverGain * fhdr.scale + 0.5);
      }

      // peakPower(dB(mW)) * scale

      if (peakPowerDbm < -9990) {
        fhdr.peak_power = UfData::NO_DATA;
      } else {
        fhdr.peak_power = (Radx::si16) (peakPowerDbm * fhdr.scale + 0.5);
      }
      
      // antennaGain(dB) * scale
      // radarParams.antennaGain has units dB.

      if (antennaGain < -9990) {
        fhdr.antenna_gain = UfData::NO_DATA;
      } else {
        fhdr.antenna_gain = (Radx::si16)(antennaGain * fhdr.scale + 0.5);
      }

    } // if (isVel)
    
    double pulseWidth = ray.getPulseWidthUsec();
    fhdr.pulse_duration = (Radx::si16)(pulseWidth * 64.0 + 0.5);
    
    // Convert float field data to short data.
    // Note we store short data field by field.
    // (ifield == field number)

    RadxBuf *outBuf = new RadxBuf;
    outBuf->reserve(numGates * sizeof(Radx::si16));
    Radx::si16 *outData = (Radx::si16 *) outBuf->getPtr();
    const Radx::fl32 *inData = field->getDataFl32();
    Radx::fl32 inMissing = field->getMissingFl32();
    for (int igate = 0; igate < numGates; igate++, inData++, outData++) {
      if (*inData == inMissing) {
        *outData = UfData::NO_DATA;
      } else {
        *outData = (Radx::si16) floor(*inData * fhdr.scale_factor + 0.5);
      }
    }
    
    _fieldHdrs.push_back(fhdr);
    _fieldBuffers.push_back(outBuf);
      
  } // ifield loop

  return 0;
  
}

//////////////////////////////////////////////////////////////
// Write record to open file
// Returns 0 on success, -1 on failure

int UfRadxFile::_writeRecord()
  
{

  // fortran record length

  Radx::si32 fortRecLen = _manHdr.record_length * sizeof(Radx::si16);
  if (!_writeNativeByteOrder) {
    ByteOrder::swap32(&fortRecLen, sizeof(fortRecLen));
  }

  // mandatory and data headers

  UfData::mandatory_header_t manHdrBE = _manHdr;
  if (!_writeNativeByteOrder) {
    UfData::swap(manHdrBE);
  }
  
  UfData::optional_header_t optHdrBE = _optHdr;
  if (!_writeNativeByteOrder) {
    UfData::swap(optHdrBE);
  }
  
  UfData::data_header_t dataHdrBE = _dataHdr;
  if (!_writeNativeByteOrder) {
    UfData::swap(dataHdrBE);
  }

  // load up field info
  
  RadxBuf fieldInfoBuf;
  
  for(int ifield = 0; ifield < (int) _fieldHdrs.size(); ifield++) {
    
    UfData::field_info_t infoBE = _fieldInfo[ifield];
    if (!_writeNativeByteOrder) {
      UfData::swap(infoBE);
    }
    
    fieldInfoBuf.add(&infoBE, sizeof(UfData::field_info_t));
    
  } // ifield

  // load up field headers and data
  
  RadxBuf fieldBuf;
  
  for(int ifield = 0; ifield < (int) _fieldHdrs.size(); ifield++) {

    string ufName = UfData::label(_fieldInfo[ifield].field_name, 2);
    
    // add header to field buffer
    
    UfData::field_header_t hdrBE = _fieldHdrs[ifield];
    if (!_writeNativeByteOrder) {
      UfData::swap(hdrBE, ufName);
    }
    
    fieldBuf.add(&hdrBE, sizeof(UfData::field_header_t));
    
    // data
    
    RadxBuf dataBuf;
    dataBuf.add(_fieldBuffers[ifield]->getPtr(),
                _fieldBuffers[ifield]->getLen());
    if (!_writeNativeByteOrder) {
      ByteOrder::swap16(dataBuf.getPtr(), dataBuf.getLen());
    }
    
    // add data to field buffer

    fieldBuf.add(dataBuf.getPtr(), dataBuf.getLen());

  } // ifield

  // Recall the UF structure for n fields: 
  //                          FORTRAN record len
  //                          Mandatory Header  
  //                          Optional Header
  //                          Local Header
  //                          Data Header
  //                          n * (Field Name, Data Position)
  //                          n * (Field Headers
  //                               data)
  //                          FORTRAN record len

  if (fwrite(&fortRecLen, sizeof(Radx::si32), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing leading FORTRAN record length");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (fwrite(&manHdrBE, sizeof(manHdrBE), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing mandatory header");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (fwrite(&optHdrBE, sizeof(optHdrBE), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing optional header");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (fwrite(&dataHdrBE, sizeof(dataHdrBE),  1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing data header");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (fwrite(fieldInfoBuf.getPtr(), 1, fieldInfoBuf.getLen(), _file) !=
      fieldInfoBuf.getLen()) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing field info");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (fwrite(fieldBuf.getPtr(), 1, fieldBuf.getLen(), _file) !=
      fieldBuf.getLen()) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing fields");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (fwrite(&fortRecLen, sizeof(Radx::si32), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - UfRadxFile::_writeRecord");
    _addErrStr("  Writing trailing FORTRAN record length");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// print data after read

void UfRadxFile::print(ostream &out) const
  
{
  
  out << "=============== UfRadxFile ===============" << endl;
  RadxFile::print(out);

  out << "  ufIsSwapped: " << _ufIsSwapped << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  radarName: " << _radarName << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  genFacility: " << _genFacility << endl;
  if (_latitudes.size() > 0) {
    out << "  latitudes: " << _latitudes[0] << endl;
    out << "  longitudes: " << _longitudes[0] << endl;
    out << "  altitudes: " << _altitudes[0] << endl;
  }
  out << "  wavelengthM: " << _wavelengthM << endl;
  out << "  beamWidthH: " << _beamWidthH << endl;
  out << "  beamWidthV: " << _beamWidthV << endl;
  out << "  receiverBandWidth: " << _receiverBandWidth << endl;
  out << "  receiverGain: " << _receiverGain << endl;
  out << "  dbz0: " << _dbz0 << endl;
  out << "  noisePower: " << _noisePower << endl;
  out << "  peakPower: " << _peakPower << endl;
  out << "  antennaGain: " << _antennaGain << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int UfRadxFile::printNative(const string &path, ostream &out,
                            bool printRays, bool printData)
  
{

  clear();
  RadxBuf buf;

  // is this a Uf file?
  
  if (!isUf(path)) {
    _addErrStr("ERROR - UfRadxFile::printNative");
    _addErrStr("  Not a uf file: ", path);
    return -1;
  }

  // check if data in this file is swapped - this sets _ufIsSwapped
  
  if (checkIsSwapped(path)) {
    _addErrStr("ERROR - UfRadxFile::printNative");
    _addErrStr("  Cannot check if swapped: ", path);
    return -1;
  }

  if (isSwapped()) {
    out << " file is byte-swapped" << endl;
  }

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - UfRadxFile::printNative");
    return -1;
  }
  
  // read through the records in the file
  
  while (!feof(_file)) {

    // read rec header - this is a 4-byte integer FORTRAN uses
    
    Radx::ui32 nbytes;
    if (fread(&nbytes, sizeof(Radx::ui32), 1, _file) != 1) {
      continue;
    }
    if (_ufIsSwapped) {
      ByteOrder::swap32(&nbytes, sizeof(Radx::ui32), true);
    }
    if (nbytes < 8) {
      // done
      break;
    }

    // reasonableness check

    if (nbytes > 10000000) {
      _addErrStr("ERROR - UfRadxFile::printNative");
      _addErrInt("  Bad record length: ", nbytes);
      _close();
      return -1;
    }

    // read data record

    Radx::ui08 *record = (Radx::ui08 *) buf.reserve(nbytes);
    if (fread(record, sizeof(Radx::ui08), nbytes, _file) != nbytes) {
      break;
    }
    
    // read record trailer - this is a 4-byte integer FORTRAN uses
    
    Radx::ui32 nbytesTrailer;
    if (fread(&nbytesTrailer, sizeof(Radx::ui32), 1, _file) != 1) {
      break;
    }
    if (_ufIsSwapped) {
      ByteOrder::swap32(&nbytesTrailer, sizeof(Radx::ui32), true);
    }
      
    if (nbytesTrailer != nbytes) {
      _addErrStr("ERROR - UfRadxFile::printNative");
      _addErrStr("  Header record len differs from trailer len");
      _addErrInt("  Header  len: ", nbytes);
      _addErrInt("  Trailer len: ", nbytesTrailer);
      _close();
      return -1;
    }

    // load up uf structs
    
    if (_disassembleReadRecord(record, nbytes)) {
      _addErrStr("ERROR - UfRadxFile::printNative");
      _addErrStr("  cannot load UF record from raw data");
      _close();
      return -1;
    }
    
    // print
    
    _printRecord(out, true, printData);

  } // while
  
  _close();

  return 0;

}

/////////////////////
// print UF record
//

void UfRadxFile::_printRecord(ostream &out,
                              bool print_headers,
                              bool print_data)
  
{

  if (print_headers) {

    UfData::print(out, _manHdr);
    UfData::print(out, _optHdr);
    UfData::print(out, _dataHdr);
    
  }
  
  for (size_t ii = 0; ii < _fieldInfo.size(); ii++) {
    if (print_headers) {
      UfData::print(out, (int) ii, _fieldInfo[ii]);
      UfData::print(out, _fieldNames[ii], (int) ii, _fieldHdrs[ii]);
    }
    if (print_data) {
      UfData::printFieldData(out, _fieldNames[ii], (int) ii,
                             _fieldHdrs[ii].num_volumes,
                             _fieldHdrs[ii].scale_factor,
                             _manHdr.missing_data_val,
                             (Radx::si16 *) _fieldBuffers[ii]->getPtr());
    }
  } // ii

}

