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
// DsFileServer.cc
//
// File Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include <dsserver/DsFileIoMsg.hh>
#include <dsserver/DsFileIo.hh>
#include "Params.hh"
#include "DsFileServer.hh"

//////////////////////////////////////////
// constructor
//
// Inherits from DsServer

DsFileServer::DsFileServer(string executableName,
			   string instanceName,
			   int port,
			   int maxQuiescentSecs,
			   int maxClients,
			   bool forkClientHandlers,
			   Params *params)
  : DsServer(executableName,
	     instanceName,
	     port,
	     maxQuiescentSecs,
	     maxClients,
	     forkClientHandlers,
	     params->debug >= Params::DEBUG_NORM,
	     params->debug >= Params::DEBUG_VERBOSE)

{
  _params = params;
  setNoThreadDebug(_params->no_threads);
}

//////////////
// destructor

DsFileServer::~DsFileServer()

{
}

// virtual 
int DsFileServer::handleDataCommand(Socket * socket,
				    const void * data, ssize_t dataSize)
{

  if (_isDebug) {
    cerr << "Entering DsFileServer::handleDataCommand()." << endl;
  }
  
  DsFileIoMsg msg;
  if (_isVerbose) {
    cerr << "Client thread disassembling message..." << endl;
  }
  
  if (msg.disassemble((void *) data, dataSize)) {
    cerr << "ERROR - DsFileServer::handleDataCommand" << endl;
    cerr << "Invalid DsFileIoMsg message" << endl;
    return(-1);
  }
  
  if (_isVerbose) {
    cerr << "------------------------------------" << endl;
    msg.print(cerr);
  }

  // create FileIo object

  DsFileIo *fileIo;
  if (_params->copy_message_memory) {
    fileIo = new DsFileIo();
  } else {
    fileIo = new DsFileIo(DsMessage::PointToMem);
  }
  
  // open for FileIo

  if (_open(&msg, fileIo, socket)) {
    delete fileIo;
    return (-1);
  }

  // serve client

  if (_serve(&msg, fileIo, socket)) {
    delete fileIo;
    return (-1);
  }

  delete fileIo;

  if (_isDebug) {
    cerr << "Exiting DsFileServer::handleDataCommand()." << endl;
  }
  
  return (0);

}
    
// virtual
bool DsFileServer::timeoutMethod()
{
  DsServer::timeoutMethod();
  return true; // Continue to wait for clients.
}

////////////////
// _open()

int DsFileServer::_open(DsFileIoMsg *msg,
			DsFileIo *fileIo,
			Socket * socket)
  
{

  void *replyBuf;
  int iret = 0;
  
  if (msg->getSubType() != DsFileIoMsg::DS_FILEIO_FOPEN) {

    replyBuf =
      msg->assembleReturn(msg->getSubType(),
			  true,
			  "Must do fOpen() first.");
    iret = -1;
    
  } else if (fileIo->fOpen(msg->getUrlStr().c_str(),
			   msg->getModeStr().c_str(), true)) {
    
    replyBuf =
      msg->assembleReturn(msg->getSubType(),
			  true,
			  fileIo->getErrorStr());
    iret = -1;
    
  } else {
    
    replyBuf = msg->assemblefOpenReturn();

  }

  // send reply

  int replyBuflen = msg->lengthAssembled();
  
  if (socket->writeMessage(DsFileIoMsg::DS_MESSAGE_TYPE_FILEIO,
			   replyBuf, replyBuflen, 1000)) {
    cerr << "ERROR - COMM -DsFileServer::open" << endl;
    cerr << "Writing reply" << endl;
    iret = -1;
  }

  return (iret);

}


////////////////
// _serve()

int DsFileServer::_serve(DsFileIoMsg *msg,
			 DsFileIo *fileIo,
			 Socket * socket)
  
{

  if (_isVerbose) {
    cerr << "In DsFileServer::_serve" << endl;
  }

  bool done = false;

  while (!done) {

    // read message from client

    if (_isVerbose) {
      cerr << "    Reading message from client" << endl;
    }

    if (socket->readMessage()) {
      cerr << "ERROR - COMM -DsFileServer::_serve" << endl;
      cerr << "Cannot read message from client" << endl;
      return(-1);
    }

    // disassemble the reply
    
    if (_isVerbose) {
      cerr << "    Disassembling message from client" << endl;
    }

    if (msg->disassemble((void *) socket->getData(),
			 socket->getNumBytes())) {
      cerr << "ERROR - DsFileServer::_serve" << endl;
      cerr << "Invalid reply" << endl;
      return(-1);
    }

    // do stuff to file

    void *reply;

    switch (msg->getSubType()) {
      
    case (DsFileIoMsg::DS_FILEIO_FCLOSE):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FCLOSE" << endl;
	}
	if (fileIo->fClose()) {
	  reply = msg->assemblefCloseReturn(true, fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefCloseReturn();
	}
	done = true;
      }
      break;
  
    case (DsFileIoMsg::DS_FILEIO_FWRITE):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FWRITE" << endl;
	}
	void *data = msg->getData();
	int size = msg->getInfo().size;
	int n = msg->getInfo().nelements;
	int nwritten = fileIo->fWrite(data, size, n);
	if (nwritten != n) {
	  reply = msg->assemblefWriteReturn(nwritten, true,
					    fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefWriteReturn(nwritten);
	}
      }
      break;

    case (DsFileIoMsg::DS_FILEIO_FREAD):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FREAD" << endl;
	}
	int size = msg->getInfo().size;
	int n = msg->getInfo().nelements;
	void *data = umalloc(size * n);
	int nread = fileIo->fRead(data, size, n);
	if (nread != n) {
	  reply = msg->assemblefReadReturn(nread, NULL, true,
					   fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefReadReturn(nread, data);
	}
	ufree(data);
      }
      break;

    case (DsFileIoMsg::DS_FILEIO_FPUTS):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FPUTS" << endl;
	}
	char *str = (char *) msg->getData();
	if (fileIo->fPuts(str)) {
	  reply = msg->assemblefPutsReturn(true,
					   fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefPutsReturn();
	}
      }
      break;

    case (DsFileIoMsg::DS_FILEIO_FGETS):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FGETS" << endl;
	}
	int size = msg->getInfo().size;
	char *str = (char *) umalloc(size);
	if (fileIo->fGets(str, size) == NULL) {
	  reply = msg->assemblefGetsReturn(NULL, true,
					   fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefGetsReturn(str);
	}
	ufree(str);
      }
      break;

    case (DsFileIoMsg::DS_FILEIO_FSEEK):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FSEEK" << endl;
	}
	int offset = msg->getInfo().offset;
	int whence = msg->getInfo().whence;
	if (fileIo->fSeek(offset, whence)) {
	  reply = msg->assemblefSeekReturn(true,
					   fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefSeekReturn();
	}
      }
      break;

    case (DsFileIoMsg::DS_FILEIO_FTELL):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FTELL" << endl;
	}
	int filepos = fileIo->fTell();
	if (filepos < 0) {
	  reply = msg->assemblefTellReturn(filepos, true,
					   fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefTellReturn(filepos);
	}
      }
      break;

    case (DsFileIoMsg::DS_FILEIO_FSTAT):
      {
	if (_isVerbose) {
	  cerr << "      DS_FILEIO_FSTAT" << endl;
	}
	off_t size;
	time_t mtime, atime, ctime;
	if (fileIo->fStat(&size, &mtime, &atime, &ctime)) {
	  reply = msg->assemblefStatReturn(size, mtime, atime, ctime,
					   true,
					   fileIo->getErrorStr());
	} else {
	  reply = msg->assemblefStatReturn(size, mtime, atime, ctime);
	}
      }
      break;

    default:
      break;

    } // switch
    
    // send reply
    
    int replyLen = msg->lengthAssembled();
    
    if (_isVerbose) {
      cerr << "    Writing reply to client" << endl;
    }

    if (socket->writeMessage(DsFileIoMsg::DS_MESSAGE_TYPE_FILEIO,
			     reply, replyLen, 1000)) {
      cerr << "ERROR - COMM -DsFileServer::_serve" << endl;
      cerr << "Writing reply" << endl;
      return (-1);
    }

  } // while

  return (0);

}


