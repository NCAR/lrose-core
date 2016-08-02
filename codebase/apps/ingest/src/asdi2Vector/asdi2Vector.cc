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

//
// This is the class that does the real work for the asdi2Vector
// application. Data are read in and written out in SPDB format.
//
#include <cstdio>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <cstring>
#include <stdlib.h>
#include <rapformats/ac_posn.h>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/file_io.h>
#include "asdi2Vector.hh"
#include <ctype.h>

//
// Constructor and destructor do very little.
//
asdi2Vector::asdi2Vector(Params *tdrpParams){ 
  //
  // Make a copy of the params and init a few variables.
  //
  _params = tdrpParams;
  _count = 0;
  _spdbBufferCount = 0;
  memset(_asciiBuffer,  0, _asciiBufferLen);
  _acVectors.clear();
  _lastCleanoutTime = 0L;

  _filenameYear = -1;
  _filenameMonth = -1;


  return;

}
///////////////////////////////////////////////
//
// Destructor.
//
asdi2Vector::~asdi2Vector(){
  //
  // Write out any data still remaining in buffers.
  //
  if (_params->saveSPDB){
    if (_spdb.put( _params->OutUrl,
		   SPDB_AC_VECTOR_ID,
		   SPDB_AC_VECTOR_LABEL)){
      cerr << "Failed to put data" << endl;
    }
  }

  if (_params->saveASCII){
    date_time_t T;
    T.unix_time = time(NULL);
    uconvert_from_utime(&T);
    //
    // Make the output directory.
    //
    char outdirname[MAX_PATH_LEN];
    sprintf(outdirname,"%s/%d%02d%02d",
	    _params->OutASCIIdir,
	    T.year, T.month, T.day);
    
    if (ta_makedir_recurse(outdirname)){
      cerr << "Failed to make directory " << outdirname << endl;
      return;
    }
    
    //
    // Open the output file append.
    //
    char outfilename[MAX_PATH_LEN];
    sprintf(outfilename,"%s/%04d%02d%02d_%02d.dat", outdirname, 
	    T.year, T.month, T.day, T.hour);
    FILE *fp;
    fp = fopen(outfilename, "a");
    if (fp == NULL){
      cerr << "Failed to create file " << outfilename << endl;
      return;
    }
    //
    // Write the message and return.
    //
    fprintf(fp,"%s\n", _asciiBuffer);
    fclose(fp);
  }
  return;
}
///////////////////////////////////////////////
//
// Method to process data from a file.
//
void asdi2Vector::ProcFile(char *FilePath, Params *P){

  if (_openReadSource(P, FilePath)){
    cerr << "Data source - file or stream - unopenable." << endl;
    return;
  }

  if ((FilePath != NULL) && (P->dateFromFilename)){
    int i = strlen(FilePath);
    while ( true ){
      if (i < 1) break;
      if (FilePath[i] == '/'){
	i++;
	break;
      }
      i--;
    }
    
    if (2!=sscanf(FilePath+i,"%4d%2d", &_filenameYear, &_filenameMonth)){
      cerr << "Failed to parse year and month from filename " << FilePath+i << endl;
      exit(-1);
    }
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
void asdi2Vector::_processAsdiMsg(char *asdiMsg){

  _count++;
  if (_count == 100){
    _count = 0;
    PMU_auto_register("Processing away...");
  }

  if (_params->debug)
    fprintf(stderr,"%s :\n", 
	    asdiMsg);

  // See if we wish to save the raw data
  // to an ASCII file.
  //
  if (_params->saveASCII){
    _save2ascii(asdiMsg);
  }
  //
  // Return if we are not saving SPDB.
  //
  if (!(_params->saveSPDB)){
    return;
  }
  //
  // See if we can scan in the ASDI time after the 4 character
  // sequence ID.
  //
  int A_day, A_hour, A_min, A_sec;

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
  //
  // Return if we are requiring a substring to be in the flight ID and
  // in this case it is not.
  //
  if (_params->checkCallsignString){
    if (_params->callsignStrings_n > 0){
      bool ok = false;
      for (int icheck=0; icheck < _params->callsignStrings_n; icheck++){
	if (NULL!=strstr(fltID, _params->_callsignStrings[icheck])){
	  ok = true;
	  break;
	}
      }
      if (!(ok)) return;
    }
    //
    // Check that we don't have one of the forbidden callsigns.
    // 
    if (_params->rejectCallsignStrings_n > 0){
      bool ok = true;
      for (int icheck=0; icheck < _params->rejectCallsignStrings_n; icheck++){
	if (NULL!=strstr(fltID, _params->_rejectCallsignStrings[icheck])){
	  ok = false;
	  break;
	}
      }
      if (!(ok)) return;
    }
  }
  
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
  //
  // Speed is easy.
  //
  double speed = atof(speedStr);
  //
  // altitude slightly less so - if it ends in a 'T'
  // then it is an altitude the aircraft has been cleared to, but is not
  // at yet. If it starts with 'OTP/' as in OTP/350 then the aircraft
  // is above 350. Neither of these define the altutide so use a value
  // of -1.0 in that case.
  //
  // If it ends in a B then a block of altitudes are returned, 
  // which is not useful.
  //
  // If it ends in a 'C' then it is correct, but is not what the aircraft is
  // supposed to be at. This does define the altitude, so accept this.
  //
  double altitude = acPosVector::missingVal;
  if (
      (NULL == strstr(altStr,"T")) &&
      (NULL == strstr(altStr,"B"))
      ){
    altitude = atof(altStr);
  }
  //
  // Location
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
  //
  // Return if we are applying a region and this is outside
  // of it.
  //
  if (_params->applyRegion){
    if (
	(lat > _params->region.latMax) ||
	(lat < _params->region.latMin) ||
	(lon > _params->region.lonMax) ||
	(lon < _params->region.lonMin)
	){
      return;
    }
  }

  if (_params->debug){
    cerr << fltID << ", " << lat << ", " << lon << ", " << speed;
    cerr << ", " << altitude << endl;
  }

  //
  // See if we want to clean out the vectors that
  // are just too old to be appended to.
  //
  if ((T.unix_time - _lastCleanoutTime) > _params->maxTimeGap){
    //
    _lastCleanoutTime = T.unix_time;
    if (_params->debug){
      cerr << "Vector size before cleanout at " << utimstr(T.unix_time) << " : ";
      cerr << _acVectors.size() << endl; 
    }
    //
    // Loop through and delete the old ones.
    //
    for (unsigned i=0; i < _acVectors.size(); i++){
      if ((T.unix_time - _acVectors[i].getTimePrevious()) > _params->maxTimeGap){
	_acVectors.erase( _acVectors.begin() + i); i--;
      }
    }
    if (_params->debug){
      cerr << "Vector size after cleanout at " << utimstr(T.unix_time) << " : ";
      cerr << _acVectors.size() << endl; 
    }
  }
  //
  // See if we already have an entry for this callsign. If so,
  // update the entry and add it to the SPDB save list.
  //  
  bool alreadyEntered = false;
  //
  for (unsigned i=0; i < _acVectors.size(); i++){

    if (!(strcmp(fltID, _acVectors[i].getCallsign().c_str()))){
      alreadyEntered = true;
      //
      //
      _acVectors[i].updatePoint(T.unix_time, lat, lon, altitude);
      //
      // Save this point out.
      // Contrive data types 1,2 for uniqueness.
      //
      si32 dataType;
      //
      if (NULL != strstr(fltID, "/")){
	//
	char *p = strstr(fltID, "/") - 4;
	dataType = Spdb::hash4CharsToInt32( p );
      } else {
	//
	// No slash - limited amount we can do ....
	//
	dataType = Spdb::hash4CharsToInt32( fltID + 2 );
      }
      si32 dataType2 = Spdb::hash4CharsToInt32( fltID + strlen(fltID)-4);
      //
      // This particular dataset has the speed included - store this in
      // the first two spares.
      //
      if (acPosVector::missingVal == _acVectors[i].getSpare(1)){
	_acVectors[i].setSpare(1,speed);
      } else {
	_acVectors[i].setSpare(0, _acVectors[i].getSpare(1));
	_acVectors[i].setSpare(1,speed);
      }
      _acVectors[i].assemble();
      //
      _spdb.addPutChunk( dataType,
			 _acVectors[i].getTimeCurrent(),
			 _acVectors[i].getTimeCurrent() + _params->expiry,
			 _acVectors[i].getBufLen(),
			 _acVectors[i].getBufPtr(),
			 dataType2 );
      //
      // Output data every 500 points for a performance gain.
      //
      _spdbBufferCount++;
      if (_spdbBufferCount == 500){
	_spdbBufferCount = 0;
	if (_spdb.put( _params->OutUrl,
		       SPDB_AC_VECTOR_ID,
		       SPDB_AC_VECTOR_LABEL)){
	  cerr << "Failed to put data" << endl;
	  return;
	}
      }
      

      break;
    }
  }
  //
  // If we didn't append this entry to an existing point, add it as a new
  // point to the vector.
  //
  if (!(alreadyEntered)){
    string fid(fltID);
    acPosVector A(T.unix_time, lat, lon, altitude, fid);
    A.setSpare(0,speed);
    _acVectors.push_back(A);
  }


  return;

}

void asdi2Vector::_getTime(date_time_t *T, 
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
  // Use the date parsed from the filename if specified.
  //
  if (_params->dateFromFilename){
    T->year = _filenameYear;
    T->month = _filenameMonth;
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


void asdi2Vector::_save2ascii( char *asdiMsg){

  if (strlen(_asciiBuffer) + strlen(asdiMsg) < _asciiBufferLen-1){
    //
    // Have enough room, just add to the buffer.
    //
    sprintf(_asciiBuffer,"%s%s\n", _asciiBuffer, asdiMsg);
    return;
  }
  //
  // Otherwise, need to flush buffer first.
  //
  date_time_t T;
  T.unix_time = time(NULL);
  uconvert_from_utime(&T);
  //
  // Make the output directory.
  //
  char outdirname[MAX_PATH_LEN];
  sprintf(outdirname,"%s/%04d%02d%02d",
	  _params->OutASCIIdir,
	  T.year, T.month, T.day);

  if (ta_makedir_recurse(outdirname)){
    fprintf(stderr,"Failed to make directory %s\n",
	    outdirname);
    return;
  }

  //
  // Open the output file append.
  //
  char outfilename[MAX_PATH_LEN];
  sprintf(outfilename,"%s/%04d%02d%02d_%02d.asd", outdirname, 
	  T.year, T.month, T.day,T.hour);
  FILE *fp;
  fp = fopen(outfilename, "a");
  if (fp == NULL){
    fprintf(stderr,"Failed to create file %s\n",
	    outfilename);
    return;
  }
  //
  // Write the message and return.
  //
  fprintf(fp,"%s\n", _asciiBuffer);
  fclose(fp);
  sprintf(_asciiBuffer,"%s\n", asdiMsg);
  return;

}


/////////////////////////////////////
//
// Small method to extract a space-delimited substring
//
void asdi2Vector::_extractString(char *instring, char *outstring){

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
int asdi2Vector::_openReadSource(Params *P, char *FileName){
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
void asdi2Vector::_closeReadSource(){
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
int asdi2Vector::_readFromSource(char *buffer, int numbytes){

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
