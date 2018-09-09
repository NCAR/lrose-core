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
// ForayNcRadxFile.hh
//
// ForayNcRadxFile object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////

#ifndef ForayNcRadxFile_HH
#define ForayNcRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRcalib.hh>
#include <Ncxx/Nc3xFile.hh>

class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class ForayNcRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  ForayNcRadxFile();
  
  /// Destructor
  
  virtual ~ForayNcRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a FORAY NC file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a FORAY NetCDF file
  /// Returns true on success, false on failure
  
  bool isForayNc(const string &path);
    
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
  
  /// Print data in file, in native format.
  ///
  /// This is not really applicable to netcdf files, and will
  /// return an error.
  ///
  /// For netcdf, use ncdump to inspect file.
  ///
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  /// Get the date and time from a dorade file path.
  /// returns 0 on success, -1 on failure

  int getTimeFromPath(const string &path, RadxTime &rtime);

  //@}

protected:
private:

  // string length

  static const int SHORT_STRING_LEN = 32;
  static const int LONG_STRING_LEN = 80;
  typedef char ShortString_t[SHORT_STRING_LEN];
  typedef char LongString_t[LONG_STRING_LEN];
  static const double missingDouble;
  static const float missingFloat;
  static const int missingInt;

  // volume for writing

  time_t _volStartTimeSecs;
  RadxVol *_writeVol; ///< volume from which data is written
  
  // format version
  
  string _version;
  
  // netcdf file
  
  Nc3xFile _file;
  string _tmpPath;

  // NetCDF dimensions
  
  Nc3Dim *_TimeDim;
  Nc3Dim *_maxCellsDim;
  Nc3Dim *_numSystemsDim;
  Nc3Dim *_fieldsDim;
  Nc3Dim *_short_stringDim; 
  Nc3Dim *_long_stringDim;

  // NetCDF scalar variables

  Nc3Var *_field_namesVar;
  Nc3Var *_volume_start_timeVar;
  Nc3Var *_base_timeVar;
  Nc3Var *_Fixed_AngleVar;
  Nc3Var *_Range_to_First_CellVar;
  Nc3Var *_Cell_Spacing_MethodVar;
  Nc3Var *_Cell_SpacingVar;
  Nc3Var *_Nyquist_VelocityVar;
  Nc3Var *_Unambiguous_RangeVar;
  Nc3Var *_LatitudeVar;
  Nc3Var *_LongitudeVar;
  Nc3Var *_AltitudeVar;

  // NetCDF numSystems variables

  Nc3Var *_Radar_ConstantVar;
  Nc3Var *_rcvr_gainVar;
  Nc3Var *_ant_gainVar;
  Nc3Var *_sys_gainVar;
  Nc3Var *_bm_widthVar;
  Nc3Var *_pulse_widthVar;
  Nc3Var *_band_widthVar;
  Nc3Var *_peak_pwrVar;
  Nc3Var *_xmtr_pwrVar;
  Nc3Var *_noise_pwrVar;
  Nc3Var *_tst_pls_pwrVar;
  Nc3Var *_tst_pls_rng0Var;
  Nc3Var *_tst_pls_rng1Var;
  Nc3Var *_WavelengthVar;
  Nc3Var *_PRFVar;
  Nc3Var *_ant_gain_h_dbVar;
  Nc3Var *_ant_gain_v_dbVar;
  Nc3Var *_xmit_power_h_dbmVar;
  Nc3Var *_xmit_power_v_dbmVar;
  Nc3Var *_two_way_waveguide_loss_h_dbVar;
  Nc3Var *_two_way_waveguide_loss_v_dbVar;
  Nc3Var *_two_way_radome_loss_h_dbVar;
  Nc3Var *_two_way_radome_loss_v_dbVar;
  Nc3Var *_receiver_mismatch_loss_dbVar;
  Nc3Var *_radar_constant_hVar;
  Nc3Var *_radar_constant_vVar;
  Nc3Var *_noise_hc_dbmVar;
  Nc3Var *_noise_vc_dbmVar;
  Nc3Var *_noise_hx_dbmVar;
  Nc3Var *_noise_vx_dbmVar;
  Nc3Var *_receiver_gain_hc_dbVar;
  Nc3Var *_receiver_gain_vc_dbVar;
  Nc3Var *_receiver_gain_hx_dbVar;
  Nc3Var *_receiver_gain_vx_dbVar;
  Nc3Var *_base_1km_hc_dbzVar;
  Nc3Var *_base_1km_vc_dbzVar;
  Nc3Var *_base_1km_hx_dbzVar;
  Nc3Var *_base_1km_vx_dbzVar;
  Nc3Var *_sun_power_hc_dbmVar;
  Nc3Var *_sun_power_vc_dbmVar;
  Nc3Var *_sun_power_hx_dbmVar;
  Nc3Var *_sun_power_vx_dbmVar;
  Nc3Var *_noise_source_power_h_dbmVar;
  Nc3Var *_noise_source_power_v_dbmVar;
  Nc3Var *_power_measure_loss_h_dbVar;
  Nc3Var *_power_measure_loss_v_dbVar;
  Nc3Var *_coupler_forward_loss_h_dbVar;
  Nc3Var *_coupler_forward_loss_v_dbVar;
  Nc3Var *_zdr_correction_dbVar;
  Nc3Var *_ldr_correction_h_dbVar;
  Nc3Var *_ldr_correction_v_dbVar;
  Nc3Var *_system_phidp_degVar;
  Nc3Var *_calibration_data_presentVar;

  // NetCDF time variables

  Nc3Var *_time_offsetVar;
  Nc3Var *_AzimuthVar;
  Nc3Var *_ElevationVar;
  Nc3Var *_clip_rangeVar;

  // netCDF data fields

  vector<Nc3Var *> _dataFieldVars;

  // times

  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;

  // attributes

  string _title;
  string _institution;
  string _references;
  string _source;
  string _history;
  string _comment;

  string _conventions;
  string _siteName;
  string _instrumentName;
  string _instrumentTypeStr;
  string _scanModeStr;
  string _projectName;

  bool _dataIsUnsigned;

  int _volumeNumber;
  int _scanNumber;
  int _nSamples;

  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::SweepMode_t _sweepMode;

  // scalar variables

  int _base_time;

  int _Cell_Spacing_Method;
  int _calibration_data_present;

  double _Fixed_Angle;
  double _Range_to_First_Cell;
  double _Cell_Spacing;
  double _Nyquist_Velocity;
  double _Unambiguous_Range;
  double _Latitude;
  double _Longitude;
  double _Altitude;
  double _Radar_Constant;
  double _Wavelength;
  double _PRF;

  double _rcvr_gain;
  double _ant_gain;
  double _sys_gain;
  double _bm_width;
  double _pulse_width;
  double _band_width;
  double _peak_pwr;
  double _xmtr_pwr;
  double _noise_pwr;
  double _tst_pls_pwr;
  double _tst_pls_rng0;
  double _tst_pls_rng1;

  double _ant_gain_h_db;
  double _ant_gain_v_db;
  double _xmit_power_h_dbm;
  double _xmit_power_v_dbm;
  double _two_way_waveguide_loss_h_db;
  double _two_way_waveguide_loss_v_db;
  double _two_way_radome_loss_h_db;
  double _two_way_radome_loss_v_db;
  double _receiver_mismatch_loss_db;
  double _radar_constant_h;
  double _radar_constant_v;
  double _noise_hc_dbm;
  double _noise_vc_dbm;
  double _noise_hx_dbm;
  double _noise_vx_dbm;
  double _receiver_gain_hc_db;
  double _receiver_gain_vc_db;
  double _receiver_gain_hx_db;
  double _receiver_gain_vx_db;
  double _base_1km_hc_dbz;
  double _base_1km_vc_dbz;
  double _base_1km_hx_dbz;
  double _base_1km_vx_dbz;
  double _sun_power_hc_dbm;
  double _sun_power_vc_dbm;
  double _sun_power_hx_dbm;
  double _sun_power_vx_dbm;
  double _noise_source_power_h_dbm;
  double _noise_source_power_v_dbm;
  double _power_measure_loss_h_db;
  double _power_measure_loss_v_db;
  double _coupler_forward_loss_h_db;
  double _coupler_forward_loss_v_db;
  double _zdr_correction_db;
  double _ldr_correction_h_db;
  double _ldr_correction_v_db;
  double _system_phidp_deg;

  // gate geometry
  
  int _nGates;
  vector<double> _rangeKm;
  double _startRangeKm, _gateSpacingKm;
  bool _gateSpacingIsConstant;

  // sweep number range if limited

  vector<int> _sweepNums;

  // storage

  RadxRangeGeom _geom;
  RadxRcalib _rCal;
  vector<RadxRay *> _rays;

  // standard strings

  const static char* ADD_OFFSET;
  const static char* LONG_NAME;
  const static char* FILL_VALUE;
  const static char* MISSING_VALUE;
  const static char* SCALE_FACTOR;
  const static char* SECS_SINCE_JAN1_1970;
  const static char* STANDARD_NAME;
  const static char* TIME;
  const static char* TIME_OFFSET;
  const static char* UNITS;

  // read methods

  int _readAggregatePaths(const string &path);
  int _readPath(const string &path);
  int _readSweepInfo(const string &path);
  int _setSweepNums();
  int _getVolumePaths(const string &path, vector<string> &paths);
  void _addToPathList(const string &dir,
                      int volNum,
                      int minHour, int maxHour,
                      vector<string> &paths);

  int _readDimensions();
  void _readGlobalAttributes();
  int _createRays();
  int _readScalarVariables();
  int _readCellDistance();
  int _readRayVariables();
  int _readRayVar(const string &name, vector<double> &vals);
  int _readRayVar(const string &name, vector<int> &vals);
  Nc3Var* _getRayVar(const string &name);
  int _readFieldVariables();
  int _addFl64FieldToRays(Nc3Var* var, int nPoints,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName);
  int _addFl32FieldToRays(Nc3Var* var, int nPoints,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName);
  int _addSi32FieldToRays(Nc3Var* var, int nPoints,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          double scale, double offset);
  int _addSi16FieldToRays(Nc3Var* var, int nPoints,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          double scale, double offset);
  int _addSi08FieldToRays(Nc3Var* var, int nPoints,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          double scale, double offset);

  int _finalizeReadVolume();

  // write methods
  
  int _writeSweepToDir(RadxVol &vol, int sweepNum, const string &dir,
                       bool addDaySubDir, bool addYearSubDir);

  int _writeSweepToPath(RadxVol &vol, const string &path);
  int _closeOnError(const string &caller);
  
  string _computeFileName(int volNum, int nSweeps,
                          double fixedAngle, string instrumentName,
                          string scanType,
                          int year, int month, int day,
                          int hour, int min, int sec,
                          int millisecs);

  int _addGlobalAttributes();
  int _addDimensions();
  int _addTimeVariables();
  int _addFieldNamesVariable();
  int _addScalarVariables();
  int _addNumSystemsVariables();
  int _addTimeArrayVariables();
  int _addDataFieldVariables();

  int _addTimeVar(Nc3Var* &var,
                  const string &name,
                  const string &longName,
                  const string &units);

  int _addVar(Nc3Var* &var,
              Nc3Type ncType,
              const string &name,
              const string &longName,
              const string &units);

  int _addVar(Nc3Var* &var,
              Nc3Dim *dim,
              Nc3Type ncType,
              const string &name,
              const string &longName,
              const string &units);

  int _addTimeOffsetVar(Nc3Var* &var,
                        Nc3Dim *dim,
                        const string &name,
                        const string &longName,
                        const string &units);

  int _writeFieldNamesVariable();
  int _writeScalarVariables();
  int _writeRadarVariables();
  int _writeCalibVariables(const RadxRcalib &calib);
  int _writeTimeOffsetVariable();
  int _writeAngleVariables();
  int _writeClipRangeVariable();
  int _writeDataFieldVariables();
  int _writeNumSystemsVar(Nc3Var *var, float val);
  int _writeCalibDataPresent(int val);

  Nc3Type _getNc3Type(Radx::DataType_t dtype);
  Nc3File::FileFormat _getFileFormat(RadxFile::netcdf_format_t format);

  int _setCompression(Nc3Var *var);
  
};

#endif
