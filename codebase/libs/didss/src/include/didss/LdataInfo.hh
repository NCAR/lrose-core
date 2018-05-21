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

#ifndef LDATA_INFO_HH
#define LDATA_INFO_HH

////////////////////////////////////////////////////////////////////
// LdataInfo.hh
//
// LdataInfo class.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Jan 1999
//
////////////////////////////////////////////////////////////////////
//
// This class handles latest data time information. It is
// intended for use by programs which must poll to see when
// new data has been added to some directory or server.
//
// The class reads/writes a number of latest data information files:
//
//    _latest_data_info: old ASCII format, deprecated
//    _latest_data_info.xml: XML format, flat file
//    _latest_data_info.stat |_ old and XML format in
//    _latest_data_info.buf  |  file message queue (FMQ)
//
// The write methods create the data directory if it does not exist.
//
// Environment variables:
//
//  LDATA_NO_WRITE -    if 'true', do not write _latest_data_info files.
//                      Default is false.
//  LDATA_FMQ_ACTIVE  - if 'false', set _useFmq to false. fmq will not be used.
//                      Default is true.
//  LDATA_FMQ_NSLOTS -  number of slots in fmq.
//                      Default is 2500.
//
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <toolsa/umisc.h>
#include <toolsa/fmq.h>
#include <toolsa/MemBuf.hh>
#include <didss/DsURL.hh>
#include <dataport/port_types.h>
using namespace std;

#ifdef LDATA_INFO_FILE_NAME
#undef LDATA_INFO_FILE_NAME
#endif
#define LDATA_INFO_FILE_NAME "latest_data_info"

#ifdef LDATA_INFO_STR_LEN
#undef LDATA_INFO_STR_LEN
#endif
#define LDATA_INFO_STR_LEN 64

#define LDATA_NSLOTS_DEFAULT 2500
#define LDATA_BUFSIZE_PER_SLOT 500

class LdataInfo {

public:
  
  // heartbeat function 

  typedef void (*heartbeat_t)(const char *label);

  ////////////////
  // Info struct
  //
  // This is intended to be a minimalist struct - only essential
  // data for storing in the file.

  typedef struct {

    // latest unix time (secs)

    ti32 latestTime;
    
    // number of forecasts in data set -
    // usually 0. There are n_fcasts
    // fcast_times following this
    // struct in the file.

    si32 nFcasts;

    // File extension if applicable.
    // Only applicable for file-based queries.
    // Defaults to "none"
    
    char dataFileExt[LDATA_INFO_STR_LEN];
  
    // User info strings, for user program use.
    // Only applicable for file-based queries.
    // Defaults to "none"
    
    char userInfo1[LDATA_INFO_STR_LEN];
    char userInfo2[LDATA_INFO_STR_LEN];
    
  } info_t;

  //////////////////////
  // Default constructor
  //
  // The default directory is '.'
  // This may be overridden by calling setDir() and setDirFromUrl().
  //
  LdataInfo();

  /////////////////////////////////////////
  // constructor - with directory supplied.
  //
  // dataDir: directory for _latest_data_info files
  // debug: set true to activate debug prints.
  // file_name: specify if you do not wish to use the default
  //            file names (_latest_data_info*).
  //
  LdataInfo(const string &dataDir,
	    bool debug = false,
	    const char *file_name = LDATA_INFO_FILE_NAME);

  //////////////////////////////////////////////////
  // constructor from URL.
  //
  // Sets the directory from the URL.
  //
  // For example, if the URL is mdvp:://::mdv/sat/vis, the
  // directory will be mdv/sat/vis.
  //
  // For other args, see other constructors.
  //
  // You can also use the default constructor, and setDirFromUrl().
  //
  LdataInfo(const DsURL &url,
	    bool debug = false,
	    const char *file_name = LDATA_INFO_FILE_NAME);

  ////////////////////
  // copy constructor
  //
  // Note: only copies state information, not FILE pointers or other
  // status.
  
  LdataInfo(const LdataInfo &orig);

  ////////////////////////
  // assignment (and copy)
  //
  // Note: only copies state information, not FILE pointers or other
  // status.
  
  LdataInfo &operator=(const LdataInfo &other);
  LdataInfo &copy(const LdataInfo &other);

  //////////////
  // destructor

  virtual ~LdataInfo();

  //////////////////////////////////////
  // setDir()
  //
  // Sets the directory from supplied string.
  //
  // returns 0 on success, -1 on failure
  
  virtual int setDir(const string &dataDir);

  //////////////////////////////////////
  // setDirFromUrl()
  //
  // Sets the directory from the URL or supplied string.
  // The directory is set to the file portion of the URL.
  //
  // For example, if the URL is mdvp:://::mdv/sat/vis, the
  // directory will be mdv/sat/vis.
  //
  // Returns 0 on success, -1 on failure
  
  virtual int setDirFromUrl(const DsURL &url);
  virtual int setDirFromUrl(const string &urlStr);
  
  ////////////
  // setDebug
  //
  // Turn on debug printing - most useful for read problems.

  void setDebug(bool debug = true) { _debug = debug; }

  ////////////////
  // setLatestTime
  //
  // Set the latest time.
  // This is the valid time for obs data, generate time for forecast data.
  
  void setLatestTime(time_t latest_time);

  ////////////////////////////////////////////////////
  // Set the path of the latest file to be written,
  // relative to the directory in which the _latest_data_file
  // is written.

  void setRelDataPath(const string &rel_path) { _relDataPath = rel_path; }
  void setRelDataPath(const char *rel_path) { _relDataPath = rel_path; }

  ////////////////////////////////////////////////////////
  // Set the name of the application writing the data

  void setWriter(const string &writer) { _writer = writer; }
  void setWriter(const char *writer) { _writer = writer; }

  //////////////
  // setFileExt
  //
  // Set the data file extension.

  void setDataFileExt(const string &ext) { _dataFileExt = ext; }
  void setDataFileExt(const char *ext) { _dataFileExt = ext; }

  /////////////////////////////////////////////////////////////
  // Set object given the file path and the top-level
  // directory for the data set.
  //
  // From dir and path, computes:
  //
  //   * valid time: see setLatestTime()
  //   * relative file path: see setRelDataPath()
  //   * file extension: see setDataFileExt()
  //
  // Returns:
  //   0 on success, -1 on failure.
  
  int setPathAndTime(const string &dir,
                     const string &filePath);
  
  //////////////
  // setDataType
  //
  // Set the data type string
  // If not set, the object will try to guess it from tha data path.

  void setDataType(const string &dtype) { _dataType = dtype; }
  void setDataType(const char *dtype) { _dataType = dtype; }

  ///////////////
  // setUserInfo1
  //
  // Set the user info 1, to communicate user information to the reader.

  void setUserInfo1(const string &info_str) { _userInfo1 = info_str; }
  void setUserInfo1(const char *info_str) { _userInfo1 = info_str; }

  ///////////////
  // setUserInfo2
  //
  // Set the user info 2, to communicate user information to the reader.

  void setUserInfo2(const string &info_str) { _userInfo2 = info_str; }
  void setUserInfo2(const char *info_str) { _userInfo2 = info_str; }

  ///////////////
  // setIsFcast
  //
  // Sets whether this is a forecast or not.
  // If false, latest_time represents the valid time.
  // If true, latest_time representts the forecast generate time,
  //          and valid_time = gen_time + lead_time.
  
  void setIsFcast(bool fcast = true) { _isFcast = fcast; }

  ///////////////
  // setLeadTime
  //
  // Set the lead time (secs) for forecast. See setIsFcast().
  
  void setLeadTime(int lead_time) {
    _fcastLeadTime = lead_time;
  }

  /////////////////////////////////////////////////////////////////////
  // setReadFmqFromStart
  //
  // For use when reading FMQs. Setting this true allows you to read the
  // entire queue on startup.
  // Be default it is false. The default behavior is to read the latest
  // element only.

  virtual void setReadFmqFromStart(bool read_from_start = true) {
    _readFmqFromStart = read_from_start;
  }

  ////////////////////
  // setLdataFileName
  //
  // Override default file name.
  // Default is _latest_data_info.
  //
  // Overriding allows you to use the class for purposes other than monitoring
  // the latest data. For example you can use it to keep state on
  // when some specific action occurred.

  virtual void setLdataFileName(const char *file_name);

  ///////////////////////////////////////////////////////////
  // Option to save the latest read info.
  //
  // If set to true, the latest read info will be saved out in
  // a file to preserve read state in case the application dies. On
  // restart, the latest read info will be read in to re-initialize
  // the object, so that data will not be lost.
  //
  // Only applicable if FMQ is active, which is the default.
  //
  // This is off by default.
  //
  // The label supplied will be used to form the file name of the
  // file used to save the state, to help keep it unique. Generally,
  // a good strategy is to build the label from the application name 
  // and the instance.
  // 
  // If the latest_read_info file is older than max_valid_age, it will
  // not be used to restore state.

  virtual void setSaveLatestReadInfo(const string &label,
                                     int max_valid_age = -1,
                                     bool save = true);
  
  ////////////////////////////////////////////////////
  // Set the directory of displaced data set.
  // Normally the dataset and the _latest_data_info files are
  // co-located. However, sometimes _latest_data_info refers to files
  // which are actually stored in a different (displaced) location.
  // If this string is non-empty, then the data resides in the
  // displaced directory. If the string is set, data resides in
  // the displaced location.

  virtual void setDisplacedDirPath(const char *dir) { 
    _displacedDirPath = dir;
  }
  virtual void setDisplacedDirPath(const string &dir) { 
    _displacedDirPath = dir;
  }

  //////////////////////////////////////
  // FMQ control
  //
  // FMQ is on by default.

  virtual void setUseFmq(bool use_fmq = true) { _useFmq = use_fmq; }
  virtual void setFmqNSlots(int nslots);

  //////////////////////////////////////
  // XML and ASCII control
  //
  // Both are on by default.

  virtual void setUseXml(bool use_xml = true) { _useXml = use_xml; }
  virtual void setUseAscii(bool use_ascii = true) { _useAscii = use_ascii; }

  //////////////
  // print as XML
  //
  // Print an XML representation of the object.
  
  void printAsXml(ostream &out) const;

  //////////////
  // print info
  //
  // deprecated
  //
  // Print an old-style representation of the object.

  void print(ostream &out) const;

  ///////////////////
  // print normal info
  //
  // Print normal human-readable representation of the object.
  
  void printNormal(ostream &out) const;

  ////////////////////////////////////////
  // print full info - includes path names
  //
  // Print full human-readable representation of the object.

  void printFull(ostream &out) const;

  /////////////////////////////////////////////////////////////////
  // read()
  //
  // Read the latest data info, if new data is available.
  //
  // max_valid_age:
  //   This is the max age (in secs) for which the 
  //   latest data info is considered valid. If the info is
  //   older than this, we need to wait for new info.
  //   If max_valid_age is set negative, the age test is not done.
  //
  // Returns:
  //    0 on success, -1 on failure.
  //

  virtual int read(int max_valid_age = -1);
  
  /////////////////////////////////////////////////////////////////
  // readBlocking()
  //
  // Read the latest data info, polling until new data is available.
  // Will optionally call a heartbeat function while polling.
  //
  // max_valid_age: see read()
  //
  // sleep_msecs (millisecs):
  //   While in the polling state, the program sleeps for sleep_msecs
  //   millisecs at a time before checking again.
  //
  //  heartbeat_func(): heartbeat function
  //    Just before sleeping each time, heartbeat_func() is called
  //    to allow any heartbeat actions to be carried out.
  //    If heartbeat_func is NULL, it is not called.
  //    The string arg passed to the heartbeat function
  //      is "LdataInfo::readBlocking".
  //

  virtual void readBlocking(int max_valid_age,
			    int sleep_msecs,
			    heartbeat_t heartbeat_func);

  ////////////////////////////////////////////////////////////////////
  // readForced()
  //
  // Force a read of the _latest_data_info, if younger than max_valid_age.
  // If max_valid_age < 0, the age test is not done.
  //
  // Returns:
  //    0 on success, -1 on failure.
  //

  virtual int readForced(int max_valid_age = -1,
                         bool update_prev_mod_time = true);
  
  /////////////////////////////////////////////////////////////
  // write()
  //
  // Writes latest info.
  //
  // The following are written:
  //    _latest_data_info: old ASCII format, deprecated
  //    _latest_data_info.xml: XML format, flat file
  //    _latest_data_info.stat |_ old and XML format in
  //    _latest_data_info.buf  |  file message queue (FMQ)
  //
  // If latest_time is not specified or is set to 0 (the default)
  // the latest time stored in the object is used.
  //
  // NOTE: datatype only used by derived classes (DsLdataInfo)
  //
  // Returns:
  //   0 on success, -1 on failure.
  //
  // NOTES: If the environment variable $LDATA_NO_WRITE is set to 'true',
  //        this funciton does nothing.  This can be useful when regenerating
  //        old data while also running in realtime mode.
  //
  //        If $LDATA_FMQ_ACTIVE is set to 'false', the FMQ files will not
  //        be written.

  virtual int write(time_t latest_time = 0,
		    const string &datatype = "") const;

  /////////////////////////////////////////////////////////////
  // writeFmq()
  //
  // Write just the FMQ message. See write() for details other
  // details.
  //
  // If latest_time is not specified or is set to 0 (the default)
  // the latest time stored in the object is used.
  //
  // Returns:
  //   0 on success, -1 on failure.

  int writeFmq(time_t latest_time = 0) const;

  ///////////////////////////////////////////
  // set the info class from an info_t struct
  //
  // Deprecated - use disassemble() instead.

  void setFromInfo(const info_t &info);

  ///////////////////////////////////////////
  // copy the class info to an info_t struct
  //
  // Deprecated - use assemble() instead.

  void copyToInfo(info_t &info) const;

  /////////////////////////////////
  // convert from BE in info struct

  void BEtoInfo(info_t &info) const;

  /////////////////////////////////
  // convert to BE in info struct

  void BEfromInfo(info_t &info) const;

  ///////////////////////////////////////
  // buffer manipulation routines

  /////////////////////////////////////////////////////
  // assemble object into buffer
  //
  // Set xml_only to true to get only XML representation in the buffer.
  // If false, both old-style and XML representations will be included.
  //
  // use getBufPtr() and getBufLen() for buffer access after assembly.

  void assemble(bool xml_only = false) const;

  ///////////////////////////////////////////////////////
  // disassemble buffer, load object
  //
  // If XML representation is included, this will be used.
  // Otherwise the old representation will be decoded.
  //
  // returns 0 on success, -1 on failure
  
  int disassemble(const void *in_buf, int _len);

  const void *getBufPtr() const { return _buf.getPtr(); }
  int getBufLen() const { return _buf.getLen(); }

  //////////////////////////////////////
  // get methods

  //////////////////
  // get latest time
  //
  // If observation data, this is the valid time.
  // If forecast data, this is the generate time.

  time_t getLatestTime() const { return _latestTime; }
  const date_time_t &getLatestTimeStruct() const;
  
  ////////////////////////////////////////////////////////////
  // get latest valid time.
  //
  // For non-forecast data this is the same as getLatestTime().
  //
  // For forecast data, this will be the gen time plus the
  //     forecast lead time.

  time_t getLatestValidTime() const;

  /////////////
  // forecasts

  bool isFcast() const { return _isFcast; }
  int getLeadTime() const { return _fcastLeadTime; }

  //////////////////
  // get max time
  //
  // This is the running max of the latestTime
  // It never decreases.
  
  time_t getMaxTime() const { return _maxTime; }

  ////////////////////////////////////////////////////
  // get info from WITHIN the _latest_data_info file

  // get the path to the actual data file

  string getDataPath() const;

  // get the extension of the actual data file

  const string &getDataFileExt() const { return _dataFileExt; }

  // get the data type

  const string &getDataType() const;

  // get the name of the app which wrote the file

  const string &getWriter() const;

  // get the relative path from the data dir, in which the _latest_data_info
  // resides, to the actual data file

  const string &getRelDataPath() const;

  // Get the user info about the data file

  const string &getUserInfo1() const;
  const string &getUserInfo2() const;

  ///////////////////////////////////////////////////
  // get info ABOUT the _latest_data_info file ITSELF

  // get the path of the _latest_data_info file

  const string &getInfoPath() const { return _infoPath; }

  // ge the file name of the latest data info file

  const string &getFileName() const { return _fileName; }

  // get the data directory, relative to $RAP_DATA_DIR or $DATA_DIR
  
  const string &getDataDir() const { return _dataDir; }

  // get the full path to the data dir

  const string &getDataDirPath() const { return _dataDirPath; }

  // Get the displaced data set dir.
  // Normally the dataset and the _latest_data_info files are
  // co-located. However, sometimes _latest_data_info refers to files
  // which are actually stored in a different (displaced) location.
  // If this string is non-empty, then the data resides in the
  // displaced directory. If the string is set, data resides in
  // the displaced location.

  const string &getDisplacedDirPath() const { return _displacedDirPath; }

  // get the host name on which the data set resides
  
  const string &getHostName() const { return _hostName; }

  // get the error string
  
  const string &getErrStr() const { return _errStr; }

protected:

  ////////////////////////////////////////////////
  // external state for this object
  // these members are set directly from the API

  bool _debug; // debugging

  string _dataDir; // data directory
  string _hostName; // name of the host on which the data files reside
  string _displacedDirPath; // Directory of displaced data set.
  string _fileName; // ldata_info file name

  bool _useXml; // use XML file
  bool _useAscii; // use ASCII file
  bool _saveLatestReadInfo; // save latest read info, for restart
  string _latestReadInfoLabel;

  bool _useFmq; // use an FMQ
  int _fmqNSlots; // how many slots in the FMQ?
  bool _readFmqFromStart; // start reading from start of FMQ
  
  ////////////////////////////////////////////////
  // internal state of this object
  // these are set as side-effects

  bool _notExistPrint;
  bool _tooOldPrint;
  bool _notModifiedPrint;

  string _rapDataDir;  // $RAP_DATA_DIR or $DATA_DIR
  string _dataDirPath; // full data directory path: $RAP_DATA_DIR/_dataDir
  string _infoPath;    // path to latest_data_info file
  string _tmpInfoPath; // tmp file, write to this then rename
  string _xmlPath;     // path to xml ldata file

  string _lockPath;    // path to lock file
  mutable FILE *_lockFile;     // open file for locking

  //////////////////////////////////////////
  // internal details of the state of the FMQ

  string _fmqStatPath;
  string _fmqBufPath;
  int _fmqBufSize;
  int _bufFileSize;
  int _statFileSize;
  bool _fmqReadOpen;
  FMQ_handle_t _fmqReadHandle;

  ////////////////////
  // latest read state

  LdataInfo *_latestReadInfo;

  ////////////////////////////////////////////////////////
  // the following is information about the data set files
  // rather than this object itself

  // file extension for data files
  
  string _dataFileExt;

  // data type for latest data written

  mutable string _dataType;
  mutable string _dataTypeGuess;

  // path of latest file written, relative to _dataDir

  string _relDataPath;
  mutable string _relDataPathGuess;

  // name of application which wrote the latest_data_info file

  string _writer;

  // user info

  string _userInfo1;
  string _userInfo2;

  // latest time
  
  mutable time_t _latestTime;
  mutable date_time_t _ltimeStruct;

  // max time
  // this is the max time ever written to this data set
  // _latestTime can go backwards, if older data comes in
  // _maxTime can never go backwards
  
  mutable time_t _maxTime;

  // previous file or data-base modify time.
  // Used to determine info age

  time_t _prevModTime;

  // forecast

  bool _isFcast;      // is this a forecast? If so, _latestTime is a gen time
  int _fcastLeadTime; // forecast lead time (secs)

  ////////////////////////////////////
  // error string
  
  mutable string _errStr;

  ////////////////////////////////////
  // buffer for assemble, disassemble
  
  mutable MemBuf _buf;
  mutable MemBuf _xmlBuf;

  //////////////////////////////////////
  // functions

  void _clearInfo();
  void _init(bool debug, const char *file_name);
  void _setLatestTime(time_t latest_time) const;
  int _setDataPath(const string &dataDir);
  int _setDirFromUrl(const DsURL &url);
  int _readAscii(int max_valid_age, bool forced, bool &newData);
  int _doReadAscii(const char *file_path);
  int _scanFileAscii(FILE *in);
  int _readXml(int max_valid_age, bool forced, bool &newData);
  int _doReadXml(const char *file_path, int file_len);
  int _readFmq(int max_valid_age, bool &newData);
  int _openReadFmq(int max_valid_age);
  void _closeReadFmq();
  void _checkFilesForReading(int max_valid_age,
			     bool &useFmq, bool &useXml, bool &useAscii);
  int _makeDir() const;
  int _lockForWrite() const;
  void _unlockForWrite() const;
  void _closeLockFile() const;
  int _writeAscii() const;
  int _writeXml() const;
  int _writeFmq() const;
  void _writeCatalog() const;
  void _guessDataType() const;
  void _loadXmlBuf() const;
  void _loadXmlForCatalog() const;
  int _findXmlField(const char *xml_buf, const char *field_name, string &val);
  int _disassembleFromXml(const char *xml_buf, int len);
  int _disassembleFromOld(const void *in_buf, int len);
  void _debugPrint(const char* format, ...);

private:
  
};

#endif
