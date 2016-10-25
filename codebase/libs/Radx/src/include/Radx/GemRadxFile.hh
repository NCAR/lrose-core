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
// GemRadxFile.hh
//
// Reading in Gematronik data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2014
//
///////////////////////////////////////////////////////////////

#ifndef GemRadxFile_HH
#define GemRadxFile_HH

#include <string>
#include <vector>
#include <cmath>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRemap.hh>

class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
class RadxRcalib;
class GemInputField;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NCAR/EOL DORADE FILE FORMAT
///
/// This subclass of RadxFile handles I/O for dorade files.

class GemRadxFile : public RadxFile

{
  
public:

  /// constructor
  
  GemRadxFile();
  
  /// destructor
  
  virtual ~GemRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a Gem file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a Gem file.
  /// Returns true on success, false on failure
  
  bool isGematronik(const string &path);

  /// set the file time from the path
  /// returns 0 on success, -1 on failure

  static int setTimeFromPath(const string &fileName, time_t &fileTime);

  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified directory.
  ///
  /// If addDaySubDir is true, a subdirectory will be
  /// created with the name dir/yyyymmdd/.
  ///
  /// If addYearSubDir is true, a subdirectory will be
  /// created with the name dir/yyyy/.
  ///
  /// If both addDaySubDir and addYearSubDir are true,
  /// the subdirectory will be dir/yyyy/yyyymmdd/.
  ///
  /// Returns 0 on success, -1 on failure
  ///
  /// Use getErrStr() to get error message if error occurs.
  /// Use getDirInUse() for directory to which the data was written.
  /// Use getPathInUse() for path to which the data was written.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);
  
  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified path.
  //
  /// Returns 0 on success, -1 on failure
  //
  /// Use getErrStr() to get error message if error occurs.
  /// Use getPathInUse() for path to which the data was written.
  
  virtual int writeToPath(const RadxVol &vol,
                          const string &path);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Perform the read:
  //@{
  
  /// Read in data file from specified path,
  /// load up volume object.
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  virtual int readFromPath(const string &path,
                           RadxVol &vol);

  //@}

  ////////////////////////
  /// \name Printing:
  //@{
  
  /// Print summary after read.
  
  virtual void print(ostream &out) const;
  
  /// Print dorade data file in native format.
  ///
  /// Returns 0 on success, -1 on failure.
  ///
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  //@}

protected:
private:

  static int _volumeNumber;

  // objects to be set on read

  RadxTime _fileTime;
  int _nSweeps;
  int _nAngles;
  int _nGates;

  vector<RadxRay *> _rays;

  // file handle

  FILE *_file;

  // input field objects

  vector<GemInputField *> _fields;
  
  // private methods
  
  void _clearFields();
  void _clearRays();

  int _openRead(const string &path);
  void _close();

  void _getFieldPaths(const string &primaryPath,
                      vector<string> &fileNames,
                      vector<string> &filePaths,
                      vector<string> &fieldNames);
  
  int _readFields(const string &path);
  
  int _loadRays(const string &path);
  int _loadSweep(int sweepNum, time_t startTime, time_t endTime, 
                 double antennaSpeed);
  int _loadMetaData(const string &path);
  
  void _printFieldData(ostream &out, int nGates, const double *data) const;
  void _printPacked(ostream &out, int count, double val) const;

  int _computeNSweeps();
  int _setSweepGeom(int sweepNum);

};

#endif
