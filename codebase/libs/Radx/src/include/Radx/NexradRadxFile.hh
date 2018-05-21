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
// NexradRadxFile.hh
//
// NexradRadxFile object
//
// NEXRAD radar data file handling
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////

#ifndef NexradRadxFile_HH
#define NexradRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/NexradData.hh>
#include <Radx/NexradVcp.hh>

class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class NexradRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  NexradRadxFile();
  
  /// Destructor
  
  virtual ~NexradRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a FORAY NC file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a NEXRAD file
  /// Returns true on success, false on failure
  /// Side effect:
  ///   sets _isBzipped if file is internally compressed
  
  bool isNexrad(const string &path);
    
  /// Check if file is bzipped (as in LDM)
  /// Make this call after isNexrad()
  
  bool isBzipped() const { return _isBzipped; }
    
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

  // converting angles

  const static double _angleMult;
  const static double _rateMult;

  // PRT table

  const static double _prtTable[5][8];

  // volume for writing

  const RadxVol *_writeVol; ///< volume from which data is written
  
  // handles
  
  FILE *_file;
  bool _isBzipped;
  string _tmpPath;

  // times

  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;

  // volume info

  int _volumeNumber;
  int _vcpNum;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  NexradData::message_31_vol_t _vol;
  NexradData::message_31_elev_t _elev;
  NexradData::message_31_radial_t _radial;
  bool _isDualPol;
  bool _isMsg1;
  
  // scalar variables

  string _instrumentName;
  string _siteName;
  
  double _latitude;
  double _longitude;
  double _altitudeM;
  double _sensorHtAglM;

  double _systemZdr;
  double _systemPhidp;
  double _atmosAttenDbPerKm;

  double _xmitFreqGhz;
  double _nyquistVelocity;
  double _unambiguousRangeKm;
  int _prtIndex;
  int _prtNum;

  double _beamWidthH;
  double _beamWidthV;
  double _antGainHDb;
  double _antGainVDb;

  double _dbz0;
  double _powerHDbm;
  double _powerVDbm;

  double _maxAbsVel;
  double _meanPulseWidthUsec;

  // Adapt data
  NexradData::adaptation_data_t _adap;

  // vcp info
  NexradData::VCP_hdr_t _vcp;

  int _vcpPatternType;
  int _vcpPatternNumber;
  int _vcpNElev;
  double _vcpVelRes;
  bool _vcpShortPulse;
  vector<NexradData::ppi_hdr_t> _vcpPpis;

  // range geometry for long and short range

  double _startRangeKmLong, _gateSpacingKmLong;
  double _startRangeKmShort, _gateSpacingKmShort;

  // writing

  int _msgSeqNum;

  // private methods
  
  int _openRead(const string &path);
  int _openWrite(const string &path);
  void _close();

  // VCPs

  NexradVcp _vcp11;
  NexradVcp _vcp12;
  NexradVcp _vcp21;
  NexradVcp _vcp31;
  NexradVcp _vcp32;
  NexradVcp _vcp35;
  NexradVcp _vcp121;
  NexradVcp _vcp211;
  NexradVcp _vcp212;
  NexradVcp _vcp215;
  NexradVcp _vcp221;
  
  // read methods

  RadxRay *_handleMessageType31(const RadxBuf &msgBuf);
  RadxRay *_handleMessageType1(const RadxBuf &msgBuf);

  void _setRayProps(int sweepNum, double elevation, RadxRay *ray);

  void _handleVcpHdr(const RadxBuf &msgBuf);
  int _handleAdaptationData(const RadxBuf &msgBuf);

  int _readMessage(NexradData::msg_hdr_t &msgHdr, RadxBuf &buf,
                   bool printHeaders, ostream &out);

  void _handleFieldType1(RadxRay *ray,
                         const string &fieldName,
                         const string &units,
                         const NexradData::message_1_t &hdr,
                         const RadxBuf &msgBuf,
                         size_t fieldOffset);

  void _handleDataBlockType31(RadxRay *ray,
                              const RadxBuf &msgBuf,
                              int blockNum,
                              size_t byteOffset);

  void _handleVolBlockType31(RadxRay *ray,
                             const RadxBuf &msgBuf,
                             int blockNum,
                             size_t byteOffset);

  void _handleElevBlockType31(RadxRay *ray,
                              const RadxBuf &msgBuf,
                              int blockNum,
                              size_t byteOffset);
  
  void _handleRadialBlockType31(RadxRay *ray,
                                const RadxBuf &msgBuf,
                                int blockNum,
                                size_t byteOffset);

  void _handleFieldType31(RadxRay *ray,
                          const RadxBuf &msgBuf,
                          int blockNum,
                          size_t byteOffset);

  void _removeLongRangeRays();
  void _removeShortRangeRays();
  void _checkIsLongRange(RadxRay *ray);
  int _finalizeReadVolume();
  void _computeFixedAngles();

  void _printVcp(const RadxBuf &msgBuf, ostream &out);
  void _printAdaptationData(const RadxBuf &msgBuf, ostream &out);
  void _printClutterFilterBypassMap(const RadxBuf &msgBuf, ostream &out);
  void _printClutterFilterMap(const RadxBuf &msgBuf, ostream &out);
  void _printMessageType1(const RadxBuf &msgBuf, ostream &out,
                          bool printRays, bool printData);

  void _printMessageType31(const RadxBuf &msgBuf, ostream &out,
                           bool printRays, bool printData);

  void _printDataBlockType31(const RadxBuf &msgBuf,
                             ostream &out,
                             int blockNum,
                             size_t byteOffset,
                             bool printRays,
                             bool printData);
  
  void _printVolBlockType31(const RadxBuf &msgBuf,
                            ostream &out,
                            int blockNum,
                            size_t byteOffset);
  
  void _printElevBlockType31(const RadxBuf &msgBuf,
                             ostream &out,
                             int blockNum,
                             size_t byteOffset);
  
  void _printRadialBlockType31(const RadxBuf &msgBuf,
                               ostream &out,
                               int blockNum,
                               size_t byteOffset);
  
  void _printFieldType31(const RadxBuf &msgBuf,
                         ostream &out,
                         int blockNum,
                         size_t fieldOffset,
                         bool printData);

  void _printFieldData(ostream &out, const string &fieldName,
                       int nGates, const double *data) const;

  void _printPacked(ostream &out, int count, double val) const;

  // write methods

  string _computeFileName(int volNum,
                          string instrumentName,
                          string scanType,
                          int year, int month, int day,
                          int hour, int min, int sec);

  int _writeMetaDataHdrs(const RadxVol &vol);

  int _writeMessage(void *buf,
                    int nBytesBuf,
                    int messageType,
                    time_t timeSecs,
                    int nanoSecs);
  
  int _writeMsg31(const RadxVol &vol,
                  int sweepIndex, const RadxSweep &sweep,
                  int rayIndex, const RadxRay &ray);

  int _loadField(const RadxRay &ray,
                 const string &name, const string &names,
                 int byteWidth,
                 double scale, double offset,
                 NexradData::message_31_field_t &fhdr,
                 RadxBuf &buf);
   
  void _computeMaxAbsVel(const RadxVol &vol);
  void _computeMeanPulseWidth(const RadxVol &vol);
  void _setPrtIndexes(double prtSec);
  
  int _unzipFile(const string &path);
  void _removeTmpFiles();
  
  void _loadSignedData(const vector<Radx::ui08> &udata,
                       vector<Radx::si08> &sdata,
                       bool interp);

  void _loadSignedData(const vector<Radx::ui16> &udata,
                       vector<Radx::si16> &sdata,
                       bool interp);

  void _interp1kmGates(int nGates,
                       Radx::si08 *idata);

  void _interp1kmGates(int nGates,
                       Radx::si16 *idata);

private:

};

#endif
