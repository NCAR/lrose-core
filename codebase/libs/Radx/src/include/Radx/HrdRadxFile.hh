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
// HrdRadxFile.hh
//
// Support for HRD data format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2013
//
///////////////////////////////////////////////////////////////

#ifndef HrdRadxFile_HH
#define HrdRadxFile_HH

#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/HrdData.hh>

class RadxVol;
class RadxRay;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR HRD FILE FORMAT
///
/// This subclass of RadxFile handles I/O for Hrd files.

class HrdRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  HrdRadxFile();
  
  /// Destructor
  
  virtual ~HrdRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a Hrd file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if the specifed file is a Hrd file.
  ///
  /// Returns true on success, false on failure.
  
  bool isHrd(const string &path);
  
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
  
  bool isSwapped() { return _hrdIsSwapped; }

  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  //////////////////////////////////////////////////////////////
  /// Writing in Hrd format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);
  
  //////////////////////////////////////////////////////////////
  /// Writing in Hrd format is not supported.
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
  
  /// Print Hrd data file in native format.
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
  
  // raw input buffer

  size_t _inBufSize;
  RadxBuf _inBuf;

  // data buffer

  int _dataLen;
  unsigned char *_dataBuf;

  // objects to be set on read
  
  bool _hrdIsSwapped;
  hrd_header_t _hdr;
  hrd_data_rec_header_t _dataRecHdr;
  bool _isTailRadar;
  hrd_radar_info_t *_radar;

  // time limits

  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;

  // derived values

  static int _volumeNumber;
  int _nGates;
  int _nSamples;
  double _pulseWidthUs;
  double _wavelengthM;
  double _prf;
  double _prt0, _prt1;
  double _nyquist;
  bool _staggered;
  double _staggerRatio;

  double _gateSpacingKm;
  double _startRangeKm;

  double _latitude;
  double _longitude;
  double _altitudeM;

  // keeping track of sweeps

  int _prevSweepNum;
  double _prevElev;
  
  // private methods
  
  int _openRead(const string &path);
  void _close();

  int _readRec();
  int _getRecType();
  void _loadHeaderRec();
  void _loadDataRec();
  void _setVolMetaData();

  void _swap(hrd_header_t &hdr);
  void _swap(hrd_radar_info_t &radar);
  void _swap(hrd_data_rec_header_t &rec);
  void _swap(hrd_ray_header_t &ray);
  void _swap(Radx::si16 *vals, int n);
  void _swap(Radx::ui16 *vals, int n);

  void _handleRays();

  void _handleLfRay(const hrd_ray_header_t &rayHdr,
                    const unsigned char *dataBuf,
                    int dataLen);

  void _handleTaRay(const hrd_ray_header_t &rayHdr,
                    const unsigned char *dataBuf,
                    int dataLen);
    
  void _setRayMetadata(RadxRay &ray, const hrd_ray_header_t &rayHdr);

  void _print(const hrd_header_t &hdr,
              ostream &out);

  void _print(const hrd_radar_info_t &radar,
              ostream &out);
  
  void _print(const hrd_data_rec_header_t &rec,
              ostream &out);
  
  void _print(const hrd_ray_header_t &ray,
              ostream &out);
  
  void _printRays(bool printData,
                  ostream &out);
  
  void _printLfData(const unsigned char *dataBuf,
                    int dataLen,
                    ostream &out);
  
  void _printTaData(const unsigned char *dataBuf,
                    int dataLen,
                    bool haveDbz,
                    bool haveVel,
                    bool haveWidth,
                    ostream &out);
  
  void _printFieldData(const string &label,
                       const Radx::fl32 *data,
                       int nGates,
                       ostream &out);
  
  void _uncompress(const unsigned char *dataBuf,
                   int dataLen,
                   RadxBuf &buf);
  
  Radx::fl32 _dbzVal(int ival);
  
  Radx::fl32 _velVal(int ival);
  
  Radx::fl32 _widthVal(int ival);
  
  void _printPacked(ostream &out, int count,
                    Radx::fl32 val, Radx::fl32 missing);
  
  double _getAngle(int binaryAngle);

};

#endif
