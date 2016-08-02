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
// This class derives from the didss/LdataInfo class.
//
// It allows remote access to the DsLdataServer
//
// See also <didss/LdataInfo.hh>
//
/////////////////////////////////////////////////////////////////////

#include <dsserver/DsLdataInfo.hh>
#include <dsserver/DmapAccess.hh>
#include <dsserver/DsLocator.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/GetHost.hh>
using namespace std;

//////////////////////
// Default constructor
//
// The default directory is '.'
// This may be overridden by calling setDir() and setDirFromUrl().
//

DsLdataInfo::DsLdataInfo() : LdataInfo()

{
  _dsInit();
}

///////////////////////////////////////////
// constructor - with url string supplied.
//
// Set debug to activate debug prints.
// Specify the file name if you do not wish to use the
// usual file name (latest_data_info).
//

DsLdataInfo::DsLdataInfo(const string &urlStr,
                         bool debug /* = false */,
                         const char *file_name /* = LDATA_INFO_FILE_NAME */) :
        LdataInfo("", debug, file_name)
  
{
  _dsInit();
  setDirFromUrl(urlStr);
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

DsLdataInfo::DsLdataInfo(const DsURL &url,
			 bool debug /* = false*/,
			 const char *file_name /* = LDATA_INFO_FILE_NAME*/ ) :
        LdataInfo(url, debug, file_name)
  
{
  _dsInit();
  setDirFromUrl(url);
}

////////////////////
// copy constructor

DsLdataInfo::DsLdataInfo(const DsLdataInfo &orig)
  
{
  _dsInit();
  LdataInfo::_init(false, LDATA_INFO_FILE_NAME);
  LdataInfo::copy(orig);
  _dsCopy(orig);
}

///////////////////////////////////
// copy constructor from LdataInfo

DsLdataInfo::DsLdataInfo(const LdataInfo &orig)

{
  _dsInit();
  LdataInfo::_init(false, LDATA_INFO_FILE_NAME);
  LdataInfo::copy(orig);
}

//////////////
// assignment
//
// Note: only copies information, not FILE pointers or other
// status.

DsLdataInfo &DsLdataInfo::operator=(const DsLdataInfo &other)
{
  if (this == &other) {
    return *this;
  }
  _dsInit();
  LdataInfo::_init(false, LDATA_INFO_FILE_NAME);
  LdataInfo::copy(other);
  _dsCopy(other);
  return *this;
}

//////////////
// destructor

DsLdataInfo::~DsLdataInfo()
{
  _closeLdataServer();
}

//////////////////////
// Initialization

void DsLdataInfo::_dsInit()

{

  _firstRead = true;
  _useServer = false;
  _noRegIfLatestTimeInPast = false;

}

/////////////////////////////////////////////////////////////
// make a copy
//
// Note: only copies information, not FILE pointers or other
// status.

DsLdataInfo &DsLdataInfo::_dsCopy(const DsLdataInfo &other)
{

  if (this == &other) {
    return *this;
  }

  _firstRead = other._firstRead;
  _useServer = other._useServer;
  _noRegIfLatestTimeInPast = other._noRegIfLatestTimeInPast;
  setDirFromUrl(other._url);

  return *this;

}

//////////////////////////////////////
// setDir()
//
// Sets the directory from supplied string.
//
// returns 0 on success, -1 on failure
  
int DsLdataInfo::setDir(const string &dataDir)

{

  // force local

  _useServer = false;

  // use base class

  return LdataInfo::setDir(dataDir);

}

//////////////////////////////////////
// setDirFromUrl()
//
// Sets the directory from the URL.
//
// For example, if the URL is mdvp:://::mdv/sat/vis, the
// directory will be mdv/sat/vis.
//
// Returns 0 on success, -1 on failure

int DsLdataInfo::setDirFromUrl(const string &urlStr)

{
  DsURL url(urlStr);
  return setDirFromUrl(url);
}

int DsLdataInfo::setDirFromUrl(const DsURL &url)

{

  int iret = 0;
  
  // Set a URL with:
  //    protocol: ldatap
  //    host: from url
  //    port: from url

  _urlStr = "ldatap:://" + url.getHost() + "::" + url.getFile();
  _url.setURLStr(_urlStr);
  if (!url.isValid() || !_url.isValid()) {
    cerr << "Trying to set dir from bad URL: " << url.getURLStr() << endl;
    cerr << _url.getErrString() << endl;
    return -1;
  }

  if (LdataInfo::setDirFromUrl(_url)) {
    cerr << "Failed to get directory from ldatap url " << _urlStr << endl;
    return -1;
  }

  // does the URL point to the local host or not?
  
  _useServer = false;

  // resolve the URL
  // this will set _useServer to true if we need to contact
  // a server

  if (_resolveUrl()) {
    cerr << "Failed to resolve ldatap url " << _urlStr << endl;
    return -1;
  }

  // wait for first read or write call to open the server
  // so at this point, all is well

  return iret;

}

////////////////////////////////////////////////////
// Set the directory of displaced data set.
// Normally the dataset and the _latest_data_info files are
// co-located. However, sometimes _latest_data_info refers to files
// which are actually stored in a different (displaced) location.
// If this string is non-empty, then the data resides in the
// displaced directory. If the string is set, data resides in
// the displaced location.

void DsLdataInfo::setDisplacedDirPath(const string &dir)

{
  LdataInfo::setDisplacedDirPath(dir);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_DISPLACED_DIR_PATH);
    _requestMsg.setDisplacedDirPath(dir);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setDisplacedDirPath" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setDisplacedDirPath" << endl;
    cerr << "   socket not open" << endl;
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

void DsLdataInfo::setLdataFileName(const char *file_name)

{
  LdataInfo::setLdataFileName(file_name);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_LDATA_FILE_NAME);
    _requestMsg.setLdataFileName(file_name);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setLdataFileName" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setLdataFileName" << endl;
    cerr << "   socket not open" << endl;
  }

}

//////////////////////////////////////
// XML and ASCII control
//
// Both are on by default.

void DsLdataInfo::setUseXml(bool use_xml /* = true */)
{

  LdataInfo::setUseXml(use_xml);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_USE_XML);
    _requestMsg.setUseXml(use_xml);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setUseXml" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setUseXml" << endl;
    cerr << "   socket not open" << endl;
  }
}

void DsLdataInfo::setUseAscii(bool use_ascii /* = true */)
{

  LdataInfo::setUseAscii(use_ascii);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_USE_ASCII);
    _requestMsg.setUseAscii(use_ascii);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setUseAscii" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setUseAscii" << endl;
    cerr << "   socket not open" << endl;
  }
}

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

void DsLdataInfo::setSaveLatestReadInfo(const string &label,
                                        int max_valid_age /* = -1*/,
                                        bool save /* = true*/)
  
{
  LdataInfo::setSaveLatestReadInfo(label, max_valid_age, save);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_SAVE_LATEST_READ_INFO);
    _requestMsg.setSaveLatestReadInfo(save);
    _requestMsg.setMaxValidAge(max_valid_age);
    _requestMsg.setLatestReadInfoLabel(label);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setSaveLatestReadInfo" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setSaveLatestReadInfo" << endl;
    cerr << "   socket not open" << endl;
  }

}

//////////////////////////////////////
// FMQ control
//
// FMQ is on by default.

void DsLdataInfo::setUseFmq(bool use_fmq /* = true*/)
{

  LdataInfo::setUseFmq(use_fmq);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_USE_FMQ);
    _requestMsg.setUseFmq(use_fmq);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setUseFmq" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setUseFmq" << endl;
    cerr << "   socket not open" << endl;
  }
}

void DsLdataInfo::setFmqNSlots(int nslots)
{

  LdataInfo::setFmqNSlots(nslots);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_FMQ_NSLOTS);
    _requestMsg.setFmqNSlots(nslots);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setFmqNSlots" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setFmqNSlots" << endl;
    cerr << "   socket not open" << endl;
  }
}

/////////////////////////////////////////////////////////////////////
// setReadFmqFromStart
//
// For use when reading FMQs. Setting this true allows you to read the
// entire queue on startup.
// Be default it is false. The default behavior is to read the latest
// element only.

void DsLdataInfo::setReadFmqFromStart(bool read_from_start /* = true*/)

{
  LdataInfo::setReadFmqFromStart(read_from_start);
  if (!_useServer) {
    return;
  }
  if (_sock.isOpen()) {
    _requestMsg.clear();
    _requestMsg.setMode(DsLdataMsg::DS_LDATA_SET_READ_FMQ_FROM_START);
    _requestMsg.setReadFmqFromStart(read_from_start);
    if (_commWithServer()) {
      cerr << "ERROR - DsLdataInfo::setReadFmqFromStart" << endl;
      cerr << "  Communicating with server" << endl;
    }
  }
  else {
    cerr << "ERROR - DsLdataInfo::setReadFmqFromStart" << endl;
    cerr << "   socket not open" << endl;
  }
}

//////////////////////////////////////////////////////////////////
// read()
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

int DsLdataInfo::read(int max_valid_age /* = -1*/ )
  
{

  // for local access, use LdataInfo object
  
  if (!_useServer) {
    return LdataInfo::read(max_valid_age);
  }

  if (!_sock.isOpen()) {
    if (_openLdataServer()) {
      cerr << "ERROR - DsLdataInfo::read - socket not open" << endl;
      return -1;
    }
  }

  // read from DsLdataServer

  if (_readFromDsLdataServer(max_valid_age, false)) {
      return -1;
  }
  
  return 0;

}

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

void DsLdataInfo::readBlocking(int max_valid_age,
			       int sleep_msecs,
			       heartbeat_t heartbeat_func)

{
  while (read(max_valid_age)) {
    if (heartbeat_func != NULL) {
      heartbeat_func("DsLdataInfo::readBlocking");
    }
    umsleep(sleep_msecs);
  }
  return;

}

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

int DsLdataInfo::readForced(int max_valid_age /* = -1*/,
			    bool update_prev_mod_time /* = true */)
  
{

  // for local access, use LdataInfo

  if (!_useServer) {
    return LdataInfo::readForced(max_valid_age);
  }

  if (!_sock.isOpen()) {
    if (_openLdataServer()) {
      cerr << "ERROR - DsLdataInfo::readForced - socket not open" << endl;
      return -1;
    }
  }

  // read from DsLdataServer
  if (_readFromDsLdataServer(max_valid_age, true)) {
    return -1;
  }
  
  return 0;

}

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

int DsLdataInfo::write(time_t latest_time /* = 0 */,
		       const string &datatype /* = ""*/ ) const
  
{
  
  // Return now if $LDATA_NO_WRITE is true.
  
  char *envStr = getenv("LDATA_NO_WRITE");
  if (envStr && STRequal(envStr, "true")) {
    return(0);
  }

  // set latest time if passed in
  
  if (latest_time != 0) {
    _latestTime = latest_time;
  }

  // set data type if passed in

  if (datatype.size() > 0) {
    _dataType = datatype;
  }
  
  if (!_useServer) {

    if (_noRegIfLatestTimeInPast &&
        _latestTime < _maxTime) {
      // If requested, do not register with the DataMapper
      // if the latest time is earlier than the max time
      // i.e. the latest time has moved backwards.
      return 0;
    }
    
    // local write
    
    if (LdataInfo::write(_latestTime)) {
      return -1;
    }

    // write to local data mapper

    if (_writeToDataMapper()) {
      return -1;
    }

  } else {

    // server write

    if (!_sock.isOpen()) {

      if (_openLdataServer()) {
	cerr << "ERROR - DsLdataInfo::write - socket not open" << endl;
	return -1;
      }
    }

    if (_writeToDsLdataServer()) {
      return -1;
    }
  }

  return 0;

}

/////////////////////////////////////
// open connection to DsLdataServer

int DsLdataInfo::_openLdataServer() const

{
  
  // close socket if already open
  
  _closeLdataServer();

  // ping server to check it is available. If the server is up on
  // the specified or default port, fine. If not, the server manager
  // will be contacted to start the server.
  // This will resolve the port as required.
  
  if (DsLocator.pingServer(_url)) {
    cerr << "ERROR - DsLdataInfo::_openLdataServer" << endl;
    cerr << "  Cannot ping server, URL: " << _urlStr << endl;
    cerr << "  Host: " << _url.getHost() << endl;
    cerr << "  Port: " << _url.getPort() << endl;
    return -1;
  }
  
  // open

  if (_sock.open(_url.getHost().c_str(), _url.getPort())) {
    cerr << "ERROR - DsLdataInfo::_openLdataServer" << endl;
    cerr << "  Cannot open socket to server, URL: " << _urlStr << endl;
    cerr << "  Host: " << _url.getHost() << endl;
    cerr << "  Port: " << _url.getPort() << endl;
    return -1;
  }

  // send open message to server

  _requestMsg.clear();
  _requestMsg.setMode(DsLdataMsg::DS_LDATA_OPEN);
  _requestMsg.setUrlStr(_urlStr);
  _requestMsg.setDisplacedDirPath(_displacedDirPath);
  _requestMsg.setLdataFileName(_fileName);
  _requestMsg.setUseXml(_useXml);
  _requestMsg.setUseAscii(_useAscii);
  _requestMsg.setSaveLatestReadInfo(_saveLatestReadInfo);
  _requestMsg.setLatestReadInfoLabel(_latestReadInfoLabel);
  _requestMsg.setUseFmq(_useFmq);
  _requestMsg.setFmqNSlots(_fmqNSlots);
  _requestMsg.setReadFmqFromStart(_readFmqFromStart);
  
  if (_commWithServer()) {
    cerr << "ERROR - DsLdataInfo::_openLdataServer" << endl;
    cerr << "  Communicating with server" << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////
// resolve the URL
// set _isServer flag if we need to communicate via server
// Returns 0 on success, -1 on error

int DsLdataInfo::_resolveUrl()

{

  // Check for validity on the url
  
  if (!_url.isValid()) {
    return -1;
  }
  
  // Make sure the host name is user-specified
  // since the fmq file cannot be located via the DsLocator/DataMapper.
  // The client must know a priori on what host the fmq is located.

  string host = _url.getHost();
  if (host.empty()) {
    cerr << "DsLdataInfo::_resolveUrl" << endl;
    cerr << "  Invalid URL specification: " << _urlStr << endl;
    cerr << "  Host name must be provided in the URL." << endl;
    return -1;
  }
  
  // Determine if this is a local or remote request - do not
  // contact the server manager at this stage
  
  if (DsLocator.resolve(_url, &_useServer, false) != 0) {
    cerr << "DsLdataInfo::_resolveUrl" << endl;
    cerr << "  Cannot resolve URL: " << _urlStr << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////
// close connection to DsLdataServer

void DsLdataInfo::_closeLdataServer() const

{

  if (!_sock.isOpen()) {
    return;
  }

  // set up request to DsLdataServer
  
  _requestMsg.clear();
  _requestMsg.setMode(DsLdataMsg::DS_LDATA_CLOSE);
  
  // read from server
  
  if (_commWithServer()) {
    cerr << "ERROR - DsLdataInfo::_closeLdataServer" << endl;
    cerr << "  Communicating with server" << endl;
  }

  _sock.close();

}

//////////////////////////////////////////////////////////////////
// read from DsLdataServer
// Returns:
//    0 on success, -1 on failure.

int DsLdataInfo::_readFromDsLdataServer(int max_valid_age, bool forced)
  
{
  
  // set up request to DsLdataServer

  _requestMsg.clear();
  _requestMsg.setMode(DsLdataMsg::DS_LDATA_READ);
  _requestMsg.setMaxValidAge(max_valid_age);
  _requestMsg.setReadForced(forced);
  
  // read from server

  if (_commWithServer()) {
    cerr << "ERROR - DsLdataInfo::_readFromDsLdataServer" << endl;
    cerr << "  Communicating with server" << endl;
    _closeLdataServer();
    return -1;
  }

  // do we have data in returned message
  
  if (!_replyMsg.partExists(DsLdataMsg::DS_LDATA_INFO_XML)) {
    // no data
    return -1;
  }

  string infoXml =
    (char *) _replyMsg.getPartByType(DsLdataMsg::DS_LDATA_INFO_XML)->getBuf();
  if (disassemble(infoXml.c_str(), infoXml.size())) {
    cerr << "ERROR - DsLdataInfo::_readFromDsLdataServer" << endl;
    cerr << "  Cannot disassemble XML info in reply" << endl;
    _replyMsg.print(cerr);
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// write to DsLdataServer
// Returns:
//   0 on success, -1 on failure.

int DsLdataInfo::_writeToDsLdataServer() const
  
{
  
  // assemble the current object into an XML buffer

  assemble(true);

  // set up request to DsLdataServer
  
  _requestMsg.clear();
  _requestMsg.setMode(DsLdataMsg::DS_LDATA_WRITE);
  _requestMsg.setLdataXml((char *) getBufPtr());
  
  // write to server

  if (_commWithServer()) {
    cerr << "ERROR - DsLdataInfo::_writeToDsLdataServer" << endl;
    cerr << "  Communicating with server" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// write to data mapper - legacy
// Returns:
//   0 on success, -1 on failure.

int DsLdataInfo::_writeToDataMapper() const
  
{

  DmapAccess dmap;
  if (_isFcast) {
    dmap.regLatestInfo(_latestTime,
		       _dataDirPath,
		       _dataType,
		       _fcastLeadTime);
  } else {
    dmap.regLatestInfo(_latestTime,
		       _dataDirPath,
		       _dataType);
  }
  
  return 0;
  
}

/////////////////////////////
// communicate with server

int DsLdataInfo::_commWithServer() const
{

  // assemble the request message

  _requestMsg.assemble();

  if (_debug) {
    cerr << "DsLdataInfo::_commWithServer" << endl;
    cerr << "========== request message ============" << endl;
    _requestMsg.print(cerr);
    cerr << "=======================================" << endl;
  }

//   cerr << "333333333333333333333333333" << endl;
//   cerr << "Writing message, len: " <<  _requestMsg.lengthAssembled() << endl;
//   ui08 *bdata = (ui08 *) _requestMsg.assembledMsg();
//   for (int ii = 0; ii < _requestMsg.lengthAssembled(); ii++) {
//     cerr << " " << (int) bdata[ii];
//   }
//   cerr << endl;
//   cerr << "333333333333333333333333333" << endl;

//   cerr << "44444444444444444444444444444" << endl;
//   DsLdataMsg testMsg;
//   testMsg.disassemble(_requestMsg.assembledMsg(),
//                       _requestMsg.lengthAssembled());
//   testMsg.print(cerr);
//   cerr << "44444444444444444444444444444" << endl;

  // Send the request
  
  if (_sock.writeMessage(DsLdataMsg::DS_MESSAGE_TYPE_LDATA,
                         _requestMsg.assembledMsg(),
                         _requestMsg.lengthAssembled())) {
    cerr << "ERROR - DsLdataInfo::_commWithServer" << endl;
    cerr << "  Failed writing request to DsLdataServer" << endl;
    cerr << _sock.getErrString() << endl;
    return -1;
  }

  // Read the server's reply

  if (_sock.readMessage()) {
    cerr << "ERROR - DsLdataInfo::_commWithServer" << endl;
    cerr << "  Failed reading reply from DsLdataServer" << endl;
    cerr << _sock.getErrString() << endl;
    return -1;
  }
  
  // Disassemble the reply

  if (_replyMsg.disassemble(_sock.getData(), _sock.getNumBytes())) {
    cerr << "ERROR - DsLdataInfo::_commWithServer" << endl;
    cerr << "  Cannot disassemble reply" << endl;
    return -1;
  }

  if (_replyMsg.getErrorOccurred()) {
    cerr << "ERROR - DsLdataInfo::_commWithServer" << endl;
    cerr << _replyMsg.getErrorStr() << endl;
    return -1;
  }

  if (_debug) {
    cerr << "========== reply message ============" << endl;
    _replyMsg.print(cerr);
    cerr << "=======================================" << endl;
  }

  return 0;

}

