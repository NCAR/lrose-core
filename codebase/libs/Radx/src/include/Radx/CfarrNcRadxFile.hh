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
// CfarrNcRadxFile.hh
//
// CfarrNcRadxFile object
//
// NetCDF data for radar radial data in CFAR netcdf format
// CFARR = Chilbolton Facility for Atmospheric and Radio Research
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2017
//
///////////////////////////////////////////////////////////////

#ifndef CfarrNcRadxFile_HH
#define CfarrNcRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>
#include <Ncxx/Nc3xFile.hh>

class RadxField;
class RadxVol;
class RadxSweep;
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR DOE NETCDF RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for DOE netcdf files.

class CfarrNcRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  CfarrNcRadxFile();
  
  /// Destructor
  
  virtual ~CfarrNcRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a DOE file
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a DOE file.
  /// Returns true on success, false on failure
  
  bool isCfarrNc(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified directory.
  ///
  /// This class is for reading only, to the write methods
  /// simply print a warning that they are not implemented.
  ///
  /// Returns 0 on success, -1 on failure

  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified path.
  ///
  /// This class is for reading only, to the write methods
  /// simply print a warning that they are not implemented.
  ///
  /// Returns 0 on success, -1 on failure
  
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
  /// \name Checks
  //@{
  
  /// Check if number of gates vary in volume
  
  // bool getNGatesVary() const { return _nGatesVary; }
    
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

  // netcdf file
  
  Nc3xFile _file;

  // udunits

  // Udunits2 _udunits;
  
  // dimensions

  Nc3Dim *_timeDim;
  Nc3Dim *_rangeDim;
  Nc3Dim *_sweepDim;

  // times
  
  Nc3Var *_timeVar;
  vector<double> _dTimes;
  time_t _refTimeSecsFile;
  bool _rayTimesIncrease;
  size_t _nTimesInFile;
  
  // range

  Nc3Var *_rangeVar;
  vector<double> _rangeKm;
  size_t _nRangeInFile;
  bool _gateSpacingIsConstant;
  RadxRangeGeom _geom;
  RadxRemap _remap;

  // georef variables
  
  Nc3Var *_latitudeVar;
  Nc3Var *_longitudeVar;
  Nc3Var *_heightVar;
  
  double _latitudeDeg;
  double _longitudeDeg;
  double _heightKm;

  // scalar variables
  
  Nc3Var *_frequencyVar;
  Nc3Var *_prfVar;
  Nc3Var *_beamwidthHVar;
  Nc3Var *_beamwidthVVar;
  Nc3Var *_antennaDiameterVar;
  Nc3Var *_pulsePeriodVar;
  Nc3Var *_transmitPowerVar;
  
  double _frequencyGhz;
  double _prfHz;
  double _beamwidthHDeg;
  double _beamwidthVDeg;
  double _antennaDiameterM;
  double _pulsePeriodUs;
  double _transmitPowerW;
  
  // ray variables

  Nc3Var *_azimuthVar;
  Nc3Var *_elevationVar;

  vector<double> _azimuths;
  vector<double> _elevations;
  
  // sweep variables

  Nc3Var *_sweepVar;
  vector<RadxSweep *> _sweeps;

  // global attributes

  int _ADC_bits_per_sample_attr;
  int _ADC_channels_attr;
  int _delay_clocks_attr;
  int _experiment_id_attr;
  int _file_number_attr;
  int _pulses_per_daq_cycle_attr;
  int _pulses_per_ray_attr;
  int _samples_per_pulse_attr;
  int _scan_number_attr;
  int _year_attr;
  int _month_attr;
  int _day_attr;


  float _cable_losses_attr;
  float _extra_attenuation_attr;
  float _max_angle_attr;
  float _max_range_attr;
  float _min_angle_attr;
  float _min_range_attr;
  float _radar_constant_attr;
  float _receiver_gain_attr;
  float _scan_angle_attr;
  float _scan_velocity_attr;

  string _British_National_Grid_Reference_attr;
  string _Conventions_attr;
  string _operator_attr;
  string _radar_attr;
  string _references_attr;
  string _scan_datetime_attr;
  string _scantype_attr;

  string _title;
  string _institution;
  string _references;
  string _source;
  string _history;
  string _comment;
  string _statusXml;
  
  string _siteName;
  string _scanName;
  int _scanId;
  string _instrumentName;

  int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // rays to be added to volume

  vector<RadxRay *> _raysToRead;
  vector<RadxRay *> _raysValid;

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes();
  int _readRangeVariable();
  int _readPositionVariables();
  int _readSweepVariables();
  int _readScalarVariables();
  void _clearRayVariables();
  int _readRayVariables();
  int _createRays(const string &path);
  int _readFieldVariables(bool metaOnly);

  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<int> &vals, bool required = true);
  
  Nc3Var* _getRayVar(const string &name, bool required);

  int _addFl64FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addFl32FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);

  int _loadReadVolume();
  void _computeFixedAngles();

};

#endif
