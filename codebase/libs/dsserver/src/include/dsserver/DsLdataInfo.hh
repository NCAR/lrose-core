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

#ifndef DS_LDATA_INFO_HH
#define DS_LDATA_INFO_HH

////////////////////////////////////////////////////////////////////
// DsLdataInfo.hh
//
// DsLdataInfo class.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// May 2009
//
////////////////////////////////////////////////////////////////////
//
// This class builds on the didss/LdataInfo class.
// It adds the capability to contact the DataMapper in the 
// write() method.
//
// See also <didss/LdataInfo.hh>
//
/////////////////////////////////////////////////////////////////////

#include <didss/LdataInfo.hh>
#include <dsserver/DsLdataMsg.hh>
#include <toolsa/Socket.hh>
using namespace std;

class DsLdataInfo : public LdataInfo {

public:
  
  //////////////////////
  // Default constructor
  //
  // The default directory is '.'
  // This may be overridden by calling setDir() and setDirFromUrl().

  DsLdataInfo();

  /////////////////////////////////////////
  // constructor - with url string supplied.
  //
  // Set debug to activate debug prints.
  // Specify the file name if you do not wish to use the
  // usual file name (latest_data_info).

  DsLdataInfo(const string &urlStr,
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

  DsLdataInfo(const DsURL &url,
	      bool debug = false,
	      const char *file_name = LDATA_INFO_FILE_NAME);

  ////////////////////
  // copy constructor
  
  DsLdataInfo(const DsLdataInfo &orig);

  ///////////////////////////////////
  // copy constructor from LdataInfo
  
  DsLdataInfo(const LdataInfo &orig);

  ////////////////////////
  // assignment (and copy)
  //
  // Note: only copies information, not FILE pointers or other
  // status.
  
  DsLdataInfo &operator=(const DsLdataInfo &other);

  //////////////
  // destructor
  virtual ~DsLdataInfo();
  
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
  // Sets the directory from the URL.
  // The directory is set to the file portion of the URL.
  //
  // For example, if the URL is mdvp:://::mdv/sat/vis, the
  // directory will be mdv/sat/vis.
  //
  // Returns 0 on success, -1 on failure
  
  virtual int setDirFromUrl(const DsURL &url);
  virtual int setDirFromUrl(const string &urlStr);

  /////////////////////////////////////////////////////////////////////
  // setReadFmqFromStart
  //
  // For use when reading FMQs. Setting this true allows you to read the
  // entire queue on startup.
  // Be default it is false. The default behavior is to read the latest
  // element only.

  virtual void setReadFmqFromStart(bool read_from_start = true);

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

  //////////////////////////////////////////////////////////////////////
  // Set flag to indicate that we will not register with the DataMapper
  // if the latest time is earlier than the max time - i.e. the
  // latest time has moved backwards.
  //
  // This is useful if you want to make sure that the DataMapper only
  // has the max time - i.e. the latest time cannot move backwards.
  //
  // The default state is FALSE - i.e. register every event.

  void setNoRegIfLatestTimeInPast() { _noRegIfLatestTimeInPast = true; }

  ////////////////////////////////////////////////////
  // Set the directory of displaced data set.
  // Normally the dataset and the _latest_data_info files are
  // co-located. However, sometimes _latest_data_info refers to files
  // which are actually stored in a different (displaced) location.
  // If this string is non-empty, then the data resides in the
  // displaced directory. If the string is set, data resides in
  // the displaced location.

  virtual void setDisplacedDirPath(const char *dir) {
    string dirStr(dir);
    setDisplacedDirPath(dirStr);
  }
  virtual void setDisplacedDirPath(const string &dir);
  
  //////////////////////////////////////
  // FMQ control
  //
  // FMQ is on by default.

  virtual void setUseFmq(bool use_fmq = true);
  virtual void setFmqNSlots(int nslots);

  //////////////////////////////////////
  // XML and ASCII control
  //
  // Both are on by default.

  virtual void setUseXml(bool use_xml = true);
  virtual void setUseAscii(bool use_ascii = true);

  /////////////////////////////////////////////////////////////////
  // read()
  //
  //   max_valid_age:
  //     This is the max age (in secs) for which the 
  //     latest data info is considered valid. If the info is
  //     older than this, we need to wait for new info.
  //     If max_valid_age is set negative, the age test is not done.
  //
  // Side effect:
  //    If new data found, sets _prevModTime to file modify time.
  //
  // Returns:
  //    0 on success, -1 on failure.
  //

  virtual int read(int max_valid_age = -1);
  
  /////////////////////////////////////////////////////////////////
  // readBlocking()
  //
  //   max_valid_age: see read()
  //
  //   sleep_msecs (millisecs):
  //     While in the blocked state, the program sleeps for sleep_msecs
  //     millisecs at a time before checking again.
  //
  //   heartbeat_func(): heartbeat function
  //     Just before sleeping each time, heartbeat_func() is called
  //     to allow any heartbeat actions to be carried out.
  //     If heartbeat_func is NULL, it is not called.
  //     The string arg passed to the heartbeat
  //     function is "In LdataInfo::readBlocking".
  //
  // Side effect:
  //    If new data found, sets _prevModTime to file modify time.
  //

  virtual void readBlocking(int max_valid_age,
			    int sleep_msecs,
			    heartbeat_t heartbeat_func);

  //////////////////////////////////////////////////////////////////
  // readForced()
  //
  // Force a read on the _latest_data_info.
  // if younger than max_valid_age.
  //     If max_valid_age is set negative, the age test is not done.
  //
  // Side effect:
  //    If new data found, sets _prevModTime to file modify time.
  //
  // Returns:
  //    0 on success, -1 on failure.
  
  virtual int readForced(int max_valid_age = -1,
                         bool update_prev_mod_time = true);
  
  /////////////////////////////////////////////////////////////
  // write()
  //
  // Writes latest info.
  //
  // Registers the latest data time with the DataMapper with the
  // specified datatype.
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
  // Returns:
  //   0 on success, -1 on failure.
  //
  // NOTES: If the environment variable $LDATA_NO_WRITE is set to 'true',
  //        this funciton does nothing.  This can be useful when regenerating
  //        old data while also running in realtime mode.

  virtual int write(time_t latest_time = 0,
		    const string &datatype = "") const; 

protected:

  string _urlStr;
  mutable DsURL _url;

  bool _firstRead;
  mutable bool _useServer;
  mutable Socket _sock;
  mutable DsLdataMsg _requestMsg;
  mutable DsLdataMsg _replyMsg;

  // flag to indicate that we will not register with the DataMapper
  // if the latest time is earlier than the max time - i.e. the
  // latest time has moved backwards

  bool _noRegIfLatestTimeInPast;

  // methods

  void _dsInit();
  DsLdataInfo &_dsCopy(const DsLdataInfo &other);

  int _openLdataServer() const;
  int _resolveUrl();
  void _closeLdataServer() const;
  
  int _readFromDsLdataServer(int max_valid_age, bool forced);
  int _writeToDsLdataServer() const;
  int _writeToDataMapper() const;
  
  int _commWithServer() const;

private:
  
};

#endif

