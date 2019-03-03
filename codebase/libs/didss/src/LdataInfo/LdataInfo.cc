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

////////////////////////////////////////////////////////////////////
// LdataInfo.cc
//
// LdataInfo class.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Feb 1999
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
//                      Default is 256.
//
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdarg>
#include <sys/stat.h>
#include <didss/RapDataDir.hh>
#include <didss/LdataInfo.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/Path.hh>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <dataport/bigend.h>
using namespace std;

//////////////////////
// Default constructor
//
// The default directory is '.'
// This may be overridden by calling setDir() and setDirFromUrl().
//

LdataInfo::LdataInfo()

{

  _init(false, LDATA_INFO_FILE_NAME);
  _setDataPath(".");

}

/////////////////////////////////////////
// constructor - with directory supplied.
//
// dataDir: directory for _latest_data_info files
// debug: set true to activate debug prints.
// file_name: specify if you do not wish to use the default
//            file names (_latest_data_info*).

LdataInfo::LdataInfo(const string &dataDir,
		     bool debug /* = false*/,
		     const char *file_name /* = LDATA_INFO_FILE_NAME*/ )

{

  _init(debug, file_name);
  _setDataPath(dataDir);

}

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

LdataInfo::LdataInfo(const DsURL &url,
		     bool debug /* = false*/,
		     const char *file_name /* = LDATA_INFO_FILE_NAME*/ )

{

  _init(debug, file_name);
  setDirFromUrl(url);

}

////////////////////
// copy constructor
//
// Note: only copies state information, not FILE pointers or other
// status.

LdataInfo::LdataInfo(const LdataInfo &orig)

{
  copy(orig);
}

//////////////
// assignment
//
// Note: only copies information, not FILE pointers or other
// status.

LdataInfo &LdataInfo::operator=(const LdataInfo &other)
{
  return copy(other);
}

//////////////
// make a copy
//
// Note: only copies information, not FILE pointers or other
// status.

LdataInfo &LdataInfo::copy(const LdataInfo &other)
{

  if (this == &other) {
    return *this;
  }
  _init(false, LDATA_INFO_FILE_NAME);

  _debug = other._debug;

  _dataDir = other._dataDir;
  _hostName = other._hostName;
  _displacedDirPath = other._displacedDirPath;
  _fileName = other._fileName;

  _useXml = other._useXml;
  _useAscii = other._useAscii;
  _saveLatestReadInfo = other._saveLatestReadInfo;
  _latestReadInfoLabel = other._latestReadInfoLabel;

  _useFmq = other._useFmq;
  _fmqNSlots = other._fmqNSlots;
  _readFmqFromStart = other._readFmqFromStart;

  _notExistPrint = other._notExistPrint;
  _tooOldPrint = other._tooOldPrint;
  _notModifiedPrint = other._notModifiedPrint;

  _rapDataDir = other._rapDataDir;
  _dataDirPath = other._dataDirPath;
  _infoPath = other._infoPath;
  _tmpInfoPath = other._tmpInfoPath;
  _xmlPath = other._xmlPath;
  _lockPath = other._lockPath;
  
  _fmqStatPath = other._fmqStatPath;
  _fmqBufPath = other._fmqBufPath;
  _fmqBufSize = other._fmqBufSize;
  _bufFileSize = other._bufFileSize;
  _statFileSize = other._statFileSize;

  _dataFileExt = other._dataFileExt;
  _dataType = other._dataType;
  _dataTypeGuess = other._dataTypeGuess;
  _relDataPath = other._relDataPath;
  _relDataPathGuess = other._relDataPathGuess;

  _writer = other._writer;
  _userInfo1 = other._userInfo1;
  _userInfo2 = other._userInfo2;

  _latestTime = other._latestTime;
  _ltimeStruct = other._ltimeStruct;
  _maxTime = other._maxTime;

  _prevModTime = other._prevModTime;

  _isFcast = other._isFcast;
  _fcastLeadTime = other._fcastLeadTime;

  _setDataPath(_dataDir);

  return *this;

}

//////////////
// destructor

LdataInfo::~LdataInfo()
{
  _closeReadFmq();
  _closeLockFile();
  if (_latestReadInfo != NULL) {
    delete _latestReadInfo;
  }
}

//////////////////////////////////////
// setDir()
//
// Sets the directory from supplied string.
//
// returns 0 on success, -1 on failure
  
int LdataInfo::setDir(const string &dataDir)

{
  if (_setDataPath(dataDir)) {
    return -1;
  } else {
    return 0;
  }
}

//////////////////////////////////////
// setDirFromUrl()
//
// Sets the directory from the URL.
// The directory is set to the file portion of the URL.
//
// For example, if the URL is mdvp:://::mdv/sat/vis, the
// directory will be mdv/sat/vis.
//
// Returns 0 on success, -1 on failure
  
int LdataInfo::setDirFromUrl(const DsURL &url)

{
  if (_setDirFromUrl(url)) {
    return -1;
  } else {
    return 0;
  }
}

int LdataInfo::setDirFromUrl(const string &urlStr)

{
  DsURL url(urlStr);
  if (_setDirFromUrl(url)) {
    return -1;
  } else {
    return 0;
  }
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

void LdataInfo::setLdataFileName(const char *file_name)
{
  _fileName = file_name;
  _setDataPath(_dataDir);
}

///////////////////////////////////////////////////////////
// Option to save the latest read info.
//
// If set to true, the latest read info will be saved out in
// a file to preserve read state in case the application dies. On
// restart, the latest read info will be read in to re-initialize
// the object, so that data will not be lost.
//
// This is off by default.
//
// The label supplied will be used to form the file name of the
// file used to save the state, to help keep it unique. Generally,
// a good strategy is to build the label from the application name 
// and the instance.

void LdataInfo::setSaveLatestReadInfo(const string &label,
				      int max_valid_age /*= -1*/,
				      bool save /* = true */)
  
{

  _saveLatestReadInfo = save;
  _latestReadInfoLabel = label;
  
  if (save && _useFmq) {

    // in this mode, we cannot also seek to the start
    
    _readFmqFromStart = false;
    
    // set up the file for saving the latest read info
    
    if (_latestReadInfo != NULL) {
      delete _latestReadInfo;
    }
    char name[MAX_PATH_LEN];
    sprintf(name, "latest_read_info.%s.%s", label.c_str(), _dataDir.c_str());
    char *delim;
    while ((delim = strstr(name, PATH_DELIM)) != NULL) {
      for (size_t ii = 0; ii < strlen(PATH_DELIM); ii++) {
	delim[ii] = '_';
      }
    }
    _latestReadInfo = new LdataInfo(RapDataDir.tmpLocation(), _debug, name);
    _latestReadInfo->setUseFmq(false);
    _latestReadInfo->setUseAscii(false);
    
    if (_debug) {
      cerr << "---->> read state file name: " << name << endl;
    }

    // try reading the file, to recover previous read state

    if (_latestReadInfo->readForced(max_valid_age) == 0) {
      // seek FMQ to the last ID read
      int lastIdRead;
      if (sscanf(_latestReadInfo->getUserInfo1().c_str(),
		 "FMQ_id:%d", &lastIdRead) == 1) {
	if (_debug) {
	  cerr << "------>> Startup read" << endl;
	  cerr << "-------->> seeking to ID: " << lastIdRead << endl;
	}
	if (_openReadFmq(max_valid_age) == 0) {
	  FMQ_seek_to_id (&_fmqReadHandle, lastIdRead);
	}
      }
      if (_debug) {
	cerr << "Latest read info object:" << endl;
	_latestReadInfo->printFull(cerr);
      }
    }
    
  }

}
  
//////////////////////////////////////
// Set number of FMQ slots

void LdataInfo::setFmqNSlots(int nslots) {
  _fmqNSlots = nslots;
  _fmqBufSize = _fmqNSlots * LDATA_BUFSIZE_PER_SLOT;
}

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

int LdataInfo::setPathAndTime(const string &dir,
                              const string &filePath)
  
{

  // set relative path and data file extension

  string relPath;
  Path::stripDir(dir, filePath, relPath);
  setRelDataPath(relPath);

  Path fpath(filePath);
  setDataFileExt(fpath.getExt());

  // compute time if possible

  bool dateOnly;
  time_t latestTime;
  if (DataFileNames::getDataTime(relPath, latestTime, dateOnly)) {
    return -1;
  }
  _setLatestTime(latestTime);

  return 0;

}

/////////////////////////////////////////////////////////////
// setLatestTime
//
// Set the latest time.
// This is the valid time for obs data, generate time for
// forecast data.

void LdataInfo::setLatestTime(time_t latest_time) { 
  _setLatestTime(latest_time);
}

//////////////
// print as XML

void LdataInfo::printAsXml(ostream &out) const

{

  _loadXmlBuf();
  out << (char *) _xmlBuf.getPtr();

}

//////////////////////////
// print info - old format

void LdataInfo::print(ostream &out) const

{

  // latest_time

  date_time_t ltime;
  ltime.unix_time = getLatestTime();
  uconvert_from_utime(&ltime);
  out << ltime.unix_time << " "
      << ltime.year << " "
      << ltime.month << " "
      << ltime.day << " "
      << ltime.hour << " "
      << ltime.min << " "
      << ltime.sec << " "
      << endl;
  
  // file_ext, user_info_1, user_info_2
  
  out << getDataFileExt() << endl;
  if (_userInfo1 == "none" && _writer != "unknown") {
    out << _writer << endl;
  } else {
    out << _userInfo1 << endl;
  }
  if (_userInfo2 == "none" && _relDataPath != "unknown") {
    out << _relDataPath << endl;
  } else {
    out << _userInfo2 << endl;
  }
  
  // forecasts

  if (_isFcast) {
    out << "1" << endl;
  out << getLeadTime() << endl;
  } else {
    out << "0" << endl;
  }

}

///////////////////
// print normal info

void LdataInfo::printNormal(ostream &out) const

{

  out << "------------LdataInfo--------------" << endl;
  DateTime ltime(getLatestTime());
  out << "LatestTime  : "
      << ltime.getYear() << "/"
      << ltime.getMonth() << "/"
      << ltime.getDay() << " "
      << ltime.getHour() << ":"
      << ltime.getMin() << ":"
      << ltime.getSec() << endl;
  out << "RelDataPath : " << getRelDataPath() << endl;
  out << "DataFileExt : " << getDataFileExt() << endl;
  out << "DataType    : " << getDataType() << endl;
  out << "Writer      : " << getWriter() << endl;
  out << "UserInfo1   : " << getUserInfo1() << endl;
  out << "UserInfo2   : " << getUserInfo2() << endl;
  out << "IsFcast     : " << (isFcast()?"true":"false") << endl;
  out << "LeadTime    : " << getLeadTime() << endl;
  if (_displacedDirPath.size() > 0) {
    out << "DisplacedDirPath : " << getDisplacedDirPath() << endl;
  }
  out << "-----------------------------------" << endl;

}

///////////////////
// print full info

void LdataInfo::printFull(ostream &out) const

{

  out << "------------LdataInfo dirs---------" << endl;
  out << "DataDirPath : " << _dataDirPath << endl;
  out << "DataDir     : " << _dataDir << endl;
  out << "InfoPath    : " << _infoPath << endl;
  printNormal(out);

}

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
// Side effect:
//    If new data found, sets _prevModTime to file modify time.
//
// Returns:
//    0 on success, -1 on failure.
//

int LdataInfo::read(int max_valid_age /* = -1*/ )

{

  bool newData = false;

  // stat files to check which are OK for reading

  bool fmqOK, xmlOK, asciiOK;
  _checkFilesForReading(max_valid_age, fmqOK, xmlOK, asciiOK);

  // try FMQ first

  if (!fmqOK) {
    _closeReadFmq();
  }
  if (_useFmq && fmqOK) {
#ifdef DEBUG_PRINT  
    if (_debug) {
      cerr << "Read - trying FMQ" << endl;
    }
#endif
    if (_readFmq(max_valid_age, newData) == 0) {
      if (newData) {
	if (_debug) {
	  cerr << "FMQ read successful" << endl;
	}
	return 0;
      } else {
	return -1;
      }
    } else {
      _closeReadFmq();
    }
  }
  
  // then try XML

  if (_useXml && xmlOK) {
    if (_debug) {
      cerr << "Read - trying XML" << endl;
    }
    if (_readXml(max_valid_age, false, newData) == 0) {
      if (newData) {
	if (_debug) {
	  cerr << "XML read successful" << endl;
	}
	return 0;
      } else {
	return -1;
      }
    }
  }
  
  // then try ASCII

  if (_useAscii && asciiOK) {
    if (_debug) {
      cerr << "Read - trying ASCII" << endl;
    }
    if (_readAscii(max_valid_age, false, newData) == 0) {
      if (newData) {
	if (_debug) {
	  cerr << "ASCII read successful" << endl;
	}
	return 0;
      } else {
	return -1;
      }
    }
  }

#ifdef DEBUG_PRINT  
  if (_debug) {
    cerr << "--> read failed" << endl;
  }
#endif

  return -1;

}

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
// Side effect:
//    If new data found, sets _prevModTime to file modify time.
//

void LdataInfo::readBlocking(int max_valid_age,
			     int sleep_msecs,
			     heartbeat_t heartbeat_func)

{

  while (read(max_valid_age)) {
    if (heartbeat_func != NULL) {
      heartbeat_func("LdataInfo::readBlocking");
    }
    umsleep(sleep_msecs);
  }
  return;

}

////////////////////////////////////////////////////////////////////
// readForced()
//
// Force a read of the _latest_data_info, if younger than max_valid_age.
// If max_valid_age < 0, the age test is not done.
//
// Side effect:
//    If new data found, sets _prevModTime to file modify time.
//
// Returns:
//    0 on success, -1 on failure.
//

int LdataInfo::readForced(int max_valid_age /* = -1*/ ,
                          bool update_prev_mod_time /* = true */)
{

  bool newData = false;
  bool fmqOK, xmlOK, asciiOK;
  _checkFilesForReading(max_valid_age, fmqOK, xmlOK, asciiOK);

  // try the fmq first
  
  if (!fmqOK) {
    _closeReadFmq();
  }

  if (_useFmq && fmqOK && _fmqReadOpen) {
    // seek to read last position
    if (FMQ_seek_back(&_fmqReadHandle) == 0) {
      if (_readFmq(max_valid_age, newData) == 0) {
	if (newData) {
	  return 0;
	} else {
	  return -1;
	}
      } else {
	_closeReadFmq();
      }
    }
  }

  // try XML

  if (_useXml && xmlOK) {
    if (_readXml(max_valid_age, update_prev_mod_time, newData) == 0) {
      return 0;
    } else {
      return -1;
    }
  }
  
  // try ASCII

  if (_useAscii && asciiOK) {
    if (_readAscii(max_valid_age, update_prev_mod_time, newData) == 0) {
      return 0;
    } else {
      return -1;
    }
  }

  return -1;

}

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

int LdataInfo::write(time_t latest_time,
		    const string &datatype) const

{

  // set latest time
  
  if (latest_time != 0) {
    _setLatestTime(latest_time);
  }

  // Return now if $LDATA_NO_WRITE is true

  char *noWrite = getenv("LDATA_NO_WRITE");
  if (noWrite && STRequal(noWrite, "true")) {
    return 0;
  }
  
  // make directory if necessary
  
  if (_makeDir()) {
    TaStr::AddStr(_errStr, "ERROR - LdataInfo::write");
    cerr << _errStr;
    return -1;
  }
  
  // Write the catalog, if required.
  // The catalog file is written if the file
  // _ldata_write_catalog exists in the data directory.

  _writeCatalog();
  
  // lock for writing

  if (_lockForWrite()) {
    TaStr::AddStr(_errStr, "ERROR - LdataInfo::write");
    cerr << _errStr;
    return -1;
  }

  // write the ascii file

  if (_useAscii) {
    if (_writeAscii()) {
      _unlockForWrite();
      return -1;
    }
  }

  // write the XML file if required

  if (_useXml) {
    if (_writeXml()) {
      _unlockForWrite();
      return -1;
    }
  }
  
  // write FMQ if required
  
  if (_useFmq) {
    if (_writeFmq()) {
      _errStr += "ERROR - LdataInfo::write\n";
      TaStr::AddStr(_errStr, "  Cannot write fmq: ", _infoPath);
      cerr << _errStr;
      _unlockForWrite();
      return -1;
    }
  }
  
  _unlockForWrite();
  return 0;
  
}

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
//

int LdataInfo::writeFmq(time_t latest_time /* = 0*/ ) const

{

  // set latest time

  if (latest_time != 0) {
    _setLatestTime(latest_time);
  }

  // write FMQ if required

  if (_writeFmq()) {
    _errStr += "ERROR - LdataInfo::writeFmq\n";
    TaStr::AddStr(_errStr, "  Cannot write fmq: ", _infoPath);
    cerr << _errStr;
    return -1;
  }

  return 0;
  
}

///////////////////////////////////////////
// set the info class from an info_t struct
//
// Deprecated - use disassemble() instead.

void LdataInfo::setFromInfo(const info_t &info)

{
  _setLatestTime(info.latestTime);
  if (info.nFcasts == 0) {
    _isFcast = false;
  } else {
    _isFcast = true;
  }
  _dataFileExt = info.dataFileExt;
  _userInfo1 = info.userInfo1;
  _userInfo2 = info.userInfo2;
}

///////////////////////////////////////////
// copy the class info to an info_t struct
//
// Deprecated - use assemble() instead.

void LdataInfo::copyToInfo(info_t &info) const

{
  info.latestTime = _latestTime;
  if (_isFcast) {
    info.nFcasts = 1;
  } else {
    info.nFcasts = 0;
  }
  STRncopy(info.dataFileExt, _dataFileExt.c_str(), LDATA_INFO_STR_LEN);
  STRncopy(info.userInfo1, _userInfo1.c_str(), LDATA_INFO_STR_LEN);
  STRncopy(info.userInfo2, _userInfo2.c_str(), LDATA_INFO_STR_LEN);
}

/////////////////////////////////
// convert from BE in info struct

void LdataInfo::BEtoInfo(info_t &info) const

{
  BE_to_array_32(&info.latestTime, sizeof(ti32));
  BE_to_array_32(&info.nFcasts, sizeof(si32));
}

/////////////////////////////////
// convert to BE in info struct

void LdataInfo::BEfromInfo(info_t &info) const

{
  BE_from_array_32(&info.latestTime, sizeof(ti32));
  BE_from_array_32(&info.nFcasts, sizeof(si32));
}

/////////////////////////////////////////////////////
// assemble object into buffer
//
// Set xml_only to true to get only XML representation in the buffer.
// If false, both old-style and XML representations will be included.

void LdataInfo::assemble(bool xml_only /* = false */) const

{

  _buf.free();

  if (!xml_only & _useAscii) {

    // non-XML representation

    info_t info;
    copyToInfo(info);
    BEfromInfo(info);
    _buf.add(&info, sizeof(info));
    if (_isFcast) {
      si32 leadTime = _fcastLeadTime;
      BE_from_array_32(&leadTime, sizeof(leadTime));
      _buf.add(&leadTime, sizeof(leadTime));
    }

  }

  // XML representation
  
  if (_useXml) {
    _loadXmlBuf();
    _buf.add(_xmlBuf.getPtr(), _xmlBuf.getLen());
  }
  
}

///////////////////////////////////////////////////////
// disassemble buffer, load object
//
// If XML representation is included, this will be used.
// Otherwise the old representation will be decoded.
//
// returns 0 on success, -1 on failure

int LdataInfo::disassemble(const void *in_buf, int len)

{

  // check for XML at start of the buffer

  const char *cbuf = (char *) in_buf;
  const char *ltest = "<latest_data_info>";
  if (!strncmp(cbuf, ltest, strlen(ltest))) {
    return _disassembleFromXml(cbuf, len);
  }

  // check for XML after old-style buffer without forecasts

  int oldMinLen = (int) sizeof(info_t);
  if (len > oldMinLen + (int) strlen(ltest)) {
    cbuf = ((char *) in_buf) + oldMinLen;
    if (!strncmp(cbuf, ltest, strlen(ltest))) {
      return _disassembleFromXml(cbuf, len - oldMinLen);
    }
  }
    
  // check for XML after old-style buffer with forecasts

  int oldMaxLen = oldMinLen + (int) sizeof(si32);
  if (len > oldMaxLen + (int) strlen(ltest)) {
    cbuf = ((char *) in_buf) + oldMaxLen;
    if (!strncmp(cbuf, ltest, strlen(ltest))) {
      return _disassembleFromXml(cbuf, len - oldMaxLen);
    }
  }

  // no XML, so use old style decode

  return _disassembleFromOld(in_buf, len);

}

/////////////////////////////////////////////////////////
// Get methods

//////////////////
// get latest time
//
// If observation data, this is the valid time.
// If forecast data, this is the generate time.

const date_time_t &LdataInfo::getLatestTimeStruct() const
{
  _ltimeStruct.unix_time = _latestTime;
  uconvert_from_utime(&_ltimeStruct);
  return (_ltimeStruct);
}

////////////////////////////////////////////////////////////
// get latest valid time.
//
// For non-forecast data this is the same as getLatestTime().
//
// For forecast data, this will be the gen time plus the
//     forecast lead time.

time_t LdataInfo::getLatestValidTime() const
{
  if (_isFcast) {
    return _latestTime + _fcastLeadTime;
  } else {
    return _latestTime;
  }
}

/////////////////////////////////////////////////////////
// get the complete path for the current data file.

string LdataInfo::getDataPath() const
{

  string filePath, timeFilePath;

  string dir = _dataDirPath;
  if (_displacedDirPath.size() > 0) {
    dir = _displacedDirPath;
  }
  
  // rel data path
  
  filePath = dir + PATH_DELIM + _relDataPath;
  if (ta_stat_is_file(filePath.c_str())) {
    return filePath;
  }
  
  // compute time-based path

  date_time_t ltime;
  ltime.unix_time = _latestTime;
  uconvert_from_utime(&ltime);

  char data_subdir[16];
  sprintf(data_subdir, "%.4d%.2d%.2d",
	  ltime.year, ltime.month, ltime.day);

  char data_basename[16];
  sprintf(data_basename, "%.2d%.2d%.2d",
	  ltime.hour, ltime.min, ltime.sec);

  timeFilePath = dir + PATH_DELIM + data_subdir + PATH_DELIM +
    data_basename + "." + _dataFileExt;
  
  // first try time-based path
  
  filePath = timeFilePath;
  if (ta_stat_is_file(filePath.c_str())) {
    return filePath;
  }
  
  // then try user info 2
  
  filePath = dir + PATH_DELIM + _userInfo2;
  if (ta_stat_is_file(filePath.c_str())) {
    return filePath;
  }
  
  // then try user info 1 + ext

  filePath = dir + PATH_DELIM + _userInfo1 + "." + _dataFileExt;
  if (ta_stat_is_file(filePath.c_str())) {
    return filePath;
  }

  // then try user info 1
  
  filePath = dir + PATH_DELIM + _userInfo1;
  if (ta_stat_is_file(filePath.c_str())) {
    return filePath;
  }
  
  // then try user info 2 + ext
  
  filePath = dir + PATH_DELIM + _userInfo2 + "." + _dataFileExt;
  if (ta_stat_is_file(filePath.c_str())) {
    return filePath;
  }
  
  // then, if forecast data, try gen-time path
  
  if (_isFcast) {

    char gentime_subdir[16];
    sprintf(gentime_subdir, "g_%.2d%.2d%.2d",
	    ltime.hour, ltime.min, ltime.sec);

    char leadtime_name[16];
    sprintf(leadtime_name, "f_%.8d", _fcastLeadTime);

    filePath = dir + PATH_DELIM + data_subdir + PATH_DELIM +
      gentime_subdir + PATH_DELIM + leadtime_name + "." + _dataFileExt;
    
    if (ta_stat_is_file(filePath.c_str())) {
      return filePath;
    }

  }

  // All failed
  
  return "unknown";

}

/////////////////////////////////////////////      
// get data type
// If data type is not set, will guess it

const string &LdataInfo::getDataType() const {
  if (_dataType == "unknown") {
    _guessDataType();
    return _dataTypeGuess;
  } else {
    return _dataType;
  }
}

//////////////////////////////////////////////////
// get writer application name
// 
// If writer is not set, return userInfo1 instead,
// for backwards-compatibility

const string &LdataInfo::getWriter() const {
  if (_writer == "unknown" && _userInfo1 != "none") {
    return _userInfo1;
  } else {
    return _writer;
  }
}

//////////////////////////////////////////////////////
// get relative file path
//
// If relDataPath is not set, return userInfo2 instead,
// for backwards-compatibility

const string &LdataInfo::getRelDataPath() const {
  if (_relDataPath == "unknown") {
    string fullPath = getDataPath();
    if (_displacedDirPath.size() == 0) {
      Path::stripDir(_dataDirPath, fullPath, _relDataPathGuess);
    } else {
      Path::stripDir(_displacedDirPath, fullPath, _relDataPathGuess);
    }
    return _relDataPathGuess;
  } else {
    return _relDataPath;
  }
}

//////////////////////////////////////////////////
// get user info 1
//
// If userInfo1 is not set, return writer instead,
// for backwards-compatibility

const string &LdataInfo::getUserInfo1() const {
  if (_userInfo1 == "none" && _writer != "unknown") {
    return _writer;
  } else {
    return _userInfo1;
  }
}

///////////////////////////////////////////////////////
// get user info 2
//
// If userInfo2 is not set, return relDataPath instead,
// for backwards-compatibility

const string &LdataInfo::getUserInfo2() const {
  if (_userInfo2 == "none" && _relDataPath != "unknown") {
    return _relDataPath;
  } else {
    return _userInfo2;
  }
}

/////////////////////
// private routines

///////////////////////////////
// _clearInfo()
//
// Clear the information fields

void LdataInfo::_clearInfo()

{

  _dataFileExt = "none";
  _dataType = "unknown";
  _relDataPath = "unknown";
  _writer = "unknown";
  _userInfo1 = "none";
  _userInfo2 = "none";

  _latestTime = 0;
  MEM_zero(_ltimeStruct);

  _maxTime = 0;

  _isFcast = false;
  _fcastLeadTime = 0;
  
  _displacedDirPath = "";

}

////////////
// _init()
//

void LdataInfo::_init(bool debug, const char *file_name)

{

  _clearInfo();

  _debug = debug;
  _notExistPrint = true;
  _tooOldPrint = true;
  _notModifiedPrint = true;
  _useAscii = true;
  _useXml = true;
  _saveLatestReadInfo = false;
  _latestReadInfoLabel.clear();
  
  _fileName = file_name;
  _rapDataDir = RapDataDir.location();
  _dataDir = ".";

  _dataDirPath.clear();
  _infoPath.clear();
  _tmpInfoPath.clear();

  _prevModTime = 0;

  _lockFile = NULL;

  // fmq

  _useFmq = true;
  _fmqReadOpen = false;
  _readFmqFromStart = false;
  _bufFileSize = 0;
  _statFileSize = 0;

  char *fmq_str = getenv("LDATA_FMQ_ACTIVE");
  if (fmq_str && STRequal(fmq_str, "false")) {
    _useFmq = false;
  }
  
  // get number of slots from environment variable if set
  // otherwise use default of LDATA_NSLOTS_DEFAULT
  // Also sets buffer size, allowing LDATA_BUFSIZE_PER_SLOT bytes per slot

  setFmqNSlots(LDATA_NSLOTS_DEFAULT);
  char *nslots_str = getenv("LDATA_FMQ_NSLOTS");
  if (nslots_str != NULL) {
    int nslots;
    if (sscanf(nslots_str, "%d", &nslots) == 1) {
      setFmqNSlots(nslots);
    }
  }
  
  // support XML?

  char *asXml = getenv("LDATA_AS_XML");
  if (asXml && STRequal(asXml, "false")) {
    _useXml = false;
  }

  // support ASCII?

  char *asAscii = getenv("LDATA_AS_ASCII");
  if (asAscii && STRequal(asAscii, "false")) {
    _useAscii = false;
  }

  // object for latest info

  _latestReadInfo = NULL;

}

/////////////////////////////////////////////////////////////
// _setLatestTime
//
// Private const version of setLatestTime().
//
// Sets _latestTime and _maxTime.

void LdataInfo::_setLatestTime(time_t latest_time) const { 
  _latestTime = latest_time;
  if (_latestTime > _maxTime) {
    _maxTime = _latestTime;
  }
}

/////////////////
// _setDataPath()

int LdataInfo::_setDataPath(const string &dataDir)
  
{

  int iret = 0;

  _dataDir = dataDir;
  RapDataDir.fillPath(_dataDir, _dataDirPath);
  
  // info paths, new style has underscore as prefix, old style does not

  _infoPath = _dataDirPath;
  _infoPath += PATH_DELIM;
  _infoPath += "_";
  _infoPath += _fileName;

  char pidStr[128];
  sprintf(pidStr, ".%d", getpid());

  _tmpInfoPath = _infoPath;
  _tmpInfoPath += ".tmp";
  _tmpInfoPath += pidStr;

  _xmlPath = _infoPath + ".xml";
  _lockPath = _infoPath + ".lock";
  _fmqStatPath = _infoPath + ".stat";
  _fmqBufPath = _infoPath + ".buf";

  return iret;

}

////////////////////
// _setDirFromUrl()
//
// URL specifies the data type and directory.
//
// Returns 0 on success, -1 on error
//

int LdataInfo::_setDirFromUrl(const DsURL &url)

{

  if (!url.isValid()) {
    _errStr = "ERROR - LdataInfo::_setDirFromUrl\n";
    TaStr::AddStr(_errStr, "  Invalid url: ", url.getURLStr());
    cerr << _errStr;
    return -1;
  }

  if (_setDataPath(url.getFile())) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// _readAscii()
//
//   max_valid_age:
//     This is the max age (in secs) for which the 
//     latest data info is considered valid. If the info is
//     older than this, we need to wait for new info.
//     If max_valid_age is set negative, the age test is not done.
//
// Side effect:
//    If new data found, sets newData true and
//    sets _prevModTime to file modify time.
//
// Returns:
//    0 on success, -1 on failure.
//

int LdataInfo::_readAscii(int max_valid_age, bool forced, bool &newData)

{

  newData = false;

  // Check if info file exists

  struct stat fileStat;
  const char *file_path = _infoPath.c_str();
  if (ta_stat(file_path, &fileStat)) {
    if (_debug && _notExistPrint) {
      cerr << "LdataInfo::_readAscii" << endl;
      cerr << "  File " << _infoPath << " does not exist." << endl;
      _notExistPrint = false;
    }
    return 0;
  }
  
  if (!forced) {

    // check for modified file time
    
    if (fileStat.st_mtime == _prevModTime) {
      if (_debug && _notModifiedPrint) {
	cerr << "LdataInfo::_readAscii" << endl;
	cerr << "  File " << _infoPath << " not modified." << endl;
	cerr << "  Last mod time: " << utimstr(_prevModTime) << endl;
	_notModifiedPrint = false;
      }
      return 0;
    }

  }

  // new data for forced, so read in file

  if (_doReadAscii(file_path)) {
    if (_debug) {
      cerr << "LdataInfo::_readAscii" << endl;
      cerr << "  Cannot read info file: " << _infoPath << endl;
    }
    // Remove bad ldata file.
    unlink(file_path);
    return -1;
  }

  if (_debug) {
    cerr << "LdataInfo::_readAscii" << endl;
    cerr << "  Success reading file: " << _infoPath << endl;
  }

  // set new data flag

  newData = true;

  if (!forced) {
    // set prev_mod_time to save it for next iteration
    _prevModTime = fileStat.st_mtime;
  }

  // reset print flags
  
  _notExistPrint = true;
  _tooOldPrint = true;
  _notModifiedPrint = true;
  
  return 0;
  
}

///////////////////////////////////////
// _doReadAscii()
//
// Open, read and close info file.
//
// returns 0 on success, -1 on failure
///

int LdataInfo::_doReadAscii(const char *file_path)
     
{

  // open file
  
  FILE *info_file;
  if ((info_file = fopen(file_path, "r")) == NULL) {
    int errNo = errno;
    _errStr = "ERROR - LdataInfo::_doReadAscii\n";
    TaStr::AddStr(_errStr, "Could not open latest data info file: ",
                  file_path);
    TaStr::AddStr(_errStr, strerror(errNo));
    cerr << _errStr;
    return -1;
  }

  // perform read
  
  if (_scanFileAscii(info_file)) {
    fclose(info_file);
    return -1;
  }

  // debug print

  if (_debug) {
    print(cerr);
  }
  
  // close file

  fclose(info_file);

  return 0;

}

//////////////////////////////////////////////////////
// _scanFileAscii()
//
// Reads info from open file descriptor
//
// returns 0 on success, -1 on failure
///

int LdataInfo::_scanFileAscii(FILE *in)
     
{

  // initialize

  _clearInfo();

  // latest time
  
  char line[BUFSIZ];
  if (fgets(line, BUFSIZ, in) == NULL) {
    _debugPrint("ASCII file: Cannot find unix time\n");
    return -1;
  }

  long ltime;
  date_time_t dtime;
  if (sscanf(line, "%ld", &ltime) != 1) {
    _debugPrint("Cannot decode unix time: %s\n", line);
    return -1;
  }
  if (ltime == -1) {
    if (sscanf(line, "%ld%d%d%d%d%d%d", &ltime,
	       &dtime.year, &dtime.month, &dtime.day,
	       &dtime.hour, &dtime.min, &dtime.sec) != 7) {
      _debugPrint("ASCII file: Cannot decode unix time: %s\n", line);
      return -1;
    }
    uconvert_to_utime(&dtime);
    _latestTime = dtime.unix_time;
  } else {
    _latestTime = ltime;
  }

  // read data file ext
  
  char tmpStr[LDATA_INFO_STR_LEN];
  
  if (fgets(tmpStr, LDATA_INFO_STR_LEN, in) == NULL) {
    _debugPrint("ASCII file: Cannot read data file ext\n");
    return -1;
  }
  tmpStr[strlen(tmpStr) - 1] = '\0';
  _dataFileExt = tmpStr;
  
  // read user_info
  
  if (fgets(tmpStr, LDATA_INFO_STR_LEN, in) == NULL) {
    _debugPrint("ASCII file: Cannot read user info 1\n");
    return -1;
  }
  tmpStr[strlen(tmpStr) - 1] = '\0';
  _userInfo1 = tmpStr;

  if (fgets(tmpStr, LDATA_INFO_STR_LEN, in) == NULL) {
    _debugPrint("ASCII file: Cannot read user info 2\n");
    return -1;
  }
  tmpStr[strlen(tmpStr) - 1] = '\0';
  _userInfo2 = tmpStr;

  // forecast?
  
  int is_fcast;
  if (fgets(line, BUFSIZ, in) == NULL) {
    _debugPrint("ASCII file: Cannot read is_fcast flag\n");
    return -1;
  }
  if (sscanf(line, "%d", &is_fcast) != 1) {
    _debugPrint("ASCII file: Cannot decode is_fcast flag: %s\n", line);
    return -1;
  }
  if (is_fcast == 0) {
    _isFcast = false;
  } else {
    _isFcast = true;
  }

  // forecast lead time

  if (_isFcast) {
    if (fgets(line, BUFSIZ, in) == NULL) {
      _debugPrint("ASCII file: Cannot read lead time\n");
      return -1;
    }
    int itime;
    if (sscanf(line, "%d", &itime) != 1) {
      _debugPrint("ASCII file: Cannot decode lead time: %s\n", line);
      return -1;
    }
    _fcastLeadTime = itime;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// _readXml()
//
//   max_valid_age:
//     This is the max age (in secs) for which the 
//     latest data info is considered valid. If the info is
//     older than this, we need to wait for new info.
//     If max_valid_age is set negative, the age test is not done.
//
// Side effect:
//    If new data found, sets newData true and
//    sets _prevModTime to file modify time.
//
// Returns:
//    0 on success, -1 on failure.
//

int LdataInfo::_readXml(int max_valid_age, bool forced, bool &newData)

{

  newData = false;

  const char *xml_path = _xmlPath.c_str();
  
  // Check if xml file exists
  
  struct stat xmlStat;
  if (ta_stat(xml_path, &xmlStat)) {
    if (_debug && _notExistPrint) {
      cerr << "LdataInfo::_readXml" << endl;
      cerr << "  File " << _xmlPath << " does not exist." << endl;
      _notExistPrint = false;
    }
    return 0;
  }
  
  if (!forced) {

    // check for modified file time
    
    if (xmlStat.st_mtime == _prevModTime) {
      if (_debug && _notModifiedPrint) {
	cerr << "LdataInfo::_readXml" << endl;
	cerr << "  File " << _xmlPath << " not modified." << endl;
	cerr << "  Last mod time: " << utimstr(_prevModTime) << endl;
	_notModifiedPrint = false;
      }
      return 0;
    }

  }

  // new data for forced, so read in file
  
 if (_doReadXml(xml_path, xmlStat.st_size)) {
    if (_debug) {
      cerr << "LdataInfo::_readXml" << endl;
      cerr << "  Cannot read info file: " << _xmlPath << endl;
    }
    // Remove bad ldata file.
    unlink(xml_path);
    return -1;
  }

  if (_debug) {
    cerr << "LdataInfo::_readXml" << endl;
    cerr << "  Success reading file: " << _xmlPath << endl;
  }

  // set new data flag

  newData = true;

  if (!forced) {
    // set prev_mod_time to save it for next iteration
    _prevModTime = xmlStat.st_mtime;
  }

  // reset print flags
  
  _notExistPrint = true;
  _tooOldPrint = true;
  _notModifiedPrint = true;
  
  return 0;
  
}

///////////////////////////////////////
// _doReadXml()
//
// Open, read and close info file.
//
// returns 0 on success, -1 on failure
///

int LdataInfo::_doReadXml(const char *file_path, int file_len)
  
{

  // open file
  
  FILE *xml_file;
  if ((xml_file = fopen(file_path, "r")) == NULL) {
    int errNo = errno;
    _errStr = "ERROR - LdataInfo::_doReadXml\n";
    TaStr::AddStr(_errStr, "Could not open latest data XML info file: ",
                  file_path);
    TaStr::AddStr(_errStr, strerror(errNo));
    cerr << _errStr;
    return -1;
  }

  // create buffer

  TaArray<char> bufArray;
  char *xmlBuf = bufArray.alloc(file_len + 1);
  memset(xmlBuf, 0, file_len + 1);
  
  // read in buffer
  
  if (ta_fread(xmlBuf, 1, file_len, xml_file) != file_len) {
    fclose(xml_file);
    return -1;
  }
  fclose(xml_file);

  // null terminate xmlBuf
  xmlBuf[file_len] = '\0';

  // disassemble from XML
	
  if (_disassembleFromXml(xmlBuf, file_len)) {
    return -1;
  }

  // debug print

  if (_debug) {
    printAsXml(cerr);
  }
  
  return 0;

}

///////////////////////////////////////
// _readFmq()
//

int LdataInfo::_readFmq(int max_valid_age,
			bool &newData)

{

  newData = false;

  // stat the fmq to check that it exists
  
  struct stat statFileStats;
  struct stat bufFileStats;
  if (ta_stat(_fmqStatPath.c_str(), &statFileStats)) {
    if (_debug && _notExistPrint) {
      cerr << "LdataInfo::_readFmq" << endl;
      cerr << "  Fmq " << _fmqStatPath << " does not exist." << endl;
    }
    _closeReadFmq();
    return -1;
  }

  // open fmq if needed

  if (!_fmqReadOpen) {
    if (_openReadFmq(max_valid_age)) {
      if (_debug && _tooOldPrint) {
	cerr << "ERROR - LdataInfo::_readFmq" << endl;
	cerr << "  Cannot open fmq: " << _infoPath << endl;
	_tooOldPrint = false;
      }
      return -1;
    }
  }

  // check size has not changed since opened
  
  if (ta_stat(_fmqStatPath.c_str(), &statFileStats)) {
    cerr << "ERROR - LdataInfo::_readFmq" << endl;
    cerr << "  Failed to stat file: " <<_fmqStatPath.c_str()  << endl;
    return -1;
  }
 
  if (ta_stat(_fmqBufPath.c_str(), &bufFileStats) ) {
    cerr << "ERROR - LdataInfo::_readFmq" << endl;
    cerr << "  Failed to stat file: " <<_fmqBufPath.c_str()  << endl;
    return -1;
  }
 

  if (statFileStats.st_size != _statFileSize ||
      bufFileStats.st_size != _bufFileSize) {
    cerr << "WARNING: FMQ files have changed size, closing FMQ" << endl;
    _closeReadFmq();
    return -1;
  }

  // FMQ is open, read in next message if available

  int gotNew = false;
  if (FMQ_read(&_fmqReadHandle, &gotNew)) {
    _errStr = "ERROR - LdataInfo::_readFmq\n";
    TaStr::AddStr(_errStr, "  Reading message from fmq: ", _infoPath);
    _closeReadFmq();
    cerr << _errStr;
    return -1;
  }
  int messageId = FMQ_msg_id(&_fmqReadHandle);

  if (!gotNew) {
    newData = false;
    return 0;
  }

  // disassemble message to load up the object internals

  if (disassemble(FMQ_msg(&_fmqReadHandle), FMQ_msg_len(&_fmqReadHandle))) {
    _errStr = "ERROR - LdataInfo::_readFmq\n";
    TaStr::AddStr(_errStr, "  Cannot disassemble buffer from fmq: ", _infoPath);
    cerr << _errStr;
    return -1;
  }

  // we have new data

  newData = true;
  _prevModTime = statFileStats.st_mtime;

  // save state?

  if (_saveLatestReadInfo) {
    
    _latestReadInfo->setRelDataPath(getRelDataPath().c_str());
    char idStr[128];
    sprintf(idStr, "FMQ_id:%d", messageId);
    _latestReadInfo->setUserInfo1(idStr);
    if (_latestReadInfo->write(getLatestValidTime())) {
      cerr << "WARNING - LdataInfo::_readFmq" << endl;
      cerr << "  Cannot write _latestReadInfo file to keep state" << endl;
    }

    if (_debug) {
      cerr << "****** Saving read state to file ******" << endl;
      _latestReadInfo->printFull(cerr);
    }
    
  } // if (_saveLatestReadInfo)



  return 0;

}

////////////////////////////////
// open FMQ for reading
//
// Returns 0 on success, -1 on failure

int LdataInfo::_openReadFmq(int max_valid_age)
  
{
  
  if (_fmqReadOpen) {
    return 0;
  }

  // stat the fmq to get age
  
  struct stat statFileStats, bufFileStats;
  if (ta_stat(_fmqStatPath.c_str(), &statFileStats)) {
    return -1;
  }
  if (ta_stat(_fmqBufPath.c_str(), &bufFileStats)) {
    return -1;
  }
  
  // compute age, check for max valid age
  
  if (max_valid_age >= 0) {
    time_t now = time(NULL);
    time_t file_age = now - statFileStats.st_mtime;
    if (file_age > max_valid_age) {
      if (_debug && _tooOldPrint) {
	cerr << "LdataInfo::_openReadFmq" << endl;
	cerr << "  Fmq " << _fmqStatPath << " too old." << endl;
	_tooOldPrint = false;
      }
      return -1;
    }
  }
  
  // open fmq

  if (FMQ_init(&_fmqReadHandle, (char *) _infoPath.c_str(),
	       _debug, "LdataInfo")) {
    _errStr = "ERROR - LdataInfo::_openReadFmq\n";
    TaStr::AddStr(_errStr, "  Cannot init fmq: ", _infoPath);
    cerr << _errStr;
    return -1;
  }
  
  if (FMQ_open_rdonly(&_fmqReadHandle)) {
    _errStr = "ERROR - LdataInfo::_openReadFmq\n";
    TaStr::AddStr(_errStr, "  Cannot open fmq: ", _infoPath);
    cerr << _errStr;
    FMQ_free(&_fmqReadHandle);
    return -1;
  }
  
  _fmqReadOpen = true;

  if (_readFmqFromStart) {
    // seek to start
    FMQ_seek_start(&_fmqReadHandle);
  } else {
    // seek to last entry
    FMQ_seek_last(&_fmqReadHandle);
  }

  _statFileSize = statFileStats.st_size;
  _bufFileSize = bufFileStats.st_size;

  return 0;
  
}

////////////////////////
// close FMQ for reading

void LdataInfo::_closeReadFmq()
{
  if (_fmqReadOpen) {
    FMQ_close(&_fmqReadHandle);
    FMQ_free(&_fmqReadHandle);
    _fmqReadOpen = false;
  }
}

////////////////////////////////
// check files for reading
//
// Sets flags to show if files are not old relative to one another

void LdataInfo::_checkFilesForReading(int max_valid_age,
				      bool &fmqOK,
				      bool &xmlOK,
				      bool &asciiOK)
  
{
  
  fmqOK = true;
  xmlOK = true;
  asciiOK = true;

  struct stat statFileStats, bufFileStats;
  struct stat xmlFileStats, asciiFileStats;

  if (ta_stat(_fmqStatPath.c_str(), &statFileStats)) {
    fmqOK = false;
  }
  if (ta_stat(_fmqBufPath.c_str(), &bufFileStats)) {
    fmqOK = false;
  }
  if (ta_stat(_xmlPath.c_str(), &xmlFileStats)) {
    xmlOK = false;
  }
  if (ta_stat(_infoPath.c_str(), &asciiFileStats)) {
    asciiOK = false;
  }

  if (max_valid_age > 0) {

    time_t now = time(NULL);

    if (xmlOK) {
      int age = now - xmlFileStats.st_mtime;
      if (age > max_valid_age) {
	xmlOK = false;
      }
    }

    if (asciiOK) {
      int age = now - asciiFileStats.st_mtime;
      if (age > max_valid_age) {
	asciiOK = false;
      }
    }

  }

}

///////////////////////////////
// make directory if necessary
  
int LdataInfo::_makeDir() const

{
  
  struct stat dirStat;
  if (ta_stat(_dataDirPath.c_str(), &dirStat)) {
    if (ta_makedir_recurse(_dataDirPath.c_str())) {
      int errNum = errno;
      _errStr = "ERROR - LdataInfo::_makeDir\n";
      TaStr::AddStr(_errStr, "  Cannot create data directory: ", _dataDirPath);
      TaStr::AddStr(_errStr, strerror(errNum));
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////
// Lock file for writing

int LdataInfo::_lockForWrite() const
  
{

  // create lock file if needed
  
  struct stat lockStat;
  if (ta_stat(_lockPath.c_str(), &lockStat)) {
    if ((_lockFile = fopen(_lockPath.c_str(), "w")) == NULL) {
      int errNum = errno;
      _errStr = "ERROR - LdataInfo::_lockForWrite\n";
      TaStr::AddStr(_errStr, "  Cannot create lock file: ", _lockPath);
      TaStr::AddStr(_errStr, strerror(errNum));
      return -1;
    }
    _closeLockFile();
  }
  
  // open lock file
  
  if ((_lockFile = fopen(_lockPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    _errStr = "ERROR - LdataInfo::_lockForWrite\n";
    TaStr::AddStr(_errStr, "  Cannot open lock file: ", _lockPath);
    TaStr::AddStr(_errStr, strerror(errNum));
    return -1;
  }

  // lock the file
  
  if (ta_lock_file(_lockPath.c_str(), _lockFile, "w")) {
    _errStr = "ERROR - LdataInfo::_lockForWrite\n";
    TaStr::AddStr(_errStr, "  Cannot lock file: ", _lockPath);
    _closeLockFile();
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////
// Unlock for writing

void LdataInfo::_unlockForWrite() const

{

  // unlock the file
  
  if (ta_unlock_file(_lockPath.c_str(), _lockFile)) {
    _errStr = "WARNING - LdataInfo::_unlockForWrite\n";
    TaStr::AddStr(_errStr, "  Cannot unlock file: ", _lockPath);
  }

  _closeLockFile();
  
}

///////////////////////////////////////
// Close lock file

void LdataInfo::_closeLockFile() const

{
  
  if (_lockFile) {
    fclose(_lockFile);
    _lockFile = NULL;
  }
  
}
  
///////////////////////////////////////
// _writeAscii()
//
// Write the ASCII file.

int LdataInfo::_writeAscii() const

{

  // remove tmp file if it exists
  
  const char *tmp_path = _tmpInfoPath.c_str();
  unlink(tmp_path);
  
  {

    // open tmp file
    
    ofstream out (tmp_path, ios::out);
    if (!out) {
      _errStr = "ERROR - LdataInfo::_writeAscii\n";
      TaStr::AddStr(_errStr, "  Cannot create tmp info file: ", tmp_path);
      cerr << _errStr;
      return -1;
    }
    
    // write the info to the tmp file
    
    print(out);

  } // out of scope, so file closes

  // remove output file to ensure that NFS cache updates correctly
  
  const char *file_path = _infoPath.c_str();
  unlink(file_path);
  
  // rename tmp file to final name
  
  if(rename(tmp_path, file_path)) {
    int errNo = errno;
    _errStr = "ERROR - LdataInfo::_writeAscii\n";
    TaStr::AddStr(_errStr, "  Cannot rename tmp file: ", tmp_path);
    TaStr::AddStr(_errStr, "  to: ", file_path);
    TaStr::AddStr(_errStr, strerror(errNo));
    cerr << _errStr;
    return -1;
  }

  return 0;

}

///////////////////////////////////////
// _writeXml()
//
// Write in XML format

int LdataInfo::_writeXml() const

{

  // remove tmp file if it exists
  
  const char *tmp_path = _tmpInfoPath.c_str();
  unlink(tmp_path);

  {

    // open tmp file
    
    ofstream out (tmp_path, ios::out);
    if (!out) {
      _errStr = "ERROR - LdataInfo::_writeXml\n";
      TaStr::AddStr(_errStr, "  Cannot create tmp info file: ", tmp_path);
      cerr << _errStr;
      return -1;
    }
    
    // write the info to the tmp file
    
    printAsXml(out);
    
  } // out of scope, so file closes

  // remove output file to ensure that NFS cache updates correctly
  
  const char *xml_path = _xmlPath.c_str();
  unlink(xml_path);

  // rename tmp file to final name
  
  if(rename(tmp_path, xml_path)) {
    int errNo = errno;
    _errStr = "ERROR - LdataInfo::_writeXml\n";
    TaStr::AddStr(_errStr, "  Cannot rename tmp file: ", tmp_path);
    TaStr::AddStr(_errStr, "  to: ", xml_path);
    TaStr::AddStr(_errStr, strerror(errNo));
    cerr << _errStr;
    return -1;
  }

  return 0;

}

///////////////////////////////////////
// _writeFmq()
//

int LdataInfo::_writeFmq() const

{

  // init handle

  FMQ_handle_t fmq;
  if (FMQ_init(&fmq, (char *) _infoPath.c_str(),
	       _debug, "LdataInfo")) {
    _errStr = "ERROR - LdataInfo::_writeFmq\n";
    TaStr::AddStr(_errStr, "  Cannot init fmq: ", _infoPath);
    return -1;
  }

  // open

  if (FMQ_open_rdwr(&fmq, _fmqNSlots, _fmqBufSize)) {
    if (FMQ_open_create(&fmq, _fmqNSlots, _fmqBufSize)) {
      _errStr = "ERROR - LdataInfo::_writeFmq\n";
      TaStr::AddStr(_errStr, "  Cannot open fmq: ", _infoPath);
      FMQ_free(&fmq);
      return -1;
    } else {
      if (_debug) {
        cerr << "--->> Creating new FMQ with requested size" << endl;
        cerr << "      nslots: " << _fmqNSlots << endl;
        cerr << "      bufsize: " << _fmqBufSize << endl;
      }
    }
  }

  // set compression on

  FMQ_set_compress(&fmq);
  FMQ_set_compression_method(&fmq, TA_COMPRESSION_ZLIB);

  // assemble the object into the buffer

  assemble();
  
  // write out serialized object

  if (FMQ_write(&fmq, _buf.getPtr(), _buf.getLen(), 0, 0)) {
    _errStr = "ERROR - LdataInfo::_writeFmq\n";
    TaStr::AddStr(_errStr, "  Cannot write to fmq: ", _infoPath);
    FMQ_close(&fmq);
    FMQ_free(&fmq);
    return -1;
  }

  // free up

  FMQ_free(&fmq);

  return 0;

}

////////////////////////////////
// write the catalog if required
//
// The catalog file is written if the file
// '_ldata_write_catalog' exists in the data directory.

void LdataInfo::_writeCatalog() const

{

  // check for existence of _ldata_write_catalog file

  char catalogTriggerPath[MAX_PATH_LEN];
  sprintf(catalogTriggerPath, "%s%s%s",
          _dataDirPath.c_str(), PATH_DELIM, "_ldata_write_catalog");
  struct stat fileStat;
  if (ta_stat(catalogTriggerPath, &fileStat)) {
    // trigger file does not exist, so return now
    return;
  }

  // create day directory if needed
  
  char dayDir[MAX_PATH_LEN];
  DateTime day(_latestTime);
  sprintf(dayDir, "%s%s%.4d%.2d%.2d",
          _dataDirPath.c_str(), PATH_DELIM,
          day.getYear(), day.getMonth(), day.getDay());
  struct stat dirStat;
  if (ta_stat(dayDir, &dirStat)) {
    if (ta_makedir_recurse(dayDir)) {
      int errNo = errno;
      _errStr = "ERROR - LdataInfo::_writeCatalog\n";
      TaStr::AddStr(_errStr, " Cannot create day directory: ", dayDir);
      TaStr::AddStr(_errStr, strerror(errNo));
      cerr << _errStr;
      return;
    }
  }

  // open catalog file for appending

  char catalogPath[MAX_PATH_LEN];
  sprintf(catalogPath, "%s%s%s",
          dayDir, PATH_DELIM, "ldata_file_catalog");

  FILE *cat;
  if ((cat = fopen(catalogPath, "a")) == NULL) {
    int errNo = errno;
    _errStr = "ERROR - LdataInfo::_writeCatalog\n";
    TaStr::AddStr(_errStr, " Cannot open catalog file: ", catalogPath);
    TaStr::AddStr(_errStr, strerror(errNo));
    cerr << _errStr;
    return;
  }

  // load xml buffer

  _loadXmlForCatalog();

  // write out

  fprintf(cat, "%s\n", (char *) _xmlBuf.getPtr());

  // close catalog file
  
  fclose(cat);
  
}
  
/////////////////
// _guessDataType
//
// Guess at the data type, from the info path

void LdataInfo::_guessDataType() const
  
{
  
  string dir = _dataDir;
  string delim = PATH_DELIM;
  const int nTypes = 11;
  string typeList[nTypes] = {"grib", "mdv", "md", "nc" "raw",
			     "sim", "simulate", "spdb",
			     "titan", "www", "www_content"};
  
  for (int ii = 0; ii < nTypes; ii++) {
    bool typeMatched = false;
    string search1 = delim + typeList[ii] + delim;
    if (dir.find(search1, 0) != string::npos) {
      typeMatched = true;
    } else {
      string search2 = typeList[ii] + delim;
      if (dir.find(search2, 0) != string::npos) {
	typeMatched = true;
      }
    }
    if (typeMatched) {
      _dataTypeGuess = typeList[ii];
      return;
    }
  } // i
}

//////////////////
// load XML buffer
//
// Load up a MemBuf with an XML representation of this object

void LdataInfo::_loadXmlBuf() const
  
{

  _xmlBuf.free();

  // header

  char str[4096];
  sprintf(str, "<latest_data_info>\n");
  _xmlBuf.add(str, strlen(str));

  // latest_time

  date_time_t ltime;
  ltime.unix_time = getLatestTime();
  uconvert_from_utime(&ltime);

  sprintf(str, "  <unix_time>%ld</unix_time>\n", ltime.unix_time);
  _xmlBuf.add(str, strlen(str));

  sprintf(str, "  <year>%.4d</year>\n", ltime.year);
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <month>%.2d</month>\n", ltime.month);
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <day>%.2d</day>\n", ltime.day);
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <hour>%.2d</hour>\n", ltime.hour);
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <min>%.2d</min>\n", ltime.min);
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <sec>%.2d</sec>\n", ltime.sec);
  _xmlBuf.add(str, strlen(str));

  // file path and extension
  
  sprintf(str, "  <rel_data_path>%s</rel_data_path>\n", _relDataPath.c_str());
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <file_ext>%s</file_ext>\n", getDataFileExt().c_str());
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <data_type>%s</data_type>\n",getDataType().c_str());
  _xmlBuf.add(str, strlen(str));
  
  //    string urlStr = "http://";
  //    urlStr += PORThostnameFull();
  //    urlStr += getDataDirPath();
  //    urlStr += PATH_DELIM;
  //    urlStr += getRelDataPath();
  //    sprintf(str, "  <url>%s</url>\n", urlStr.c_str());
  //    _xmlBuf.add(str, strlen(str));
  
  // user_info_1, user_info_2
  
  sprintf(str, "  <user_info1>%s</user_info1>\n", _userInfo1.c_str());
  _xmlBuf.add(str, strlen(str));
  sprintf(str, "  <user_info2>%s</user_info2>\n", _userInfo2.c_str());
  _xmlBuf.add(str, strlen(str));

  // forecast?

  if (_isFcast) {
    sprintf(str, "  <is_forecast>true</is_forecast>\n");
    _xmlBuf.add(str, strlen(str));
  } else {
    sprintf(str, "  <is_forecast>false</is_forecast>\n");
    _xmlBuf.add(str, strlen(str));
  }
  sprintf(str, "  <forecast_lead_secs>%d</forecast_lead_secs>\n", getLeadTime());
  _xmlBuf.add(str, strlen(str));

  // displaced dir path

  if (_displacedDirPath.size() > 0) {
    sprintf(str, "  <displaced_dir_path>%s</displaced_dir_path>\n",
            _displacedDirPath.c_str());
    _xmlBuf.add(str, strlen(str));
  }
  
  // writer app

  sprintf(str, "  <writer>%s</writer>\n", _writer.c_str());
  _xmlBuf.add(str, strlen(str));

  // maximum time for this data set
  // does not go backwards

  sprintf(str, "  <max_time>%ld</max_time>\n", _maxTime);
  _xmlBuf.add(str, strlen(str));

  // previous mod time

  sprintf(str, "  <prev_mod_time>%ld</prev_mod_time>\n", _prevModTime);
  _xmlBuf.add(str, strlen(str));

  // end

  sprintf(str, "</latest_data_info>\n");
  _xmlBuf.add(str, strlen(str));
  
  // add null

  _xmlBuf.add("\0", 1);

}

///////////////////////
// load XML for catalog
//
// Single line, no newlines or spaces

void LdataInfo::_loadXmlForCatalog() const
  
{

  _xmlBuf.free();
  char str[4096];

  // latest_time

  date_time_t ltime;
  ltime.unix_time = getLatestTime();
  uconvert_from_utime(&ltime);

  sprintf(str, "<utime>%ld</utime>", ltime.unix_time);
  _xmlBuf.add(str, strlen(str));
  
  sprintf(str, "<time>%.4d-%.2d-%.2dT%.2d:%.2d:%.2d</time>",
          ltime.year, ltime.month, ltime.day, ltime.hour, ltime.min, ltime.sec);
  _xmlBuf.add(str, strlen(str));
  
  sprintf(str, "<max_time>%ld</max_time>", _maxTime);
  _xmlBuf.add(str, strlen(str));
  
  // forecast?

  if (_isFcast) {
    sprintf(str, "<lead_secs>%d</lead_secs>", getLeadTime());
    _xmlBuf.add(str, strlen(str));
  }

  // file path and extension
  
  sprintf(str, "<rpath>%s</rpath>", _relDataPath.c_str());
  _xmlBuf.add(str, strlen(str));
  
  if (getDataFileExt().size() > 0) {
    sprintf(str, "<ext>%s</ext>", getDataFileExt().c_str());
    _xmlBuf.add(str, strlen(str));
  }

  if (getDataType().size() > 0) {
    sprintf(str, "<dtype>%s</dtype>",getDataType().c_str());
    _xmlBuf.add(str, strlen(str));
  }
  
  // add null
  
  _xmlBuf.add("\0", 1);

}

//////////////////////////////
// find field from XML buffer
//
// returns 0 on success, -1 on failure

int LdataInfo::_findXmlField(const char *xml_buf,
			     const char *field_name, string &val)
  
{

  // compute start and end tokens

  int fieldNameLen = strlen(field_name);
  TaArray<char> startArray, endArray;
  char *startTok = startArray.alloc(fieldNameLen + 8);
  char *endTok = endArray.alloc(fieldNameLen + 8);
  
  sprintf(startTok, "<%s>", field_name);
  sprintf(endTok, "</%s>", field_name);
  
  // find start and end tokens

  const char *startPtr = strstr(xml_buf, startTok);
  const char *endPtr = strstr(xml_buf, endTok);
  
  if (startPtr == NULL) {
    return -1;
  }
  if (endPtr == NULL) {
    _debugPrint("Cannot find field end tok in XML buffer: %s", endTok);
    return -1;
  }

  startPtr += strlen(startTok);
  
  if (endPtr < startPtr) {
    _debugPrint("Bad format in XML buffer, field: %s", startTok);
    return -1;
  }

  // copy the text between the tokens to the value field

  int valLen = endPtr - startPtr;
  TaArray<char> tmpArray;
  char *tmpStr = tmpArray.alloc(valLen + 8);
  memcpy(tmpStr, startPtr, valLen);
  tmpStr[valLen] = '\0';
  val = tmpStr;
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble XML-style buffer, load object
//
// returns 0 on success, -1 on failure

int LdataInfo::_disassembleFromXml(const char *xml_buf, int len)
  
{

  if (_debug) {
    cerr << "---------- _disassembleFromXml --------------" << endl;
  }

  // initialize

  _clearInfo();

  // make sure the buffer is null-terminated

  TaArray<char> bufArray;
  char *buf = bufArray.alloc(len + 1);
  memcpy(buf, xml_buf, len);
  buf[len] = '\0';

  // check for closing phrase

  if (strstr(buf, "</latest_data_info>") == NULL) {
    _debugPrint("Cannot find </latest_data_info> token in XML buffer");
    return -1;
  }

  // find the time strings - must be present

  bool unixTimeFound = true;
  bool dateTimeFound = true;

  string unix_time_str = "-1";
  if (_findXmlField(xml_buf, "unix_time", unix_time_str)) {
    unixTimeFound = false;
  }
  string year_str;
  if (_findXmlField(xml_buf, "year", year_str)) {
    dateTimeFound = false;
  }
  string month_str;
  if (_findXmlField(xml_buf, "month", month_str)) {
    dateTimeFound = false;
  }
  string day_str;
  if (_findXmlField(xml_buf, "day", day_str)) {
    dateTimeFound = false;
  }
  string hour_str;
  if (_findXmlField(xml_buf, "hour", hour_str)) {
    dateTimeFound = false;
  }
  string min_str;
  if (_findXmlField(xml_buf, "min", min_str)) {
    dateTimeFound = false;
  }
  string sec_str;
  if (_findXmlField(xml_buf, "sec", sec_str)) {
    dateTimeFound = false;
  }

  // set time

  long unix_time;
  int year, month, day, hour, min, sec;

  if (unixTimeFound) {
    if (sscanf(unix_time_str.c_str(), "%ld", &unix_time) != 1) {
      _debugPrint("Cannot decode unix_time in XML buffer");
      unixTimeFound = false;
    }
  }
  if (dateTimeFound) {
    if (sscanf(year_str.c_str(), "%d", &year) != 1) {
      _debugPrint("Cannot decode year in XML buffer");
      dateTimeFound = false;
    }
    if (sscanf(month_str.c_str(), "%d", &month) != 1) {
      _debugPrint("Cannot decode month in XML buffer");
      dateTimeFound = false;
    }
    if (sscanf(day_str.c_str(), "%d", &day) != 1) {
      _debugPrint("Cannot decode day in XML buffer");
      dateTimeFound = false;
    }
    if (sscanf(hour_str.c_str(), "%d", &hour) != 1) {
      _debugPrint("Cannot decode hour in XML buffer");
      dateTimeFound = false;
    }
    if (sscanf(min_str.c_str(), "%d", &min) != 1) {
      _debugPrint("Cannot decode min in XML buffer");
      dateTimeFound = false;
    }
    if (sscanf(sec_str.c_str(), "%d", &sec) != 1) {
      _debugPrint("Cannot decode sec in XML buffer");
      dateTimeFound = false;
    }
  }

  if (!unixTimeFound && !dateTimeFound) {
    _debugPrint("Could not find unix or normal times\n");
    return -1;
  }

  if (dateTimeFound) {
    DateTime dtime(year, month, day, hour, min, sec);
    if (!unixTimeFound) {
      unix_time = dtime.utime();
    } else {
      if (unix_time != dtime.utime()) {
	_debugPrint("WARNING - unix time not correct: %ld, should be %ld\n"
		    "  Setting to %.4d/%.2d%/.2d %.2d:%.2d:%.2d\n",
		    unix_time,  dtime.utime(),
		    dtime.getYear(), dtime.getMonth(), dtime.getDay(),
		    dtime.getHour(), dtime.getMin(), dtime.getSec());
	unix_time = dtime.utime();
      }
    }
  }

  _latestTime = unix_time;

  // get other info

  string rel_data_path_str;
  if (_findXmlField(xml_buf, "rel_data_path", rel_data_path_str) == 0) {
    setRelDataPath(rel_data_path_str.c_str());
  }
  
  string writer_str;
  if (_findXmlField(xml_buf, "writer", writer_str) == 0) {
    setWriter(writer_str.c_str());
  }

  string file_ext_str;
  if (_findXmlField(xml_buf, "file_ext", file_ext_str) == 0) {
    setDataFileExt(file_ext_str);
  }

  string data_type_str;
  if (_findXmlField(xml_buf, "data_type", data_type_str) == 0) {
    setDataType(data_type_str.c_str());
  }

  string user_info1_str;
  if (_findXmlField(xml_buf, "user_info1", user_info1_str) == 0) {
    setUserInfo1(user_info1_str.c_str());
  }

  string user_info2_str;
  if (_findXmlField(xml_buf, "user_info2", user_info2_str) == 0) {
    setUserInfo2(user_info2_str.c_str());
  }

  string is_forecast_str;
  if (_findXmlField(xml_buf, "is_forecast", is_forecast_str) == 0) {
    if (is_forecast_str == "true") {
      setIsFcast(true);
      string forecast_lead_secs_str;
      if (_findXmlField(xml_buf, "forecast_lead_secs", forecast_lead_secs_str) == 0) {
	int lead_time = 0;
	if (sscanf(forecast_lead_secs_str.c_str(), "%d", &lead_time) == 1) {
	  setLeadTime(lead_time);
	} else {
	  _debugPrint("Cannot decode forecast_lead_secs in XML buffer");
	}
      }
    } else {
      setIsFcast(false);
    } // if (is_forecast_str == "true")
  }
    
  string displaced_str;
  if (_findXmlField(xml_buf, "displaced_dir_path", displaced_str) == 0) {
    setDisplacedDirPath(displaced_str.c_str());
  }

  // max time

  string max_time_str;
  if (_findXmlField(xml_buf, "max_time", max_time_str) == 0) {
    if ( sscanf(max_time_str.c_str(), "%ld", &_maxTime) != 1){
       _maxTime = _latestTime;
    }
  } else {
    _maxTime = _latestTime;
  }

  // previous mod time

  string prev_mod_time_str;
  if (_findXmlField(xml_buf, "prev_mod_time", prev_mod_time_str) == 0) {
    if (sscanf(prev_mod_time_str.c_str(), "%ld", &_prevModTime) != 1){
        _debugPrint("Error reading and setting _prevModTime member from xml string");
    }
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble old-style buffer, load object
//
// returns 0 on success, -1 on failure

int LdataInfo::_disassembleFromOld(const void *in_buf, int len)

{

  if (_debug) {
    cerr << "---------- _disassembleFromOld --------------" << endl;
  }

  // some fields are not available

  _relDataPath = "unknown";
  _writer = "unknown";
  _dataType = "unknown";

  int minLen = (int) sizeof(info_t);
  if (len < minLen) {
    _errStr = "ERROR - LdataInfo::disassemble\n";
    TaStr::AddInt(_errStr, "Buffer too short, len: ", len);
    TaStr::AddInt(_errStr, "Expected len: ", minLen);
    cerr << _errStr;
    return -1;
  }

  ui08 *ptr = (ui08 *) in_buf;
  info_t info;
  memcpy(&info, ptr, sizeof(info_t));
  ptr += sizeof(info_t);
  BEtoInfo(info);
  setFromInfo(info);
  _maxTime = _latestTime;

  if (_isFcast) {
    int fcastLen = minLen + sizeof(si32);
    if (len < fcastLen) {
      _errStr = "ERROR - LdataInfo::disassemble\n";
      TaStr::AddInt(_errStr, "Buffer too short, len: ", len);
      TaStr::AddInt(_errStr, "Expected len: ", fcastLen);
      cerr << _errStr;
      return -1;
    }
    si32 leadTime;
    memcpy(&leadTime, ptr, sizeof(si32));
    ptr += sizeof(si32);
    BE_to_array_32(&leadTime, sizeof(si32));
    _fcastLeadTime = leadTime;
  }

  return 0;

}

//////////////////////////////
// print if in debug mode
//
// returns 0 on success, -1 on failure

void LdataInfo::_debugPrint(const char* format, ...)
  
{

  if (!_debug) {
    return;
  }

  char formattedMsg[4096];
  va_list args;
  va_start(args, format);
  vsprintf(formattedMsg, format, args);
  va_end(args);
  cerr << "WARNING - LdataInfo class" << endl;
  cerr << "  " << formattedMsg << endl;

}

