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


#include <cstring>
#include <cstdlib>
#include <ctype.h>

#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <rapformats/ac_data.h>
#include <rapformats/ac_route.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/file_io.h>
#include <dataport/bigend.h>

#include "asdi2spdb.hh"

//
// Constructor and destructor do nothing.
//
asdi2spdb::asdi2spdb(Params *tdrpParams){ 
  //
  // Make a copy of the params and init a few variables.
  //
  _params = tdrpParams;
  _count = 0;
  _spdbBufferCount = 0;
  _spdbRouteBufferCount = 0;
  _inputFileYear = 0;
  _inputFileMonth = 0;
  memset(_asciiBuffer,  0, _asciiBufferLen);
  if(_params->decodeRoute) {
    _route = new routeDecode(tdrpParams);
  } else 
    _route = NULL;
}
//
// Destructor.
//
asdi2spdb::~asdi2spdb(){

  flush();

  if(_params->decodeRoute) {
    delete _route;
  }
}

void asdi2spdb::flush()
{
  if (_params->saveSPDB){
    if (_spdb.put( _params->PosnOutUrl,
		   SPDB_AC_DATA_ID,
		   SPDB_AC_DATA_LABEL)){
      fprintf(stderr,"ERROR: Failed to put posn data\n");
    }
    if (_spdbRoute.put( _params->RouteOutUrl,
			SPDB_AC_ROUTE_ID,
			SPDB_AC_ROUTE_LABEL)){
      fprintf(stderr,"ERROR: Failed to put data\n");
      return;
    }    
  }
  if (_params->saveRawASCII || _params->saveParsedASCII){

    //
    // Figure out the output file name
    // if input file mode try to figure out from filename
    //
    date_time_t T;
    bool parsedInput = false;
    if(_readingFromFile) {
      // If the input file is one of our previously outputed
      // saveRawASCII files...
      Path P(_inputFile);
      
      if(P.getExt() == "asd") {
	if( 4 == sscanf(P.getBase().c_str(), "%4d%2d%2d_%2d", &(T.year), &(T.month), &(T.day), &(T.hour)) ) {
	  parsedInput = true;
	  T.min = 0;
	  T.sec = 0;
	}
      }
      
    }
    
    if(parsedInput == false) 
    {
      //
      // Realtime use current time as output file
      // or if parsing input failed use current time
      
      T.unix_time = time(NULL);
      uconvert_from_utime(&T);
    }
    
    //
    // Make the output directory.
    //
    char outdirname[MAX_PATH_LEN];
    sprintf(outdirname,"%s/%d%02d%02d",
	    _params->OutASCIIdir,
	    T.year, T.month, T.day);
    
    if (ta_makedir_recurse(outdirname)){
      fprintf(stderr,"ERROR: Failed to make directory %s\n",
	      outdirname);
      return;
    }
    
    char ext[6] = "asd";
    if(_params->saveParsedASCII) {
      strcpy(ext, "ascii");
    }
    
    //
    // Open the output file append.
    //
    char outfilename[MAX_PATH_LEN];
    sprintf(outfilename,"%s/%d%02d%02d_%02d.%s", outdirname, T.year, T.month, T.day, T.hour, ext);
    FILE *fp;
    fp = fopen(outfilename, "a");
    if (fp == NULL){
      fprintf(stderr,"ERROR: Failed to create file %s\n",
	      outfilename);
      return;
    }
    
    //
    // Write the message and return.
    //
    fprintf(fp,"%s", _asciiBuffer);
    fclose(fp);
  }

}


void asdi2spdb::ProcFile(char *FilePath, Params *P){

  if (_openReadSource(P, FilePath)){
    fprintf(stderr,"ERROR: Data source - file or stream - unopenable.\n");
    return;
  }
  if(_params->decodeRoute && _route->isOK() == false) {
    fprintf(stderr,"ERROR: Failed to initialize route decoder class.\n");
    return;
  }

  if(_readingFromFile) {
    _inputFileYear = 0;
    _inputFileMonth = 0;
    // If the input file is one of our previously outputed saveRawASCII files...
    // We can get the year and month from it
    Path P(_inputFile);
    
    if(P.getExt() == "asd") {
      date_time_t T;
      if( 4 == sscanf(P.getBase().c_str(), "%4d%2d%2d_%2d", &(T.year), &(T.month), &(T.day), &(T.hour)) ) {
	_inputFileYear = T.year;
	_inputFileMonth = T.month;
      }
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
    if (numRead == 1) {
      if ((buffer[i] == EOL)) {
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
    } else {
      if (_readingFromFile && feof(_fp))
	break;
    }
  } while (true); // Infinite read loop.
 
  _closeReadSource();

}


////////////////////////////////////////////////////////////////
//
// Processs a message.
//
void asdi2spdb::_processAsdiMsg(char *asdiMsg){

  _count++;
  if (_count == 100){
    _count = 0;
    PMU_auto_register("Processing away...");
  }

  //if (_params->debug)
  //fprintf(stderr,"%s :\n", 
  //    asdiMsg);

  // See if we wish to save the raw data
  // to an ASCII file.
  //
  if (_params->saveRawASCII){
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
  // Only Parse messages we want.
  // All messages start with a 4 byte sequence number, 8 byte time stamp
  //   and 4 byte facility indent. So we skip 16 bytes.
  //
  if(strncmp(asdiMsg + 16, "TZ", 2) == 0){
    _parseTZ(asdiMsg + 16, T);
  } else if(strncmp(asdiMsg + 16, "TO", 2) == 0){
    _parseTO(asdiMsg + 16, T);
  } else if(_params->decodeRoute) {
    if(strncmp(asdiMsg + 16, "FZ", 2) == 0) {
      _parseFZ(asdiMsg + 16, T);
    } else if(strncmp(asdiMsg + 16, "AF", 2) == 0) {
      _parseAF(asdiMsg + 16, T);
    } else if(strncmp(asdiMsg + 16, "UZ", 2) == 0) {
      _parseUZ(asdiMsg + 16, T);
    } else if(strncmp(asdiMsg + 16, "AZ", 2) == 0) {
      _parseAZ(asdiMsg + 16, T);
    } else if(strncmp(asdiMsg + 16, "DZ", 2) == 0) {
      _parseDZ(asdiMsg + 16, T);
    } else if(strncmp(asdiMsg + 16, "RZ", 2) == 0) {
      _parseRZ(asdiMsg + 16, T);
    }
  }

  return;
}

// TZ - Track Message
// Provides a position update.
//
// Field Format legend:
// d=digit, L=Upper Case Letter, a=alphanumeric(d|L), ()=optional field, {d}=repeat d times, |=logical or
// Any other character is defined as that character only.
//
// TZ Field Format: 
//    TZ Flight_ID Speed Altitude Position
//    TZ La(a){5}(/dda) ddd (d)dd|(d)ddT|(d)ddB(d)dd|(d)ddC|OTP/(d)dd ddddL/dddddL|ddddddL/dddddddL
void asdi2spdb::_parseTZ(char *tzMsg, date_time_t T)
{

  //
  // Parse out the flight ID, speed, altitude and location strings.
  //
  char fltID[_internalStringLen];
  unsigned offset = strlen("TZ ");
  if (offset >= strlen(tzMsg)) return;
  char *p = tzMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char speedStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(tzMsg)) return;
  p = tzMsg + offset;
  _extractString(p, speedStr);

  char altStr[_internalStringLen];
  offset += strlen(speedStr) + 1;
  if (offset >= strlen(tzMsg)) return;
  p = tzMsg + offset;
  _extractString(p, altStr);

  char locStr[_internalStringLen];
  offset += strlen(altStr) + 1;
  if (offset >= strlen(tzMsg)) return;
  p = tzMsg + offset;
  _extractString(p, locStr);
  //
  // Parse out the data from these strings.
  //
  // Speed is easy.
  //
  double speed = atof(speedStr);

  //
  // Location
  //
  int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
  char latChar, lonChar;
  double lat, lon;
  if (6 != sscanf(locStr,"%2d%2d%c/%3d%2d%c",
		 &latDeg, &latMin, &latChar,
		 &lonDeg, &lonMin, &lonChar)){
    if(8 != sscanf(locStr,"%2d%2d%2d%c/%3d%2d%2d%c",
		 &latDeg, &latMin, &latSec, &latChar,
		 &lonDeg, &lonMin, &lonSec, &lonChar)){
      return;
    }
    lat = latDeg + (latMin + (latSec / 60.0)) / 60.0;
    lon = lonDeg + (lonMin + (lonSec / 60.0)) / 60.0;
  } else {
    lat = latDeg + latMin / 60.0;
    lon = lonDeg + lonMin / 60.0;
  }

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

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    tzMsg);
  }
  
  ac_data_t A;
  memset(&A, 0, sizeof(A));

  //
  // Parse altitude 
  // If it ends in a 'T' then it is an altitude the aircraft has been
  //  cleared to, but is not at yet. 
  // If it starts with 'OTP/' as in OTP/350 then the aircraft is above 350. 
  // If it has a B then it is a lower and upper altitude, save the average.
  // If it ends in a 'C' then it is correct, but is not what the aircraft is
  // supposed to be at.
  // If it has none of these then it is the assigned altitude.
  //
  double altitude = -1.0;
  double loweralt = -1.0;
  char a1;
  if(altStr[strlen(altStr)-1] == 'C') {
    altitude = atof(altStr);
    A.alt_type = ALT_TRANSPONDER;
  } else if(strlen(altStr) == 3 || strlen(altStr) == 2) {
    altitude = atof(altStr);
    A.alt_type = ALT_NORMAL;
  } else if(altStr[strlen(altStr)-1] == 'T') {
    altitude = atof(altStr);
    A.alt_type = ALT_INTERIM;
  } else if (4 != sscanf(altStr,"%c%c%c/%3d",&a1,&a1,&a1,&altitude) || 
	     (4 != sscanf(altStr,"%c%c%c/%2d",&a1,&a1,&a1,&altitude))) {
    A.alt_type = ALT_VFR_ON_TOP;
  } else if(3 != sscanf(altStr,"%3d%c%3d",&loweralt,&a1,&altitude) ||
	    (3 != sscanf(altStr,"%2d%c%3d",&loweralt,&a1,&altitude)) ||
	    (3 != sscanf(altStr,"%3d%c%2d",&loweralt,&a1,&altitude)) ||
	    (3 != sscanf(altStr,"%2d%c%2d",&loweralt,&a1,&altitude))) {
    if(a1 == 'B') {
      altitude = (loweralt + altitude) / 2.0;
      A.alt_type = ALT_AVERAGE;
    } else 
      altitude = AC_DATA_UNKNOWN_VALUE;
  } else
    altitude = AC_DATA_UNKNOWN_VALUE;


  A.lat = lat; 
  A.lon = lon;
  A.ground_speed = speed;
  sprintf(A.callsign,"%s", fltID);

  if (altitude < 0.0){
    A.alt = AC_DATA_UNKNOWN_VALUE;
  } else {
    //
    // Convert from flight level (hundreds of feet) to Km.
    //
    A.alt = altitude*100.0*0.3048/1000.0;
  }

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  if (_params->saveParsedASCII) {
    char alt_type[5] = "Unk";
    if(A.alt_type == ALT_NORMAL)
      strcpy(alt_type, "Norm");
    else if(A.alt_type == ALT_TRANSPONDER)
      strcpy(alt_type, "Tran");
    else if(A.alt_type == ALT_INTERIM)
      strcpy(alt_type, "Int");
    else if(A.alt_type == ALT_VFR_ON_TOP)
      strcpy(alt_type, "Top");
    else if(A.alt_type == ALT_AVERAGE)
      strcpy(alt_type, "Avg");

    char buffer[256];
    sprintf(buffer,"%02d/%02d/%d %02d:%02d:%02d %s %3f %3f %3d %3d %s", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,A.callsign,A.lat,A.lon,
	    (int)A.ground_speed,(int)altitude,alt_type);
    _save2ascii(buffer);
  }

  ac_data_to_BE( &A );


  _spdb.addPutChunk( dataType,
		     T.unix_time,
		     T.unix_time + _params->expiry,
		     sizeof(ac_data_t), &A,
		     dataType2 );
  //
  // Output data every N points.
  //
  _spdbBufferCount++;
  if (_spdbBufferCount == _params->nPosnWrite){
    _spdbBufferCount = 0;
    if (_spdb.put( _params->PosnOutUrl,
		   SPDB_AC_DATA_ID,
		   SPDB_AC_DATA_LABEL)){
      fprintf(stderr,"ERROR: Failed to put posn data\n");
      return;
    }
  }

  return;

}

// FZ - Flight Plan Information Message 
// Provides flight plan data for eligible flight plans.
//
// FZ Field Format:
//    FZ Flight_ID Aircraft Speed CoordinationFix CoordinationTime Altitude Route
//    FZ La(a){5}(/dda) (da|a)/aa(a)(a)(/L) dd(d)(d)|Lddd|SC aa(a)(a)(a)(/)(a){6} Ldddd (d)dd|(d)ddB(d)dd variable
void asdi2spdb::_parseFZ(char *fzMsg, date_time_t T)
{
  char fltID[_internalStringLen];
  unsigned offset = strlen("FZ ");
  if (offset >= strlen(fzMsg)) return;
  char *p = fzMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char typeStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(fzMsg)) return;
  p = fzMsg + offset;
  _extractString(p, typeStr);

  char speedStr[_internalStringLen];
  offset += strlen(typeStr) + 1;
  if (offset >= strlen(fzMsg)) return;
  p = fzMsg + offset;
  _extractString(p, speedStr);

  char boundryStr[_internalStringLen];
  offset += strlen(speedStr) + 1;
  if (offset >= strlen(fzMsg)) return;
  p = fzMsg + offset;
  _extractString(p, boundryStr);

  char boundryTimeStr[_internalStringLen];
  offset += strlen(boundryStr) + 1;
  if (offset >= strlen(fzMsg)) return;
  p = fzMsg + offset;
  _extractString(p, boundryTimeStr);

  char altStr[_internalStringLen];
   offset += strlen(boundryTimeStr) + 1;
  if (offset >= strlen(fzMsg)) return;
  p = fzMsg + offset;
  _extractString(p, altStr);
  
  offset += strlen(altStr) + 1;
  if (offset >= strlen(fzMsg)) return;
  char *routeStr = fzMsg + offset; //Route is the last part

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    fzMsg);
  }

  char fixStr[_internalStringLen];
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];
  int destTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  double altitude = AC_ROUTE_MISSING_FLOAT;

  fixStr[0] = char(0);
  destStr[0] = char(0);
  deptStr[0] = char(0);

  if(speedStr[0] == 'M')
    airSpeed =  (atoi(speedStr+1) / 100) * MACH1;
  else
    airSpeed = atoi(speedStr);

  altitude = atof(altStr);
  if (altitude != AC_ROUTE_MISSING_FLOAT)
    // Convert from flight level (hundreds of feet) to Km.
    altitude = altitude*100.0*0.3048/1000.0;


  //
  // Decode the route string
  //
  _route->decodeRoute(routeStr);

  int routeSize;
  ac_route_posn_t *routeArray = _route->getRouteArray(&routeSize, &destTime,
						      destStr, deptStr,_internalStringLen);
  if(routeArray == NULL)
    return;
  if(routeSize < 1 || routeSize > 1000) {
    fprintf(stderr,"WARNING: getRouteArray returned bad routeSize: %d \n", routeSize);
    return;
  }
  //
  // Return if we are applying a region and the route 
  // has no point in it.
  //
  if (_params->applyRegion) {
    bool entered = false;
    for(int a = 0; a < routeSize; a++) {
      if ((routeArray[a].lat < _params->region.latMax) &&
	  (routeArray[a].lat > _params->region.latMin) &&
	  (routeArray[a].lon < _params->region.lonMax) &&
	  (routeArray[a].lon > _params->region.lonMin)) {
	entered = true;
      }
    }
    if(entered == false)
      return;
  }

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_FlightPlan, ALT_NORMAL, routeSize,
				   destTime, deptTime, altitude, airSpeed,
				   fixTime, fixStr, typeStr, destStr,
				   deptStr, fltID, routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  delete [] routeArray;
  free(ac_route);

  return;
}

void asdi2spdb::_parseAF(char *afMsg, date_time_t T)
{
  char fltID[_internalStringLen];
  unsigned offset = strlen("AF ");
  if (offset >= strlen(afMsg)) return;
  char *p = afMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL) {
      return;
    }
  }

  char departureStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(afMsg)) return;
  p = afMsg + offset;
  _extractString(p, departureStr);

  char destinationStr[_internalStringLen];
  offset += strlen(departureStr) + 1;
  if (offset >= strlen(afMsg)) return;
  p = afMsg + offset;
  _extractString(p, destinationStr);

  // The rest of the message is a code followed by the updated
  // value for that code (all space seperated)
  // 03 is the code for aircraftdata, 05 is air speed
  // 06 is the code for position, 07 is for estArrival,
  // 08 is the code for altitude (assigned, 09 is altitude (proposed),
  // 10 is the code for Flight plan
  offset += strlen(destinationStr) + 1;

  char Str[_internalStringLen];
  char routeStr[_internalStringLen];
  char aircraftStr[_internalStringLen];
  char fixStr[_internalStringLen];
  char timeStr[_internalStringLen];
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  float altitude = AC_ROUTE_MISSING_FLOAT;
  ac_data_alt_type_t alt_type = ALT_NORMAL;

  int destTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];

  destStr[0] = char(0);
  deptStr[0] = char(0);
  routeStr[0] = char(0);
  aircraftStr[0] = char(0);
  fixStr[0] = char(0);
  timeStr[0] = char(0);

  while(offset < strlen(afMsg)) {
    p = afMsg + offset;
    _extractString(p, Str);
    offset += strlen(Str) + 1;
    if(strcmp(Str, "03") == 0) { 
      p = afMsg + offset;
      _extractString(p, aircraftStr);
      offset += strlen(aircraftStr) + 1;

    } else if(strcmp(Str, "05") == 0) {  
      p = afMsg + offset;
      _extractString(p, Str);
      offset += strlen(Str) + 1;
      if(Str[0] == 'M')
	airSpeed = (atoi(Str) / 100) * MACH1;
      else
	airSpeed = atoi(Str);

    } else if(strcmp(Str, "06") == 0) {  
      p = afMsg + offset;
      _extractString(p, fixStr);
      offset += strlen(fixStr) + 1;

    } else if(strcmp(Str, "07") == 0) {  
      p = afMsg + offset;
      _extractString(p, timeStr);
      offset += strlen(timeStr) + 1;

    } else if(strcmp(Str, "08") == 0) {  
      p = afMsg + offset;
      _extractString(p, Str);
      offset += strlen(Str) + 1;
      altitude = atof(Str);
      alt_type = ALT_NORMAL;

    } else if(strcmp(Str, "09") == 0) {  
      p = afMsg + offset;
      _extractString(p, Str);
      offset += strlen(Str) + 1;
      altitude = atof(Str);
      alt_type = ALT_INTERIM;

    } else if(strcmp(Str, "10") == 0) {  
      p = afMsg + offset;
      _extractString(p, routeStr);
      offset += strlen(routeStr) + 1;
      if(offset+1 < strlen(afMsg))  // Case where route is not last part
	 return;    // Ignore the whole message, I dont know what it is
    }
  }

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    afMsg);
  }

  //
  // Does it have an updated route?
  //
  int routeSize = 0;
  ac_route_posn_t *routeArray = NULL;
  if(strlen(routeStr) > 0) {
    //
    // Decode the route
    _route->decodeRoute(routeStr);

    routeArray = _route->getRouteArray(&routeSize, &destTime, destStr,
				       deptStr, _internalStringLen);

    if(routeArray == NULL)
      routeSize = 0;
    else if(routeSize < 1 || routeSize > 1000) {
      fprintf(stderr,"WARNING: getRouteArray returned bad routeSize: %d \n", routeSize);
      routeSize = 0;
    } else {
      // Return if we are applying a region and the route 
      // has no point in it.
      if (_params->applyRegion) {
	bool entered = false;
	for(int a = 0; a < routeSize; a++) {
	  if ((routeArray[a].lat < _params->region.latMax) &&
	      (routeArray[a].lat > _params->region.latMin) &&
	      (routeArray[a].lon < _params->region.lonMax) &&
	      (routeArray[a].lon > _params->region.lonMin)) {
	    entered = true;
	  }
	}
	if(entered == false)
	  return;
      }
    }
  }

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Revised, alt_type, routeSize,
				   destTime, deptTime, altitude, airSpeed,
				   fixTime, fixStr, aircraftStr, destStr,
				   deptStr, fltID, routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  delete [] routeArray;
  free(ac_route);

  return;
}

// UZ - ARTCC boundary crossing Message 
// Sent to provide current flight plan information on active
// eligible flights that enter an ARTCC.
//
// UZ Field Format:
//    UZ Flight_ID Aircraft Speed Boundry_rossing CrossingTime Altitude Route
//    UZ La(a){5}(/dda) (da|a)/aa(a)(a)(/L) dd(d)(d)|Lddd|SC ddd(L)/(d)dddd(L) Edddd (d)dd|(d)ddB(d)dd variable
void asdi2spdb::_parseUZ(char *uzMsg, date_time_t T)
{
  char fltID[_internalStringLen];
  unsigned offset = strlen("UZ ");
  if (offset >= strlen(uzMsg)) return;
  char *p = uzMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char typeStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(uzMsg)) return;
  p = uzMsg + offset;
  _extractString(p, typeStr);

  char speedStr[_internalStringLen];
  offset += strlen(typeStr) + 1;
  if (offset >= strlen(uzMsg)) return;
  p = uzMsg + offset;
  _extractString(p, speedStr);

  char boundryStr[_internalStringLen];
  offset += strlen(speedStr) + 1;
  if (offset >= strlen(uzMsg)) return;
  p = uzMsg + offset;
  _extractString(p, boundryStr);

  char boundryTimeStr[_internalStringLen];
  offset += strlen(boundryStr) + 1;
  if (offset >= strlen(uzMsg)) return;
  p = uzMsg + offset;
  _extractString(p, boundryTimeStr);

  char altStr[_internalStringLen];
  offset += strlen(boundryTimeStr) + 1;
  if (offset >= strlen(uzMsg)) return;
  p = uzMsg + offset;
  _extractString(p, altStr);
  
  offset += strlen(altStr) + 1;
  if (offset >= strlen(uzMsg)) return;
  char *routeStr = uzMsg + offset;    // Route is the last part

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    uzMsg);
  }

  int airSpeed = AC_ROUTE_MISSING_INT;
  if(speedStr[0] == 'M')
    airSpeed = (atoi(speedStr+1) / 100) * MACH1;
  else
    airSpeed = atoi(speedStr);

  double altitude = AC_ROUTE_MISSING_FLOAT;
  altitude = atof(altStr);

  if (altitude != AC_ROUTE_MISSING_FLOAT)
    // Convert from flight level (hundreds of feet) to Km.
    altitude = altitude*100.0*0.3048/1000.0;

  // Convert boundry string into lat/lon
  int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
  char latChar, lonChar;
  double lat, lon;
  if (6 != sscanf(boundryStr,"%2d%2d%c/%3d%2d%c",
		  &latDeg, &latMin, &latChar,
		  &lonDeg, &lonMin, &lonChar)) {
    if(8 != sscanf(boundryStr,"%2d%2d%2d%c/%3d%2d%2d%c",
		   &latDeg, &latMin, &latSec, &latChar,
		   &lonDeg, &lonMin, &lonSec, &lonChar)) {
      return;
    }
    lat = latDeg + (latMin + (latSec / 60.0)) / 60.0;
    lon = lonDeg + (lonMin + (lonSec / 60.0)) / 60.0;
  } else {
    lat = latDeg + latMin / 60.0;
    lon = lonDeg + lonMin / 60.0;
  }
  
  if (latChar == 'S') lat = -lat;
  if (lonChar == 'W') lon = -lon;
  
  int deptTime = AC_ROUTE_MISSING_INT;
  int destTime = AC_ROUTE_MISSING_INT;
  int boundryTime = AC_ROUTE_MISSING_INT;
  char destStr[_internalStringLen];
  char deptStr[_internalStringLen];

  //
  // Decode the route string
  //
  _route->decodeRoute(routeStr);

  int routeSize;
  ac_route_posn_t *routeArray = _route->getRouteArray(&routeSize, &destTime, destStr,
						      deptStr, _internalStringLen);
  if(routeArray == NULL)
    return;
  if(routeSize < 1 || routeSize > 1000) {
    fprintf(stderr,"WARNING: getRouteArray returned bad routeSize: %d \n", routeSize);
    return;
  }

  //
  // Return if we are applying a region and the route 
  // has no point in it.
  //
  if (_params->applyRegion) {
    bool entered = false;
    for(int a = 0; a < routeSize; a++) {
      if ((routeArray[a].lat < _params->region.latMax) &&
	  (routeArray[a].lat > _params->region.latMin) &&
	  (routeArray[a].lon < _params->region.lonMax) &&
	  (routeArray[a].lon > _params->region.lonMin)) {
	entered = true;
      }
    }
    if(entered == false)
      return;
  }

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_BoundryCross, ALT_NORMAL, routeSize,
				   destTime, deptTime, altitude, airSpeed,
				   boundryTime, boundryStr, typeStr, destStr,
				   deptStr, fltID, routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  delete [] routeArray;
  free(ac_route);

  return;
}

// AZ - Arrival Message 
// Arrival data for all eligible arriving flights.
//
// AZ Field Format:
//    AZ Flight_ID Departure Destination ArrivalTime
//    AZ La(a){5}(/dda) aa(a){10} aa(a){10} (L)dddd
void asdi2spdb::_parseAZ(char *azMsg, date_time_t T)
{

  //
  // Parse out the flight ID string.
  //
  char fltID[_internalStringLen];
  unsigned offset = strlen("AZ ");
  if (offset >= strlen(azMsg)) return;
  char *p = azMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char deptStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(azMsg)) return;
  p = azMsg + offset;
  _extractString(p, deptStr);

  char destStr[_internalStringLen];
  offset += strlen(deptStr) + 1;
  if (offset >= strlen(azMsg)) return;
  p = azMsg + offset;
  _extractString(p, destStr);

  char arrTimeStr[_internalStringLen];
  offset += strlen(destStr) + 1;
  if (offset >= strlen(azMsg)) return;
  p = azMsg + offset;
  _extractString(p, arrTimeStr);

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    azMsg);
  }

  float altitude = AC_ROUTE_MISSING_FLOAT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int destTime = AC_ROUTE_MISSING_INT;
  destTime = atoi(arrTimeStr);

  char fixStr[_internalStringLen];
  char typeStr[_internalStringLen];
  fixStr[0] = char(0);
  typeStr[0] = char(0);
  ac_route_posn_t *routeArray = NULL;

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Arrival, ALT_NORMAL, 0,
				   destTime, deptTime, altitude, 
				   airSpeed, fixTime,
				   fixStr, typeStr, destStr, deptStr, fltID, 
				   routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  return;
}

// DZ - Departure Message 
// Transmitted for all eligible initially activated flight plans when
//  the activation message is not from an adjacent NAS.
//
// DZ Field Format:
//    DZ Flight_ID Aircraft Departure Departure_Time Destination EstArrival
//    DZ La(a){5}(/dda) (da|a)/aa(a)(a)(/L) aa(a){10} Ldddd aa(a){10} dddd
void asdi2spdb::_parseDZ(char *dzMsg, date_time_t T)
{

  //
  // Parse out the flight ID string.
  //
  char fltID[_internalStringLen];
  unsigned offset = strlen("DZ ");
  if (offset >= strlen(dzMsg)) return;
  char *p = dzMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char typeStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(dzMsg)) return;
  p = dzMsg + offset;
  _extractString(p, typeStr);

  char deptStr[_internalStringLen];
  offset += strlen(typeStr) + 1;
  if (offset >= strlen(dzMsg)) return;
  p = dzMsg + offset;
  _extractString(p, deptStr);

  char deptTimeStr[_internalStringLen];
  offset += strlen(deptStr) + 1;
  if (offset >= strlen(dzMsg)) return;
  p = dzMsg + offset;
  _extractString(p, deptTimeStr);

  char destStr[_internalStringLen];
  offset += strlen(deptTimeStr) + 1;
  if (offset >= strlen(dzMsg)) return;
  p = dzMsg + offset;
  _extractString(p, destStr);

  char arrTimeStr[_internalStringLen];
  offset += strlen(destStr) + 1;
  if (offset >= strlen(dzMsg)) 
    gcvt (AC_ROUTE_MISSING_INT, 3, arrTimeStr);
  else {
    p = dzMsg + offset;
    _extractString(p, arrTimeStr);
  }

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    dzMsg);
  }

  float altitude = AC_ROUTE_MISSING_FLOAT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int destTime = AC_ROUTE_MISSING_INT;
  deptTime = atoi(deptTimeStr);
  destTime = atoi(arrTimeStr);

  char fixStr[_internalStringLen];
  fixStr[0] = char(0);
  ac_route_posn_t *routeArray = NULL;

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Departure, ALT_NORMAL, 0,
				   destTime, deptTime, altitude, 
				   airSpeed, fixTime, 
				   fixStr, typeStr, destStr, deptStr, fltID, 
				   routeArray, &ac_route_size);

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  return;
}

// RZ - Cancellation Message 
// Cancelation of flight for all eligible flight plans.
//
// RZ Field Format:
//    RZ Flight_ID Departure Destination
//    RZ La(a){5}(/dda) aa(a){10} aa(a){10}
void asdi2spdb::_parseRZ(char *rzMsg, date_time_t T)
{

  //
  // Parse out the flight ID string.
  //
  char fltID[_internalStringLen];
  unsigned offset = strlen("RZ ");
  if (offset >= strlen(rzMsg)) return;
  char *p = rzMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char deptStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(rzMsg)) return;
  p = rzMsg + offset;
  _extractString(p, deptStr);

  char destStr[_internalStringLen];
  offset += strlen(deptStr) + 1;
  if (offset >= strlen(rzMsg)) return;
  p = rzMsg + offset;
  _extractString(p, destStr);

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    rzMsg);
  }

  float altitude = AC_ROUTE_MISSING_FLOAT;
  int airSpeed = AC_ROUTE_MISSING_INT;
  int fixTime = AC_ROUTE_MISSING_INT;
  int deptTime = AC_ROUTE_MISSING_INT;
  int destTime = AC_ROUTE_MISSING_INT;

  char fixStr[_internalStringLen];
  char typeStr[_internalStringLen];
  fixStr[0] = char(0);
  typeStr[0] = char(0);
  ac_route_posn_t *routeArray = NULL;

  //
  // Turn the route array into a ac_route for spdb
  //
  int ac_route_size;
  void *ac_route = create_ac_route(AC_ROUTE_Cancelled, ALT_NORMAL, 0,
				   destTime, deptTime, altitude, 
				   airSpeed, fixTime, 
				   fixStr, typeStr, destStr, deptStr, fltID, 
				   routeArray, &ac_route_size);
  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Save out to Spdb
  _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		   ac_route_size, ac_route, dataType2);

  return;
}

// TO - Oceanic Position
// Provides sparse position reports while outside radar tracking via ARINC.
//
// TO Field Format:
//    TO Flight_ID Speed Time1 Altitude1 Position1 (Time2 Altitude2 Position2 )(Time3 Altitude3 Position3 )Departure Destination
//    TO La(a){5} ddd dd/dddd ddd ddddL/dddddL (dd/dddd ddd ddddL/dddddL){2} LLLL|- LLLL|-
void asdi2spdb::_parseTO(char *toMsg, date_time_t T)
{
  //
  // Parse out the flight ID string.
  //
  char fltID[_internalStringLen];
  unsigned offset = strlen("TO ");
  if (offset >= strlen(toMsg)) return;
  char *p = toMsg + offset;
  _extractString(p, fltID);
  //
  // Return if we are only saving certain flightIDs.
  //
  if(_params->applyFlightName) {
    if(strstr(fltID, _params->flightName) == NULL)
      return;
  }

  char speedStr[_internalStringLen];
  offset += strlen(fltID) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  _extractString(p, speedStr);

  char timeStr[_internalStringLen];
  offset += strlen(speedStr) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  _extractString(p, timeStr);

  char altStr[_internalStringLen];
  offset += strlen(timeStr) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  _extractString(p, altStr);

  char locStr[_internalStringLen];
  offset += strlen(altStr) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  _extractString(p, locStr);

  char deptStr[_internalStringLen];
  offset += strlen(locStr) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  _extractString(p, deptStr);

  char destStr[_internalStringLen];
  offset += strlen(deptStr) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  _extractString(p, destStr);

  offset += strlen(destStr) + 1;

  //
  // Parse out the data from these strings.
  //
  // Speed is easy.
  //
  double speed = atof(speedStr);

  //
  // Location
  //
  int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
  char latChar, lonChar;
  double lat, lon;
  if (6 != sscanf(locStr,"%2d%2d%c/%3d%2d%c",
		 &latDeg, &latMin, &latChar,
		 &lonDeg, &lonMin, &lonChar)){
    return;
  } else {
    lat = latDeg + latMin / 60.0;
    lon = lonDeg + lonMin / 60.0;
  }
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

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    toMsg);
  }

  //
  // Time Oceanic Reports have a seperate time
  //
  if (3 != sscanf(timeStr,"%2d/%2d%2d",
		  &(T.day), &(T.hour), &(T.min)) ){
    return;
  }
  T.sec = 0;
  uconvert_to_utime( &T );

  ac_data_t A;
  memset(&A, 0, sizeof(A));

  // Set client data type to letter O to 
  // distinguish Oceanic position.
  A.client_data_type = char(79);
  A.lat = lat; 
  A.lon = lon;
  A.ground_speed = speed;
  strncpy(A.callsign, fltID, AC_DATA_CALLSIGN_LEN);
  strncpy(A.origin, deptStr, AC_DATA_AIRPORT_LEN);
  strncpy(A.destination, destStr, AC_DATA_AIRPORT_LEN);

  //
  // Convert from flight level (hundreds of feet) to Km.
  //
  A.alt = atof(altStr)*100.0*0.3048/1000.0;
  A.alt_type = ALT_NORMAL;

  //
  // Contrive data type for uniqueness.
  si32 dataType;
  si32 dataType2;
  _contriveDataType(fltID, &dataType, &dataType2);

  //
  // Create a route based on the lat/lons available
  //
  if(_params->decodeRoute) {
    int routeSize = 3;
    ac_route_posn_t routeArray[5];
    
    if(offset < strlen(toMsg)) {
      routeSize = 4;

      char locStr2[_internalStringLen];
      p = toMsg + offset;
      _extractString(p, locStr2);
      strncpy(routeArray[2].name, locStr2, AC_ROUTE_N_POSNAME);

      int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
      char latChar, lonChar;
      if (6 != sscanf(locStr2,"%2d%2d%c/%3d%2d%c",
		      &latDeg, &latMin, &latChar,
		      &lonDeg, &lonMin, &lonChar)){
	return;
      } else {
	routeArray[2].lat = latDeg + latMin / 60.0;
	routeArray[2].lon = lonDeg + lonMin / 60.0;
      }
      if (latChar == 'S') routeArray[2].lat = -routeArray[2].lat;
      if (lonChar == 'W') routeArray[2].lon = -routeArray[2].lon;

      offset += strlen(locStr2) + 1;
      if(offset < strlen(toMsg)) {
	routeSize = 5;

	char timeStr3[_internalStringLen];
	p = toMsg + offset;
	_extractString(p, timeStr3);
	
	char altStr3[_internalStringLen];
	offset += strlen(timeStr3) + 1;
	if (offset >= strlen(toMsg)) return;
	p = toMsg + offset;
	_extractString(p, altStr3);
	
	char locStr3[_internalStringLen];
	offset += strlen(altStr3) + 1;
	if (offset >= strlen(toMsg)) return;
	p = toMsg + offset;
	_extractString(p, locStr3);

	strncpy(routeArray[3].name, locStr3, AC_ROUTE_N_POSNAME);
	
	int latDeg, lonDeg, latMin, lonMin, latSec, lonSec;
	char latChar, lonChar;
	if (6 != sscanf(locStr3,"%2d%2d%c/%3d%2d%c",
			&latDeg, &latMin, &latChar,
			&lonDeg, &lonMin, &lonChar)){
	  return;
	} else {
	  routeArray[3].lat = latDeg + latMin / 60.0;
	  routeArray[3].lon = lonDeg + lonMin / 60.0;
	}
	if (latChar == 'S') routeArray[3].lat = -routeArray[3].lat;
	if (lonChar == 'W') routeArray[3].lon = -routeArray[3].lon;

	offset += strlen(locStr3) + 1;
      }

      if (offset >= strlen(toMsg)) return;
      p = toMsg + offset;
      _extractString(p, deptStr);
      
      offset += strlen(deptStr) + 1;
      if (offset >= strlen(toMsg)) return;
      p = toMsg + offset;
      _extractString(p, destStr);

      //
      // Set the first and last points on the route
      // to the Departure and destination airports.
      //
      routeArray[0].lat = AC_ROUTE_MISSING_INT;
      routeArray[0].lon = AC_ROUTE_MISSING_INT;
      routeArray[routeSize-1].lat = AC_ROUTE_MISSING_INT;
      routeArray[routeSize-1].lon = AC_ROUTE_MISSING_INT;

      strncpy(routeArray[0].name, deptStr, AC_ROUTE_N_POSNAME);
      _route->lookupAirport(deptStr, &(routeArray[0].lat), &(routeArray[0].lon) );
      strncpy(routeArray[routeSize-1].name, destStr, AC_ROUTE_N_POSNAME);
      _route->lookupAirport(destStr, &(routeArray[routeSize-1].lat),
		    &(routeArray[routeSize-1].lon) );

      //
      // Set the second point on the route
      // to the current position.
      //
      routeArray[1].lat = A.lat;
      routeArray[1].lon = A.lon;
      strncpy(routeArray[1].name, locStr, AC_ROUTE_N_POSNAME);

      int deptTime = AC_ROUTE_MISSING_INT;
      int destTime = AC_ROUTE_MISSING_INT;
      int boundryTime = AC_ROUTE_MISSING_INT;
      char typeStr[_internalStringLen];
      typeStr[0] = char(0);

      if(timeStr[strlen(timeStr)-5] == '/') {
	char *time = timeStr + strlen(timeStr)-4;
	boundryTime = atoi(time);
      }

      //
      // Turn the route array into a ac_route for spdb
      //
      int ac_route_size;
      void *ac_route = create_ac_route(AC_ROUTE_Oceanic, ALT_NORMAL, routeSize,
				       destTime, deptTime, A.alt, (int)A.ground_speed,
				       boundryTime, locStr, typeStr, destStr,
				       deptStr, fltID, routeArray, &ac_route_size);

      //
      // Save out to Spdb
      _saveRouteToSPDB(dataType, T.unix_time, T.unix_time + _params->expiry,
		       ac_route_size, ac_route, dataType2);

    }
  } else {

    while(offset < strlen(toMsg)) {
      strcpy(deptStr, destStr);
      
      p = toMsg + offset;
      _extractString(p, destStr);
      
      offset += strlen(destStr) + 1;
    }
  }


  ac_data_to_BE( &A );

  _spdb.addPutChunk( dataType,
		     T.unix_time,
		     T.unix_time + _params->expiry,
		     sizeof(ac_data_t), &A,
		     dataType2 );

  //
  // Output data every N points.
  //
  _spdbBufferCount++;
  if (_spdbBufferCount == _params->nPosnWrite){
    _spdbBufferCount = 0;
    if (_spdb.put( _params->PosnOutUrl,
		   SPDB_AC_DATA_ID,
		   SPDB_AC_DATA_LABEL)){
      fprintf(stderr,"ERROR: Failed to put posn data\n");
      return;
    }
  }

  return;
}

/*
 *  Saves a ac_route to Spdb
 *  Returns true on success
 */
bool asdi2spdb::_saveRouteToSPDB(const si32 data_type, const time_t valid_time,
				 const time_t expire_time, const int chunk_len,
				 const void *chunk_data, const si32 data_type2)
{
  //
  // Add the route to the buffer
  _spdbRoute.addPutChunk(data_type, valid_time, expire_time, 
			 chunk_len, chunk_data, data_type2 );
  //
  // Output the buffer every N routes.
  //
  _spdbRouteBufferCount++;
  if (_spdbRouteBufferCount == _params->nRouteWrite){
    _spdbRouteBufferCount = 0;
    if (_spdbRoute.put( _params->RouteOutUrl, SPDB_AC_ROUTE_ID,
			SPDB_AC_ROUTE_LABEL)){
      fprintf(stderr,"ERROR: Failed to put route data\n");
      return false;
    }
  }
  return true;
}


/* 
 * Creates a two dataTypes based on flightID
 * first dataType is the last four characters before a slash (actual flightID)
 * second dataType is the last four characters of the flightID character array.
 * Examples:
 *   "UAL224":     dataType="L224",  dataType2="L224"
 *   "UAL224/532": dataType="L224",  dataType2="/532"
 *   "UAL2/332":   dataType="UAL2",  dataType2="/332"
 *   "UAL59":      dataType="AL59",  dataType2="AL59"
 *   "N423":       dataType="N423",  dataType2="N423"
 */
void asdi2spdb::_contriveDataType(char *fltID, si32 *dataType, si32 *dataType2)
{
  
  if (NULL != strstr(fltID,"/")){
    char *p = strstr(fltID,"/") - 4;
    *dataType = Spdb::hash4CharsToInt32( p );
  } else {
    if(strlen(fltID) < 4)
      *dataType = Spdb::hash4CharsToInt32(fltID);
    else
      *dataType = Spdb::hash4CharsToInt32( fltID + strlen(fltID) - 4);
  }

  if(strlen(fltID) < 4)
    *dataType2 = Spdb::hash4CharsToInt32(fltID);
  else
    *dataType2 = Spdb::hash4CharsToInt32(fltID + strlen(fltID)-4);    

}

void asdi2spdb::_getTime(date_time_t *T, 
			 int A_day, int A_hour, 
			 int A_min, int A_sec){
  //
  // If the date was determined from the file name
  // then use that first.
  //
  if(_inputFileYear > 0) {
    T->year = _inputFileYear = 0;
    T->month =  _inputFileMonth = 0;
    T->day = A_day; T->hour = A_hour; T->min = A_min; T->sec = A_sec;
    uconvert_to_utime( T );
    return;
  }

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


void asdi2spdb::_save2ascii( char *asdiMsg){

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
  flush();

  sprintf(_asciiBuffer,"%s\n", asdiMsg);
  return;

}


/////////////////////////////////////
//
// Small method to extract a space-delimited substring
//
void asdi2spdb::_extractString(char *instring, char *outstring){

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
int asdi2spdb::_openReadSource(Params *P, char *FileName){
  //
  // Decide if we are reading from a file.
  //
  if (P->mode == Params::REALTIME_STREAM){
    _readingFromFile = false;
  } else {
    _readingFromFile = true;
    _inputFile = FileName;
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
      fprintf(stderr,"ERROR: Failed to read from file %s\n",
	      FileName);
    } else {
      fprintf(stderr,"ERROR: Attempt to open port %d at %s returned %d\n",
	      P->port, P->hostname, retVal);

      fprintf(stderr,"Error string : %s\n",_S.getErrString().c_str());

      switch(_S.getErrNum()){

      case Socket::UNKNOWN_HOST :
	fprintf(stderr,"ERROR: Unknown host.\n");
	break;

      case Socket::SOCKET_FAILED :
	fprintf(stderr,"ERROR: Could not set up socked (maxed out decriptors?).\n");
	break;
      
      case Socket::CONNECT_FAILED :
	fprintf(stderr,"ERROR: Socket Connect failed.\n");
	break;
 
      case Socket::TIMED_OUT :
	fprintf(stderr,"ERROR: Socket Timed out..\n");
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"ERROR: Unexpected socket error.\n");
	break;

      default :
	fprintf(stderr,"ERROR: Unknown socket error.\n");
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
void asdi2spdb::_closeReadSource(){
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
int asdi2spdb::_readFromSource(char *buffer, int numbytes){

  int retVal;
  if (_readingFromFile){
    retVal = fread(buffer, sizeof(unsigned char), numbytes, _fp);
  } else {

    if (_S.readSelectPmu()){

      switch (_S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"ERROR: Read select timed out.\n");
	exit(-1);
	break;

      case Socket::SELECT_FAILED :
	fprintf(stderr,"ERROR: Read select failed.\n");
	exit(-1);
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"ERROR: Read select - unexpected error.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"ERROR: Unkown error with read select.\n");
	exit(-1);
	break;

      }
    }

    if (_S.readBufferHb(buffer,
			numbytes,
			numbytes,
			(Socket::heartbeat_t)PMU_auto_register, 3600) != 0 ){

      switch (_S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"ERROR: Read buffer timed out.\n");
	exit(-1);
	break;

      case Socket::BAD_BYTE_COUNT :
	fprintf(stderr,"ERROR: Read buffer gave bad byte count.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"ERROR: Unkown error with read buffer.\n");
	exit(-1);
	break;
      }

      return -1;
    }

    retVal = _S.getNumBytes();
  }
  
  return retVal;
}
