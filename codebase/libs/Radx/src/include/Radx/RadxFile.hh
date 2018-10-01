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
// RadxFile.hh
//
// RadxFile object
//
// Base class for radar radial data files
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#ifndef RadxFile_HH
#define RadxFile_HH

#include <string>
#include <vector>
#include <Radx/Radx.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxField.hh>
#include <ostream>
#include <ctime>
class RadxField;
class RadxVol;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO BASE CLASS
///
/// RadxFile, and its derived classes, provide the file I/O
/// for the RadxVol class.
///
/// RadxVol contains the data, either to be written or after a read is
/// complete.
///
/// RadxFile provides the methods for writing and reading RadxVol.
///
/// A number of file formats are supported, using derived classes:
///
/// \code
///    Cf/Radial NetCDF Format: using NcfRadxFile
///    NCAR/EOL  DORADE Format: using DoradeRadxFile
///    NCAR/EOL  FORAY Format: using ForayNcRadxFile
///    NEXRAD - message 1 and 31: using NexradRadxFile
///    SIGMET - raw volumes: using SigmetRadxFile
///    UF  -  Universal Format: using UfRadxFile
///    LEOSPHERE -  Leosphere lidar format: using LeoRadxFile
/// \endcode
///
/// Compression is supported as follows:
///
/// \code
///    Cf/Radial NetCDF Format: optionally uses HDF5 compression
///    NCAR/EOL  DORADE Format: uses run-length encoding compression
///                             for 1-byte data
/// \endcode
///

class RadxFile {
  
public:

  /// file type

  typedef enum {
    FILE_FORMAT_UNKNOWN,      ///< NetCDF CF RADIAL
    FILE_FORMAT_CFRADIAL,     ///< NetCDF CF RADIAL
    FILE_FORMAT_DORADE,       ///< NCAR/EOL Dorade
    FILE_FORMAT_UF,           ///< Universal format
    FILE_FORMAT_FORAY_NC,     ///< FORAY NetCDF format
    FILE_FORMAT_NEXRAD_AR2,   ///< NEXRAD ARCHIVE LEVEL 2
    FILE_FORMAT_SIGMET_RAW,   ///< SIGMET RAW DATA VOLUME
    FILE_FORMAT_LEOSPHERE,    ///< LEOSPHERE LIDAR format
    FILE_FORMAT_DOE_NC,       ///< DOE netcdf format
    FILE_FORMAT_MDV_RADIAL,   ///< MDV for radial data
    FILE_FORMAT_NEXRAD_NIDS3, ///< NEXRAD NIDS level 3
    FILE_FORMAT_HRD,          ///< HURRICANE RESEARCH DIVISION
    FILE_FORMAT_TDWR,         ///< TERMINAL DOPPLER WEATHER RADAR
    FILE_FORMAT_GAMIC_HDF5,   ///< HDF5 version of GAMIC data
    FILE_FORMAT_ODIM_HDF5,    ///< OPERA ODIM HDF5
    FILE_FORMAT_GEM_XML,      ///< GEMATRONIK XML
    FILE_FORMAT_NEXRAD_CMD,   ///< NEXRAD CMD OUTPUT
    FILE_FORMAT_TWOLF,        ///< TWOLF LIDAR OUTPUT
    FILE_FORMAT_D3R_NC,       ///< D3R CSU/NASA netcdf format
    FILE_FORMAT_NSSL_MRD,     ///< NSSL MRD format for NOAA aircraft tail radars
    FILE_FORMAT_NOXP_NC,      ///< netcdf for OU NOXP
    FILE_FORMAT_EDGE_NC,      ///< EEC EDGE netcdf format
    FILE_FORMAT_NCXX,         ///< NetCDF CF RADIAL using Ncxx Classes
    FILE_FORMAT_CFRADIAL2,    ///< NetCDF CF RADIAL2
    FILE_FORMAT_CFARR,        ///< Chilbolton radars
    FILE_FORMAT_NIMROD,       ///< UK Met Office Polar NIMROD
    FILE_FORMAT_NOAA_FSL      ///< NOAA Forecast Systems Lab NetCDF
  } file_format_t;

  /// write format for CfRadial
  
  typedef enum {
    NETCDF_CLASSIC,      ///< netcdf 3 classic
    NETCDF4_CLASSIC,     ///< netcdf 4 classic
    NETCDF_OFFSET_64BIT, ///< offset 64-bit
    NETCDF4              ///< full netcdf 4 data model
  } netcdf_format_t;

  /// file naming by time

  typedef enum {
    FILENAME_WITH_START_AND_END_TIMES,
    FILENAME_WITH_START_TIME_ONLY,
    FILENAME_WITH_END_TIME_ONLY
  } file_name_mode_t;

  /// Constructor
  
  RadxFile();
  
  /// Destructor
  
  virtual ~RadxFile();
  
  /// clear the object of all data

  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name Debugging:
  //@{
  
  /// Set normal debugging on/off.
  ///
  /// If set on, basic debugging messages will be printed to stderr
  /// during file operations.

  void setDebug(bool state) { _debug = state; }

  /// Set verbose debugging on/off.
  ///
  /// If set on, verbose debugging messages will be printed to stderr
  /// during file operations.

  void setVerbose(bool state) {
    _verbose = state;
    if (_verbose) _debug = true;
  }

  //@}
  
  //////////////////////////////////////////////////////////////
  /// Check if specified file is a file supported by Radx
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  ////////////////////////////////////////////////////////////
  // Check if path is a NetCDF file
  // Returns true if file is NetCDF, or false otherwise
  
  bool isNetCDF(const string &path);

  ////////////////////////////////////////////////////////////
  // Check if path is an HDF5 file
  // Returns true if file is HDF5, or false otherwise
  
  bool isHdf5(const string &path);

  //////////////////////////////////////////////////////////////
  /// \name Get methods:
  //@{

  /// Get the full file path actually used for reading or writing.
  
  const string &getPathInUse() const { return _pathInUse; }

  /// Get the directory actually used for reading or writing.
  
  const string &getDirInUse() const { return _dirInUse; }
  
  /// Get the set of file paths used on read
  
  const vector<string> &getReadPaths() const {
    return _readPaths;
  }

  /// Get the set of file paths used on write
  
  const vector<string> &getWritePaths() const {
    return _writePaths;
  }

  /// Get the set of file times used on write
  
  const vector<time_t> &getWriteDataTimes() const {
    return _writeDataTimes;
  }

  /// Get file format in use, as an enum.

  file_format_t getFileFormat() const { return _fileFormat; }
  
  /// Get file format in use, as a string.
  
  string getFileFormatAsString() const;

  /// Get file name mode in use, as a string.
  
  string getFileNameModeAsString() const;

  //@}

  ////////////////////////////////////////////
  /// \name Set up object for writing:
  //@{
  
  /// Set file format for writing.
  /// 
  /// NOTE: for reading, the type will be set automatically determined
  /// by the reading method.

  void setFileFormat(file_format_t val) { _fileFormat = val; }
  
  /// Set compression for writing.
  /// The default is compression is true.
  
  void setWriteCompressed(bool state) {
    _writeCompressed = state;
  }
  
  /// Set compression level.
  ///
  /// This applies only to NetCDF files.
  ///
  /// The default is 5, and the valid range is 1 through 9.  The
  /// higher the value, the better the compression ratio, but the more
  /// CPU will be used.

  void setCompressionLevel(int level) {
    _compressionLevel = level;
  }
  
  /// Set to write latest_data_info on write
  
  void setWriteLdataInfo(bool state) {
    _writeLdataInfo = state;
  }
  
  /// Set to write out individual sweeps if appropriate.
  /// If set to true, a volume will be decomposed into individual sweeps
  /// which will then be writted to separate files.
  /// The default is false.

  void setWriteIndividualSweeps(bool val) { _writeIndividualSweeps = val; }

  /// Set the mode to be used to determine the output file name.
  ///
  /// Options are:
  ///   FILENAME_WITH_START_AND_END_TIMES
  ///   FILENAME_WITH_START_TIME_ONLY
  ///   FILENAME_WITH_END_TIME_ONLY
  ///
  /// The default is FILENAME_WITH_START_AND_END_TIMES

  void setWriteFileNameMode(file_name_mode_t val) { _writeFileNameMode = val; }

  /// Set a prefix for the file name - optional.
  /// If not set, the standard prefix be used, determined
  /// by the output format.
  /// Only applies to CfRadial and ODIM.

  void setWriteFileNamePrefix(const string &val)  {
    _writeFileNamePrefix = val; 
  }

  /// Set a suffix for the file name - optional.
  /// Defaults to empty string.
  /// If set, the suffix will appear just before the extension.
  /// Only applies to CfRadial and ODIM.

  void setWriteFileNameSuffix(const string &val)  {
    _writeFileNameSuffix = val; 
  }

  /// Include instrument name in file name
  /// default is true

  void setWriteInstrNameInFileName(bool val) {
    _writeInstrNameInFileName = val; 
  }

  /// Include site name in file name
  /// default is true

  void setWriteSiteNameInFileName(bool val) {
    _writeSiteNameInFileName = val; 
  }

  /// Include subsecs in time part of filename
  /// Default is true
  
  void setWriteSubsecsInFileName(bool val) {
    _writeSubsecsInFileName = val; 
  }
  
  /// Add scan type name to output file name
  /// Default is true
  
  void setWriteScanTypeInFileName(bool val) {
    _writeScanTypeInFileName = val; 
  }

  /// Add volume number to output file name
  /// Default is false
  
  void setWriteVolNumInFileName(bool val) {
    _writeVolNumInFileName = val; 
  }

  /// Use hyphen instead of underscore in datetime part of filename
  /// Default is false
  
  void setWriteHyphenInDateTime(bool val) {
    _writeHyphenInDateTime = val; 
  }

  /// Set the option to write out the data out in native byte ordering.
  ///
  /// The default is for big-endian byte ordering.
  /// 
  /// NOTE: has no effect for CF_RADIAL files,
  /// the option only applies to DORADE and UF

  void setWriteNativeByteOrder(bool val) { _writeNativeByteOrder = val; }

  /// Set the NetCDF format for writing files - CfRadial only.
  /// 
  /// Options are:
  ///  - Nc3File::Netcdf4
  ///  - Nc3File::Classic
  ///  - Nc3File::Offset64Bits
  ///  - Nc3File::Netcdf4Classic
  
  void setNcFormat(netcdf_format_t val) { _ncFormat = val; }
  
  /// Set the option to write assuming that the number of gates varies,
  /// using ragged arrays, even if the number of gates is constant.
  /// Default is FALSE.
  ///
  /// NOTE: only applies to CF_RADIAL files,

  void setWriteForceNgatesVary(bool val) { _writeForceNgatesVary = val; }
  
  /// Set the option to write CfRadial files using the
  /// 'proposed_standard_name' attribute instead of
  /// the usual 'standard_name' attribute.
  /// Default is FALSE.
  ///
  /// NOTE: only applies to CF_RADIAL files,

  void setWriteProposedStdNameInNcf(bool val)
  {
    _writeProposedStdNameInNcf = val;
  }
  
  //////////////////////////////////////////////////////////////
  /// force writing of ragged arrays
  /// even if the number of gates is constant

  /// Copy the write directives from another object.
  ///
  /// Use this to copy only those members related to the options
  /// available for writing, i.e. those in this section.

  void copyWriteDirectives(const RadxFile &other);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  /// Clear all write requests, reset defaults.
  
  virtual void clearWrite();
  
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
  /// \name Set up object for reading:
  //@{
  
  /// Clear all read requests, reset defaults.
  
  virtual void clearRead();
  
  /// Add a field for reading.
  ///
  /// If not called after clearRead(), all fields in the file will be read.
  
  void addReadField(const string &field_name);

  /// Is a field name needed on read?
  ///
  /// Returns true or false.
  /// If no fields have been specified on read, this method
  /// will always return true because all fields are required.

  bool isFieldRequiredOnRead(const string &field_name) const;
  
  /// Set the sweep angle limits to be used on read.
  ///
  /// This will limit the sweep data to be read.
  ///
  /// If strict limits are not set (see below), 1 sweep is guaranted
  /// to be returned. Even if no sweep lies within the limits the closest
  /// sweep will be returned.
  ///
  /// If strict limits are set (which is the default), then only data from
  /// sweeps within the limits will be included.

  void setReadFixedAngleLimits(double min_fixed_angle,
                               double max_fixed_angle);

  /// Set the sweep number limits to be used on read.
  ///
  /// This will limit the sweep data to be read.
  ///
  /// If strict limits are not set (see below), 1 sweep is guaranted
  /// to be returned. Even if no sweep lies within the limits the closest
  /// sweep will be returned.
  ///
  /// If strict limits are set (which is the default), then only data from
  /// sweeps within the limits will be included.

  void setReadSweepNumLimits(int min_sweep_num,
                             int max_sweep_num);

  /// Set flag to indicate strict checking on the sweep angle/num limits.
  /// Applies to both setReadFixedAngleLimits() and
  /// setReadFixedAngleLimits().
  ///
  /// The default is for strict limits on:
  ///   _readStrictAngleLimits = true.
  ///
  /// If strict limits are off, if no sweep meets the limits set, the
  /// closest sweep is chosen so that at least 1 sweep is returned in
  /// the data read in.  If strict limits are set, then the read will
  /// return an error if no sweep lies within the defined limits.

  void setReadStrictAngleLimits(bool val);

  /// Set aggregation into volume on read.
  /// If true, individual sweep files will be aggregated into a
  /// volume when the files are read, by checking for the 
  /// volume number in the input list
  
  void setReadAggregateSweeps(bool val);

  /// Set to ignore idle mode on read.
  /// If true, data from IDLE scan mode will not be read
  /// Default is true.
  
  void setReadIgnoreIdleMode(bool val);

  /// Set to ignore transitions on read.
  /// If true, rays for which the antenna is in transtion
  /// not be read.
  /// Default is false.
  
  void setReadIgnoreTransitions(bool val);

  /// Set the number of transition rays to include as a margin.
  /// Sometimes the transition flag is turned on too early in
  /// a transition, on not turned off quickly enough after a transition.
  /// If you set this to a number greater than 0, that number of rays
  /// will be included at each end of the transition, i.e. the
  /// transition will effectively be shorter at each end by this
  /// number of rays

  void setReadTransitionNraysMargin(int val);

  /// Set flag to indicate removal of rays, on read, for which
  /// all fields have missing data at all gates.
  /// Defaults to false.
  /// If true, individual sweep files will be aggregated into a
  /// volume when the files are read
  
  void setReadRemoveRaysAllMissing(bool val);

  /// Set max range for data on read.
  /// Gates beyond this range are removed.
  
  void setReadMaxRangeKm(double maxRangeKm);

  /// Set flag to indicate that we want to preserve the
  /// sweep details in the file we read in.
  /// This generally applies to NEXRAD data - by default we
  /// consolidate sweeps by combining split-cut sweeps
  /// into a single sweep.
  /// If this flag is true, we leave the sweeps unchanged.
  /// Defaults to false.
  
  void setReadPreserveSweeps(bool val);

  /////////////////////////////////////////////////////////////////
  /// Set flag to indicate that we want to compute the
  /// sweep angles using the VCP tables.
  /// This applies to NEXRAD data - by default we
  /// use the fixed angles in the file.
  /// If this flag is true, the sweeps are computed using
  /// the VCP tables.
  /// Defaults to false.

  void setReadComputeSweepAnglesFromVcpTables(bool val);

  /// Set flag to indicate removal of rays for long range sweeps.
  /// This generally only applies to NEXRAD data, in which
  /// the long range sweeps only include DBZ data.
  /// Defaults to false.
  
  void setReadRemoveLongRange(bool val);

  /// Set flag to indicate removal of rays for short range sweeps.
  /// This generally only applies to NEXRAD data, in which
  /// the long range sweeps only include DBZ data.
  /// Defaults to false.

  void setReadRemoveShortRange(bool val);

  /// Set flag to indicate we should only read the main metadata,
  /// including the sweep and field information, and NOT read the
  /// rays and data fields.
  /// Defaults to false.

  void setReadMetadataOnly(bool val);

  /// Set flag to indicate we should only read the times only,
  /// and set the start and end times.
  /// Defaults to false.

  void setReadTimesOnly(bool val);

  /// Set radar number to be read in.
  /// Only applies to file formats with data from more than 1 radar
  /// in a file.
  /// For example, HRD files have data from both the lower fuselage
  /// (LF = 1) radar and the tail radar (TA = 2).
  ///
  /// Defaults to -1. Mostly not used. If not set, then for HRD data
  /// the tail radar data will be returned.

  void setRadarNumOnRead(int val);

  /// Set flag to indicate we should change the latitude sign
  /// on read - applies to Rapic files
  /// Defaults to false.

  void setChangeLatitudeSignOnRead(bool val);

  /// Set flag to indicate we should apply the georeference
  /// information on read, to compute earth-relative azimuth and
  /// elevation.
  /// Only applies to moving radars.
  /// Defaults to false.

  void setApplyGeorefsOnRead(bool val);

  /// Copy the read directives from another object.
  ///
  /// Use this to copy only those members related to the options
  /// available for reading, i.e. those in this section.

  void copyReadDirectives(const RadxFile &other);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Set read time search modes
  //@{
  
  /////////////////////////////////////////////////////////////////
  // setReadModeInterval
  //
  // Set the time list so that it finds all of the valid data
  // times between the start and end times.
  
  void setReadModeInterval(RadxTime start_time,
                           RadxTime end_time)
  {
    _readTimeList.setModeInterval(start_time, end_time);
  }
  
  /////////////////////////////////////////////////////////////////
  // setReadModeFirst
  //
  // Set the time list mode so that it finds the first available data time
  
  void setReadModeFirst()
  {
    _readTimeList.setModeFirst();
  }
  
  /////////////////////////////////////////////////////////////////
  // SetModeLast
  //
  // set the time list mode so that it finds the last available data time
  
  void setReadModeLast()
  {
    _readTimeList.setModeLast();
  }
  
  /////////////////////////////////////////////////////////////////
  // SetModeClosest
  //
  // set the time list mode so that it finds the closest available data time
  // to the search time within the search margin
  
  void setReadModeClosest(RadxTime search_time, double time_margin)
  {
    _readTimeList.setModeClosest(search_time, time_margin);
  }
  
  /////////////////////////////////////////////////////////////////
  // SetModeFirstBefore
  //
  // set the time list mode so that it finds the first available data time
  // before the search time within the search margin
  
  void setReadModeFirstBefore(RadxTime search_time, double time_margin)
  {
    _readTimeList.setModeFirstBefore(search_time, time_margin);
  }
  
  /////////////////////////////////////////////////////////////////
  // SetModeFirstAfter
  //
  // set the time list mode so that it finds the first available data time
  // after the search time within the search margin
  
  void setReadModeFirstAfter(RadxTime search_time, double time_margin)
  {
    _readTimeList.setModeFirstAfter(search_time, time_margin);
  }
  
  /////////////////////////////////////////////////////////////////
  // setReadRaysInInterval
  //
  // Set up for reading rays from a time interval.
  
  void setReadRaysInInterval
    (const RadxTime &start_time,
     const RadxTime &end_time,
     double dwellSecs = 0.0,
     RadxField::StatsMethod_t statsMethod = RadxField::STATS_METHOD_MIDDLE) {

    _readRaysInInterval = true;
    _readRaysStartTime = start_time;
    _readRaysEndTime = end_time;
    _readTimeList.setModeInterval(start_time, end_time);
    _readDwellSecs = dwellSecs;
    _readDwellStatsMethod = statsMethod;
    
  }

  ////////////////////////////////
  // print read time list request
  
  void printTimeListReadRequest(ostream &out)
  {
    _readTimeList.printRequest(out);
  }

  string getTimeListReadRequestString() {
    return _readTimeList.getRequestString();
  }

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Perform the read:
  //@{
  
  /////////////////////////////////////////////////////////////////////
  /// Read in data file from specified path, and load up volume
  /// object.
  ///
  /// Returns 0 on success, -1 on failure.
  ///
  /// If error occurs, use getErrStr() to get error string.
  
  virtual int readFromPath(const string &path,
                           RadxVol &vol);

  //////////////////////////////////////////////////////////
  // Read in data file from a series of specified paths and
  // aggregate into a single volume object.
  // Returns 0 on success, -1 on failure
  // Use getErrStr() if error occurs
  
  int aggregateFromPaths(const vector<string> &paths,
                         RadxVol &vol);
  
  /////////////////////////////////////////////////////////
  /// Read in data file from specified directory.
  /// NOTE: before calling this function, you must first call one
  ///       of the setReadMode....() functions to set up the search
  /// Load up volume object.
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  int readFromDir(const string &dir,
                          RadxVol &vol);

  //@}

  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Clear error string.
  
  void clearErrStr();

  /// Get the Error String.
  ///
  /// The contents are only meaningful if an error has returned.
  
  string getErrStr() const { return _errStr; }
  
  //@}

  /// file name path delimiter
  
  static const char *PATH_SEPARATOR;
  
  //////////////////////////////////////////////////////////////
  /// \name Utility methods for directories and files:

  /// Make directory, given the path
  
  static int makeDir(const string &dir);

  /// Make directory recursively, given the path
  
  static int makeDirRecurse(const string &dir);
  
  /// Given a dir, returns a string with a temporary file path.
  ///
  /// The intended use is to provide a tmp file path to which a file
  /// is written prior to renaming to the final name.
  /// The tmp path is in the same directory as the final path.
  /// If tmpFileName is non-empty, it is used for the file name.
  /// If it is empty, the name is 'tmp.pid.timesec.timeusec.tmp',
  /// where pid is determined using the getpid() function.
  
  static string tmpPathFromDir(const string &dir,
                               const string &tmpFileName);

  /// Given a file path, fills a string with a
  /// temporary file path.
  ///
  /// The intended use is to provide a tmp file path to which a file
  /// is written prior to renaming to the final name.
  /// The tmp path is in the same directory as the final file.
  /// If tmpFileName is non-empty, it is used for the file name.
  /// If it is empty, the name is 'tmp.pid.timesec.timeusec.tmp',
  /// where pid is determined using the getpid() function.
  
  static string tmpPathFromFilePath(const string &finalFilePath,
                                    const string &tmpFileName);
  
  //@}

  ////////////////////////
  /// \name Printing:
  //@{
  
  /// Print file in use etc for this object

  virtual void print(ostream &out) const;

  /// Print read settings on this object

  virtual void printReadRequest(ostream &out) const;

  /// Print write settings on this object

  virtual void printWriteRequest(ostream &out) const;

  /// Print data in file, in native format.
  ///
  /// Only applicable to DORADE and UF.
  /// For netcdf, use ncdump to inspect file.
  ///
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  //@}

protected:
  
  // error string

  string _errStr; ///< Error string is set on read or write error
  
  // debug state
  
  bool _debug; ///< normal debug flag
  bool _verbose; ///< verbose debug flag

  // file format

  file_format_t _fileFormat; ///< current file format
  
  // read parameters

  vector<string> _readFieldNames; ///< list of file names if specified

  bool _readFixedAngleLimitsSet; ///< are fixed angle limits set?
  double _readMinFixedAngle; ///< min fixed angle on read
  double _readMaxFixedAngle; ///< max fixed angle on read
  bool _readSweepNumLimitsSet; ///< are sweep number limits set?
  int _readMinSweepNum; ///< min sweep number on read
  int _readMaxSweepNum; ///< max sweep number on read
  bool _readStrictAngleLimits; ///< strict checking in sweep angle limits
  bool _readIgnoreIdleMode; ///< ignore rays in IDLE mode
  bool _readIgnoreTransitions; ///< ignore rays in antenna transitions
  int _readTransitionNraysMargin;
  bool _readAggregateSweeps; ///< aggregate sweeps into a volume on read
  bool _readRemoveRaysAllMissing; ///< check all fields/gates on each ray, do not add ray if all data is missing
  bool _readSetMaxRange; ///< remove gates beyond a given max range
  double _readMaxRangeKm; ///< max read range in km
  bool _readPreserveSweeps; ///< preserve sweeps as they are read in
  bool _readComputeSweepAnglesFromVcpTables; ///< compute Sweep angles from VCP tables
  bool _readRemoveLongRange; ///< remove long range scans on read
  bool _readRemoveShortRange; ///< remove short range scans on read
  bool _readMetadataOnly; ///< only read sweep metadata, not rays
  bool _readTimesOnly; ///< only read start and end times
  int _readSetRadarNum; ///< set the radar number, for files with more
                        ///< than 1 radar, e.g. HRD files
  int _readRadarNum; ///< radar number - see setRadarNum
  bool _readChangeLatitudeSign; ///< change latitude sign on read
  bool _readApplyGeorefs; ///< apply georefs on read

  bool _readRaysInInterval;
  RadxTime _readRaysStartTime;
  RadxTime _readRaysEndTime;
  double _readDwellSecs; ///< length of dwell when reading rays in interval
  RadxField::StatsMethod_t _readDwellStatsMethod; ///< method for computing stata when reading rays in interval

  RadxTimeList _readTimeList;

  // write parameters

  bool _writeNativeByteOrder; ///< write out in byte order native to host
  bool _writeForceNgatesVary; ///< force write with ragged arrays
  file_name_mode_t _writeFileNameMode; ///< used for computing write path
  string _writeFileNamePrefix;  ///< prefix for computing file name
  string _writeFileNameSuffix;  ///< suffix for computing file name
  bool _writeInstrNameInFileName; ///< include instrument name in file name
  bool _writeSiteNameInFileName; ///< include site name in file name
  bool _writeSubsecsInFileName; ///< include subseconds in file name
  bool _writeScanTypeInFileName; ///< include scan type in file name
  bool _writeVolNumInFileName; ///< include volume number in file name
  bool _writeHyphenInDateTime; ///< use a hyphen instead of underscore in datetime part of file names
  bool _writeIndividualSweeps; ///< write individual sweeps, if applicable
  bool _writeCompressed; ///< write out compressed? CfRadial only
  int _compressionLevel; ///< write compression level
  bool _writeLdataInfo; ///< write latest_data_info on write
  
  ///< Use 'proposed_standard_name' instead of 'standard_name' in CfRadial files
  bool _writeProposedStdNameInNcf;

  // netcdf format for writing - CfRadial only
  
  netcdf_format_t _ncFormat;

  // dir and path in use - for reading/writing
  
  string _dirInUse; ///< directory in use
  string _pathInUse; ///< path in use
  vector<string> _readPaths; ///< list of file paths for reads
  vector<string> _writePaths; ///< list of file paths for writes
  vector<time_t> _writeDataTimes; ///< list of data times for writes

  /// volume for reading

  RadxVol *_readVol; ///< volume to which data is read in
  
  /// methods

  /// Initialize for reading
  
  void _initForRead(const string &path,
                    RadxVol &vol);

  /// Read in data file from netCDF file

  int _readFromPathNetCDF(const string &path, RadxVol &vol);
  
  /// Read in data file from HDF5 file

  int _readFromPathHdf5(const string &path, RadxVol &vol);
  
  /// Read in data file from other types

  int _readFromPathOther(const string &path, RadxVol &vol);
  
  /// add integer value to error string, with label

  void _addErrInt(string label, int iarg,
                  bool cr = true);

  /// add double value to error string, with label

  void _addErrDbl(string label, double darg,
                  string format = "%g", bool cr = true);

  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);

  /// read in rays in specified time interval
  /// must be overridden for particular formats

  int _doReadRaysInInterval(const string &dir,
                            RadxVol &vol);

  /// print native for netCDF

  int _printNativeNetCDF(const string &path, ostream &out,
                         bool printRays, bool printData);

  /// print native for Hdf5

  int _printNativeHdf5(const string &path, ostream &out,
                       bool printRays, bool printData);
  
  /// print native for other types

  int _printNativeOther(const string &path, ostream &out,
                        bool printRays, bool printData);
  
  /// check is supported for various file classes

  bool _isSupportedNetCDF(const string &path);
  bool _isSupportedHdf5(const string &path);
  bool _isSupportedOther(const string &path);

private:

};

#endif
