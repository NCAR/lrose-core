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
///////////////////////////////////////////////////////////////
// DsFCopyServer.cc
//
// File Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include <didss/RapDataDir.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Socket.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <sys/stat.h>
#include <cerrno>
#include "Params.hh"
#include "DsFCopyServer.hh"
using namespace std;

//////////////////////////////////////////
// constructor
//
// Inherits from DsServer

DsFCopyServer::DsFCopyServer(string executableName,
			     string instanceName,
			     int port,
			     int maxQuiescentSecs,
			     int maxClients,
			     Params *params)
  : DsProcessServer(executableName,
		    instanceName,
		    port,
		    maxQuiescentSecs,
		    maxClients,
		    params->debug >= Params::DEBUG_NORM,
		    params->debug >= Params::DEBUG_VERBOSE,
		    params->run_secure,
                    false, true)
    
{
  _params = params;
  setNoThreadDebug(_params->no_threads);
  char *lock_str = getenv("FCOPY_SERVER_ALLOW_NO_LOCK");
  if (lock_str && STRequal(lock_str, "true")) {
    _useLock = false;
  } else {
    _useLock = true;
  }
}

//////////////
// destructor

DsFCopyServer::~DsFCopyServer()

{
}

// virtual 
int DsFCopyServer::handleDataCommand(Socket * clientSocket,
				     const void * data, ssize_t dataSize)
{

  if (_isVerbose) {
    cerr << "Entering DsFCopyServer::handleDataCommand()." << endl;
    cerr << "Client thread disassembling message..." << endl;
  }
  
  DsFileCopyMsg copyMsg;
 
  if (copyMsg.disassemble((void *) data, dataSize)) {
    cerr << "ERROR - DsFCopyServer::handleDataCommand" << endl;
    cerr << "  Invalid DsFileCopyMsg message" << endl;
    cerr << copyMsg.getErrorStr() << endl;
    return(-1);
  }
  
  if (_isDebug) {
    cerr << "------------------------------------" << endl;
    copyMsg.print(cerr);
  }

  // check security
  
  if (_isSecure) {
    string urlStr = copyMsg.getDataUrl().getURLStr();
    string securityErr;
    if (!DsServerMsg::urlIsSecure(urlStr, securityErr)) {
      cerr << "ERROR - DsFCopyServer::handleDataCommand" << endl;
      cerr << "  Running in secure mode." << endl;
      cerr << securityErr;
      cerr << "  URL: " << urlStr << endl;
      return -1;
    }
  }

  // handle messages of different types

  switch (copyMsg.getSubType()) {

  case DsFileCopyMsg::DS_FILECOPY_ENQUIRE_BY_TIME: { // deprecated

    bool doPut;
    string putDir;
    string putName;
    string putPath;
    string ldataDir;
  
    if (_handleEnquireByTime(clientSocket, copyMsg, doPut, putDir,
			     putName, putPath, ldataDir)) {
      _removeFile(putPath);
      return (-1);
    }
    if (doPut) {
      if (_readPutMessage(clientSocket, copyMsg, putPath)) {
	_removeFile(putPath);
	return -1;
      }
      string errorStr;
      if (_handlePut(clientSocket, copyMsg, putName, putPath,
		     ldataDir, 0, errorStr)) {
	_putAfterEnquireReturn(clientSocket, copyMsg, true, errorStr);
	_removeFile(putPath);
	return (-1);
      } else {
	_putAfterEnquireReturn(clientSocket, copyMsg);
      }
    }

  }
  break;

  case DsFileCopyMsg::DS_FILECOPY_ENQUIRE_FOR_PUT: {

    bool doPut;
    string putDir;
    string putName;
    string putPath;
    string ldataDir;
    time_t fileModTime;
    
    if (_handleEnquireForPut(clientSocket, copyMsg, doPut, putDir,
			     putName, putPath, ldataDir,
			     fileModTime)) {
      _removeFile(putPath);
      return (-1);
    }
    //      if (doPut) {
    //        if (_readPutMessage(clientSocket, copyMsg, putPath)) {
    //  	_removeFile(putPath);
    //  	return -1;
    //        }
    //        string errorStr;
    //        if (_handlePut(clientSocket, copyMsg, putName, putPath,
    //  		     ldataDir, fileModTime, errorStr)) {
    //  	_putAfterEnquireReturn(clientSocket, copyMsg, true, errorStr);
    //  	_removeFile(putPath);
    //  	return (-1);
    //        } else {
    //  	_putAfterEnquireReturn(clientSocket, copyMsg);
    //        }
    //      }

  }
  break;

  case DsFileCopyMsg::DS_FILECOPY_PUT_BY_TIME: {  // deprecated
    if (_handlePutByTime(clientSocket, copyMsg)) {
      return (-1);
    }
  }
  break;

  case DsFileCopyMsg::DS_FILECOPY_PUT_FORCED: {
    if (_handlePutForced(clientSocket, copyMsg)) {
      return (-1);
    }
  }
  break;

  case DsFileCopyMsg::DS_FILECOPY_PUT_AFTER_ENQUIRE: {
    cerr << "ERROR - DsFCopyServer::handleDataCommand" << endl;
    cerr << "  PUT message out of order" << endl;
    return(-1);
  }
  break;

  default: {
    cerr << "ERROR - DsFCopyServer::handleDataCommand" << endl;
    cerr << "  Unexpected message sub type: "
	 << copyMsg.getSubType() << endl;
    return(-1);
  }

  } // switch
  
  if (_isVerbose) {
    cerr << "Exiting DsFCopyServer::handleDataCommand()." << endl;
  }
  
  return (0);

}
    
//////////////////////////
// _handleEnquireByTime()

int DsFCopyServer::_handleEnquireByTime(Socket * clientSocket,
					DsFileCopyMsg &copyMsg,
					bool &doPut,
					string &putDir,
					string &putName,
					string &putPath,
					string &ldataDir)
  
{

  string errorStr = "Running DsFCopyServer::_handleEnquireByTime\n";

  if (_isDebug) {
    cerr << "  In _handleEnquireByTime()" << endl;
  }

  // check that put directory can be made

  if (_checkPutDir(copyMsg, true, putDir, ldataDir, errorStr)) {
    doPut = false;
    _enquireForPutReturn(clientSocket, copyMsg, doPut, true, errorStr);
    return (-1);
  }
  
  // compute path
  
  putPath = putDir;
  putPath += PATH_DELIM;
  char tmpStr[128];
  const LdataInfo &ldata = copyMsg.getLdataInfo();
  const date_time_t &dtime = ldata.getLatestTimeStruct();
  if (ldata.isFcast() == false) {
    sprintf(tmpStr, "%.2d%.2d%.2d.%s",
	    dtime.hour, dtime.min, dtime.sec,
	    ldata.getDataFileExt().c_str());
  } else {
    sprintf(tmpStr, "f_%.8d.%s",
	    (int) ldata.getLeadTime(), ldata.getDataFileExt().c_str());
  }
  putName = tmpStr;
  putPath += putName;

  // determine whether to put or not

  _setDoPut(copyMsg, doPut, putPath);

  if (_enquireForPutReturn(clientSocket, copyMsg, doPut)) {
    return (-1);
  }

  return (0);

}

//////////////////////////
// _handleEnquireForPut()

int DsFCopyServer::_handleEnquireForPut(Socket * clientSocket,
					DsFileCopyMsg &copyMsg,
					bool &doPut,
					string &putDir,
					string &putName,
					string &putPath,
					string &ldataDir,
					time_t &fileModTime)
  
{

  string errorStr = "Running DsFCopyServer::_handleEnquireForPut\n";

  if (_isDebug) {
    cerr << "  In _handleEnquireForPut()" << endl;
  }

  // check that put directory exists

  if (_checkPutDir(copyMsg, false, putDir, ldataDir, errorStr)) {
    doPut = false;
    _enquireForPutReturn(clientSocket, copyMsg, doPut, true, errorStr);
    return (-1);
  }
  
  // compute path
  
  putName = copyMsg.getFileName();
  putPath = putDir;
  putPath += PATH_DELIM;
  putPath += putName;

  // save mod time

  fileModTime = copyMsg.getFileInfo().mod_time;

  // determine whether to put or not

  _setDoPut(copyMsg, doPut, putPath);

  // send return message

  if (_enquireForPutReturn(clientSocket, copyMsg, doPut)) {
    return (-1);
  }

  return (0);

}

//////////////////////////
// _handlePutByTime()

int DsFCopyServer::_handlePutByTime(Socket * clientSocket,
				    DsFileCopyMsg &copyMsg)
  
{
  
  string errorStr = "Running DsFCopyServer::_handlePutByTime\n";

  if (_isVerbose) {
    cerr << "  In _handleEnquireByTime()" << endl;
  }

  // check that put directory exists
  
  string putDir;
  string ldataDir;
  if (_checkPutDir(copyMsg, true, putDir, ldataDir, errorStr)) {
    _putForcedReturn(clientSocket, copyMsg, true, errorStr);
    return (-1);
  }
  
  // compute path
  
  string putPath = putDir;
  putPath += PATH_DELIM;
  char tmpStr[128];
  const LdataInfo &ldata = copyMsg.getLdataInfo();
  const date_time_t &dtime = ldata.getLatestTimeStruct();
  if (ldata.isFcast() == false) {
    sprintf(tmpStr, "%.2d%.2d%.2d.%s",
	    dtime.hour, dtime.min, dtime.sec,
	    ldata.getDataFileExt().c_str());
  } else {
    sprintf(tmpStr, "f_%.8d.%s",
	    (int) ldata.getLeadTime(), ldata.getDataFileExt().c_str());
  }
  string putName = tmpStr;
  putPath += putName;

  if (_handlePut(clientSocket, copyMsg, putName,
		 putPath, ldataDir, 0, errorStr)) {
    _putForcedReturn(clientSocket, copyMsg, true, errorStr);
    return (-1);
  } else {
    _putForcedReturn(clientSocket, copyMsg);
  }

  return (0);

}

//////////////////////////
// _handlePutForced()

int DsFCopyServer::_handlePutForced(Socket * clientSocket,
				    DsFileCopyMsg &copyMsg)
  
{

  string errorStr = "Running DsFCopyServer::_handlePutForced\n";

  if (_isVerbose) {
    cerr << "  In _handlePutForced()" << endl;
  }

  // check that put directory exists

  string putDir;
  string ldataDir;
  if (_checkPutDir(copyMsg, false, putDir, ldataDir, errorStr)) {
    _putForcedReturn(clientSocket, copyMsg, true, errorStr);
    return (-1);
  }
  
  // compute path

  string putName = copyMsg.getFileName();
  string putPath = putDir;
  putPath += PATH_DELIM;
  putPath += putName;

  // get mod time

  time_t fileModTime = copyMsg.getFileInfo().mod_time;

  if (_handlePut(clientSocket, copyMsg, putName,
		 putPath, ldataDir, fileModTime, errorStr)) {
    _putForcedReturn(clientSocket, copyMsg, true, errorStr);
    return (-1);
  } else {
    _putForcedReturn(clientSocket, copyMsg);
  }
  
  return (0);

}

//////////////////////////////////////
// check that the put directory exists

int DsFCopyServer::_checkPutDir(DsFileCopyMsg &copyMsg,
				bool by_time,
				string &putDir,
				string &ldataDir,
				string &errorStr)

{

  const DsURL &dataUrl = copyMsg.getDataUrl();
  RapDataDir.fillPath(dataUrl.getFile(), putDir);
  ldataDir = dataUrl.getFile();
  const LdataInfo &ldata = copyMsg.getLdataInfo();

  if (by_time) {
    putDir += PATH_DELIM;
    char tmpStr[128];
    const date_time_t &dtime = ldata.getLatestTimeStruct();
    if (ldata.isFcast() == false) {
      sprintf(tmpStr, "%.4d%.2d%.2d", dtime.year, dtime.month, dtime.day);
    } else {
      sprintf(tmpStr, "%.4d%.2d%.2d%sg_%.2d%.2d%.2d",
	      dtime.year, dtime.month, dtime.day, PATH_DELIM,
	      dtime.hour, dtime.min, dtime.sec);
    }
    putDir += tmpStr;
  }

  if (_isVerbose) {
    cerr << "  In _checkPutDir()" << endl;
    cerr << "     putDir: " << putDir << endl;
  }

  // check that directory exists, or can be made
  
  if (ta_makedir_recurse(putDir.c_str())) {
    int errNum = errno;
    errorStr += "ERROR DsFCopyServer::_enquireByTime\n.";
    errorStr += "Cannot make directory: '";
    errorStr += putDir;
    errorStr += "'\n";
    errorStr += strerror(errNum);
    errorStr += "'\n";
    if (_isDebug) {
      cerr << errorStr << endl;
    }
    return (-1);
  } 

  return (0);

}

//////////////////////////////////
// determine whether to put or not
//

void DsFCopyServer::_setDoPut(DsFileCopyMsg &copyMsg,
			      bool &doPut,
			      const string &putPath)

{

  if (_isVerbose) {
    cerr << "  In _setDoPut()" << endl;
    cerr << "     setting the put flag" << endl;
    cerr << "     put path: " << putPath << endl;
  }

  doPut = true;
  
  // stat the path
  struct stat fileStat;
  
  if (stat(putPath.c_str(), &fileStat) == 0) {
    
    // file exists
    
    const DsFileCopyMsg::file_info_t &info = copyMsg.getFileInfo();
    
    if (_isVerbose) {
      cerr << "       File exists" << endl;
      cerr << "       Mod time: " << DateTime::str(fileStat.st_mtime) << endl;
    }
    
    if (info.overwrite_age == -1) {
      
      // never overwrite
      
      if (_isVerbose) {
	cerr << "       info.overwrite_age = -1, never overwrite" << endl;
      }
      doPut = false;
      
    } else {
      
      // overwrite if file exceeds overwrite age
      
      time_t now = time(NULL);
      int age = now - fileStat.st_mtime;
      if (_isVerbose) {
	cerr << "       info.overwrite_age: " << info.overwrite_age << endl;
	cerr << "       age: " << age << endl;
      }
      if (age < info.overwrite_age) {
	doPut = false;
      } else {
	doPut = true;
      }
      
    } // if (info.overwrite_age == -1)
    
  } // if (stat(putPath.c_str(), &fileStat) == 0) 
  
  if (_isVerbose) {
    cerr << "     doPut:" << doPut << endl;
  }

  if (doPut) {
    // create zero-length file to reserve the the path, so that other
    // threads will find the file and return doPut = false
    // The zero-length file will later be overwritten
    // by the real file.
    _writeZeroLenFile(putPath);
  }
    
}

/////////////////////////////////////////
// read the put message after an enquire

int DsFCopyServer::_readPutMessage(Socket * clientSocket,
				   DsFileCopyMsg &copyMsg,
				   const string &putPath)
  
{

  if (_isDebug) {
    cerr << "  Starting _readPutMessage()" << endl;
  }

  // read put message from client
  
  if (_isVerbose) {
    cerr << "  Reading put message from client" << endl;
  }

  // compute comm timeout
  
  int commTimeoutMsecs = DS_DEFAULT_COMM_TIMEOUT_MSECS;
  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    int timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%d", &timeout) == 1) {
      commTimeoutMsecs = timeout;
    }
  }

  if (clientSocket->readMessage(commTimeoutMsecs)) {
    cerr << "  ERROR - COMM -DsFCopyServer::_readPutMessage()" << endl;
    cerr << "    putPath: " << putPath << endl;
    cerr << "    Cannot read put message from client" << endl;
    cerr << "    commTimeoutMsecs: " << commTimeoutMsecs << endl;
    cerr << "    errorStr: " << clientSocket->getErrStr() << endl;
    return(-1);
  }
  
  // disassemble the put message
  
  if (_isVerbose) {
    cerr << "  Disassembling put message from client" << endl;
  }
  if (copyMsg.disassemble((void *) clientSocket->getData(),
			  clientSocket->getNumBytes())) {
    cerr << "ERROR - DsFCopyServer::_readPutMessage()" << endl;
    cerr << "  Invalid put message" << endl;
    cerr << copyMsg.getErrorStr() << endl;
    cerr << "  putPath: " << putPath << endl;
    return(-1);
  }

  return 0;

}

////////////////////////////////
// _handlePut()

int DsFCopyServer::_handlePut(Socket * clientSocket,
			      DsFileCopyMsg &copyMsg,
			      const string &putName,
			      const string &putPath,
			      const string &ldataDir,
			      time_t fileModTime,
			      string &errorStr)
  
{

  if (_isVerbose) {
    cerr << "  Starting _handlePut()" << endl;
  }

  // compute tmp file path

  char tmpPathStr[MAX_PATH_LEN];
  struct timeval tv;
  gettimeofday(&tv, NULL);
  sprintf(tmpPathStr, "%s.%d.%ld.%ld.tmp", putPath.c_str(), getpid(),
          tv.tv_sec, tv.tv_usec);
  
  // check for existence of env var specifying tmp dir

  char *tmpDir = getenv("FCOPY_SERVER_TMP_DIR");
  if (tmpDir && strlen(tmpDir) > 0) {
    // FCOPY_SERVER_TMP_DIR exists
    // Compute tmp name based on tmp path
    char tmpName[MAX_PATH_LEN];
    strcpy(tmpName, tmpPathStr);
    for (size_t ii = 0; ii < strlen(tmpName); ii++) {
      // replace / with _
      if (tmpName[ii] == '/') {
        tmpName[ii] = '_';
      }
    }
    // compute tmp path to put file in tmp dir
    sprintf(tmpPathStr, "%s%s%s",
            tmpDir, PATH_DELIM, tmpName);
  }
  
  // make sure parent directory exists

  Path tmpPath(tmpPathStr);
  if (ta_makedir_recurse(tmpPath.getDirectory().c_str())) {
    int errNum = errno;
    errorStr = "ERROR - DsFCopyServer::_handlePut\n.";
    errorStr += "Cannot make directory: '";
    errorStr += tmpPath.getDirectory();
    errorStr += "'\n";
    errorStr += strerror(errNum);
    errorStr += "'\n";
    if (_isVerbose) {
      cerr << errorStr << endl;
    }
    return (-1);
  } 
  
  // open the file

  if (_isDebug) {
    cerr << "  Writing file:" << tmpPathStr << endl;
  }
  FILE *fp;
  if ((fp = fopen(tmpPathStr, "w")) == NULL) {
    int errNo = errno;
    errorStr = "ERROR - DsFCopyServer::_handlePut()\n";
    errorStr += "  Cannot open file '";
    errorStr += tmpPathStr;
    errorStr += "'\n";
    errorStr += strerror(errNo);
    return (-1);
  }

  // lock the file

  if (_useLock) {
    if (ta_lock_file_procmap(tmpPathStr, fp, "w")) {
      errorStr = "ERROR - DsFCopyServer::_handlePut()\n";
      errorStr += "  Cannot lock file '";
      errorStr += tmpPathStr;
      errorStr += "'\n";
      fclose(fp);
      return (-1);
    }
  }

  // write file buffer

  if (copyMsg.getFileLen() > 0) {

    // check for compression

    void *buf;
    unsigned int len;
    bool isCompressed;

    if (ta_compressed((void *) copyMsg.getFileBuf())) {
      isCompressed = true;
      buf = ta_decompress((void *) copyMsg.getFileBuf(), &len);
    } else {
      isCompressed = false;
      buf = (void *) copyMsg.getFileBuf();
      len = copyMsg.getFileLen();
    }

    if (_isDebug){
      if (isCompressed){
	cerr << "Buffer is compressed, details :" << endl;
	ta_compression_debug( (void *) copyMsg.getFileBuf() );
      } else {
	cerr << "Buffer is not compressed." << endl;
      }
    }

    if (ufwrite(buf, 1, len, fp) != (int) len) {
      int errNo = errno;
      errorStr = "ERROR - DsFCopyServer::_handlePut()\n";
      errorStr += "  Cannot write buffer to file '";
      errorStr += tmpPathStr;
      errorStr += "': ";
      errorStr += strerror(errNo);
      errorStr += "\n";
      if (_useLock) {
	ta_unlock_file(tmpPathStr, fp);
      }
      fclose(fp);
      if (isCompressed) {
	ta_compress_free(buf);
      }
      return (-1);
    }

    if (isCompressed) {
      ta_compress_free(buf);
    }
    
  }

  // unlock and close file

  if (_useLock) {
    ta_unlock_file(tmpPathStr, fp);
  }
  fclose(fp);

  // rename the file

  if (_isDebug) {
    cerr << DateTime::str() << endl;
    cerr << "Renaming file: " << tmpPathStr << endl;
    cerr << "           to: " << putPath << endl;
  }
  
  if (rename(tmpPathStr, putPath.c_str())) {
    int errNo = errno;
    errorStr = "ERROR - DsFCopyServer::_handlePut()\n";
    errorStr += "  Cannot rename tmp file to '";
    errorStr += putPath;
    errorStr += "'\n";
    errorStr += strerror(errNo);
    return (-1);
  }

  // write the LdataInfo file

  umsleep(1000);

  // fully qualify the ldata dir and put path

  string fullLdataDir;
  RapDataDir.fillPath(ldataDir, fullLdataDir);
  string fullPutPath;
  RapDataDir.fillPath(putPath, fullPutPath);

  // subtract the paths to get the rel file path

  string relFilePath;
  Path::stripDir(fullLdataDir, fullPutPath, relFilePath);

  if (_writeLdataInfo(copyMsg, relFilePath, ldataDir, fileModTime)) {
    errorStr = "ERROR - DsFCopyServer::_handlePut()\n";
    errorStr += "  Cannot write latest_data_info file.\n";
    return (-1);
  }

  if (_isVerbose) {
    cerr << "Done" << endl;
    cerr << DateTime::str() << endl;
  }

  return (0);

}

/////////////////////////
// _enquireForPutReturn()

int DsFCopyServer::_enquireForPutReturn(Socket * clientSocket,
					DsFileCopyMsg &copyMsg,
					bool &doPut,
					const bool errorOccurred,
					string errorStr)
  
{

  if (_isVerbose) {
    cerr << "  In _enquireForPutReturn()" << endl;
  }

  void *returnBuf;
  int iret = 0;
  
  if (errorOccurred) {
    returnBuf =
      copyMsg.assembleEnquireForPutReturn(doPut,
					  errorOccurred,
					  errorStr.c_str());
  } else {
    returnBuf = copyMsg.assembleEnquireForPutReturn(doPut);
  }
  
  // send return
  
  int returnBuflen = copyMsg.lengthAssembled();
  
  if (clientSocket->writeMessage(DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY,
				 returnBuf, returnBuflen, 1000)) {
    cerr << "ERROR - COMM -DsFCopyServer::_enquireForPutReturn" << endl;
    cerr << "Writing enquire for put return" << endl;
    iret = -1;
  }

  return (iret);

}

////////////////////////////
// _putAfterEnquireReturn()

int DsFCopyServer::_putAfterEnquireReturn(Socket * clientSocket,
					  DsFileCopyMsg &copyMsg,
					  const bool errorOccurred,
					  string errorStr)
  
{

  if (_isVerbose) {
    cerr << "  In _putAfterEnquireReturn()" << endl;
  }

  void *returnBuf;
  int iret = 0;
  
  if (errorOccurred) {
    returnBuf =
      copyMsg.assemblePutAfterEnquireReturn(errorOccurred,
					    errorStr.c_str());
  } else {
    returnBuf = copyMsg.assemblePutAfterEnquireReturn();
  }

  // send return
  
  int returnBuflen = copyMsg.lengthAssembled();
  
  if (clientSocket->writeMessage(DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY,
				 returnBuf, returnBuflen, 1000)) {
    cerr << "ERROR - COMM -DsFCopyServer::_putReturn" << endl;
    cerr << "Writing put after enquire return" << endl;
    iret = -1;
  }

  return (iret);

}

////////////////////////////
// _putForcedReturn()

int DsFCopyServer::_putForcedReturn(Socket * clientSocket,
				    DsFileCopyMsg &copyMsg,
				    const bool errorOccurred,
				    string errorStr)
  
{

  if (_isDebug) {
    cerr << "  In _putForcedReturn()" << endl;
  }
  
  void *returnBuf;
  int iret = 0;
  
  if (errorOccurred) {
    returnBuf =
      copyMsg.assemblePutForcedReturn(errorOccurred,
				      errorStr.c_str());
    if (_isDebug) {
      cerr << "  ERROR - DsFCopyServer::_putForcedReturn()" << endl;
      cerr << errorStr << endl;
    }
  } else {
    returnBuf = copyMsg.assemblePutForcedReturn();
  }

  // send return
  
  int returnBuflen = copyMsg.lengthAssembled();
  
  if (clientSocket->writeMessage(DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY,
				 returnBuf, returnBuflen, 1000)) {
    cerr << "ERROR - COMM -DsFCopyServer::_putReturn" << endl;
    cerr << "Writing put forced return" << endl;
    iret = -1;
  }

  return (iret);

}

///////////////////////////////////
// write the latest data info file

int DsFCopyServer::_writeLdataInfo(DsFileCopyMsg &copyMsg,
				   const string &putName,
				   const string &ldataDir,
				   time_t fileModTime)

{

  DsLdataInfo info(copyMsg.getLdataInfo());
  info.setDir(ldataDir);
  info.setWriter("DsFCopyServer");
  info.setRelDataPath(putName.c_str());

  if (_isVerbose) {
    cerr << "=============================================" << endl;
    cerr << "Writing latest data info file" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "=============================================" << endl;
    info.printFull(cerr);
  }

  if (info.write(info.getLatestTime())) {
    return (-1);
  }
  
  return (0);

}

////////////////////////////
// _writeZeroLenFile()

int DsFCopyServer::_writeZeroLenFile(const string &path)
  
{

  // create zero-length file to reserve the the path, so that other
  // threads will find the file and return doPut = false
  // The zero-length file will later be overwritten
  // by the real file.
  FILE *fp;
  if ((fp = fopen(path.c_str(), "w")) == NULL) {
    if (_isDebug) {
      cerr << "  Failed to write 0-length file: " << path << endl;
    }
    return -1;
  }
  fclose(fp);
  if (_isDebug) {
    cerr << "  Writing 0-length file, to prevent competing puts." << endl;
    cerr << "    Path: " << path << endl;
  }
  return 0;

}

////////////////////////////
// _removeFile()

void DsFCopyServer::_removeFile(const string &path)
{
  unlink(path.c_str());
}

