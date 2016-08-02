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


#include <stdio.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <cstring>
#include <stdlib.h>
#include <rapformats/ac_posn.h>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/file_io.h>
#include "asdi2mdv.hh"


#include <ctype.h>

//
// Constructor and destructor do nothing.
//
asdi2mdv::asdi2mdv(Params *tdrpParams){ 
  //
  // Make a copy of the params and init a few variables.
  //
  _params = tdrpParams;
  _count = 0;
  _gridder = new Gridder( _params );
}
//
// Destructor.
//
asdi2mdv::~asdi2mdv(){
  delete _gridder;
}

void asdi2mdv::ProcFile(char *FilePath, Params *P){

  if (_openReadSource(P, FilePath)){
    fprintf(stderr,"Data source - file or stream - unopenable.\n");
    return;
  }

  const int bufferSize = 4096;
  char buffer[bufferSize];

  //
  // Define the End Of Line char.
  //
  char EOL = char(10);
  char NUL = char(0);
  //
  //
  // Read until we get an EOL
  //
  int i = 0;
  do {
    int numRead = _readFromSource(&buffer[i], 1);
    if (numRead != 1) return;
    if (
	(buffer[i] == EOL)
	){
      buffer[i]=NUL;
      i=0;
      _processAsdiMsg( buffer );
    } else {
      if (
	  (!(iscntrl(buffer[i]))) &&
	  ((int)buffer[i] > 0)
	  ){
	if (i < bufferSize-1) i++;
      }
    }
      
  } while (true); // Infinite read loop.
 

}


////////////////////////////////////////////////////////////////
//
// Processs a message.
//
void asdi2mdv::_processAsdiMsg(char *asdiMsg){

  _count++;
  if (_count == 100){
    _count = 0;
    PMU_auto_register("Processing away...");
  }
  //
  // See if we can scan in the ASDI time after the 4 character
  // sequence ID.
  //
  int A_day, A_hour, A_min, A_sec;
  //
  if (4 != sscanf(asdiMsg + strlen("513D"),
		  "%2d%2d%2d%2d",
		  &A_day, &A_hour, &A_min, &A_sec)){
    return;
  }

  date_time_t T;
  _getTime(&T, A_day, A_hour, A_min, A_sec);
  //
  //
  // Return if it is not a TZ message.
  //
  if (strncmp(asdiMsg + strlen("513D20180453KZKC"), "TZ", 2)){
    return;
  }
  //
  // Parse out the flight ID, speed, altitude and location strings.
  //
  char fltID[_internalStringLen];
  unsigned offset = strlen("E00720182123KNCTTZ ");
  if (offset >= strlen(asdiMsg)) return;
  char *p = asdiMsg + offset;
  _extractString(p, fltID);

  char speedStr[_internalStringLen];
  offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 1;
  if (offset >= strlen(asdiMsg)) return;
  p = asdiMsg + offset;
  _extractString(p, speedStr);

  char altStr[_internalStringLen];
  p = asdiMsg + strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(speedStr) + 2;
  _extractString(p, altStr);

  char locStr[_internalStringLen];
  offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(speedStr) + strlen(altStr) + 3;
  if (offset >= strlen(asdiMsg)) return;
  p = asdiMsg + offset;
  _extractString(p, locStr);
  //
  // Parse out the data from these strings.
  // Get the location.
  //
  int latDeg, lonDeg, latMin, lonMin;
  char latChar, lonChar;
  if (6!= sscanf(locStr,"%2d%2d%c/%3d%2d%c",
		 &latDeg, &latMin, &latChar,
		 &lonDeg, &lonMin, &lonChar)){
    return;
  }


  double lat = latDeg + latMin / 60.0;
  double lon = lonDeg + lonMin / 60.0;

  if (latChar == 'S') lat = -lat;
  if (lonChar == 'W') lon = -lon;

  double alt = -1.0;
  if (NULL == strstr(altStr,"T")){
    alt = atof(altStr);
  }
  //
  //
  //
  _gridder->addToGrid(lat, lon, alt, T.unix_time);
  //
  return;

}

void asdi2mdv::_getTime(date_time_t *T, 
			 int A_day, int A_hour, 
			 int A_min, int A_sec){

  //
  // If the date is specified (probably for archive mode,
  // ie. reading old files) then use that.
  //
  if (_params->dateSpecified){
    T->year = _params->year;
    T->month = _params->month;
    T->day = A_day; T->hour = A_hour; T->min = A_min; T->sec = A_sec;
    uconvert_to_utime( T );
    return;
  }
  //
  // Get the current time.
  //
  T->unix_time = time(NULL);
  uconvert_from_utime( T );
  if (A_day != T->day){
    //
    // Must be yesterday's data
    //
    T->unix_time = T->unix_time - 86400;
    uconvert_from_utime( T );
  }

  T->day = A_day; T->hour = A_hour; T->min = A_min; T->sec = A_sec;

  uconvert_to_utime( T );

  return;
}



/////////////////////////////////////
//
// Small method to extract a space-delimited substring
//
void asdi2mdv::_extractString(char *instring, char *outstring){

  int j=0;
  do {
    if (instring[j] == ' ')
      outstring[j] = char(0);
    else
      outstring[j]=instring[j];
    j++;
  } while (
	   (outstring[j-1] != char(0)) &&
	   (j < _internalStringLen-1)
	   );

  if (j == _internalStringLen-1)
    outstring[j] = char(0);

  return;

}


////////////////////////////////////////////////////////////////
//
// Open read source, be it file or stream.
//
int asdi2mdv::_openReadSource(Params *P, char *FileName){
  //
  // Decide if we are reading from a file.
  //
  if (P->mode == Params::REALTIME_STREAM){
    _readingFromFile = false;
  } else {
    _readingFromFile = true;
  }
  //
  // And open the source appropriately.
  //
  int retVal = 0;
  if ( _readingFromFile ){
    //
    // Set up for file reads.
    //
    _fp = fopen(FileName,"rb");
    if (_fp == NULL){
      retVal = -1;
    }
  } else {
    retVal = _S.open(P->hostname, P->port);
  }

  if (retVal != 0){
    if ( _readingFromFile ){
      fprintf(stderr,"Failed to read from file %s\n",
	      FileName);
    } else {
      fprintf(stderr,"Attempt to open port %d at %s returned %d\n",
	      P->port, P->hostname, retVal);

      fprintf(stderr,"Error string : %s\n",_S.getErrString().c_str());

      switch(_S.getErrNum()){

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


    }
  }
  return retVal;
}

////////////////////////////////////////////////////////////////
//
// Close read source, be it file or stream.
//
void asdi2mdv::_closeReadSource(){
  if (_readingFromFile){
    fclose(_fp);
  } else {
    _S.close();
    _S.freeData();
  }
}

///////////////////////////////////////////////////
//
// Read N bytes from the input source into buffer.
// Size of buffer to read to not checked.
// Returns number of bytes read.
//
int asdi2mdv::_readFromSource(char *buffer, int numbytes){

  int retVal;
  if (_readingFromFile){
    retVal = fread(buffer, sizeof(unsigned char), numbytes, _fp);
  } else {

    if (_S.readSelectPmu()){

      switch (_S.getErrNum()){

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

    if (_S.readBufferHb(buffer,
			numbytes,
			numbytes,
			(Socket::heartbeat_t)PMU_auto_register) != 0 ){

      switch (_S.getErrNum()){

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

      return -1;
    }

    retVal = _S.getNumBytes();
  }
  
  return retVal;
}
