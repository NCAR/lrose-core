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

#include <Ncxx/Nc3File.hh>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <dataport/bigend.h>

#include "asdi2netcdf.hh"

//
// Constructor and destructor do nothing.
//
asdi2netcdf::asdi2netcdf(Params *tdrpParams){ 
  //
  // Make a copy of the params and init a few variables.
  //
  _params = tdrpParams;
  _count = 0;
  _ncdfBufferCount = 0;
  _inputFileYear = 0;
  _inputFileMonth = 0;
  _fileStartT = 0;
  memset(_asciiBuffer,  0, _asciiBufferLen);
}

void asdi2netcdf::flush()
{
  _flushAscii();
  if (!(_params->saveNCDF))
    _dumpNetCDF();
}

//
// Destructor.
//
asdi2netcdf::~asdi2netcdf(){
  flush();
}


void asdi2netcdf::ProcFile(char *FilePath, Params *P){

  if (_openReadSource(P, FilePath)){
    fprintf(stderr,"ERROR: Data source - file or stream - unopenable.\n");
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
	_inputFileDay = T.day;
	_inputFileHour = T.hour;
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
void asdi2netcdf::_processAsdiMsg(char *asdiMsg){

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
  if (!(_params->saveNCDF)){
    return;
  }

  //
  // See if we can scan in the ASDI time after the 4 character
  // sequence ID.
  //
  int A_day, A_hour, A_min, A_sec;

  if(strncmp(asdiMsg + 18, " ", 1) != 0) {
    return;
  }

  if (4 != sscanf(asdiMsg + strlen("513D"),
		  "%2d%2d%2d%2d",
		  &A_day, &A_hour, &A_min, &A_sec)){
    return;
  }

  _getTime(&_T, A_day, A_hour, A_min, A_sec);

  if(_fileStartT == 0) {
    _fileStartT = _T.unix_time;
    _fileStartT -= _fileStartT % (_params->minsWrite * 60);
  }

  if(_T.unix_time - _fileStartT >= _params->minsWrite * 60) {
    _dumpNetCDF();
  }

  //
  // Only Parse messages we want.
  // All messages start with a 4 byte sequence number, 8 byte time stamp
  //   and 4 byte facility indent. So we skip 16 bytes.
  //
  if(strncmp(asdiMsg + 16, "TZ", 2) == 0){
    _parseTZ(asdiMsg + 16, _T);
  } else if(strncmp(asdiMsg + 16, "TO", 2) == 0){
    _parseTO(asdiMsg + 16, _T);
  } else if(strncmp(asdiMsg + 16, "AZ", 2) == 0) {
    _parseAZ(asdiMsg + 16, _T);
  } else if(strncmp(asdiMsg + 16, "DZ", 2) == 0) {
    _parseDZ(asdiMsg + 16, _T);
  } else if(0) {
    if(strncmp(asdiMsg + 16, "FZ", 2) == 0) {
      //_parseFZ(asdiMsg + 16, _T);
    } else if(strncmp(asdiMsg + 16, "AF", 2) == 0) {
      //_parseAF(asdiMsg + 16, _T);
    } else if(strncmp(asdiMsg + 16, "UZ", 2) == 0) {
      //_parseUZ(asdiMsg + 16, _T);
    } else if(strncmp(asdiMsg + 16, "RZ", 2) == 0) {
      //_parseRZ(asdiMsg + 16, _T);
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
void asdi2netcdf::_parseTZ(char *tzMsg, date_time_t T)
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
  if(_params->limitGA) {
    if(fltID[0] == 'N' && fltID[1] >= '0' && fltID[1] <= '9')
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
  if(speed == 0.0)
    return;

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


  //remove / from IDs
  char *pos = strchr(fltID, '/');
  if(pos != NULL)
    pos[0] = char(0);

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    tzMsg);
  }


  A.lat = lat; 
  A.lon = lon;
  A.ground_speed = speed;
  sprintf(A.callsign,"%s", fltID);

  if (altitude < 0.0){
    A.alt = AC_DATA_UNKNOWN_VALUE;
  } else {
    //
    // Convert from flight level (hundreds of feet) to ft.
    //
    A.alt = altitude*100.0;
  }

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

  A.client_data_type = 0;

  _ncdfBufferCount++;
  _ncdfBuffer.push_back(A);
  _ncdfTimeBuf.push_back(_T.unix_time);

  return;
}

// FZ - Flight Plan Information Message 
// Provides flight plan data for eligible flight plans.
//
// FZ Field Format:
//    FZ Flight_ID Aircraft Speed CoordinationFix CoordinationTime Altitude Route
//    FZ La(a){5}(/dda) (da|a)/aa(a)(a)(/L) dd(d)(d)|Lddd|SC aa(a)(a)(a)(/)(a){6} Ldddd (d)dd|(d)ddB(d)dd variable
void asdi2netcdf::_parseFZ(char *fzMsg, date_time_t T)
{

  return;
}

void asdi2netcdf::_parseAF(char *afMsg, date_time_t T)
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
  int airSpeed = AC_DATA_UNKNOWN_VALUE;
  int fixTime = AC_DATA_UNKNOWN_VALUE;
  float altitude = AC_DATA_UNKNOWN_VALUE;
  ac_data_alt_type_t alt_type = ALT_NORMAL;


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


  return;
}

// UZ - ARTCC boundary crossing Message 
// Sent to provide current flight plan information on active
// eligible flights that enter an ARTCC.
//
// UZ Field Format:
//    UZ Flight_ID Aircraft Speed Boundry_rossing CrossingTime Altitude Route
//    UZ La(a){5}(/dda) (da|a)/aa(a)(a)(/L) dd(d)(d)|Lddd|SC ddd(L)/(d)dddd(L) Edddd (d)dd|(d)ddB(d)dd variable
void asdi2netcdf::_parseUZ(char *uzMsg, date_time_t T)
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

  int airSpeed = AC_DATA_UNKNOWN_VALUE;
  if(speedStr[0] == 'M')
    airSpeed = (atoi(speedStr+1) / 100) * MACH1;
  else
    airSpeed = atoi(speedStr);

  double altitude = AC_DATA_UNKNOWN_VALUE;
  altitude = atof(altStr);

  if (altitude != AC_DATA_UNKNOWN_VALUE)
    // Convert from flight level (hundreds of feet) to ft.
    altitude = altitude*100.0;

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
  

  return;
}

// AZ - Arrival Message 
// Arrival data for all eligible arriving flights.
//
// AZ Field Format:
//    AZ Flight_ID Departure Destination ArrivalTime
//    AZ La(a){5}(/dda) aa(a){10} aa(a){10} (L)dddd
void asdi2netcdf::_parseAZ(char *azMsg, date_time_t T)
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

  //remove / from IDs
  char *pos = strchr(fltID, '/');
  if(pos != NULL)
    pos[0] = char(0);

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    azMsg);
  }

  ac_data_t A;
  memset(&A, 0, sizeof(A));

  strncpy(A.callsign, fltID, AC_DATA_CALLSIGN_LEN);
  strncpy(A.origin, deptStr, AC_DATA_AIRPORT_LEN);
  strncpy(A.destination, destStr, AC_DATA_AIRPORT_LEN);
  strncpy((char *)&(A.spare[0]), arrTimeStr, 5);

  A.client_data_type = 2;

  _ncdfBufferCount++;
  _ncdfBuffer.push_back(A);
  _ncdfTimeBuf.push_back(_T.unix_time);

  return;
}

// DZ - Departure Message 
// Transmitted for all eligible initially activated flight plans when
//  the activation message is not from an adjacent NAS.
//
// DZ Field Format:
//    DZ Flight_ID Aircraft Departure Departure_Time Destination EstArrival
//    DZ La(a){5}(/dda) (da|a)/aa(a)(a)(/L) aa(a){10} Ldddd aa(a){10} dddd
void asdi2netcdf::_parseDZ(char *dzMsg, date_time_t T)
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
    gcvt (AC_DATA_UNKNOWN_VALUE, 3, arrTimeStr);
  else {
    p = dzMsg + offset;
    _extractString(p, arrTimeStr);
  }

  //remove / from IDs
  char *pos = strchr(fltID, '/');
  if(pos != NULL)
    pos[0] = char(0);

  if (_params->debug) {
    fprintf(stderr,"%02d/%02d/%d %02d:%02d:%02d %s\n", 
	    T.month,T.day,T.year,T.hour,T.min,T.sec,
	    dzMsg);
  }

  ac_data_t A;
  memset(&A, 0, sizeof(A));

  strncpy(A.callsign, fltID, AC_DATA_CALLSIGN_LEN);
  strncpy(A.origin, deptStr, AC_DATA_AIRPORT_LEN);
  strncpy(A.destination, destStr, AC_DATA_AIRPORT_LEN);
  strncpy((char *)&(A.spare[0]), arrTimeStr, 5);

  A.client_data_type = 1;

  _ncdfBufferCount++;
  _ncdfBuffer.push_back(A);
  _ncdfTimeBuf.push_back(_T.unix_time);

  return;
}

// RZ - Cancellation Message 
// Cancelation of flight for all eligible flight plans.
//
// RZ Field Format:
//    RZ Flight_ID Departure Destination
//    RZ La(a){5}(/dda) aa(a){10} aa(a){10}
void asdi2netcdf::_parseRZ(char *rzMsg, date_time_t T)
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

  return;
}

// TO - Oceanic Position
// Provides sparse position reports while outside radar tracking via ARINC.
//
// TO Field Format:
//    TO Flight_ID Speed Time1 Altitude1 Position1 (Time2 Altitude2 Position2 )(Time3 Altitude3 Position3 )Departure Destination
//    TO La(a){5} ddd dd/dddd ddd ddddL/dddddL (dd/dddd ddd ddddL/dddddL){2} LLLL|- LLLL|-
void asdi2netcdf::_parseTO(char *toMsg, date_time_t T)
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

  offset += strlen(locStr) + 1;
  if (offset >= strlen(toMsg)) return;
  p = toMsg + offset;
  offset += strlen(p) - 10;
  p = toMsg + offset;
  char deptStr[_internalStringLen];
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

  //remove / from IDs
  char *pos = strchr(fltID, '/');
  if(pos != NULL)
    pos[0] = char(0);

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

  A.lat = lat; 
  A.lon = lon;
  A.ground_speed = speed;
  strncpy(A.callsign, fltID, AC_DATA_CALLSIGN_LEN);
  strncpy(A.origin, deptStr, AC_DATA_AIRPORT_LEN);
  strncpy(A.destination, destStr, AC_DATA_AIRPORT_LEN);

  //
  // Convert from flight level (hundreds of feet) to ft.
  //
  A.alt = atof(altStr)*100.0;
  A.alt_type = ALT_NORMAL;

  A.client_data_type = 3;

  _ncdfBufferCount++;
  _ncdfBuffer.push_back(A);
  _ncdfTimeBuf.push_back(_T.unix_time);

  return;
}

void asdi2netcdf::_getTime(date_time_t *T, 
			 int A_day, int A_hour, 
			 int A_min, int A_sec){
  //
  // If the date was determined from the file name
  // then use that first.
  //
  if(_inputFileYear > 0) 
  {
    T->year = _inputFileYear;
    T->month =  _inputFileMonth;
    //
    // Hack to handle msg in this file that belong in the previous day
    if(_inputFileDay != A_day && A_hour == 23) {
      T->day = _inputFileDay; T->hour = 0; T->min = 0; T->sec = 0;
      uconvert_to_utime( T );
      T->unix_time -= 1;
      uconvert_from_utime( T );
      if(T->day != A_day)
	fprintf(stderr,"ERROR: Msg date does not match file date: %d %d:%d:%d\n",
		A_day, A_hour, A_min, A_sec);
    }

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


void asdi2netcdf::_save2ascii( char *asdiMsg){

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
  _flushAscii();

  sprintf(_asciiBuffer,"%s\n", asdiMsg);
  return;

}


/////////////////////////////////////
//
// Small method to extract a space-delimited substring
//
void asdi2netcdf::_extractString(char *instring, char *outstring){

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
int asdi2netcdf::_openReadSource(Params *P, char *FileName){
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
void asdi2netcdf::_closeReadSource(){
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
int asdi2netcdf::_readFromSource(char *buffer, int numbytes){

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


void asdi2netcdf::_flushAscii()
{

  if (_params->saveRawASCII || _params->saveParsedASCII)
  {

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


void asdi2netcdf::_dumpNetCDF()
{
  time_t fileEndT = _fileStartT + (_params->minsWrite * 60) - 1;
  date_time_t fileT;
  fileT.unix_time = _fileStartT;

  uconvert_from_utime(&fileT);

  if(_ncdfBuffer.size() == 0) {
    _fileStartT = 0;
    _ncdfBuffer.clear();
    _ncdfTimeBuf.clear();
    return;
  }

  //
  // Make the output directory.
  //
  char outdirname[MAX_PATH_LEN];
  sprintf(outdirname,"%s/%d%02d%02d",
	  _params->OutNCDFdir,
	  fileT.year, fileT.month, fileT.day);
  
  if (ta_makedir_recurse(outdirname)){
    fprintf(stderr,"ERROR: Failed to make directory %s\n",
	    outdirname);
    exit(-1);
  }
  
  char ext[6] = "nc";
  
  //
  // Create the file name.
  //
  char outfilename[MAX_PATH_LEN];
  sprintf(outfilename,"%s/ASDI.%d%02d%02d_%02d%02d.%s", outdirname, fileT.year, fileT.month,
	  fileT.day, fileT.hour, fileT.min, ext);

  //
  // Create and Set up the new file
  //
  Nc3File *ncFile = new Nc3File( outfilename, Nc3File::Replace );
  if( !ncFile || !ncFile->is_valid() ) {
    fprintf(stderr, "ERROR: Could not create file '%s' for writing\n", outfilename);
    exit(-1);
  }


  Nc3Dim *recDim = ncFile->add_dim("records");
    
  Nc3Dim *fltDim = ncFile->add_dim("callsign_len", AC_DATA_CALLSIGN_LEN);

  Nc3Dim *aptDim = ncFile->add_dim("airport_len", AC_DATA_AIRPORT_LEN);

  ncFile->set_fill(Nc3File::Fill);

  Nc3Var *startVar = ncFile->add_var("start_time", nc3Double);

  Nc3Var *endVar = ncFile->add_var("end_time", nc3Double);

  Nc3Var *fltVar = ncFile->add_var("callsign", nc3Char, recDim, fltDim);

  Nc3Var *typeVar = ncFile->add_var("msg_type", nc3Short, recDim);

  Nc3Var *timeVar = ncFile->add_var("msg_time", nc3Double, recDim);

  Nc3Var *latVar = ncFile->add_var("latitude", nc3Float, recDim);

  Nc3Var *lonVar  = ncFile->add_var("longitude", nc3Float, recDim);

  Nc3Var *altVar  = ncFile->add_var("altitude", nc3Float, recDim);

  Nc3Var *altTypeVar  = ncFile->add_var("alt_type", nc3Short, recDim);

  Nc3Var *speedVar  = ncFile->add_var("ground_speed", nc3Float, recDim);

  Nc3Var *deptVar = ncFile->add_var("origin", nc3Char, recDim, aptDim);  

  Nc3Var *destVar = ncFile->add_var("destination", nc3Char, recDim, aptDim);  

  Nc3Var *arrTimeVar = ncFile->add_var("arrival_time", nc3Char, recDim, fltDim);  

  ncFile->add_att("Conventions", "CF-1.5");
  ncFile->add_att("title", "Aircraft Position Data");
  ncFile->add_att("source", "Aircraft Situation Display To Industry (ASDI)");
  ncFile->add_att("institution", "National Center For Atmospheric Research, Research Applications Lab (NCAR-RAL)");

  float range[2];
  range[0] = -90.;
  range[1] = 90.;
  latVar->add_att("long_name", "Observation latitude");
  latVar->add_att("standard_name", "latitude");
  latVar->add_att("valid_range", 2, range);
  latVar->add_att("units", "degrees_north");
  latVar->add_att("_FillValue", (float)AC_DATA_UNKNOWN_VALUE);
  range[0] = -180.;
  range[1] = 180.;
  lonVar->add_att("long_name", "Observation longitude");
  lonVar->add_att("standard_name", "longitude");
  lonVar->add_att("valid_range", 2, range);
  lonVar->add_att("units", "degrees_east");
  lonVar->add_att("_FillValue", (float)AC_DATA_UNKNOWN_VALUE);
  range[0] = 0.;
  range[1] = 60000.;
  altVar->add_att("long_name", "Altitude");
  altVar->add_att("units", "ft");
  altVar->add_att("positive", "up");
  altVar->add_att("valid_range", 2, range);
  altVar->add_att("_FillValue", (float)AC_DATA_UNKNOWN_VALUE);
  range[0] = 0.;
  range[1] = 1000.;
  speedVar->add_att("long_name", "Ground Speed");
  speedVar->add_att("standard_name", "platform_speed_wrt_ground");
  speedVar->add_att("valid_range", 2, range);
  speedVar->add_att("units", "mi/hr");
  speedVar->add_att("_FillValue", (float)AC_DATA_UNKNOWN_VALUE);

  timeVar->add_att("long_name", "reference time");
  timeVar->add_att("units", "seconds since 1970-1-1");

  startVar->add_att("long_name", "file start time");
  startVar->add_att("units", "seconds since 1970-1-1");
  endVar->add_att("long_name", "file end time");
  endVar->add_att("units", "seconds since 1970-1-1");

  fltVar->add_att("long_name", "Airplane CallSign");

  altTypeVar->add_att("Types", "0 = Normal, 1 = VFR on top, 2 = Interim alt, 3 = Average, 4 = Transponder");
  typeVar->add_att("Types", "0 = Track, 1 = Departure, 2 = Arrival, 3 = Oceanic Track");

  deptVar->add_att("long_name", "Departure Airport Code");
  destVar->add_att("long_name", "Destination Airport Code");

  arrTimeVar->add_att("long_name", "Local Arrival Time Estimate");
  arrTimeVar->add_att("units", "HHMM");

  // End NetCDF Define Mode

  startVar->put(&_fileStartT, 1);

  endVar->put(&fileEndT, 1);

  int num_recs = _ncdfBuffer.size();

  for(int a = 0; a < num_recs; a++)
  {
    fltVar->set_rec(a);
    fltVar->put_rec(_ncdfBuffer[a].callsign);
    typeVar->set_rec(a);
    typeVar->put_rec((const int*)&(_ncdfBuffer[a].client_data_type));
    timeVar->set_rec(a);
    timeVar->put_rec(&(_ncdfTimeBuf[a]));

    if(_ncdfBuffer[a].client_data_type == 0 || _ncdfBuffer[a].client_data_type == 3)
    {
      latVar->set_rec(a);
      latVar->put_rec(&(_ncdfBuffer[a].lat));
      lonVar->set_rec(a);
      lonVar->put_rec(&(_ncdfBuffer[a].lon));
      altVar->set_rec(a);
      altVar->put_rec(&(_ncdfBuffer[a].alt));
      altTypeVar->set_rec(a);
      altTypeVar->put_rec((const int*)&(_ncdfBuffer[a].alt_type));
      speedVar->set_rec(a);
      speedVar->put_rec(&(_ncdfBuffer[a].ground_speed));
    } 
    if(_ncdfBuffer[a].client_data_type > 0)
    {
      deptVar->set_rec(a);
      deptVar->put_rec(_ncdfBuffer[a].origin);
      destVar->set_rec(a);
      destVar->put_rec(_ncdfBuffer[a].destination);
      if(_ncdfBuffer[a].client_data_type != 3) 
      {
	arrTimeVar->set_rec(a);
	arrTimeVar->put_rec((char *)&(_ncdfBuffer[a].spare[0]) );
      } 
    }

  }

  delete ncFile;

  fprintf(stdout, "Wrote out file %s\n", outfilename);

  _fileStartT += _params->minsWrite * 60;
  _ncdfBuffer.clear();
  _ncdfTimeBuf.clear();

}
