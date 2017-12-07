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
// SigmetRadxFile.hh
//
// SigmetRadxFile object
//
// Support for SIGMET RAW data format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2011
//
///////////////////////////////////////////////////////////////

#ifndef SigmetRadxFile_HH
#define SigmetRadxFile_HH

#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/SigmetData.hh>

class RadxVol;
class RadxRay;
class RadxBuf;
class RadxGeoref;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR SIGMET RAW FILE FORMAT
///
/// This subclass of RadxFile handles I/O for Sigmet files.

class SigmetRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  SigmetRadxFile();
  
  /// Destructor
  
  virtual ~SigmetRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a Sigmet file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if the specifed file is a Sigmet file.
  ///
  /// Returns true on success, false on failure.
  
  bool isSigmet(const string &path);
  
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
  
  bool isSwapped() { return _sigmetIsSwapped; }

  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  //////////////////////////////////////////////////////////////
  /// Writing in Sigmet format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);
  
  //////////////////////////////////////////////////////////////
  /// Writing in Sigmet format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
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
  
  /// Print Sigmet data file in native format.
  ///
  /// Returns 0 on success, -1 on failure.
  ///
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  //@}

protected:
private:

  // constants

  // binary angles to float
  static const double _angleConversion;
  
  // file handle
  
  FILE *_file;
  
  // objects to be set on read
  
  bool _sigmetIsSwapped;
  bool _isDualPol;
  bool _isHrdTailRadar; // P3 tail radar
  bool _isHrdLfRadar; // Lower fuselage radar
  bool _hasSubsecTime;

  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;
  RadxTime _sweepStartTime;

  static int _volumeNumber;
  int _sweepNumber; // starts at 1
  int _sweepIndex; // starts at 0
  int _nSweeps;

  int _nFields;
  int _nRaysSweep;
  int _nGates;
  int _nBytesData;

  // sigmet headers read in

  prod_header_t _prodHdr;
  ingest_header_t _inHdr;
  int _nbytesExtendedHdr;
  raw_product_header_t _rawHdr;
  ingest_data_header_t _inDatHdr0;
  vector<ingest_data_header_t> _inDatHdrs;

  // record to hold raw data as it is read in

  Radx::ui08 _record[RAW_RECORD_LEN];

  // derived values

  double _fixedAngle;
  double _pulseWidthUs;
  double _wavelengthCm;
  double _wavelengthM;
  double _prf;
  double _prtSec;
  double _nyquist;
  bool _dualPrt;
  double _prtRatio;
  double _unambigRangeKm;

  // input data buffer

  RadxBuf _inBuf;

  // field data buffer

  RadxBuf _dataBuf;
  vector<int> _nBytesRayField;

  // ray info in buffer
  
  class RayInfo {
  public:
    Radx::ui08 *offset;
    ray_header_t hdr;
    vector<int> nBytesField;
    int nBytesTotal;
    RayInfo() {
      offset = NULL;
      memset(&hdr, 0, sizeof(hdr));
      nBytesTotal = 0;
    }
  };

  vector<RayInfo> _rayInfo;

  // position and wind methods for HRD extended headers

  bool _hrdSourcesSet;
  typedef enum {
    USE_IRS_POSN, USE_GPS_POSN, USE_AAMPS_POSN
  } hrd_posn_source_t;
  hrd_posn_source_t _hrdPosnSource;
  
  typedef enum {
    USE_IRS_WIND, USE_AAMPS_WIND
  } hrd_wind_source_t;
  hrd_wind_source_t _hrdWindSource;

  // functions

  // private methods
  
  void _clearRays();

  int _openRead(const string &path);
  void _close();

  int _finalizeReadVolume();

  int _readRecord();
  int _readHeaders(bool doPrint,
                   ostream &out);
  int _readSweepData(bool doPrint,
                     ostream &out);
  int _processSweep(bool doPrint,
                    bool printData,
                    ostream &out);
  int _handleExtendedHeader(RadxRay *ray,
                            int rayIndex,
                            const RayInfo &rayInfo,
                            const Radx::ui08 *rayPtr,
                            int fieldLen,
                            bool doPrint,
                            ostream &out);

  void _setGeoref(const ray_header_t &rayHdr,
                  const ext_header_ver1 &xh1,
                  RadxRay *ray,
                  RadxGeoref &georef);
  void _setGeoref(const ray_header_t &rayHdr,
                  const hrd_tdr_ext_header_t &hrd,
                  RadxRay *ray,
                  RadxGeoref &georef);

  void _computeAzElTail(RadxRay *ray,
                        const RadxGeoref &georef);

  void _computeAzElLf(RadxRay *ray,
                      const RadxGeoref &georef);

  void _setRayInfo();
  void _setVolMetaData();
  void _setRayMetadata(RadxRay &ray, const ray_header_t &rayHdr);

  double _getFixedAngle(int sweepIndex, double el, double az);
  void _setHrdSources();

  string _fieldId2Name(int fieldId);
  string _fieldId2Units(int fieldId);
  void _fieldId2ScaleBias(int fieldId, double &scale, double &bias);
  void _convertBias2Signed(int fieldId, double scale, double &bias);
  void _checkDualPol(int fieldId);

  double _binAngleToDouble(Radx::ui16 binAngle);
  double _binAngleToDouble(Radx::si16 binAngle);
  double _binAngleToDouble(Radx::ui32 binAngle);
  double _binAngleToDouble(Radx::si32 binAngle);

  void _printCharArray(ostream &out, const char *buf, int len);
  string _time2Str(const sigmet_time_t &time);
  string _label2Str(const char *label, int maxLen);

  void _print(const sigmet_id_hdr_t &hdr, ostream &out);
  void _print(const prod_header_t &hdr, ostream &out);
  void _print(const prod_conf_t &prod, ostream &out);
  void _print(const prod_end_t &end, ostream &out);
  void _print(const ingest_header_t &hdr, ostream &out);
  void _print(const ingest_conf_t &conf, ostream &out);
  void _print(const task_conf_t &conf, ostream &out);
  void _print(const task_sched_info_t &info, ostream &out);
  void _print(const string &label,
              const dsp_data_mask_t &mask, ostream &out);
  void _print(const task_dsp_info_t &info, ostream &out);
  void _print(const task_calib_info_t &info, ostream &out);
  void _print(const task_range_info_t &info, ostream &out);
  void _print(const task_scan_info_t &info, ostream &out);
  void _print(const task_misc_info_t &info, ostream &out);
  void _print(const task_end_info_t &info, ostream &out);
  void _print(const raw_product_header_t &hdr, ostream &out);
  void _print(const ingest_data_header_t &hdr, ostream &out);
  void _print(const ray_header_t &hdr, ostream &out);
  void _print(const ext_header_ver0 &hdr, ostream &out);
  void _print(const ext_header_ver1 &hdr, ostream &out);
  void _print(const ext_header_ver2 &hdr, ostream &out);
  void _print(const hrd_tdr_ext_header_t &hdr, ostream &out);

  void _swap(Radx::si16 *vals, int n);
  void _swap(Radx::ui16 *vals, int n);
  void _swap(Radx::si32 *vals, int n);
  void _swap(Radx::ui32 *vals, int n);
  void _swap(Radx::fl32 *vals, int n);
  void _swap(sigmet_id_hdr_t &val);
  void _swap(sigmet_time_t &val);
  void _swap(prod_header_t &val);
  void _swap(prod_conf_t &val);
  void _swap(prod_end_t &val);
  void _swap(ingest_header_t &val);
  void _swap(ingest_conf_t &val);
  void _swap(task_conf_t &val);
  void _swap(task_sched_info_t &val);
  void _swap(dsp_data_mask_t &val);
  void _swap(task_dsp_info_t &val);
  void _swap(task_calib_info_t &val);
  void _swap(task_range_info_t &val);
  void _swap(scan_info_union_t &val);
  void _swap(task_scan_info_t &val);
  void _swap(task_misc_info_t &val);
  void _swap(task_end_info_t &val);
  void _swap(raw_product_header_t &val);
  void _swap(ingest_data_header_t &val);
  void _swap(ray_header_t &val);
  void _swap(ext_header_ver0 &val);
  void _swap(ext_header_ver1 &val);
  void _swap(ext_header_ver2 &val);
  void _swap(hrd_tdr_ext_header_t &val);

};

#endif
