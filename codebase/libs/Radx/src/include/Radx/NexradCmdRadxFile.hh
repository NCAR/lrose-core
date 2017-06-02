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
// NexradCmdRadxFile.hh
//
// NexradCmdRadxFile object
//
// NetCDF data for radar radial data in early DOE netcdf format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#ifndef NexradCmdRadxFile_HH
#define NexradCmdRadxFile_HH

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
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class NexradCmdRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  NexradCmdRadxFile();
  
  /// Destructor
  
  virtual ~NexradCmdRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a CfRadialFile
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a NEXRAD CMD file.
  /// Returns true on success, false on failure
  
  bool isNexradCmd(const string &path);
    
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

  // dimensions

  Nc3Dim *_timeDim;
  Nc3Dim *_rangeDim;
  Nc3Dim *_rangeMaskDim;

  // times
  
  bool _rayTimesIncrease;
  size_t _nTimesInFile;
  
  // range

  Nc3Var *_rangeVar;
  vector<double> _rangeKm;
  size_t _nRangeInFile;
  size_t _nRangeMask;
  bool _gateSpacingIsConstant;
  RadxRangeGeom _geom;
  RadxRemap _remap;

  // georef variables
  
  // Nc3Var *_latitudeVar;
  // Nc3Var *_longitudeVar;
  // Nc3Var *_altitudeVar;
  
  // double _latitudeDeg;
  // double _longitudeDeg;
  // double _altitudeKm;

  // ray variables

  Nc3Var *_startAzVar;
  Nc3Var *_endAzVar;
  Nc3Var *_startElVar;
  Nc3Var *_endElVar;
  Nc3Var *_startTimeVar;
  Nc3Var *_endTimeVar;
  Nc3Var *_hNoiseVar;
  Nc3Var *_vNoiseVar;
  Nc3Var *_dbz0Var;

  vector<double> _startAz;
  vector<double> _endAz;
  vector<double> _rayAz;
  
  vector<double> _startEl;
  vector<double> _endEl;
  vector<double> _rayEl;

  vector<double> _startTime;
  vector<double> _endTime;
  vector<double> _rayTime;

  vector<double> _hNoise;
  vector<double> _vNoise;
  vector<double> _dbz0;
  
  // global attributes

  string _title_attr;
  string _history_attr;

  // double _pulseLenNs;
  // double _frequencyGhz;
  // double _wavelengthM;
  // double _nyquistMps;
  // double _prfHz;
  // double _antennaHtAgl;
  // double _antennaDiameterInches;

  string _title;
  // string _institution;
  // string _references;
  // string _source;
  string _history;
  // string _comment;
  string _statusXml;
  
  // string _siteName;
  // string _scanName;
  // int _scanId;
  // string _instrumentName;
  
  int _volumeNumber;
  int _sweepNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // storing ray information for reading from file
  
  class RayInfo {
  public:
    size_t indexInFile;
    RadxRay *ray;
  };
  vector<RayInfo> _raysToRead;

  // rays to be added to volume

  vector<RadxRay *> _raysValid;

  int _readDimensions();
  int _readGlobalAttributes();
  int _readStatusVariables();
  int _readRangeVariable();
  // int _readPositionVariables();
  void _clearRayVariables();
  int _readRayVariables();
  int _createRays(const string &path);
  int _readFieldVariables();

  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<int> &vals, bool required = true);
  
  Nc3Var* _getRayVar(const string &name, bool required);
  
  int _addFl64FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &longName);

  int _addFl32FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &longName);

  int _addSi08FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &longName);

  int _loadReadVolume();
  void _computeFixedAngles();

};

#endif
