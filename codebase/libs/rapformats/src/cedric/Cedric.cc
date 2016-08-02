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

/*********************************************************************
 * Cedric.cc
 *
 * Object represting a single UF record
 *
 *********************************************************************/
 
#include <didss/LdataInfo.hh>
#include <rapformats/Cedric.hh>
#include <rapformats/DsRadarMsg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/TaFile.hh>
#include <toolsa/Path.hh>
#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <cerrno>
using namespace std;

///////////////
// constructor

Cedric::Cedric()

{
  MEM_zero(_fieldData);
  _debug = false;
  clear();
}

///////////////
// destructor

Cedric::~Cedric()
  
{
  clear();
}

///////////////////////
// clear 

void Cedric::clear()

{
  
  MEM_zero(_fileHdr);
  MEM_zero(_volHdr);
  _levelHdrs.clear();
  _zArray.clear();
  _zScale = 1000.0; // default is meters - CARTESIAN, is 100.0 for PPI
  _dzScale = 1000.0; // meters, or thousand's of a degree
  _needsSwapOnRead = false;

  setBitsDatum(16);
  setBlockingMode(2);
  setBlockSize(3200);
  setMissingDataVal(-32768);
  setScaleFactor(100);
  setAngleFactor(64);
  setIndexNumberRange(1);
  setIndexNumberAzimuth(2);
  setIndexNumberCoplane(3);
  setId("NONE");
  setProgram("NONE");
  setProject("NONE");
  setScientist("NONE");
  setRadar("NONE");
  setCoordType("CRT ");
  setTape("NONE");
  setHeaderRecordLength(sizeof(_volHdr) / sizeof(si16));

  for (int ii = 0; ii < CED_MAX_FIELDS; ii++) {
    if (_fieldData[ii] != NULL) {
      delete[] _fieldData[ii];
      _fieldData[ii] = NULL;
    }
  }

  // always need 1st landmark at ORIGIN

  addLandmark("ORIGIN", 0.0, 0.0, 0.0);

}

//////////////////////////////////////////////////////////////
// Read a cedric file, load up this object
// Returns 0 on success, -1 on failure

int Cedric::readFromPath(const string &path)
  
{

  _pathInUse = path;

  // clear all data
  
  clear();

  // open file
  
  TaFile inFile; // file closes automatically on destruct
  FILE *in;
  if ((in = inFile.fopenUncompress(path.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Cedric::readFromPath" << endl;
    cerr << "  Cannot read path: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read file header

  if((fread(&_fileHdr, sizeof(_fileHdr), 1, in) != 1)) {
    int errNum = errno;
    cerr << "ERROR - Cedric::readFromPath" << endl;
    cerr << "  Cannot read file header, path: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  _swapWords(_fileHdr.vol_label, 25 * 56);

  // check file header
  
  if (strncmp(_fileHdr.id, "CED1", 4) != 0 &&
      strncmp(_fileHdr.id, "D1CE", 4) != 0) {
    cerr << "ERROR - Cedric::readFromPath" << endl;
    cerr << "  File header does not start with 'CED1'" << endl;
    cerr << "  Not a CEDRIC file" << endl;
    return -1;
  }

  // determine whether we need to swap on read

  _needsSwapOnRead = false;
  if (_fileHdr.byte_order == 0) {
    // file is big-endian
    if (!BE_is_big_endian()) {
      _needsSwapOnRead = true;
    }
  } else {
    // file is little-endian
    if (BE_is_big_endian()) {
      _needsSwapOnRead = true;
    }
  }

  if (_needsSwapOnRead) {
    _swap(_fileHdr);
  }
  
  // read volume header
  
  if((fread(&_volHdr, sizeof(_volHdr), 1, in) != 1)) {
    int errNum = errno;
    cerr << "ERROR - Cedric::readFromPath" << endl;
    cerr << "  Cannot read volume header, path: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  _swapWords(&_volHdr, sizeof(_volHdr));

  if (_needsSwapOnRead) {
    _swap(_volHdr);
  }

  // set scaling factor for min and max z

  _setZScale();

  // allocate space for field data
  
  int bytesPerField = _volHdr.ny * _volHdr.nx * _volHdr.nz * sizeof(si16);
  for (int ifield = 0; ifield < _volHdr.num_fields; ifield++) {
    _fieldData[ifield] = new si16[bytesPerField];
  }

  // read in each level, which in turn reads in the fields

  for (int iz = 0; iz < _volHdr.nz; iz++) {
    if (_readLevel(in, iz)) {
      cerr << "ERROR - Cedric::readFromPath" << endl;
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////
// Read header and data for a level
// Returns 0 on success, -1 on failure

int Cedric::_readLevel(FILE *in, int levelNum)

{

  // read in level header

  CED_level_head_t levelHdr;
  if((fread(&levelHdr, sizeof(levelHdr), 1, in) != 1)) {
    int errNum = errno;
    cerr << "ERROR - Cedric::readFromPath" << endl;
    cerr << "  Cannot read level header, path: " << _pathInUse << endl;
    cerr << "  Level num: " << levelNum << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_needsSwapOnRead) {
    _swap(levelHdr);
  }
  _swapWords(&levelHdr, sizeof(levelHdr));

  _levelHdrs.push_back(levelHdr);
  _zArray.push_back(levelHdr.coord);

  // read in data for each plane

  int pointsPerPlane = _volHdr.ny * _volHdr.nx;
  int bytesPerPlane = pointsPerPlane * sizeof(si16);

  for (int ifield = 0; ifield < _volHdr.num_fields; ifield++) {
    si16 *plane = _fieldData[ifield] + levelNum * pointsPerPlane;
    int nread = fread(plane, 1, bytesPerPlane, in);
    if (nread != bytesPerPlane) {
      int errNum = errno;
      cerr << "ERROR - Cedric::_readLevel" << endl;
      cerr << "  Cannot read plane data, nbytes: " << bytesPerPlane << endl;
      cerr << "  File path: " << _pathInUse << endl;
      cerr << "  Field num: " << ifield << endl;
      cerr << "  Plane num: " << levelNum << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    _swapWords(plane, bytesPerPlane);
    if (_needsSwapOnRead) {
      SWAP_array_16(plane, bytesPerPlane);
    }
  } // ifield

  return 0;
  
}

//////////////////////////////////////////////////////////////
// Write file to specified dir
// Returns 0 on success, -1 on failure

int Cedric::writeToDir(const string &dir,
                       const string &appName,
                       const string &volLabel)
  
{

  if (_debug) {
    cerr << "Writing to dir: " << dir << endl;
  }

  DateTime startTime(getVolStartTime());
  DateTime endTime(getVolEndTime());

  DateTime fileTime = startTime;

  char dayStr[BUFSIZ];
  sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_DELIM,
          fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
  string outDir(dir);
  outDir += dayStr;

  // compute path
  
  string scanType = "SUR";

  char fileName[BUFSIZ];
  sprintf(fileName,
          "%s_%.4d%.2d%.2d_%.2d%.2d%.2d"
          "_to_%.4d%.2d%.2d_%.2d%.2d%.2d"
          "_%s_%s.ced",
          volLabel.c_str(),
          startTime.getYear(), startTime.getMonth(), startTime.getDay(),
          startTime.getHour(), startTime.getMin(), startTime.getSec(),
          endTime.getYear(), endTime.getMonth(), endTime.getDay(),
          endTime.getHour(), endTime.getMin(), endTime.getSec(),
          getRadar().c_str(), getCoordType().c_str());
  
  char outPath[BUFSIZ];
  sprintf(outPath, "%s%s%s",
          outDir.c_str(), PATH_DELIM, fileName);
  
  if (writeToPath(outPath, volLabel)) {
    cerr << "ERROR - Cedric::writeToPath()" << endl;
  }

  // write latest data info file
  
  LdataInfo ldata(dir);
  if (_debug) {
    ldata.setDebug(true);
  }
  string relPath;
  Path::stripDir(dir, outPath, relPath);
  Path rpath(relPath);
  ldata.setDataFileExt(rpath.getExt());
  ldata.setRelDataPath(relPath);
  ldata.setWriter(appName);
  if (ldata.write(getVolStartTime())) {
    cerr << "WARNING - Cedric::writeToDir" << endl;
    cerr << "  Cannot write latest data info file to dir: "
         << dir << endl;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// Write file to specified path
// Returns 0 on success, -1 on failure

int Cedric::writeToPath(const string &path,
                        const string &volLabel)
  
{

  if (_debug) {
    cerr << "  vol label: " << volLabel << endl;
  }

  _pathInUse = path;
  
  // create tmp dir

  Path outPath(path);
  Path tmpDir(outPath.getDirectory() + PATH_DELIM + "_tmp");
  Path tmpPath(tmpDir.getPath() + PATH_DELIM + outPath.getFile());
  if (ta_makedir_recurse(tmpPath.getDirectory().c_str())) {
    int errNum = errno;
    cerr << "ERROR - Cedric::writeToPath" << endl;
    cerr << "  Cannot make tmp dir: " << tmpPath.getDirectory() << endl;
    cerr << "  Output path: " << _pathInUse << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // open the file

  if (_debug) {
    cerr << "  writing to tmp path: " << tmpPath.getPath() << endl;
  }

  TaFile outFile; // file closes automatically on destruct
  FILE *out;
  if ((out = outFile.fopen(tmpPath.getPath().c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Cedric::writeToPath" << endl;
    cerr << "  Cannot open file, path: " << tmpPath.getPath() << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // set the file header

  int pointsPerPlane = _volHdr.ny * _volHdr.nx;
  int bytesPerPlane = pointsPerPlane * sizeof(si16);
  int bytesPerField = bytesPerPlane * _volHdr.nz;
  int fileSize = sizeof(_fileHdr) + sizeof(_volHdr) + _volHdr.num_fields * bytesPerField;

  MEM_zero(_fileHdr);
  memcpy(_fileHdr.id, "CED1", 4);
  if (!BE_is_big_endian()) {
    _fileHdr.byte_order = 1;
  }
  _fileHdr.file_size = fileSize;
  _fileHdr.vol_index[0] = sizeof(_fileHdr);
  STRncopy(_fileHdr.vol_label[0], volLabel.c_str(), 56);

  // write the file header
  // must make copy and swap words before write

  CED_file_head_t fileHdr = _fileHdr;
  _swapWords(fileHdr.vol_label[0], sizeof(fileHdr.vol_label[0]));

  if (fwrite(&fileHdr, sizeof(fileHdr), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - Cedric::writeToPath" << endl;
    cerr << "  Cannot write file header, path: " << _pathInUse << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write the volume header
  
  CED_vol_head_t volHdr = _volHdr;
  _swapWords(&volHdr, sizeof(volHdr));

  if (fwrite(&volHdr, sizeof(volHdr), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - Cedric::writeToPath" << endl;
    cerr << "  Cannot write vol header, path: " << _pathInUse << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write the planes

  TaArray<si16> planeCopy_;
  si16 *planeCopy = planeCopy_.alloc(pointsPerPlane);
  for (int iz = 0; iz < _volHdr.nz; iz++ /* , z += _volHdr.dz*/) {

    // write the level header
    
    CED_level_head_t levelCopy = _levelHdrs[iz];
    _swapWords(&levelCopy, sizeof(levelCopy));
    
    if (fwrite(&levelCopy, sizeof(levelCopy), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - Cedric::writeToPath" << endl;
      cerr << "  Cannot write level header, path: " << _pathInUse << endl;
      cerr << "  Level num: " << iz << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    for (int ifield = 0; ifield < _volHdr.num_fields; ifield++) {
      
      const si16 *ptr = _fieldData[ifield] + iz * pointsPerPlane;
      memcpy(planeCopy, ptr, bytesPerPlane);
      _swapWords(planeCopy, bytesPerPlane);
      if ((int) fwrite(planeCopy, 1, bytesPerPlane, out) != bytesPerPlane) {
        int errNum = errno;
        cerr << "ERROR - Cedric::writeToPath" << endl;
        cerr << "  Cannot write level data, path: " << _pathInUse << endl;
        cerr << "  Level num: " << iz << endl;
        cerr << "  Field num: " << ifield << endl;
        cerr << "  " << strerror(errNum) << endl;
        return -1;
      }

    } // ifield

  } // iz

  outFile.fclose();

  if (_debug) {
    cerr << "  Renaming tmp file to path: " << path << endl;
  }

  // rename the tmp to final output file path

  if (rename(tmpPath.getPath().c_str(), path.c_str())) {
    int errNum = errno;
    cerr << "ERROR - Cedric::writeToPath" << endl;
    cerr << "  Cannot rename tmp path to final path" << endl;
    cerr << "  tmp path: " << tmpPath.getPath() << endl;
    cerr << "  final path: " << path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
////////////////////////////////////////////

//////////////////////////////
// add a vertical level

void Cedric::addVlevel(int num, double val,
                       int nfields,
                       int nx, int ny,
                       double nyquist)
{

  if (nfields > CED_MAX_FIELDS) {
    nfields = CED_MAX_FIELDS;
  }

  CED_level_head_t hdr;
  memcpy(hdr.id, "LEVEL ", 6);
  hdr.coord = (si16) floor(val * 1000.0 + 0.5);
  hdr.level_number = num + 1;
  hdr.number_fields = nfields;
  hdr.points_per_plane = nx * ny;
  hdr.records_per_field = (nx * ny * sizeof(si16)) / getBlockSize() + 1;
  hdr.records_per_plane = nfields * hdr.records_per_field;
  hdr.nyquist_vel = (int) floor(nyquist * _volHdr.scale_factor + 0.5);

  setPointsPlane(hdr.points_per_plane);
  setRecordsPlane(hdr.records_per_plane);
  setRecordsField(hdr.records_per_field);

  _levelHdrs.push_back(hdr);
  _zArray.push_back(val);

  setRecordsVolume(hdr.records_per_plane * _levelHdrs.size());
  setTotRecords(getRecordsVolume());

  setNumPlanes(_levelHdrs.size());
  setNz(_levelHdrs.size());
  // setTotalPoints(_levelHdrs.size() * nx * ny);

}

//////////////////////////////
// add a field given floats
// returns 0 on success, -1 on failure

int Cedric::addField(const string &name, const fl32 *data, fl32 missingVal)
{

  if (_volHdr.num_fields >= CED_MAX_FIELDS) {
    cerr << "ERROR - Cedric::addField" << endl;
    cerr << "  Cannot add field, name: " << name << endl;
    cerr << "  Too many fields, max: " << CED_MAX_FIELDS << endl;
    return - 1;
  }
  int fieldNum = _volHdr.num_fields;
  _volHdr.num_fields++;
  int npts = _volHdr.nz * _volHdr.ny * _volHdr.nx;

  // compute max absolute value

  double maxAbsVal = 0;
  for (int ii = 0; ii < npts; ii++) {
    if (data[ii] != missingVal) {
      double absVal = fabs(data[ii]);
      if (absVal > maxAbsVal) maxAbsVal = absVal;
    }
  }
  
  // compute scale using offset of 0
  
  int scale = 100;
  if (maxAbsVal > 0) {
    scale = (int) (32760.0 / maxAbsVal) + 1;
    if (scale > 100) {
      scale = 100;
    }
  }

  // compute si16 values

  si16 *si16Data = new si16[npts];
  for (int ii = 0; ii < npts; ii++) {
    if (data[ii] == missingVal) {
      si16Data[ii] = -32768;
    } else {
      int ival = (int) (data[ii] * scale + 0.5);
      if (ival < -32767) {
        ival = -32767;
      } else if (ival > 32767) {
        ival = 32767;
      }
      si16Data[ii] = (si16) ival;
    }
  }

  _setFieldName(fieldNum, name);
  _setFieldData(fieldNum, si16Data, scale);

  delete[] si16Data;

  if (_debug) {
    cerr << "Adding field: " << name << endl;
  }
  
  return 0;

}

// set field details

void Cedric::_setFieldName(int fieldNum, const string &name)
{
  if (fieldNum >= CED_MAX_FIELDS) {
    return;
  }
  _setString(name, _volHdr.field[fieldNum].field_name, 8);
}

//////////////////////////////
// set field data given shorts

void Cedric::_setFieldData(int fieldNum, const si16 *data, int scale_factor)
{
  if (fieldNum >= CED_MAX_FIELDS) {
    return;
  }
  _volHdr.field[fieldNum].field_sf = scale_factor;
  int npts = _volHdr.nz * _volHdr.ny * _volHdr.nx;
  if (_fieldData[fieldNum] != NULL) {
    delete[] _fieldData[fieldNum];
  }
  _fieldData[fieldNum] = new si16[npts];
  memcpy(_fieldData[fieldNum], data, npts * sizeof(si16));
}

//////////////////////////////
// set field data given floats

void Cedric::_setFieldData(int fieldNum, const fl32 *data, fl32 missingVal)
{

  if (fieldNum >= CED_MAX_FIELDS) {
    return;
  }

  int npts = _volHdr.nz * _volHdr.ny * _volHdr.nx;

  // compute max absolute value

  double maxAbsVal = -1.0e99;
  for (int ii = 0; ii < npts; ii++) {
    double absVal = fabs(data[ii]);
    if (absVal > maxAbsVal) maxAbsVal = absVal;
  }

  // compute scale using offset of 0
  
  int scale = (int) (32760.0 / maxAbsVal) + 1;

  // compute si16 values

  si16 *si16Data = new si16[npts];
  for (int ii = 0; ii < npts; ii++) {
    if (data[ii] == missingVal) {
      si16Data[ii] = -32768;
    } else {
      int ival = (int) (data[ii] * scale + 0.5);
      if (ival < -32767) {
        ival = -32767;
      } else if (ival > 32767) {
        ival = 32767;
      }
      si16Data[ii] = (si16) ival;
    }
  }

  _setFieldData(fieldNum, si16Data, scale);

  delete[] si16Data;
}

//////////////////////////////
// add a landmark
// returns 0 on success, -1 on failure

int Cedric::addLandmark(const std::string &name,
                        double xpos, double ypos, double zpos)
{
  
  if (_volHdr.num_landmarks >= CED_MAX_LANDMARKS) {
    cerr << "ERROR - Cedric::addLandmark" << endl;
    cerr << "  Cannot add landmark, name: " << name << endl;
    cerr << "  Too many landmarks, max: " << CED_MAX_LANDMARKS << endl;
    return - 1;
  }
  int landmarkNum = _volHdr.num_landmarks;
  _volHdr.num_landmarks++;
  
  _setLandmarkName(landmarkNum, name);
  _setLandmarkXpos(landmarkNum, xpos);
  _setLandmarkYpos(landmarkNum, ypos);
  _setLandmarkZpos(landmarkNum, zpos);

  return 0;

}

///////////////////////
// Set landmark details

void Cedric::_setLandmarkName(int landmarkNum, const string &name)
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return;
  }
  _setString(name, _volHdr.landmark[landmarkNum].name, 6);
}

void Cedric::_setLandmarkXpos(int landmarkNum, double val)
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return;
  }
  _setDouble(val, _volHdr.landmark[landmarkNum].x_position);
}

void Cedric::_setLandmarkYpos(int landmarkNum, double val)
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return;
  }
  _setDouble(val, _volHdr.landmark[landmarkNum].y_position);
}

void Cedric::_setLandmarkZpos(int landmarkNum, double val)
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return;
  }
  _setDouble(val, _volHdr.landmark[landmarkNum].z_position);
}

////////////////////////////////////////////
////////////////////////////////////////////
// get methods

// get field name from field number

string Cedric::getFieldName(int fieldNum) const
{
  if (fieldNum >= CED_MAX_FIELDS) {
    return "InvalidField";
  }
  return _getString(_volHdr.field[fieldNum].field_name, 8);
}

// look up field number from the name
// return -1 if not found

int Cedric::getFieldNum(const string &name) const
{
  for (int ii = 0; ii < _volHdr.num_fields; ii++) {
    string cedName = getFieldName(ii);
    if (name == cedName) {
      return ii;
    }
  }
  return -1;
}

double Cedric::getFieldScaleFactor(int fieldNum) const
{
  if (fieldNum >= CED_MAX_FIELDS) {
    return 1.0;
  }
  return (double) _volHdr.field[fieldNum].field_sf;
}

// get field data in place, as si16
// memory remains owned by this object

const si16 *Cedric::getFieldData(int fieldNum) const
{
  if (fieldNum >= CED_MAX_FIELDS) {
    return NULL;
  }
  return  _fieldData[fieldNum];
}

// get field data as fl32 
// memory is allocated
// caller must delete[] the returned array

fl32 *Cedric::getFieldData(int fieldNum, fl32 missingFl32) const
{
  if (fieldNum >= CED_MAX_FIELDS) {
    return NULL;
  }
  int npts = _volHdr.nz * _volHdr.ny * _volHdr.nx;
  fl32 *fdata = new fl32[npts];
  const si16 *sdata = _fieldData[fieldNum];
  fl32 scale = _volHdr.field[fieldNum].field_sf;
  si16 missingSi16 = _volHdr.missing_val;
  for (int ii = 0; ii < npts; ii++) {
    if (sdata[ii] == missingSi16) {
      fdata[ii] = missingFl32;
    } else {
      fdata[ii] = (fl32) sdata[ii] / scale;
    }
  }
  return fdata;
}

string Cedric::getLandmarkName(int landmarkNum) const
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return "InvalidLandmark";
  }
  return _getString(_volHdr.landmark[landmarkNum].name, 6);
}

double Cedric::getLandmarkXposKm(int landmarkNum) const
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return 0.0;
  }
  return _getDouble(_volHdr.landmark[landmarkNum].x_position);
}

double Cedric::getLandmarkYposKm(int landmarkNum) const
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return 0.0;
  }
  return _getDouble(_volHdr.landmark[landmarkNum].y_position);
}

double Cedric::getLandmarkZposKm(int landmarkNum) const
{
  if (landmarkNum >= CED_MAX_LANDMARKS) {
    return 0.0;
  }
  return _getKmDouble(_volHdr.landmark[landmarkNum].z_position);
}

////////////////////////////////////////////
////////////////////////////////////////////
// private methods

////////////////////////////////////////////
// get string from text that is not guaranteed
// to be null-terminated

string Cedric::_getString(const char *text, int maxLen)
{
  char copy[128];
  memcpy(copy, text, maxLen);
  copy[maxLen] = '\0'; // ensure null termination
  for (int ii = maxLen - 1; ii >= 0; ii--) {
    if (isspace(copy[ii])) {
      copy[ii] = '\0';
    } else {
      break;
    }
  }
  return copy;
}
  
////////////////////////////////////////////
// set string in text in volume header

void Cedric::_setString(const string &str,
                        char *text, size_t maxLen)
{
  // initialize with spaces
  for (size_t ii = 0; ii < maxLen; ii++) {
    text[ii] = ' ';
  }
  // copy in text
  if (str.size() < maxLen) {
    memcpy(text, str.c_str(), str.size());
  } else {
    memcpy(text, str.c_str(), maxLen);
  }
}
  
////////////////////////////////////////////
// get time in unix secs

time_t Cedric::_getTime(si16 year, si16 month, si16 day,
                        si16 hour, si16 min, si16 sec) const
{
  if (year < 50) {
    year += 2000;
  } else if (year < 100) {
    year += 1900;
  }
  DateTime dtime(year, month, day, hour, min, sec);
  return dtime.utime();
}
  
////////////////////////////////////////////
// set time in unix secs

void Cedric::_setTime(time_t utime,
                      si16 &year, si16 &month, si16 &day,
                      si16 &hour, si16 &min, si16 &sec)
{
  DateTime dtime(utime);
  year = dtime.getYear();
  if (year > 1999) {
    year -= 2000;
  } else {
    year -= 1900;
  }
  month = dtime.getMonth();
  day = dtime.getDay();
  hour = dtime.getHour();
  min = dtime.getMin();
  sec = dtime.getSec();
}
  
////////////////////////////////////////////
// set coordinate type
// and relevant scaling factors

void Cedric::setCoordType(const std::string &val) {
  _setString(val, _volHdr.scan_mode, 4);
  _setZScale();
}

////////////////////////////////////////////
// set Z scale depending on coordinate type

void Cedric::_setZScale() {
  string coordType = getCoordType();
  if (coordType.find("CRT") != string::npos ||
      coordType.find("LLZ") != string::npos) {
    _zScale = 1000.0; // km to meters
  } else {
    _zScale = 100.0; // degrees uses SF of 100.0
  }
}

////////////////////////////////////////////
// set run time

void Cedric::setDateTimeRun(time_t runTime)
{
  DateTime rtime(runTime);
  char timeStr[32], dateStr[32];
  sprintf(timeStr, "%.2d:%.2d:%.2d",
          rtime.getHour(), rtime.getMin(), rtime.getSec());
  sprintf(dateStr, "%.2d/%.2d/%.2d",
          rtime.getMonth(), rtime.getDay(), rtime.getYear() % 100);
  setDateRun(dateStr);
  setTimeRun(timeStr);
}
  
////////////////////////////////////////////
// convert deg/min/sec into decimal degrees

double Cedric::_getDecDegrees(si16 ideg, si16 imin, si16 isec) const
{
  double ddeg = (double) ideg;
  double dmin = (double) imin;
  double dsec = _getDouble(isec);
  double deg = ddeg + dmin / 60.0 + dsec / 3600.0;
  return deg;
}

////////////////////////////////////////////
// convert decimal degrees into deg/min/sec

void Cedric::_setDegMinSec(double degrees,
                           si16 &ideg, si16 &imin, si16 &isec)
{

  double ddeg = trunc(degrees);
  double dmin = trunc((degrees - ddeg) * 60.0);
  double dsec = (degrees - ddeg - dmin / 60.0) * 3600.0;
  
  ideg = (si16) floor(ddeg + 0.5);
  imin = (si16) floor(dmin + 0.5);
  isec = (si16) floor(dsec * (double) _volHdr.scale_factor + 0.5);

}
  
///////////////
// print object
//

void Cedric::print(ostream &out,
                   bool printData)
  
{

  printMetaData(out);
  
  if (printData) {
    for (int ii = 0; ii < getNumFields(); ii++) {
      printFieldData(ii, out);
    }
  }

}

/////////////////////////
// print headers natively

void Cedric::printNative(ostream &out,
                         bool printData /* = false */)
  
{

  print(_fileHdr, out);
  print(_volHdr, out);
  for (int iz = 0; iz < _volHdr.nz; iz++) {
    out << "============= Level num: " << iz << " ============" << endl;
    print(_levelHdrs[iz], out);
  }

  if (printData) {
    for (int ii = 0; ii < getNumFields(); ii++) {
      printFieldData(ii, out);
    }
  }

}

/////////////////////////////
// print header data

void Cedric::printMetaData(ostream &out)
  
{

  out << "======== CEDRIC VOLUME METADATA =========" << endl;

  out << "  VolNum: " << getVolumeNum() << endl;
  out << "  VolLabel: " << getVolumeLabel() << endl;
  
  out << "  Id: " << getId() << endl;
  out << "  Program: " << getProgram() << endl;
  out << "  Project: " << getProject() << endl;
  out << "  Scientist: " << getScientist() << endl;
  out << "  Radar: " << getRadar() << endl;
  out << "  CoordType: " << getCoordType() << endl;
  out << "  Tape: " << getTape() << endl;

  out << "  TapeStartTime: " << DateTime::strm(getTapeStartTime()) << endl;
  out << "  TapeEndTime: " << DateTime::strm(getTapeEndTime()) << endl;

  out << "  LatitudeDeg: " << getLatitudeDeg() << endl;
  out << "  LongitudeDeg: " << getLongitudeDeg() << endl;
  out << "  OriginHtM: " << getOriginHtM() << endl;
  
  out << "  XaxisAngleFromNDeg: " << getXaxisAngleFromNDeg() << endl;

  out << "  OriginX: " << getOriginX() << endl;
  out << "  OriginY: " << getOriginY() << endl;
  
  out << "  TimeZone: " << getTimeZone() << endl;
  out << "  Sequence: " << getSequence() << endl;
  out << "  Submitter: " << getSubmitter() << endl;
  out << "  DateRun: " << getDateRun() << endl;
  out << "  TimeRun: " << getTimeRun() << endl;

  out << "  TapeEdNumber: " << getTapeEdNumber() << endl;
  out << "  HeaderRecordLength: " << getHeaderRecordLength() << endl;
  
  out << "  Computer: " << getComputer() << endl;

  out << "  BitsDatum: " << getBitsDatum() << endl;
  out << "  BlockingMode: " << getBlockingMode() << endl;
  out << "  BlockSize: " << getBlockSize() << endl;
  out << "  MissingDataVal: " << getMissingDataVal() << endl;
  
  out << "  ScaleFactor: " << getScaleFactor() << endl;
  out << "  AngleFactor: " << getAngleFactor() << endl;
  
  out << "  Source: " << getSource() << endl;

  out << "  TapeLabel2: " << getTapeLabel2() << endl;
  out << "  TapeLabel3: " << getTapeLabel3() << endl;
  out << "  TapeLabel4: " << getTapeLabel4() << endl;
  out << "  TapeLabel5: " << getTapeLabel5() << endl;
  out << "  TapeLabel6: " << getTapeLabel6() << endl;

  out << "  RecordsPlane: " << getRecordsPlane() << endl;
  out << "  RecordsField: " << getRecordsField() << endl;
  out << "  RecordsVolume: " << getRecordsVolume() << endl;
  out << "  TotalRecords: " << getTotalRecords() << endl;
  out << "  TotRecords: " << getTotRecords() << endl;

  out << "  VolName: " << getVolName() << endl;

  out << "  NumPlanes: " << getNumPlanes() << endl;

  out << "  CubicKm: " << getCubicKm() << endl;
  out << "  TotalPoints: " << getTotalPoints() << endl;
  out << "  SamplingDensity: " << getSamplingDensity() << endl;

  out << "  NumPulses: " << getNumPulses() << endl;
  out << "  VolumeNumber: " << getVolumeNumber() << endl;
  
  out << "  VolStartTime: " << DateTime::strm(getVolStartTime()) << endl;
  out << "  VolEndTime: " << DateTime::strm(getVolEndTime()) << endl;
  
  out << "  VolumeTimeSec: " << getVolumeTimeSec() << endl;
  out << "  IndexNumberTime: " << getIndexNumberTime() << endl;

  out << "  MinRangeKm: " << getMinRangeKm() << endl;
  out << "  MaxRangeKm: " << getMaxRangeKm() << endl;

  out << "  NumGatesBeam: " << getNumGatesBeam() << endl;
  out << "  GateSpacingKm: " << getGateSpacingKm() << endl;
  out << "  GateSpacingM: " << getGateSpacingM() << endl;
  out << "  MinGates: " << getMinGates() << endl;
  out << "  MaxGates: " << getMaxGates() << endl;
  out << "  IndexNumberRange: " << getIndexNumberRange() << endl;
  out << "  MinAzimuthDeg: " << getMinAzimuthDeg() << endl;
  out << "  MaxAzimuthDeg: " << getMaxAzimuthDeg() << endl;
  out << "  NumBeamsPlane: " << getNumBeamsPlane() << endl;
  out << "  AveAngleDeg: " << getAveAngleDeg() << endl;
  out << "  MinBeamsPlane: " << getMinBeamsPlane() << endl;
  out << "  MaxBeamsPlane: " << getMaxBeamsPlane() << endl;
  out << "  NumStepsBeam: " << getNumStepsBeam() << endl;
  out << "  IndexNumberAzimuth: " << getIndexNumberAzimuth() << endl;

  out << "  PlaneType: " << getPlaneType() << endl;

  out << "  MinElevDeg: " << getMinElevDeg() << endl;
  out << "  MaxElevDeg: " << getMaxElevDeg() << endl;
  out << "  NumElevs: " << getNumElevs() << endl;
  out << "  AveDeltaElevDeg: " << getAveDeltaElevDeg() << endl;
  out << "  AveElevDeg: " << getAveElevDeg() << endl;
  out << "  Direction: " << getDirection() << endl;
  out << "  BaselineAngleDeg: " << getBaselineAngleDeg() << endl;
  out << "  IndexNumberCoplane: " << getIndexNumberCoplane() << endl;
  
  out << "  MinX: " << getMinX() << endl;
  out << "  MaxX: " << getMaxX() << endl;
  out << "  Nx: " << getNx() << endl;
  out << "  Dx: " << getDx() << endl;
  out << "  FastAxis: " << getFastAxis() << endl;
  
  out << "  MinY: " << getMinY() << endl;
  out << "  MaxY: " << getMaxY() << endl;
  out << "  Ny: " << getNy() << endl;
  out << "  Dy: " << getDy() << endl;
  out << "  MidAxis: " << getMidAxis() << endl;
  
  out << "  MinZ: " << getMinZ() << endl;
  out << "  MaxZ: " << getMaxZ() << endl;
  out << "  Nz: " << getNz() << endl;
  out << "  Dz: " << getDz() << endl;
  out << "  SlowAxis: " << getSlowAxis() << endl;
  
  out << "  NumFields: " << getNumFields() << endl;
  for (int ii = 0; ii < getNumFields(); ii++) {
    out << "    FIELD:" << endl;
    out << "      num: " << ii << endl;
    out << "      name: " << getFieldName(ii) << endl;
    out << "      scale_factor: " << getFieldScaleFactor(ii) << endl;
  } // ii
  // const si16 *getFieldData(int fieldNum) const;

  out << "  PointsPlane: " << getPointsPlane() << endl;
  out << "  NumRadars: " << getNumRadars() << endl;

  out << "  NyquistVel: " << getNyquistVel() << endl;
  out << "  RadarConst: " << getRadarConst() << endl;
  
  out << "  NumLandmarks: " << getNumLandmarks() << endl;
  for (int ii = 0; ii < getNumLandmarks(); ii++) {
    out << "    LANDMARK:" << endl;
    out << "      num: " << ii << endl;
    out << "      name: " << getLandmarkName(ii) << endl;
    out << "      x pos km: " << getLandmarkXposKm(ii) << endl;
    out << "      y pos km: " << getLandmarkYposKm(ii) << endl;
    out << "      z pos km: " << getLandmarkZposKm(ii) << endl;
  } // ii

  out << "=========================================" << endl;

}

/////////////////////////////////////////////////////////
// print with data

void Cedric::printFieldData(int fieldNum, ostream &out) const
  
{

  fl32 missingFl32 = -9999.0;
  fl32 *fdata = getFieldData(fieldNum, missingFl32);
  if (fdata == NULL) {
    out << "WARNING - invalid field num: " << fieldNum << endl;
    return;
  }

  // compute min and max

  int nptsPlane = _volHdr.ny * _volHdr.nx;
  double minVal = 1.0e99;
  double maxVal = -1.0e99;
  for (int iz = 0; iz < _volHdr.nz; iz++) {
    const fl32 *planeData = fdata + iz * nptsPlane;
    for (int ii = 1; ii < nptsPlane; ii++) {
      fl32 val = planeData[ii];
      if (val != missingFl32) {
        if (val > maxVal) {
          maxVal = val;
        }
        if (val < minVal) {
          minVal = val;
        }
      }
    } // ii
  }

  out << "================== Data ===================" << endl;
  out << "==>> Field name: " << getFieldName(fieldNum) << endl;
  out << "     minVal: " << minVal << endl;
  out << "     maxVal: " << maxVal << endl;
  int printed = 0;
  int count = 1;
  for (int iz = 0; iz < _volHdr.nz; iz++) {
    out << "====>> Plane num, z: " << iz << ", " << getZ(iz) << endl;
    const fl32 *planeData = fdata + iz * nptsPlane;
    fl32 prevVal = planeData[0];
    for (int ii = 1; ii < nptsPlane; ii++) {
      fl32 val = planeData[ii];
      if (val != prevVal) {
        _printPacked(out, count, prevVal, missingFl32);
        printed++;
        if (printed > 6) {
          out << endl;
          printed = 0;
        }
        prevVal = val;
        count = 1;
      } else {
        count++;
      }
    } // ii
    _printPacked(out, count, prevVal, missingFl32);
    out << endl;
  }
  out << "===========================================" << endl;

  delete[] fdata;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void Cedric::_printPacked(ostream &out, int count, fl32 val, fl32 missingFl32)

{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == missingFl32) {
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

void Cedric::print(CED_file_head_t &hdr,
                   ostream &out)
  
{

  out << "========= CEDRIC FILE HEADER ==========" << endl;
  out << "  id: " << _getString(hdr.id, 4) << endl;
  out << "  byte_order: " << hdr.byte_order << endl;
  out << "  file_size: " << hdr.file_size << endl;
  out << "  reserved1: " << hdr.reserved1 << endl;
  out << "  vol_index[0]: " << hdr.vol_index[0] << endl;
  out << "  vol_label[0]: " << _getString(hdr.vol_label[0], 56) << endl;
  out << "  reserved2: " << hdr.reserved2 << endl;
  out << "  reserved3: " << hdr.reserved3 << endl;
  out << "  reserved4: " << hdr.reserved4 << endl;
  out << "  reserved5: " << hdr.reserved5 << endl;
  out << "  reserved6: " << hdr.reserved6 << endl;
  out << "  reserved7: " << hdr.reserved7 << endl;
  out << "=======================================" << endl;

}

void Cedric::print(CED_vol_head_t &hdr,
                   ostream &out)
  
{

  out << "======== CEDRIC VOLUME HEADER =========" << endl;
  out << "  id: " << _getString(hdr.id, 8) << endl;
  out << "  program: " << _getString(hdr.program, 6) << endl;
  out << "  project: " << _getString(hdr.project, 4) << endl;
  out << "  scientist: " << _getString(hdr.scientist, 6) << endl;
  out << "  radar: " << _getString(hdr.radar, 6) << endl;
  out << "  scan_mode: " << _getString(hdr.scan_mode, 4) << endl;
  out << "  tape: " << _getString(hdr.tape, 6) << endl;
  
  out << "  tape_start_year: " << hdr.tape_start_year << endl;
  out << "  tape_start_month: " << hdr.tape_start_month << endl;
  out << "  tape_start_day: " << hdr.tape_start_day << endl;
  out << "  tape_start_hour: " << hdr.tape_start_hour << endl;
  out << "  tape_start_min: " << hdr.tape_start_min << endl;
  out << "  tape_start_sec: " << hdr.tape_start_sec << endl;
  out << "  tape_end_year: " << hdr.tape_end_year << endl;
  out << "  tape_end_month: " << hdr.tape_end_month << endl;
  out << "  tape_end_day: " << hdr.tape_end_day << endl;
  out << "  tape_end_hour: " << hdr.tape_end_hour << endl;
  out << "  tape_end_min: " << hdr.tape_end_min << endl;
  out << "  tape_end_sec: " << hdr.tape_end_sec << endl;
  
  out << "  lat_deg: " << hdr.lat_deg << endl;
  out << "  lat_min: " << hdr.lat_min << endl;
  out << "  lat_sec: " << hdr.lat_sec << endl;
  out << "  lon_deg: " << hdr.lon_deg << endl;
  out << "  lon_min: " << hdr.lon_min << endl;
  out << "  lon_sec: " << hdr.lon_sec << endl;
  
  out << "  origin_height: " << hdr.origin_height << endl;
  
  out << "  angle1: " << hdr.angle1 << endl;
  out << "  origin_x: " << hdr.origin_x << endl;
  out << "  origin_y: " << hdr.origin_y << endl;
  
  out << "  time_zone: " << _getString(hdr.time_zone, 4) << endl;
  
  out << "  sequence: " << _getString(hdr.sequence, 6) << endl;
  out << "  submitter: " << _getString(hdr.submitter, 6) << endl;
  out << "  dateRun: " << _getString(hdr.date_run, 8) << endl;
  out << "  time_run: " << _getString(hdr.time_run, 8) << endl;
  
  out << "  sh59: " << hdr.sh59 << endl;
  out << "  tape_ed_number: " << hdr.tape_ed_number << endl;
  out << "  header_record_length: " << hdr.header_record_length << endl;
  
  out << "  computer: " << _getString(hdr.computer, 2) << endl;
  out << "  bits_datum: " << hdr.bits_datum << endl;
  out << "  blocking_mode: " << hdr.blocking_mode << endl;
  out << "  block_size: " << hdr.block_size << endl;
  out << "  sh66: " << hdr.sh66 << endl;
  out << "  missing_val: " << hdr.missing_val << endl;

  out << "  scale_factor: " << hdr.scale_factor << endl;
  out << "  angle_factor: " << hdr.angle_factor << endl;

  out << "  sh70: " << hdr.sh70 << endl;
  out << "  source: " << _getString(hdr.source, 8) << endl;
  out << "  tape_label2: " << _getString(hdr.tape_label2, 8) << endl;
  out << "  tape_label3: " << _getString(hdr.tape_label3, 8) << endl;
  out << "  tape_label4: " << _getString(hdr.tape_label4, 8) << endl;
  out << "  tape_label5: " << _getString(hdr.tape_label5, 8) << endl;
  out << "  tape_label6: " << _getString(hdr.tape_label6, 8) << endl;
  out << "  sh95: " << hdr.sh95 << endl;
  out << "  records_plane: " << hdr.records_plane << endl;
  out << "  records_field: " << hdr.records_field << endl;
  out << "  records_volume: " << hdr.records_volume << endl;
  out << "  total_records: " << hdr.total_records << endl;
  out << "  tot_records: " << hdr.tot_records << endl;
  out << "  vol_name: " << _getString(hdr.vol_name, 8) << endl;
  out << "  sh105: " << hdr.sh105 << endl;
  out << "  num_planes: " << hdr.num_planes << endl;
  out << "  cubic_km: " << hdr.cubic_km << endl;
  out << "  total_points: " << hdr.total_points << endl;
  out << "  sampling_density: " << hdr.sampling_density << endl;
  out << "  num_pulses: " << hdr.num_pulses << endl;
  out << "  volume_number: " << hdr.volume_number << endl;
  out << "  sh112: " << hdr.sh112 << endl;
  out << "  sh113: " << hdr.sh113 << endl;
  out << "  sh114: " << hdr.sh114 << endl;
  out << "  sh115: " << hdr.sh115 << endl;

  out << "  vol_start_year: " << hdr.vol_start_year << endl;
  out << "  vol_start_month: " << hdr.vol_start_month << endl;
  out << "  vol_start_day: " << hdr.vol_start_day << endl;
  out << "  vol_start_hour: " << hdr.vol_start_hour << endl;
  out << "  vol_start_min: " << hdr.vol_start_min << endl;
  out << "  vol_start_second: " << hdr.vol_start_second << endl;
  out << "  vol_end_year: " << hdr.vol_end_year << endl;
  out << "  vol_end_month: " << hdr.vol_end_month << endl;
  out << "  vol_end_day: " << hdr.vol_end_day << endl;
  out << "  vol_end_hour: " << hdr.vol_end_hour << endl;
  out << "  vol_end_min: " << hdr.vol_end_min << endl;
  out << "  vol_end_second: " << hdr.vol_end_second << endl;
  
  out << "  volume_time: " << hdr.volume_time << endl;
  out << "  index_number_time: " << hdr.index_number_time << endl;
  out << "  sh130: " << hdr.sh130 << endl;
  out << "  sh131: " << hdr.sh131 << endl;
  out << "  min_range: " << hdr.min_range << endl;
  out << "  max_range: " << hdr.max_range << endl;
  out << "  num_gates_beam: " << hdr.num_gates_beam << endl;
  out << "  gate_spacing: " << hdr.gate_spacing << endl;
  out << "  min_gates: " << hdr.min_gates << endl;
  out << "  max_gates: " << hdr.max_gates << endl;
  out << "  sh138: " << hdr.sh138 << endl;
  out << "  index_number_range: " << hdr.index_number_range << endl;
  out << "  sh140: " << hdr.sh140 << endl;
  out << "  sh141: " << hdr.sh141 << endl;
  out << "  min_azimuth: " << hdr.min_azimuth << endl;
  out << "  max_azimuth: " << hdr.max_azimuth << endl;
  out << "  num_beams_plane: " << hdr.num_beams_plane << endl;
  out << "  ave_angle: " << hdr.ave_angle << endl;
  out << "  min_beams_plane: " << hdr.min_beams_plane << endl;
  out << "  max_beams_plane: " << hdr.max_beams_plane << endl;
  out << "  num_steps_beam: " << hdr.num_steps_beam << endl;
  out << "  index_number_azimuth: " << hdr.index_number_azimuth << endl;
  out << "  sh150: " << hdr.sh150 << endl;
  out << "  plane_type: " << _getString(hdr.plane_type, 2) << endl;
  out << "  min_elev: " << hdr.min_elev << endl;
  out << "  max_elev: " << hdr.max_elev << endl;
  out << "  num_elevs: " << hdr.num_elevs << endl;
  out << "  ave_delta_elev: " << hdr.ave_delta_elev << endl;
  out << "  ave_elev: " << hdr.ave_elev << endl;
  out << "  direction: " << hdr.direction << endl;
  out << "  baseline_angle: " << hdr.baseline_angle << endl;
  out << "  index_number_coplane: " << hdr.index_number_coplane << endl;

  out << "  min_x: " << hdr.min_x << endl;
  out << "  max_x: " << hdr.max_x << endl;
  out << "  nx: " << hdr.nx << endl;
  out << "  dx: " << hdr.dx << endl;
  out << "  fast_axis: " << hdr.fast_axis << endl;
  
  out << "  min_y: " << hdr.min_y << endl;
  out << "  max_y: " << hdr.max_y << endl;
  out << "  ny: " << hdr.ny << endl;
  out << "  dy: " << hdr.dy << endl;
  out << "  mid_axis: " << hdr.mid_axis << endl;

  out << "  min_z: " << hdr.min_z << endl;
  out << "  max_z: " << hdr.max_z << endl;
  out << "  nz: " << hdr.nz << endl;
  out << "  dz: " << hdr.dz << endl;
  out << "  slow_axis: " << hdr.slow_axis << endl;

  out << "  num_fields: " << hdr.num_fields << endl;

  for (int ii = 0; ii < hdr.num_fields; ii++) {
    out << "    FIELD:" << endl;
    out << "      num: " << ii << endl;
    out << "      name: " << _getString(hdr.field[ii].field_name, 8) << endl;
    out << "      scale_factor: " << hdr.field[ii].field_sf << endl;
  } // ii

  out << "  points_plane: " << hdr.points_plane << endl;

  out << "  num_landmarks: " << hdr.num_landmarks << endl;
  out << "  num_radars: " << hdr.num_radars << endl;
  out << "  nyquist_vel: " << hdr.nyquist_vel << endl;
  out << "  radar_const: " << hdr.radar_const << endl;
  
  for (int ii = 0; ii < hdr.num_landmarks; ii++) {
    out << "    LANDMARK:" << endl;
    out << "      num: " << ii << endl;
    out << "      name: " << _getString(hdr.landmark[ii].name, 8) << endl;
    out << "      x_position: " << hdr.landmark[ii].x_position << endl;
    out << "      y_position: " << hdr.landmark[ii].y_position << endl;
    out << "      z_position: " << hdr.landmark[ii].z_position << endl;
  } // ii

  out << "=======================================" << endl;

}

void Cedric::print(CED_level_head_t &hdr, ostream &out)
  
{

  out << "======== CEDRIC LEVEL HEADER =========" << endl;
  out << "  id: " << _getString(hdr.id, 6) << endl;
  out << "  coord: " << hdr.coord << endl;
  out << "  level_number: " << hdr.level_number << endl;
  out << "  number_fields: " << hdr.number_fields << endl;
  out << "  points_per_plane: " << hdr.points_per_plane << endl;
  out << "  records_per_field: " << hdr.records_per_field << endl;
  out << "  records_per_plane: " << hdr.records_per_plane << endl;
  out << "  nyquist_vel: " << hdr.nyquist_vel << endl;
  out << "======================================" << endl;

}

void Cedric::_swapWords(void *array, int nBytes)
{
  int nWords = nBytes / 2;
  si16 *si16s = (si16 *) array;
  for (int ii = 0; ii < nWords; ii += 2) {
    si16 tmp = si16s[ii];
    si16s[ii] = si16s[ii+1];
    si16s[ii+1] = tmp;
  }
}

/******************************************************************************
 * SWAP_CEDRIC_FILE_HEADER: Swap the bytes if necessary.
 *
 */

void Cedric::_swap(CED_file_head_t &hdr)
{
  SWAP_array_32(&hdr.byte_order, 28 * 4);
}


/******************************************************************************
 * SWAP_CEDRIC_VOL_HEADER: Swap the bytes if necessary
 *
 */

void Cedric::_swap(CED_vol_head_t &hdr)
{
  
  SWAP_array_16(&hdr.tape_start_year, 4); /* 21-42 */
  SWAP_array_16(&hdr.tape_ed_number, 4); /* 60-61 */
  SWAP_array_16(&hdr.bits_datum, 14); /* 63-69 */
  SWAP_array_16(&hdr.records_plane, 10); /* 96-100 */
  SWAP_array_16(&hdr.num_planes, 88); /* 106-149 */
  SWAP_array_16(&hdr.min_elev, 48); /* 152-175 */
  SWAP_array_16(&hdr.points_plane, 10); /* 301-305 */

  for(int i = 0; i < hdr.num_fields && i < CED_MAX_FIELDS; i++) {
    SWAP_array_16(&hdr.field[i].field_sf, 2);
  }
 
  for(int i = 0; i < hdr.num_landmarks && i < CED_MAX_LANDMARKS; i++) {
    SWAP_array_16(&hdr.landmark[i].x_position, 6);
  }

}

/******************************************************************************
 * SWAP_CEDRIC_LEVEKL_HEADER: Swap the bytes if necessary
 *
 */

void Cedric::_swap(CED_level_head_t &hdr)
{
  SWAP_array_16(&hdr.coord, sizeof(CED_level_head_t) - 6);
}

