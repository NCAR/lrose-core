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
// DoradeRadxFile.hh
//
// DoradeRadxFile object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef DoradeRadxFile_HH
#define DoradeRadxFile_HH

#include <string>
#include <vector>
#include <cmath>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/DoradeData.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRemap.hh>

class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NCAR/EOL DORADE FILE FORMAT
///
/// This subclass of RadxFile handles I/O for dorade files.

class DoradeRadxFile : public RadxFile

{
  
public:

  /// constructor
  
  DoradeRadxFile();
  
  /// destructor
  
  virtual ~DoradeRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a Dorade file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a Dorade file.
  /// Returns true on success, false on failure
  
  bool isDorade(const string &path);

  /// Check if this file needs to be byte-swapped on this host.
  ///
  /// Returns 0 on success, -1 on failure
  ///
  /// Use isSwapped() to get result after making this call.
  
  int checkIsSwapped(const string &path);
  
  /// Get the result of checkIsSwapped().
  ///
  /// If true, file is byte-swapped with respect to the native 
  /// architecture, and must be swapped on read.
  
  bool isSwapped() { return _ddIsSwapped; }

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

  /// compute output file name

  static string computeFileName(int volNum,
                                int nSweeps,
                                double fixedAngle,
                                string instrumentName,
                                string scanType,
                                int year, int month, int day,
                                int hour, int min, int sec,
                                int millisecs);

  /// get the date and time from a dorade file path

  static RadxTime getTimeFromPath(const string &path);

protected:
private:

  // utility functions
  
  static inline double RADIANS(double x) { return x * 0.017453292; }
  static inline double DEGREES(double x) { return x * 57.29577951; }
  static inline double FMOD360(double x) { return fmod(x + 720.0, 360.0); }
  static inline double CART_ANGLE(double x) { return (90.0 - x); }
  
  // volume for writing

  RadxVol *_writeVol; ///< volume from which data is written
  
  // objects to be set on read

  vector<RadxRay *> _rays;
  RadxRay *_latestRay;
  bool _rayInProgress;
  int _nNullsFound;

  // range geometry

  RadxRemap _remap;
  bool _constantGateSpacing;
  int _nGatesIn;
  vector<double> _rangeArray;
  
  bool _parmIsOldVersion; // parameter struct is old short version
  bool _radarIsOldVersion; // radar struct is old short version

  int _volumeNum;
  int _sweepNumOnAg;

  // dorade data - read and write

  bool _ddIsSwapped;
  DoradeData::comment_t _ddComment;
  DoradeData::super_SWIB_t _ddSwib;
  DoradeData::volume_t _ddVol;
  DoradeData::radar_t _ddRadar;
  DoradeData::lidar_t _ddLidar;
  DoradeData::correction_t _ddCfac;
  DoradeData::cell_vector_t _ddCellv;
  DoradeData::cell_spacing_fp_t _ddCsfp;
  DoradeData::sweepinfo_t _ddSweep;
  DoradeData::platform_t _ddPlat;
  DoradeData::ray_t _ddRay;
  DoradeData::paramdata_t _ddPdat;
  DoradeData::qparamdata_t _ddQdat;
  DoradeData::extra_stuff_t _ddExtra;
  DoradeData::rot_angle_table_t _ddRotTable;
  vector<DoradeData::parameter_t> _ddParms;

  DoradeData::radar_test_status_t _ddRadarTestStatus;
  DoradeData::field_radar_t _ddFieldRadar;
  DoradeData::field_lidar_t _ddFieldLidar;
  DoradeData::insitu_descript_t _ddInsituDescript;
  DoradeData::insitu_data_t _ddInsituData;
  DoradeData::indep_freq_t _ddIndepFreq;
  DoradeData::minirims_data_t _ddMinirimsData;
  DoradeData::nav_descript_t _ddNavDescript;
  DoradeData::time_series_t _ddTimeSeries;
  DoradeData::waveform_t _ddWaveform;

  string _sedsStr;
  string _radarName;

  // writing
  
  vector<DoradeData::rot_table_entry_t> _rotEntries;

  long _rotationTableOffset;
  int _rotationTableSize;

  long _sedsBlockOffset;
  int _sedsBlockSize;
  
  // file handle

  FILE *_file;

  // private methods
  
  void _clearRays();
  void _clearLatestRay();
  void _clearSweeps();
  void _clearCals();
  void _clearFields();

  int _writeSweepToDir(RadxVol &vol, const string &dir,
                       bool addDaySubDir, bool addYearSubDir);
  int _writeSweepToPath(RadxVol &vol, const string &path);

  int _readAggregatePaths(const string &path);
  int _readSweepFile(const string &path);
  int _getVolumePaths(const string &path, vector<string> &paths);
  void _addToPathList(const string &dir,
                      int minHour, int maxHour,
                      vector<string> &paths);
  
  int _openRead(const string &path);
  int _openWrite(const string &path);
  void _close();

  int _handleBlock(const string &idStr, int nBytes, const char *block);
  int _handleRay(int nBytes, const char *block);
  void _addGeorefToLatestRay();
  int _handleField(const DoradeData::parameter_t &parm,
                   int nBytesBlock, int minOffset,
                   const char *block);
  int _handleField16(const DoradeData::parameter_t &parm,
                     int nBytesBlock, int minOffset,
                     const char *block);

  int _loadReadVolume();

  int _printBlock(const string &idStr, int nBytes,
                  const char *block, ostream &out,
                  bool printRays = false,
                  bool printData = false);

  int _printField(const DoradeData::parameter_t &parm,
                  int nBytesBlock, int minOffset,
                  const char *block, ostream &out);
  
  int _printField16(const DoradeData::parameter_t &parm,
                    int nBytesBlock, int minOffset,
                    const char *block, ostream &out);
  
  int _printRotTable(const DoradeData::key_table_info_t &info, 
                     ostream &out);

  void _printFieldData(ostream &out, int nGates, const double *data) const;
  void _printPacked(ostream &out, int count, double val) const;

  int _writeComment();
  int _writeSuperSwib(int fileSize = 0);
  int _writeVolume();
  int _writeRadar();
  int _writeLidar();
  int _writeParameter(int fieldNum);
  int _writeCellVector();
  int _writeCellSpacingFp();
  int _writeCorrectionFactors();
  int _writeSweepInfo(int sweepNum);
  int _writeRayInfo(int rayNum,
                    DoradeData::ray_t &ddRay);
  int _writeRayGeoref(int rayNum,
                      const DoradeData::ray_t &ddRay,
                      DoradeData::rot_table_entry_t &rotEntry);
  int _writeRayData(int rayNum, int fieldNum);
  int _writeNullBlock();
  int _writeRotAngTable();
  int _writeSedsBlock();

  // angular correction

  double _ddLatitude(const DoradeData::platform_t &plat);
  double _ddLongitude(const DoradeData::platform_t &plat);
  double _ddAltitude(const DoradeData::platform_t &plat);
  double _ddAltitudeAgl(const DoradeData::platform_t &plat);
  double _ddRoll(const DoradeData::platform_t &plat);
  double _ddPitch(const DoradeData::platform_t &plat);
  double _ddHeading(const DoradeData::platform_t &plat);
  double _ddDrift(const DoradeData::platform_t &plat);
  double _ddAzimuth(const DoradeData::ray_t &ray,
                    const DoradeData::platform_t &plat);
  double _ddElevation(const DoradeData::ray_t &ray,
                      const DoradeData::platform_t &plat);
  double _ddRotation(const DoradeData::ray_t &ray,
                     const DoradeData::platform_t &plat);
  double _ddNavRotation(const DoradeData::ray_t &ray,
                        const DoradeData::platform_t &plat);
  double _ddTilt(const DoradeData::ray_t &ray,
                 const DoradeData::platform_t &plat);
  double _ddNavTilt(const DoradeData::ray_t &ray,
                    const DoradeData::platform_t &plat);

  void _ddRadarAngles(const DoradeData::ray_t &ray,
                      const DoradeData::platform_t &plat,
                      DoradeData::radar_angles_t &rAngles);
  
  double _getScale(const string &name);

};

#endif
