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
// D3rNcRadxFile.hh
//
// D3rNcRadxFile object
//
// NetCDF data for radar radial data NASA/CSU D3R netcdf format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2015
//
///////////////////////////////////////////////////////////////

#ifndef D3rNcRadxFile_HH
#define D3rNcRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>
#include <Radx/NetcdfClassic.hh>

class RadxField;
class RadxVol;
class RadxSweep;
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NASA/CSU D3R RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class D3rNcRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  D3rNcRadxFile();
  
  /// Destructor
  
  virtual ~D3rNcRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a DOE file
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a DOE file.
  /// Returns true on success, false on failure
  
  bool isD3rNc(const string &path);
    
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
  
  NetcdfClassic _file;

  // dimensions

  NcDim *_timeDim;  // "Radial"
  NcDim *_rangeDim; // "Gate"
  
  // times
  
  NcVar *_timeVar;
  vector<int> _iTimes;
  vector<double> _dTimes;
  bool _rayTimesIncrease;
  size_t _nTimesInFile;
  
  // range

  vector<double> _rangeKm;
  size_t _nRangeInFile;
  bool _gateSpacingIsConstant;
  RadxRangeGeom _geom;
  RadxRemap _remap;

  // ray variables
  
  NcVar *_azimuthVar;
  NcVar *_elevationVar;
  NcVar *_gateWidthVar;
  NcVar *_startRangeVar;

  vector<double> _azimuth;
  vector<double> _elevation;
  vector<double> _gateWidth;
  vector<double> _startRangeInt;
  vector<double> _startRange;

  string _azimuthUnits, _elevationUnits;
  string _gateWidthUnits, _startRangeUnits;

  NcVar *_startGateVar;
  NcVar *_gcfStateVar;
  NcVar *_polarizationModeVar;
  NcVar *_prtModeVar;

  vector<int> _startGate;
  vector<int> _gcfState;
  vector<int> _polarizationMode;
  vector<int> _prtMode;
  
  NcVar *_txFreqShortVar;
  NcVar *_txFreqMediumVar;
  NcVar *_txLengthShortVar;
  NcVar *_txLengthMediumVar;
  
  vector<double> _txFreqShort;
  vector<double> _txFreqMedium;
  vector<double> _txLengthShort;
  vector<double> _txLengthMedium;

  string _txFreqShortUnits, _txFreqMediumUnits;
  string _txLengthShortUnits, _txLengthMediumUnits;
  
  NcVar *_txPowerHShortVar;
  NcVar *_txPowerHMediumVar;
  NcVar *_txPowerVShortVar;
  NcVar *_txPowerVMediumVar;

  vector<double> _txPowerHShort;
  vector<double> _txPowerHMedium;
  vector<double> _txPowerVShort;
  vector<double> _txPowerVMedium;

  string _txPowerHShortUnits, _txPowerHMediumUnits;
  string _txPowerVShortUnits, _txPowerVMediumUnits;

  NcVar *_txPhaseHShortVar;
  NcVar *_txPhaseHMediumVar;
  NcVar *_txPhaseVShortVar;
  NcVar *_txPhaseVMediumVar;
  
  vector<double> _txPhaseHShort;
  vector<double> _txPhaseHMedium;
  vector<double> _txPhaseVShort;
  vector<double> _txPhaseVMedium;

  string _txPhaseHShortUnits, _txPhaseHMediumUnits;
  string _txPhaseVShortUnits, _txPhaseVMediumUnits;

  NcVar *_noiseSourcePowerHShortVar;
  NcVar *_noiseSourcePowerVShortVar;

  vector<double> _noiseSourcePowerHShort;
  vector<double> _noiseSourcePowerVShort;

  string _noiseSourcePowerHShortUnits, _noiseSourcePowerVShortUnits;

  NcVar *_rxGainHShortVar;
  NcVar *_rxGainHMediumVar;
  NcVar *_rxGainVShortVar;
  NcVar *_rxGainVMediumVar;
  
  vector<double> _rxGainHShort;
  vector<double> _rxGainHMedium;
  vector<double> _rxGainVShort;
  vector<double> _rxGainVMedium;
  
  string _rxGainHShortUnits, _rxGainHMediumUnits;
  string _rxGainVShortUnits, _rxGainVMediumUnits;

  NcVar *_zdrBiasAppliedShortVar;
  NcVar *_zdrBiasAppliedMediumVar;
  
  vector<double> _zdrBiasAppliedShort;
  vector<double> _zdrBiasAppliedMedium;

  string _zdrBiasAppliedShortUnits, _zdrBiasAppliedMediumUnits;

  // global attributes

  string _netcdfRevision;
  string _gmaptdRevision;
  string _configRevision;
  string _campaignName;
  string _radarName;
  
  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeKm;

  int _numGates;
  int _scanId;
  int _scanType;
  int _sweepNumber;
  double _refTimeSecsFile;
  
  string _title;
  string _institution;
  string _references;
  string _source;
  string _history;
  string _comment;
  string _statusXml;
  
  string _siteName;
  string _scanName;
  // int _scanId;
  string _instrumentName;
  
  int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // rays to be added to volume

  vector<RadxRay *> _rays;

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes();
  int _readRangeVariables();
  void _clearRayVariables();
  int _readRayVariables();
  int _createRays(const string &path);
  int _readFieldVariables(bool metaOnly);
  
  int _readRayVar(NcVar* &var, const string &name, string &units,
                  vector<double> &vals, bool required = true);
  int _readRayVar(NcVar* &var, const string &name, string &units,
                  vector<int> &vals, bool required = true);
  int _readRayVar(NcVar* &var, const string &name,
                  vector<int> &vals, bool required = true);
  
  NcVar* _getRayVar(const string &name, bool required);

  int _addFl64FieldToRays(NcVar* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addFl32FieldToRays(NcVar* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);

  int _loadReadVolume();
  void _computeFixedAngles();

};

#endif
