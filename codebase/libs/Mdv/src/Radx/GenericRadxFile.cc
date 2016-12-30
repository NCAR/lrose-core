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
// GenericRadxFile.cc
//
// GenericRadxFile object
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
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Mdv/Mdvx.hh>
using namespace std;

//////////////
// Constructor

GenericRadxFile::GenericRadxFile() : RadxFile()
        
{

}

/////////////
// destructor

GenericRadxFile::~GenericRadxFile()

{
}

/////////////////////////////////////////////////////////
// Check if specified file is supported by Radx
// Returns true if supported, false otherwise

bool GenericRadxFile::isSupported(const string &path)

{
  
  // try file types directly supported by RadxFile

  if (RadxFile::isSupported(path)) {
    return true;
  }

  // try MDV

  if (path.find(".mdv") != string::npos) {
    Mdvx mdv;
    if (mdv.verify(path)) {
      return true;
    }
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

int GenericRadxFile::writeToDir(const RadxVol &vol,
                                const string &dir,
                                bool addDaySubDir,
                                bool addYearSubDir)
  
{

  if (vol.getNRays() < 1) {
    _addErrStr("ERROR - GenericRadxFile::writeToPath");
    _addErrStr("  Output dir: ", dir);
    _addErrStr("  No rays in file, time: ", RadxTime::strm(vol.getStartTimeSecs()));
    return -1;
  }

  int iret = 0;

  if (_fileFormat == FILE_FORMAT_MDV_RADIAL) {
    
    // MDV is special case
    
    if (_debug) {
      cerr << "INFO: GenericRadxFile::writeToDir" << endl;
      cerr << "  Writing MDV file to dir: " << dir << endl;
    }
    
    MdvRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Wrote MDV file to path: " << _pathInUse << endl;
    }

  } else {

    // by default use base class
    
    iret = RadxFile::writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  }

  return iret;

}
  
/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int GenericRadxFile::writeToPath(const RadxVol &vol,
                                 const string &path)

{

  if (vol.getNRays() < 1) {
    _addErrStr("ERROR - GenericRadxFile::writeToPath");
    _addErrStr("  Output path: ", path);
    _addErrStr("  No rays in file, time: ", RadxTime::strm(vol.getStartTimeSecs()));
    return -1;
  }

  int iret = 0;

  if (_fileFormat == FILE_FORMAT_MDV_RADIAL) {
    
    // MDV is special case

    if (_debug) {
      cerr << "INFO: GenericRadxFile::writeToPath" << endl;
      cerr << "  Writing MDV file to path: " << path << endl;
    }

    MdvRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
    
    if (_debug) {
      cerr << "INFO: GenericRadxFile::writeToPath" << endl;
      cerr << "  Wrote MDV file to path: " << _pathInUse << endl;
    }

  } else {

    // use base class

    iret = RadxFile::writeToPath(vol, path);

  }
    
  return iret;

}

/////////////////////////////////////////////////////////
// Read in data file from specified path,
// load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int GenericRadxFile::readFromPath(const string &path,
                                  RadxVol &vol)

{
  
  // try base class first

  if (RadxFile::readFromPath(path, vol) == 0) {
    return 0;
  }
  
  // try MDV as special case

  MdvRadxFile file;
  file.copyReadDirectives(*this);
  if (file.isSupported(path)) {
    int iret = file._readFromPath(path, vol);
    if (_verbose) file.print(cerr);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _readPaths = file.getReadPaths();
    if (iret == 0) {
      if (_debug) {
        cerr << "INFO: GenericRadxFile::readFromPath" << endl;
        cerr << "  Read MDV file, path: " << _pathInUse << endl;
      }
    }
    return iret;
  }

  // do not recognize file type

  _addErrStr("ERROR - GenericRadxFile::readFromPath");
  _addErrStr("  File format not recognized: ", path);
  return -1;

}

/////////////////////////////////////////////////////////
// print

void GenericRadxFile::print(ostream &out) const
  
{
  
  out << "=============== GenericRadxFile ===============" << endl;

  out << "  fileFormat: " << getFileFormatAsString() << endl;
  out << "  dirInUse: " << _dirInUse << endl;
  out << "  pathInUse: " << _pathInUse << endl;

  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int GenericRadxFile::printNative(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{

  // try base class

  if (RadxFile::printNative(path, out, printRays, printData) == 0) {
    return 0;
  }

  // try MDV as special case

  MdvRadxFile file;
  file.copyReadDirectives(*this);
  if (file.isSupported(path)) {
    int iret = file.printNative(path, out, printRays, printData);
    if (iret) {
      _errStr = file.getErrStr();
    }
    return iret;
  }

  // do not recognize file type
  
  _addErrStr("ERROR - GenericRadxFile::printNative");
  _addErrStr("  File format not recognized: ", path);
  return -1;

}

