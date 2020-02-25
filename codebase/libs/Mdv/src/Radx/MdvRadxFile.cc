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
// MdvRadxFile.cc
//
// MdvRadxFile object
//
// MDV support for radar radial data files
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#include <Mdv/MdvRadxFile.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Mdv/DsMdvx.hh>
#include <Radx/RadxVol.hh>
using namespace std;

//////////////
// Constructor

MdvRadxFile::MdvRadxFile() : RadxFile()
        
{

}

/////////////
// destructor

MdvRadxFile::~MdvRadxFile()

{
}

/////////////
// clear data

void MdvRadxFile::clear()

{
  RadxFile::clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is supported by Radx
// Returns true if supported, false otherwise

bool MdvRadxFile::isSupported(const string &path)

{
  
  if (isMdv(path)) {
    return true;
  } else {
    return false;
  }

}

/////////////////////////////////////////////////////////
// Check if specified file is and MDV file
// Returns true if supported, false otherwise

bool MdvRadxFile::isMdv(const string &path)

{
  
  Mdvx mdv;
  if (mdv.verify(path)) {
    return true;
  }
  return false;

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

int MdvRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  clearErrStr();
  _dirInUse = dir;
  _writePaths.clear();
  _writeDataTimes.clear();

  if (_debug) {
    cerr << "DEBUG - MdvRadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }

  // translate to MDV
  
  Mdvx mdv;
  Ncf2MdvTrans trans;
  RadxVol copy(vol);
  if (trans.translateRadxVol2Mdv(vol.getPathInUse(), copy, mdv)) {
    _addErrStr("ERROR - MdvRadxFile::writeToDir");
    _addErrStr("  Cannot convert RadxVol to Mdv");
    _addErrStr(trans.getErrStr());
    return -1;
  }
  
  // set up write
  
  if (_verbose) {
    mdv.setDebug(true);
  }
  if (_writeLdataInfo) {
    mdv.setWriteLdataInfo();
  }
  if (addYearSubDir) {
    mdv.setWriteUsingExtendedPath();
  }
  
  // perform write
  
  if (mdv.writeToDir(dir)) {
    _addErrStr("ERROR - MdvRadxFile::writeToDir");
    _addErrStr("  Cannot write file to dir: ", dir);
    _addErrStr(mdv.getErrStr());
    return -1;
  }

  _pathInUse = mdv.getPathInUse();
  
  if (_debug) {
    cerr << "Wrote file: " << mdv.getPathInUse() << endl;
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

int MdvRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)

{

  clearErrStr();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _writePaths.clear();
  _writeDataTimes.clear();

  // translate to MDV
    
  Mdvx mdv;
  Ncf2MdvTrans trans;
  RadxVol copy(vol);
  if (trans.translateRadxVol2Mdv(vol.getPathInUse(), copy, mdv)) {
    _addErrStr("ERROR - MdvRadxFile::writeToPath");
    _addErrStr("  Cannot convert RadxVol to Mdv");
    _addErrStr(trans.getErrStr());
    return -1;
  }

  // set up write

  if (_verbose) {
    mdv.setDebug(true);
  }
  if (_writeLdataInfo) {
    mdv.setWriteLdataInfo();
  }

  // perform write

  if (mdv.writeToPath(path)) {
    _addErrStr("ERROR - MdvRadxFile::writeToPath");
    _addErrStr("  Cannot write file to path: ", path);
    _addErrStr(mdv.getErrStr());
    return -1;
  }
  
  _pathInUse = mdv.getPathInUse();
  
  if (_debug) {
    cerr << "Wrote file: " << mdv.getPathInUse() << endl;
  }
    
  return 0;
  
}

/////////////////////////////////////////////////////////
// Read in data file from specified path,
// load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int MdvRadxFile::readFromPath(const string &path,
                              RadxVol &vol)

{

  // initialize for read
  
  _initForRead(path, vol);
  
  // perform read

  return _readFromPath(path, vol);

}

/////////////////////////////////////////////////////////
// Read in data file from specified path.
// Does not perform _initForRead().
// load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int MdvRadxFile::_readFromPath(const string &path,
                               RadxVol &vol)

{

  // MDV is special case
  
  Mdvx mdv;
  if (mdv.verify(path)) {
    if (_readMdvFile(path, vol)) {
      _addErrStr("ERROR - MdvRadxFile::readFromPath");
      return -1;
    } else {
      if (_debug) {
        cerr << "INFO: MdvRadxFile::readFromPath" << endl;
        cerr << "  Read MDV file, path: " << path << endl;
      }
      return 0;
    }
  }

  // do not recognize file type
  
  clearErrStr();
  _addErrStr("ERROR - MdvRadxFile::readFromPath");
  _addErrStr("  File format not recognized: ", path);
  return -1;

}

//////////////////////////////////////////////////
// Read in an MDV file, store in RadxVol
// Returns 0 on success, -1 on failure

int MdvRadxFile::_readMdvFile(const string &path,
                              RadxVol &vol)
{

  // set up read
  
  DsMdvx mdv;
  mdv.setReadPath(path);
  if (_verbose) {
    mdv.setDebug(true);
  }
  if (_readFixedAngleLimitsSet) {
    mdv.setReadVlevelLimits(_readMinFixedAngle,
                            _readMaxFixedAngle);
  } else if (_readSweepNumLimitsSet) {
    mdv.setReadPlaneNumLimits(_readMinSweepNum,
                              _readMaxSweepNum);
  }
  if (_readFieldNames.size() > 0) {
    for (size_t ii = 0; ii < _readFieldNames.size(); ii++) {
      mdv.addReadField(_readFieldNames[ii]);
    }
  }
  if (_verbose) {
    mdv.printReadRequest(cerr);
  }

  // perform MDV read

  if (mdv.readVolume()) {
    cerr << "WARNING - RadxConvert::_processMdvFile" << endl;
    cerr << "  Cannot read MDV file: " << path << endl;
    cerr << mdv.getErrStr() << endl;
    return -1;
  }

  // convert to RadxVol

  Mdv2NcfTrans trans;
  if (trans.convertToRadxVol(mdv, vol)) {
    cerr << "WARNING - RadxConvert::_processMdvFile" << endl;
    cerr << "  Cannot convert to RadxVol: " << path << endl;
    cerr << trans.getErrStr() << endl;
    return -1;
  }

  // apply other read constraints
  
  if (_readRemoveRaysAllMissing) {
    vol.removeRaysWithDataAllMissing();
  }
  if (_readSetMaxRange) {
    vol.setMaxRangeKm(_readMaxRangeKm);
  }

  return 0;

}

/////////////////////////////////////////////////////////
// print

void MdvRadxFile::print(ostream &out) const
  
{
  
  out << "=============== MdvRadxFile ===============" << endl;

  out << "  fileFormat: " << getFileFormatAsString() << endl;
  out << "  dirInUse: " << _dirInUse << endl;
  out << "  pathInUse: " << _pathInUse << endl;

  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int MdvRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  // try base class

  if (RadxFile::printNative(path, out, printRays, printData) == 0) {
    return 0;
  }

  // try MDV
  Mdvx mdv;

  if (path.find(".mdv") != string::npos && mdv.verify(path)) {
    if (mdv.readVolume()) {
      _addErrStr("ERROR - MdvRadxFile::printNative");
      _addErrStr("  Cannot read MDV file, path: ", path);
      _addErrStr(mdv.getErrStr());
      return -1;
    }
    Mdvx::printVol(cout, &mdv, true, printData);
    return 0;
  }

  // do not recognize file type

  clearErrStr();
  _addErrStr("ERROR - MdvRadxFile::printNative");
  _addErrStr("  File format not recognized: ", path);
  return -1;

}

