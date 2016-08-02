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
// RapicRadxFile.hh
//
// Support for reading RAPIC data format
// from the Australian BOM
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2013
//
///////////////////////////////////////////////////////////////

#ifndef RapicRadxFile_HH
#define RapicRadxFile_HH

#include <string>
#include <vector>
#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>

class RadxVol;
class PPIField;
class Linebuff;
class ScanParams;
class sRadl;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR RAPIC FILE FORMAT
///
/// This subclass of RadxFile handles I/O for Rapic files.

class RapicRadxFile : public RadxFile

{
  
public:
  
  /// Constructor
  
  RapicRadxFile();
  
  /// Destructor
  
  virtual ~RapicRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a Rapic file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if the specifed file is a Rapic file.
  ///
  /// Returns true on success, false on failure.
  
  bool isRapic(const string &path);
  
  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  //////////////////////////////////////////////////////////////
  /// Writing in Rapic format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);
  
  //////////////////////////////////////////////////////////////
  /// Writing in Rapic format is not supported.
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
  
  /// Print Rapic data file in native format.
  ///
  /// Returns 0 on success, -1 on failure.
  ///
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  //@}

protected:
private:

  // file handle
  
  FILE *_file;

  // scan description parameters

  ScanParams *_scanParams;

  typedef struct {
    int scan_num;
    int sweep_num;
    int station_id;
    int flag1;
    double elev_angle;
    int field_num;
    int n_fields;
    int flag2;
    int flag3;
    char time_str[32];
  } scan_description_t;

  vector<scan_description_t> _scanList;

  // objects to be set on read
  
  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;

  static int _volumeNumber;
  int _nFields;

  bool _dualPol;

  bool _scanListComplete;
  int _nScansFull;
  int _nFieldsRead;
  vector<double> _elevList;
  vector<PPIField *> _ppiFields;
  int _maxRaysPpi;
  int _maxGatesPpi;

  // private methods

  int _finalizeReadVolume();
  void _setVolMetaData();

  int _processImage(const char *file_path, Linebuff &lineBuf, int imageNum);
  int _loadPpi(int sweepNum, Linebuff &line_buf);
  void _clearPpiFields();
  void _addRaysPpi(int sweepNum);
  int _decodeRadial(Linebuff &lineBuf, sRadl &radial,
		    bool &isBinary, int rLevels);

  int _findImageStart(Linebuff &lineBuf);
  int _findImageEnd(Linebuff &lineBuf);

  void _clearScanList();
  int _addToScanList(Linebuff &lineBuf);
  void _printScanList(ostream &out);

  int _loadScanParams(Linebuff &lineBuf);
  int _checkScanParams();
  
  int _addRays(int ppi_num,
               int maxGates,
               int maxBeams);

  bool _azLessThan(double az1, double az2);

};

#endif
