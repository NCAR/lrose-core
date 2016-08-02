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
// UfRadxFile.hh
//
// UfRadxFile object
//
// UF support for radar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef UfRadxFile_HH
#define UfRadxFile_HH

#include <string>
#include <vector>
#include <cmath>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/UfData.hh>

class RadxVol;
class RadxRay;
class RadxBuf;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR UNIVERSAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for UF files.

class UfRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  UfRadxFile();
  
  /// Destructor
  
  virtual ~UfRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a UF file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if the specifed file is a UF file.
  ///
  /// Returns true on success, false on failure.
  
  bool isUf(const string &path);
  
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
  
  bool isSwapped() { return _ufIsSwapped; }

  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Field name translation:
  //@{

  /// Add an entry to the name translation table.
  ///
  /// Since UF only allows 2-character field name, this class provides
  /// a default translation table for converting longer names to UF
  /// names, and vice versa. The table also provides usits for common
  /// UF field names.
  ///
  /// For details on the default name table, see the method
  /// UfRadxFile::_createDefaultNameTable().
  ///
  /// This method allows you to add an entry to the translation
  /// table. If you call clearNameTable() first, before adding
  /// entries, you will remove the default table and start with an
  /// empty table.
  
  void addToNameTable(const UfData::NameEntry &entry) {
    _nameTable.push_back(entry);
  }

  /// clear translation table
  
  void clearNameTable() { _nameTable.clear(); }

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
  
  /// Option to expand Uf field names to longer names when reading a
  /// UF file.
  ///
  /// The default is false.
  
  void setExpandUfNames(bool val) { _expandUfNames = val; }

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
  
  /// Print UF data file in native format.
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

protected:
private:

  // volume for writing

  const RadxVol *_writeVol; ///< volume from which data is written
  
  // objects to be set on read
  
  bool _ufIsSwapped;

  int _volumeNumber;
  string _radarName;
  string _siteName;
  string _projName;
  string _tapeName;
  string _genFacility;

  vector<double> _latitudes;
  vector<double> _longitudes;
  vector<double> _altitudes;

  double _pulseWidth;
  double _wavelengthM;
  double _prtSec;
  double _nyquist;
  double _unambigRange;
  double _beamWidthH;
  double _beamWidthV;
  double _receiverBandWidth;
  double _receiverGain;
  double _dbz0;
  double _radarConstant;
  double _noisePower;
  double _peakPower;
  double _antennaGain;

  vector<RadxRay *> _rays;

  // UF structures

  UfData::mandatory_header_t _manHdr;
  UfData::optional_header_t _optHdr;
  bool _optFound;
  UfData::data_header_t _dataHdr;
  vector<UfData::field_info_t> _fieldInfo;
  vector<UfData::field_header_t> _fieldHdrs;
  vector<string> _fieldNames;
  vector<RadxBuf *> _fieldBuffers;
  
  // table for translating names to/from UF.

  UfData::NameTable _nameTable;
  bool _expandUfNames; /* option to expand Uf field names to
                        * longer names when reading UF */
  
  // file handle

  FILE *_file;

  // private methods
  
  void _createDefaultNameTable();

  void _clearRays();
  void _clearUfStructs();

  int _openRead(const string &path);
  int _openWrite(const string &path);
  void _close();

  int _disassembleReadRecord(const void *buf, int nBytes);
  
  int _handleReadRecord();
  
  int _loadReadVolume();
  
  int _loadWriteRecordFromRay(const RadxVol &vol, int rayNumber);

  int _writeRecord();

  void _printRecord(ostream &out,
                    bool print_headers,
                    bool print_data);
  
};

#endif
