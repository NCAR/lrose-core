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
// basicIO.cc
//
// Reads bytes for super resolution nexrad data. Will do it
// from file, or from socket, depending on which constructor
// is called.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



#include "basicIO.hh"


// Constructor, from filename, file IO assumed.
basicIO::basicIO ( char *fileName, Params *TDRP_params ){

  _readMode = _readingFromFile;
  _ok = true;
  _params = TDRP_params;

  struct stat buf;
  if (stat(fileName, &buf) == 0 && 
      S_ISREG(buf.st_mode)){
    _fileSize = buf.st_size;
  } else {
    _ok = false;
    return;
  }

  _fp = fopen(fileName, "r");
  if (_fp == NULL){
    fprintf(stderr, "Unable to open %s\n", fileName);
    _ok = false;
  }


  return;
}

// Constructor, from hostname and port, socket IO assumed.
basicIO::basicIO ( char *hostName, int portNum, Params *TDRP_params ){

  _readMode = _readingFromSocket;
  _ok = true;
  _params = TDRP_params;
  _fp = NULL; // Kind of irrelevant but why not?

  int retVal;

  do {

    retVal = _socket.open(hostName, portNum);

    if (retVal != 0){
 
      fprintf(stderr,"Attempt to open port %d at %s returned %d\n",
	      portNum, hostName, retVal);
      
      fprintf(stderr,"Error string : %s\n",_socket.getErrString().c_str());
      
      switch(_socket.getErrNum()){
	
      case Socket::UNKNOWN_HOST :
	fprintf(stderr,"Unknown host.\n");
	break;

      case Socket::SOCKET_FAILED :
	fprintf(stderr,"Could not set up socked (maxed out decriptors?).\n");
	break;
      
      case Socket::CONNECT_FAILED :
	fprintf(stderr,"Connect failed.\n");
	break;
	
      case Socket::TIMED_OUT :
	fprintf(stderr,"Timed out..\n");
	break;
	
      case Socket::UNEXPECTED :
	fprintf(stderr,"Unexpected error.\n");
	break;
	
      default :
	fprintf(stderr,"Unknown error.\n");
	break;
	
      }
      PMU_auto_register("Trying to connect to scoket...");
      sleep(5);
    }
  } while (retVal != 0);

  return;
}

// Get status.
bool basicIO::isOk(){
  return _ok;
}

// Get read mode.
int basicIO::getReadMode(){
  return _readMode;
}

bool basicIO::atEnd(){

  if (_readMode == _readingFromFile){


    if (_fp == NULL) return true;

    // cerr << _fileSize - ftell(_fp) << " bytes remaining." << endl;

    // If number of bytes remaining is less than the size of an
    // unzipped message31 then we're done.
    if (_fileSize - ftell(_fp) < 100){
      return true;
    }
  }

  return false;
}


// Read some bytes. User must allocate space. Returns
// the number of bytes actually read. If the number of
// bytes read is different from what was asked for then
// isOk() will return false.
int basicIO::readBytes(int numBytesToRead, unsigned char *buffer){

  if (!(_ok)) return 0; // Something went wrong in the init or previous read.

  int nBytesRead = 0;

  //
  // Reading from a file is pretty trivial.
  //
  if (_readMode == _readingFromFile){
    nBytesRead = fread(buffer, sizeof(unsigned char), numBytesToRead, _fp);
  }


  //
  // Reading from a socket, less so.
  //
  if (_readMode == _readingFromSocket){
    
    if (_socket.readSelectPmu()){

      switch (_socket.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read select timed out.\n");
	exit(-1);
	break;

      case Socket::SELECT_FAILED :
	fprintf(stderr,"Read select failed.\n");
	exit(-1);
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"Read select - unexpected error.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read select.\n");
	exit(-1);
	break;

      }
    }
    
    if (_socket.readBufferHb(buffer,
			     numBytesToRead,
			     numBytesToRead,
			     (Socket::heartbeat_t)PMU_auto_register) != 0 ){

      switch (_socket.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read buffer timed out.\n");
	exit(-1);
	break;

      case Socket::BAD_BYTE_COUNT :
	fprintf(stderr,"Read buffer gave bad byte count.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read buffer.\n");
	exit(-1);
	break;
      }

      nBytesRead = 0;
    }

    nBytesRead = _socket.getNumBytes();
  }

  //
  // Regardless of if we read from a socket or a file,
  // set the _ok flag approriately depending on how many
  // bytes we got compared to what we asked for, and return.
  //

  if (nBytesRead != numBytesToRead) _ok = false;

  return nBytesRead;

}

// Destructor.
basicIO::~basicIO (){

  if (_readMode == _readingFromFile){
    //
    // Close the file, if it was ever opened.
    //
    if (_fp != NULL)
      fclose(_fp);
  }

  if (_readMode == _readingFromSocket){
    //
    // Close socket.
    //
    _socket.close();
    _socket.freeData();

  }

  return;
}




