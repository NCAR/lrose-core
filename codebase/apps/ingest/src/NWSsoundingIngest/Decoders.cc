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
#include "Params.hh"
#include "Decoders.hh"
#include "SoundingMerge.hh"

#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <physics/physics.h>
#include <physics/thermo.h>
#include <toolsa/umisc.h>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <didss/DataFileNames.hh>
using namespace std;

const double Decoders::_badVal = -9999.0;

//
// Constructor tries to determine the year, month and day
// for the data - first from the file name, failing that the
// ctime of the file is used.
//
// Also resets internal variables.
//
Decoders::Decoders(const Params *params,
                   time_t fileTime, const char *fileName) :
  _params(params)

{

  //
  reset();
  _month = -1;
  _year = -1;

  //
  // Set source to NULL by default.
  //
  _source = NULL;

  // set the year and month

  time_t dtime;
  bool dateOnly;
  if (DataFileNames::getDataTime(fileName, dtime, dateOnly) == 0) {

    DateTime dataTime(dtime);
    _year = dataTime.getYear();
    _month = dataTime.getMonth();
    
  } else {
    
    _setYearMonthFromFilename(fileTime, fileName);
    
  }

  if (_params->debug) {
    cerr << "Got year, month from file name: "
         << _year << ", " << _month << endl;
  }

}

///////////////////////////////////////////////////////////////////////
// set the year and month from the file name

void Decoders::_setYearMonthFromFilename(time_t fileTime, const char *fileName)

{

  //
  // Find the last "/" in the fileName - if there
  // is no "/" just start at the beginning of the string.
  //
  if (
      (fileName != NULL) &&
      (strlen(fileName) >= strlen("YYYYMM"))
       ){
    const char *p;
    if (strstr(fileName,"/") == NULL){ // No slash.
      p = fileName; // Start at beginning.
    } else {
      //
      // There are "/" characters in fileName, find the last one.
      //
      p = fileName + strlen(fileName);
      do {
	p--;
	if (*p == '/') break;
      }while(p != fileName );
      if (*p == '/') p++;
    }
    //
    // OK - have now positioned ourselves to parse out the
    // year and month.
    //
    int year, month;
    //
    //
    if (2==sscanf(p,"%4d%2d", &year, &month)){
      if (
	  (year > 1000) &&
	  ( year < 3000) &&
	  (month > 0) &&
	  (month < 13)
	  ) {
	_year = year;
	_month = month;
      }
    } else {
      //
      // Check if we have the upp.YYYYMM.... format.
      //
      if (NULL != strstr(p,".")){ // See if there is a dot in the filename
	const char *c = strstr(p,"."); c++; // If so, go to it and parse from there
	//
	if (2==sscanf(c,"%4d%2d", &year, &month)){
	  if (
	      (year > 1000) &&
	      ( year < 3000) &&
	      (month > 0) &&
	      (month < 13)
	      ) {
	    _year = year;
	    _month = month;
	  }
	}
      }
    }
  }
  //  
  //
  if (
      (_year == -1) ||
      (_month == -1)
      ){ 
    //
    // Failed to get year, time from filename, use input 
    // file ctime instead. This is a little unusual so print
    // a message to this effect.
    //
    date_time_t T;
    T.unix_time = fileTime;
    uconvert_from_utime( &T );
    _year = T.year; _month = T.month;
    fprintf(stderr,"NOTE : Taking data year, month from file time.\n");
  }

}

///////////////////////////////////////////
//
// Return to initial state.
//
void Decoders::reset(){

  _hour =-1;
  _day = -1;
  _unitsKnots = -1; // 0 = m/s, 1 = knots, -1 = unkown.
  _windPressure = -1;
  _numPoints = 0;
  _stationID = -1;
  _lat = _badVal;
  _lon = _badVal;
  //
  // Set alt to 0.0 not _badVal since it may be used if the 
  // station is not located and we have decided to process
  // anyway.
  //
  _surfPres = 1013.2;
  _presScale = 1.0;
  _alt = 0.0;
  //
  //
  //
  _dataTime = 0;
  //
  // Set all the arrays to bad.
  //
  for (int i=0; i < _maxNumPoints; i++){
   _pressure[i] = _badVal;   _temp[i] = _badVal;
   _rh[i] = _badVal;   _u[i] = _badVal;
   _v[i] = _badVal;   _h[i] = _badVal;   _dp[i] = _badVal;
  }


}
//////////////////////////////////////////////////////
//
// Destructor - Frees source string if it was assigned.
//
Decoders::~Decoders(){

}

//////////////////////////////////////////////////////
//
// Routine to see if we have any sounding points.
//
int Decoders::gotData(){
  return _numPoints;
}

/////////////////////////////////////////////////////////
//
// Routines for decoding the various message types.
//
int Decoders::TTAA_decode(string code, string msg)
{
  
  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);
  const char *p = strtok(copy," "); // Skip over TTAA identifier.

  //
  // Decode the day, hour and the wind pressure.
  //
  p = strtok(NULL," ");
  if (p == NULL) return 0;
  
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  _decodeWindPressureHundreds(p); // Not fatal if this fails.
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Decode the first triple of P,T,Dp,u,v.
  // Special case since no height info (I assume
  // this means it is at the surface).
  //
  p = strtok(NULL," ");
  if (p == NULL)    return 0;
  if (_decodeIsEndMessage( p )) return 0;

  double Pr,T,Dp,u,v, ht;

  Pr = _decode_AA_99Pressure( p );
  //
  // Dew point and temperature.
  //
  p = strtok(NULL," ");
  if (p == NULL)    return 0;

  T = _decode_AA_Temp( p );
  Dp = _decode_AA_DewPoint( p );

  p = strtok(NULL," ");
  if (p == NULL)    return 0;

  _decode_AA_Wind(p, &u, &v );
  //
  // Store the correction from the surface pressure.
  // Not actually used.
  //
  _surfPres = Pr;
  _presScale = _surfPres / PHYmeters2mb( _alt );
  //
  // Put the station level data in the array.
  //
  _pressure[_numPoints] = Pr;
  _temp[_numPoints] = T;
  _h[_numPoints] = _alt; // Surface altitude.
  _u[_numPoints] = u;
  _v[_numPoints] = v;
  _dp[_numPoints] = Dp;

  if (
      (T == _badVal) ||
      (Dp == _badVal)
      ){
    _rh[_numPoints] = _badVal;
  } else {
    _rh[_numPoints] = 100.0*PHYhumidity(273.0 + T, 273.0 + T - Dp);
  }
  _numPoints++;
  //
  // Cycle through the triples of above-ground data.
  //
  do{

    //
    // Pressure and height.
    //
    p = strtok(NULL," ");
    if (p == NULL) break;            // Break out of loop - at end.
    if (!(strncmp("88",p,2))) break; // Another end condition.
    if (_decodeIsEndMessage( p )) break; // End of this section.
    //
    _decode_AA_PPhhh(p,  &Pr, &ht );
    //
    // Exit if we get a bad pressure.
    // Stuff trailing this is local and eyeballing it it generally
    // does not seem that useful.
    //
    if (Pr == _badVal) break; 

    //
    // Dew point and temperature.
    //
    p = strtok(NULL," ");
    if (p == NULL)  break;
    if (!(strncmp("88",p,2))) break; // Another end condition.
    if (_decodeIsEndMessage( p )) break; // End of this section.
    
    T = _decode_AA_Temp( p );
    Dp = _decode_AA_DewPoint( p );
    
    p = strtok(NULL," ");
    if (p == NULL)   break;
    if (!(strncmp("88",p,2))) break; // Another end condition.
    if (_decodeIsEndMessage( p )) break; // End of this section.
    
    _decode_AA_Wind(p, &u, &v );
    //
    // Fill arrays, if we have any data.
    // Data for points below the terrain are set to missing.
    //
    if (
	(u != _badVal) &&
	(v != _badVal) &&
	(T != _badVal) &&
	(Dp != _badVal)
	){
      _pressure[_numPoints] = Pr;
      _temp[_numPoints] = T;
      _h[_numPoints] = ht;
      _u[_numPoints] = u;
      _v[_numPoints] = v;
      _dp[_numPoints] = Dp;
      //
      if (
	  (T == _badVal) ||
	  (Dp == _badVal)
	  ){
	_rh[_numPoints] = _badVal;
      } else {
	_rh[_numPoints] = 100*PHYhumidity(273.0 + T, 273.0 + T -Dp);
      }
      _numPoints++;
      if (_numPoints == _maxNumPoints){   // Highly unlikely.
	fprintf(stderr,"Maximum number of points exceeded!\n");
	break;
      }
    } // End of IF we have any valid data.
  } while( 1 ); // Break statements above exit loop.

  return 1;  // Did OK.

}

//////////////////////////////////////////////////////////////////
//
// Decoder for TTBB line.
//
int Decoders::TTBB_decode(string code, string msg){ 

  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);
  const char *p = strtok(copy," "); // Skip over TTAA identifier.

  //
  // Decode the day, hour and the wind.
  //
  p = strtok(NULL," ");
  if (p==NULL) return 0;
 
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  //
  // Get the source string.
  //
  _decode_BB_Source( p );
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Start decoding pressure / temp_dewPoint pairs. Exit
  // at the end, or when the 21212 or 31313 codes
  // are met (end messages).
  //
  do {
    p = strtok(NULL," ");
    if (p==NULL) break;
    if (_decodeIsEndMessage( p )) break;
    //
    // Assume it's a pressure/ temp_dewPoint pair.
    //    
    double Pr =  _decode_BB_nnPPP( p );

    p = strtok(NULL," ");
    if (p==NULL) break;
    if (_decodeIsEndMessage( p )) break;

    double T = _decode_AA_Temp( p );
    double Dp = _decode_AA_DewPoint( p );
    double rh;
    if (
	(T == _badVal) ||
	(Dp == _badVal)
	){
      rh = _badVal;
    } else {
      rh = 100*PHYhumidity(273.0 + T, 273.0 + T -Dp); 
    }
    //
    // Only write it to the array if the pressure was correctly decoded.
    //
    if (Pr != _badVal){
      _pressure[_numPoints] = Pr;
      _temp[_numPoints] = T;
      _h[_numPoints] = PHYmb2meters( Pr ); // All we can do I guess
      
      _dp[_numPoints] = Dp;
      _rh[_numPoints] = rh;
      
      _numPoints++;
      if (_numPoints == _maxNumPoints){   // Highly unlikely.
	fprintf(stderr,"Maximum number of points exceeded!\n");
	break;
      }
    }
  } while (1); // break statements in loop cause exit.
  //
  //
  ////////////////////////////////////////////////////
  //
  // See if we have a trailing 21212 section to decode (often we don't).
  //
  if (
      (p != NULL) &&
      (strlen(p) > 4) &&
      (!(strncmp(p,"21212",5)))
      ){
    //
    // Add this section to the end of the sounding arrays.
    //
    do {
      //
      // Assume it's a pressure/ wind_dirSpeed pair.
      //    
      p = strtok(NULL," ");
      if (p==NULL) break;

      if (_decodeIsEndMessage(p)) break; // Start of new section.

      double Pr =  _decode_BB_nnPPP( p );

      p = strtok(NULL," ");
      if (p==NULL) return 0;
      if (_decodeIsEndMessage( p )) return 0;

      double u,v;
      _decode_AA_Wind( p, &u, &v );
      //
      // I have not seen a 21212 section yet so I am
      // somewhat leary of this code. Niles.
      //
      if (Pr != _badVal){
	_u[_numPoints] = u;
	_v[_numPoints] = v;
	_pressure[_numPoints]=Pr;
	_h[_numPoints] = PHYmb2meters( Pr ); 
	
	_numPoints++;
      }
    } while(1);
  } // End of IF we have a 21212 section.
  return 1;  // Did OK
}
////////////////////////////////////////
//
// Decode TTCC messages. Similar to TTAA message but
// no surface station information and some pressures in tenths rather
// than units.
//
int Decoders::TTCC_decode(string code, string msg){ 

  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);

  char *p = strtok(copy," "); // Skip over TTCC identifier.
  //
  // Decode the day, hour and the wind pressure.
  //
  p = strtok(NULL," ");
  if (p == NULL) return 0;

  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  _decodeWindPressureTens(p); // Not fatal if this fails.

  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  double Pr,T,Dp,u,v, ht;
  //
  // Cycle through the triples of above-ground data.
  //
  do{
    //
    // Pressure and height.
    //
    p = strtok(NULL," ");
    if (p == NULL) break;            // Break out of loop - at end.
    if (!(strncmp("88",p,2))) break; // Another end condition.
    if (_decodeIsEndMessage( p )) break; // End of this section.
    //
    _decode_CC_PPhhh(p,  &Pr, &ht );
    //
    // Exit if we get a bad pressure.
    // Stuff trailing this is local and eyeballing it it generally
    // does not seem that useful.
    //
    if (Pr == _badVal) break; 
    //
    // Dew point and temperature.
    //
    p = strtok(NULL," ");
    if (p == NULL)    break;
    if (_decodeIsEndMessage( p )) break;

    T = _decode_AA_Temp( p );
    Dp = _decode_AA_DewPoint( p );
    
    p = strtok(NULL," ");
    if (p == NULL)    return 0;
    
    _decode_AA_Wind(p, &u, &v );
    //
    // Fill arrays.
    //
    _pressure[_numPoints] = Pr;
    _temp[_numPoints] = T;
    _h[_numPoints] = ht;
    _u[_numPoints] = u;
    _v[_numPoints] = v;
    _dp[_numPoints] = Dp;
    //
    if (
	(T == _badVal) ||
	(Dp == _badVal)
	){
      _rh[_numPoints] = _badVal;
    } else {
      _rh[_numPoints] = 100*PHYhumidity(273.0 + T, 273.0 + T -Dp);
    }
    _numPoints++;
    if (_numPoints == _maxNumPoints){   // Highly unlikely.
      fprintf(stderr,"Maximum number of points exceeded!\n");
      break;
    }
  } while( 1 ); // Break statements above exit loop.

  return 1;  // Did OK.
  
}
///////////////////////////////////////////////
//
// TTDD Decoder. Similar to TTBB decoder but pressures are in tenths
// of mb, not whole mb.
//
int Decoders::TTDD_decode(string code, string msg){ 

  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);

  char *p = strtok(copy," "); // Skip over TTBB identifier.
  //
  // Decode the day, hour and the wind.
  //
  p = strtok(NULL," ");
  if (p==NULL) return 0;
 
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  //
  // Get the source string.
  //
  _decode_BB_Source( p );
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Start decoding pressure / temp_dewPoint pairs. Exit
  // at the end, or when the 21212 or 31313 codes
  // are met (end messages).
  //
  do {
    p = strtok(NULL," ");
    if (p==NULL) break;
    if (_decodeIsEndMessage( p )) break;
    //
    // Assume it's a pressure/ temp_dewPoint pair.
    //    
    double Pr =  _decode_BB_nnPPP( p );
    if (Pr != _badVal) Pr = Pr / 10.0; // Only difference from TTBB decoder.
    //
    p = strtok(NULL," ");
    if (p==NULL) break;
    if (_decodeIsEndMessage( p )) break;

    double T = _decode_AA_Temp( p );
    double Dp = _decode_AA_DewPoint( p );
    double rh;
    if (
	(T == _badVal) ||
	(Dp == _badVal)
	){
      rh = _badVal;
    } else {
      rh = 100*PHYhumidity(273.0 + T, 273.0 + T -Dp); 
    }
    //
    // Only write it to the array if the pressure was correctly decoded.
    //
    if (Pr != _badVal){
      _pressure[_numPoints] = Pr;
      _temp[_numPoints] = T;
      _h[_numPoints] = PHYmb2meters( Pr ); // All we can do I guess
      
      _dp[_numPoints] = Dp;
      _rh[_numPoints] = rh;
      
      _numPoints++;
      if (_numPoints == _maxNumPoints){   // Highly unlikely.
	fprintf(stderr,"Maximum number of points exceeded!\n");
	break;
      }
    }
  } while (1); // break statements in loop cause exit.
  //
  //
  ////////////////////////////////////////////////////
  //
  // See if we have a trailing 21212 section to decode (often we don't).
  //
  if (
      (p != NULL) &&
      (strlen(p) > 4) &&
      (!(strncmp(p,"21212",5)))
      ){
    //
    // Add this section to the end of the sounding arrays.
    //
    do {
      //
      // Assume it's a pressure/ wind_dirSpeed pair.
      //    
      p = strtok(NULL," ");
      if (p==NULL) break;

      if (_decodeIsEndMessage(p)) break; // Start of new section.

      double Pr =  _decode_BB_nnPPP( p );
      if (Pr != _badVal) Pr = Pr / 10.0; // Only difference from TTBB decoder.
      p = strtok(NULL," ");
      if (p==NULL) return 0;
      double u,v;
      _decode_AA_Wind( p, &u, &v );
      //
      // I have not seen a 21212 section yet so I am
      // somewhat leary of this code. Niles.
      //
      if (Pr != _badVal){
	_u[_numPoints] = u;
	_v[_numPoints] = v;
	_pressure[_numPoints]=Pr;
	_h[_numPoints] = PHYmb2meters( Pr ); 
	
	_numPoints++;
      }
    } while(1);
  } // End of IF we have a 21212 section.
  return 1;  // Did OK

}
////////////////////////////////////////////////////////////
//
// PPAA decoder.
//
int Decoders::PPAA_decode(string code, string msg){ 

  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);

  char *p = strtok(copy," "); // Skip over PPAA identifier.
  //
  // Decode the day, hour and the wind.
  //
  p = strtok(NULL," ");
  if (p==NULL) return 0;
 
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  //
  // Get the source string.
  //
  _decode_BB_Source( p );
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Decode the 44nPP or 55nPP messages.
  // 
  do {
    p = strtok(NULL," ");
    if (p==NULL) break;

    if (_decodeIsEndMessage( p )) break;
 
    int n; double Pr;
    _decode_AA_44nPP(p, &n, &Pr );
    if (n==0) break; // Did not decode, was not 44nPP or 55nPP message

    for(int k=0; k<n; k++){
      p = strtok(NULL," ");
      if (p==NULL) break;

      double u,v;
      _decode_AA_Wind(p, &u, &v );

      _u[_numPoints]=u;  _v[_numPoints]=v;
      _pressure[_numPoints] = Pr;
      _h[_numPoints] = PHYmb2meters( Pr );
      
      _numPoints++;
      //
      // Move on to the next assigned pressure.
      //
      Pr = _decode_GetNext_AA_Pressure( Pr );
    }

  } while(1); // break statements exit loop.
  //
  // See if we have a 77PPP, 66PPP group.
  //
  if (
      (NULL == p) ||
      (strlen(p) < 5) ||
      (!(strncmp(p,"77999",5)))
      ){
    return 1; // Nothing there, but we decoded last section OK.
  }

  if (
      (!(strncmp(p,"77",2))) ||
      (!(strncmp(p,"66",2)))
      ){
    //
    // Have a 77PPP, 66PPP group. Decode the pressure
    // and the maximum wind. Just decode the pressure right here.
    //
    work[0]=p[2]; work[1]=p[3]; work[2]=p[4]; work[3]=char(0);
    if (strstr(work,"/")!=NULL) return 1;

    double Pr = double(atoi(work));
    //
    // Get the wind message.
    //
    p = strtok(NULL," ");
    if (p==NULL) return 1;
    double u,v;
    _decode_AA_Wind(p, &u, &v );
    //
    _u[_numPoints]=u;  _v[_numPoints]=v;
    _pressure[_numPoints] = Pr;
    _h[_numPoints] = PHYmb2meters( Pr );
    
    _numPoints++;
  }
  //
  // See if we have a 6HHHH or 7HHHH group.
  //
  if (
      (!(strncmp(p,"7",1))) ||
      (!(strncmp(p,"6",1)))
      ){
    //
    // Have a 7HHHH, 6HHHH group. Decode the height
    // and the maximum wind. Just decode the height right here.
    //
    work[0]=p[1]; work[1]=p[2]; 
    work[2]=p[3]; work[3]=p[4];
    work[4]=char(0);

    if (strstr(work,"/")!=NULL) return 1;
    double h = double(atoi(work)) * 10.0; // Tens of meters.
    //
    // Get the wind message.
    //
    p = strtok(NULL," ");
    if (p==NULL) return 1;
    double u,v;
    _decode_AA_Wind(p, &u, &v );
    //
    _u[_numPoints]=u;  _v[_numPoints]=v;
    //    _pressure[_numPoints] = PHYmeters2mb(h);
    _h[_numPoints] = h;
    _numPoints++;
  }

  return 1;
}
//////////////////////////////////////////////////////////
//
// PPBB decoder.
//
int Decoders::PPBB_decode(string code, string msg){ 

  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);

  char *p = strtok(copy," "); // Skip over PPBB identifier.
  //
  // Decode the day, hour and the wind.
  //
  p = strtok(NULL," ");
  if (p==NULL) return 0;
 
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  //
  // Get the source string.
  //
  _decode_BB_Source( p );
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Decode the 9TUUU triples.
  //
  do{
    p = strtok(NULL," ");
    if (p==NULL) break;

    if (_decodeIsEndMessage( p )) break;
    //
    // Get the three heights. Note that they may be _badVal
    // in which case we have no way of placing the data.
    //
    double H1,H2,H3;
    if (!(_decode_DD_ITUUU( p, &H1, &H2, &H3))) break;
    //
    // Altitude of 0.0 => surface I guess. The rest are MSL.
    //
    if (fabs(H1) < 0.1) H1 = _alt;
    if (fabs(H2) < 0.1) H2 = _alt;
    if (fabs(H3) < 0.1) H3 = _alt;
    //
    // Then, get the winds. A missing altitude seems to mean that the
    // wind data are just not there, too.
    //
    double u1, u2, u3;
    double v1, v2, v3;
    //
    // Write these to the array, if the heights are OK.
    //
    if (H1 != _badVal){
      p = strtok(NULL," ");
      if ((p==NULL) || (_decodeIsEndMessage( p ))) break;
      _decode_AA_Wind(p, &u1, &v1);
      _h[_numPoints] = H1;
      //      _pressure[_numPoints] = PHYmeters2mb( H1 );
      _u[_numPoints] = u1; _v[_numPoints] = v1; 
      _numPoints++;
    }

    if (H2 != _badVal){
      p = strtok(NULL," ");
      if ((p==NULL) || (_decodeIsEndMessage( p ))) break;
      _decode_AA_Wind(p, &u2, &v2);
      _h[_numPoints] = H2;
      //    _pressure[_numPoints] = PHYmeters2mb( H2 );
      _u[_numPoints] = u2; _v[_numPoints] = v2; 
      _numPoints++;
    }

    if (H3 != _badVal){
      p = strtok(NULL," ");
      if ((p==NULL) || (_decodeIsEndMessage( p ))) break;
      _decode_AA_Wind(p, &u3, &v3);
      _h[_numPoints] = H3;
      //     _pressure[_numPoints] = PHYmeters2mb( H3 );
      _u[_numPoints] = u3; _v[_numPoints] = v3; 
      _numPoints++;
    }
  }while(1);
  //
  // See if we have a 21212 code. If not, that's it.
  //

  if (
      (p==NULL) ||
      (strlen(p) < 5) ||
      (strncmp(p,"21212",5))
      ){
    return 1; // No 21212 section, return what we have.
  }
  //
  // We have a 21212 section if we got here - decode.
  //

  do {
    //
    // Assume it's a pressure/ wind_dirSpeed pair.
    //    
    p = strtok(NULL," ");
    if (p==NULL) break;

    if (_decodeIsEndMessage(p)) break; // Start of new section.

    double Pr =  _decode_BB_nnPPP( p );

    p = strtok(NULL," ");
    if (p==NULL) return 0;
    double u,v;
    _decode_AA_Wind( p, &u, &v );
    //
    // I have not seen a 21212 section yet so I am
    // somewhat leary of this code. I just add
    // these wind data to the end of the array. Niles.
    //
    if (Pr != _badVal){
      _u[_numPoints] = u;
      _v[_numPoints] = v;
      _pressure[_numPoints] = Pr;
      _h[_numPoints] = PHYmb2meters( Pr );
      
      _numPoints++;
    }
    //  
  } while(1);

  return 1; // Did OK.

}

//////////////////////////////////////////////////
//
// PPCC decoder. Very similar to PPAA but the pressure levels differ - just
// different enough to have its own decoder.
//
int Decoders::PPCC_decode(string code, string msg){ 

  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);

  char *p = strtok(copy," "); // Skip over PPCC identifier.
  //
  // Decode the day, hour and the wind.
  //
  p = strtok(NULL," ");
  if (p==NULL) return 0;
 
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  //
  // Get the source string.
  //
  _decode_BB_Source( p );
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Decode the 44nPP or 55nPP messages.
  // 
  do {
    p = strtok(NULL," ");
    if (p==NULL) break;

    if (_decodeIsEndMessage( p )) break;
 
    int n; double Pr;
    _decode_CC_44nPP(p, &n, &Pr );
    if (n==0) break; // Did not decode, was not 44nPP or 55nPP message

    for(int k=0; k<n; k++){
      p = strtok(NULL," ");
      if (p==NULL) break;

      double u,v;
      _decode_AA_Wind(p, &u, &v );

      _u[_numPoints]=u;  _v[_numPoints]=v;
      _pressure[_numPoints] = Pr;
      _h[_numPoints] = PHYmb2meters( Pr );
      
      _numPoints++;
      //
      // Move on to the next assigned pressure.
      //
      Pr = _decode_GetNext_CC_Pressure( Pr );
    }

  } while(1); // break statements exit loop.
  //
  // See if we have a 77PPP, 66PPP group.
  //
  if (
      (NULL == p) ||
      (strlen(p) < 5) ||
      (!(strncmp(p,"77999",5)))
      ){
    return 1; // Nothing there, but we decoded last section OK.
  }

  if (
      (!(strncmp(p,"77",2))) ||
      (!(strncmp(p,"66",2)))
      ){
    //
    // Have a 77PPP, 66PPP group. Decode the pressure
    // and the maximum wind. Just decode the pressure right here.
    //
    work[0]=p[2]; work[1]=p[3]; work[2]=p[4]; work[3]=char(0);
    if (strstr(work,"/")!=NULL) return 1;

    double Pr = double(atoi(work));
    //
    // Get the wind message.
    //
    p = strtok(NULL," ");
    if (p==NULL) return 1;
    double u,v;
    _decode_AA_Wind(p, &u, &v );
    //
    _u[_numPoints]=u;  _v[_numPoints]=v;
    _pressure[_numPoints] = Pr;
    _h[_numPoints] = PHYmb2meters( Pr );
    
    _numPoints++;
  }
  //
  // See if we have a 6HHHH or 7HHHH group.
  //
  if (
      (!(strncmp(p,"7",1))) ||
      (!(strncmp(p,"6",1)))
      ){
    //
    // Have a 7HHHH, 6HHHH group. Decode the height
    // and the maximum wind. Just decode the height right here.
    //
    work[0]=p[1]; work[1]=p[2]; 
    work[2]=p[3]; work[3]=p[4];
    work[4]=char(0);

    if (strstr(work,"/")!=NULL) return 1;
    double h = double(atoi(work)) * 10.0; // Tens of meters.
    //
    // Get the wind message.
    //
    p = strtok(NULL," ");
    if (p==NULL) return 1;
    double u,v;
    _decode_AA_Wind(p, &u, &v );
    //
    _u[_numPoints]=u;  _v[_numPoints]=v;
    //    _pressure[_numPoints] = PHYmeters2mb(h);
    _h[_numPoints] = h;
    _numPoints++;
  }

  return 1;


}
/////////////////////////////////////////////////////
//
// PPDD decoder. Just slightly different than the
// PPBB decoder.
//
int Decoders::PPDD_decode(string code, string msg){ 
  reset();

  if (_params->debug){
    fprintf(stderr, "\n\n Decoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
  }
  
  // find code in message
  size_t start = msg.find(code);
  if (start == string::npos) {
    fprintf(stderr, "ERROR ecoding code: %s\n", code.c_str());
    fprintf(stderr, "msg : %s\n", msg.c_str());
    fprintf(stderr, "Cannot find code in message\n");
    return -1;
  }

  // make copy, starting with code

  TaArray<char> copy_;
  char *copy = copy_.alloc(msg.size() + 1);
  memset(copy, 0, msg.size() + 1);
  memcpy(copy, msg.c_str() + start, msg.size() - start);

  char *p = strtok(copy," "); // Skip over PPDD identifier.
  //
  // Decode the day, hour and the wind.
  //
  p = strtok(NULL," ");
  if (p==NULL) return 0;
 
  if (!_decodeTime(p)) {
    // try advancing one token
    p = strtok(NULL," ");
    if (!_decodeTime(p)) {
      return 0;
    }
  }

  //
  // Get the source string.
  //
  _decode_BB_Source( p );
  //
  // Decode the station ID.
  //
  p = strtok(NULL," ");
  if (p != NULL){
    _stationID = atoi( p ); if (0==_stationID) _stationID = -1;
  } else {
    return 0;
  }
  //
  // See if we can locate that station.
  //
  if ((_getLocation()) && (!(_params->processIfNotLocated)))
    return 0;
  //
  // Decode the 9TUUU triples.
  //
  do{
    p = strtok(NULL," ");
    if (p==NULL) break;

    if (_decodeIsEndMessage( p )) break;
    if (p[0] == '1') break; // This code not allowed in PPDD. cf PPBB.
    //
    // Get the three heights.
    //
    double H1,H2,H3;
    if (!(_decode_DD_ITUUU( p, &H1, &H2, &H3))) break;
    //
    // Altitude of 0.0 => surface I guess. The rest are MSL.
    //
    if (fabs(H1) < 0.1) H1 = _alt;
    if (fabs(H2) < 0.1) H2 = _alt;
    if (fabs(H3) < 0.1) H3 = _alt;
    //
    // Then, get the three winds.
    //
    double u1, u2, u3;
    double v1, v2, v3;
    //
    // Write these to the array, if the heights are OK.
    //
    if (H1 != _badVal){
      p = strtok(NULL," ");
      if ((p==NULL) || (_decodeIsEndMessage( p ))) break;
      _decode_AA_Wind(p, &u1, &v1);
      _h[_numPoints] = H1;
      // _pressure[_numPoints] = PHYmeters2mb( H1 );
      _u[_numPoints] = u1; _v[_numPoints] = v1; 
      _numPoints++;
    }

    if (H2 != _badVal){
      p = strtok(NULL," ");
      if ((p==NULL) || (_decodeIsEndMessage( p ))) break;
      _decode_AA_Wind(p, &u2, &v2);
      _h[_numPoints] = H2;
      // _pressure[_numPoints] = PHYmeters2mb( H2 );
      _u[_numPoints] = u2; _v[_numPoints] = v2; 
      _numPoints++;
    }

    if (H3 != _badVal){
      p = strtok(NULL," ");
      if ((p==NULL) || (_decodeIsEndMessage( p ))) break;
      _decode_AA_Wind(p, &u3, &v3);
      _h[_numPoints] = H3;
      // _pressure[_numPoints] = PHYmeters2mb( H3 );
      _u[_numPoints] = u3; _v[_numPoints] = v3; 
      _numPoints++;
    }

  }while(1);
  //
  // See if we have a 21212 code. If not, that's it.
  //

  if (
      (p==NULL) ||
      (strlen(p) < 5) ||
      (strncmp(p,"21212",5))
      ){
    return 1; // No 21212 section, return what we have.
  }
  //
  // We have a 21212 section if we got here - decode.
  //
  do {
    //
    // Assume it's a pressure/ wind_dirSpeed pair.
    //    
    p = strtok(NULL," ");
    if (p==NULL) break;

    if (_decodeIsEndMessage(p)) break; // Start of new section.

    double Pr =  _decode_BB_nnPPP( p );

    p = strtok(NULL," ");
    if (p==NULL) return 0;
    double u,v;
    _decode_AA_Wind( p, &u, &v );
    //
    // I have not seen a 21212 section yet so I am
    // somewhat leary of this code. I just add
    // these wind data to the end of the array. Niles.
    //
    if (Pr != _badVal){
      _u[_numPoints] = u;
      _v[_numPoints] = v;
      _pressure[_numPoints] = Pr;
      _h[_numPoints] = PHYmb2meters( Pr ); 
      
      _numPoints++;
    }
    //  
  } while(1);

  return 1; // Did OK.

}

//////////////////////////////////////////////////
//
// Decode the time and day of the month.
//
int Decoders::_decodeTime( const char *token ){

  if (token == NULL) return 0;

  if (strlen(token) < 5) return 0;

  //
  // Decode the two digit day of the month. This has
  // 50 added to it if the units of wind are knots,
  // otherwise they are m/s.
  //
  work[0] = token[0]; 
  work[1] = token[1]; 
  work[2] = char(0);

  if (NULL != strstr(work,"/")) return 0;

  _day = atoi(work);
  if (_day > 50) {
    _day = _day - 50;
    _unitsKnots = 1;
  } else {
    _unitsKnots = 0; // Units are m/s
  }
  //
  // Decode the two digit hour of the day, UTC.
  //
  work[0] = token[2]; 
  work[1] = token[3]; 
  work[2] = char(0);

  if (NULL != strstr(work,"/")) return 0;

  _hour = atoi(work);

  if (
      (_hour < 0) &&
      (_hour > 23) &&
      (_day < 1) &&
      (_day > 31)
      ){
    return 0; // Got weird values.
  }

  //
  // If we have a valid _year and _day, fill in
  // _dataTime
  //
  if (
      (_year != -1) &&
      (_month != -1)
      ){
    date_time_t T;
    T.year = _year; T.month = _month; T.day = _day;
    T.hour = _hour; T.min = 0; T.sec = 0;
    uconvert_to_utime( &T );
    _dataTime = T.unix_time;
  }

  return 1; // did OK

}
/////////////////////////////////////
//
// Decode the pressure level.
//
int Decoders::_decodeWindPressureHundreds( const char *token ){

  if (strlen(token) < 5) return 0;

  work[0] = token[4];
  work[1] = char(0);

  if (NULL != strstr(work,"/")) return 0;

  _windPressure = 100*atoi( work );

  if (
      (_windPressure > 0.0) &&
      (_windPressure < 2000.0)
      ){
    return 1;
  }
  return 0;

}
int Decoders::_decodeWindPressureTens( const char *token ){

  if (strlen(token) < 5) return 0;

  work[0] = token[4];
  work[1] = char(0);

  if (NULL != strstr(work,"/")) return 0;

  _windPressure = 10*atoi( work );

  if (
      (_windPressure > 0.0) &&
      (_windPressure < 2000.0)
      ){
    return 1;
  }
  return 0;
}

void Decoders::print(int printIfNoPoints /* = 0*/ ){

  if ((!(printIfNoPoints)) && (_numPoints == 0) )
    return;

  fprintf(stderr,"Station ID : ");
  if (_stationID < 0){
    fprintf(stderr,"Unknown.\n");
  } else {
    fprintf(stderr,"%d\n",_stationID);
  }
  fprintf(stderr,"Lat, Lon, Alt (%g implies unknown) : %g, %g, %g\n",
	  _badVal, _lat, _lon, _alt);

  fprintf(stderr,"Data source : ");
  if (_source == NULL){
    fprintf(stderr,"Unknown.\n");
  } else {
    fprintf(stderr,"%s\n", _source);
  }

  if (_unitsKnots == 1)
    fprintf(stderr,"Wind Units : knots\n");
   if (_unitsKnots == 0)
    fprintf(stderr,"Wind Units : m/s\n");
   if (_unitsKnots == -1)
     fprintf(stderr,"Wind Units : Unknown\n");

   fprintf(stderr,"Data time : %d/%02d/%02d %02d:%02d UTC \n",
	   _year, _month, _day, _hour, 0);

   fprintf(stderr,"Any data will be saved as being at %s\n",
	   utimstr( _dataTime ));

  fprintf(stderr,"Wind pressure : ");
  if (_windPressure < 0.0){
    fprintf(stderr,"Unknown.\n");
  } else {
    fprintf(stderr,"%f\n",_windPressure);
  }

  fprintf(stderr,"Number of points : %d\n",_numPoints);

  for (int i=0; i < _numPoints; i++){
    fprintf(stderr,"Point=%d ",i);
    fprintf(stderr,"Height=%g ",_h[i]);
    fprintf(stderr,"U=%g ",_u[i]);
    fprintf(stderr,"V=%g ",_v[i]);
    fprintf(stderr,"DPD=%g ",_dp[i]);
    fprintf(stderr,"RH=%g ",_rh[i]);
    fprintf(stderr,"TEMP=%g ",_temp[i]);
    fprintf(stderr,"PRESSURE=%g ",_pressure[i]);
    if (_pressure[i] != _badVal){
      fprintf(stderr,"PHt=%g ", PHYmb2meters(_pressure[i]));
      fprintf(stderr,"(D=%g) ", _h[i]-PHYmb2meters(_pressure[i]));
    }
    fprintf(stderr,"\n");
  }

  fprintf(stderr,"\n");
  return;

}

///////////////////////////////////////////////////
//
// Dew point decoder - returns bad if decoding fails.
//
double Decoders::_decode_AA_DewPoint(const char *TTTDD ){


  if (strlen(TTTDD) < 5) return _badVal;

  //
  // Decode dew point from the TTTDD string. This is
  // done as specified in the WMO manual on codes
  // in table 0777.
  //
  work[0] = TTTDD[3];
  work[1] = TTTDD[4];
  work[2] = char(0);

  if (NULL != strstr(work,"/")) return _badVal;

  int codeVal = atoi( work );

  if (
      (codeVal < 0) ||
      (codeVal > 99)
      ){ // Outside of range of valid codes.
    return _badVal;
  }

  if (
      (codeVal < 56) &&
      (codeVal > 50)
      ){ // 51 - 55 are outside of the range of valid codes.
    return _badVal;
  }
  //
  // OK - we are within the range of good codes
  // so return a value. According  to the table, up to
  // code 50 the physical value is the code divided by 10,
  // above 56 it is the code less 50.
  //
  if (codeVal <= 50) return double(codeVal) / 10.0;

  return codeVal - 50;

}
//////////////////////////////////////////////////
//
// Decode the temperature from the TTTDD string.
//
double Decoders::_decode_AA_Temp(const char *TTTDD ){

  if (strlen(TTTDD) < 5) return _badVal;

  //
  // Temperature coded in tenths of degrees.
  //
  work[0] = TTTDD[0];
  work[1] = TTTDD[1];
  work[2] = TTTDD[2];
  work[3] = char(0);

  if (NULL != strstr(work,"/")) return _badVal;
  //
  // Magnitude is simply a division by 10.0
  //
  double magnitude = double(atoi(work)) / 10.0;
  //
  // The sign is somewhat more tricky - if the
  // least significant digit is odd, then the temperature is negative.
  // See code table 3931 in the WMO manual of codes.
  //
  work[0] = TTTDD[2]; work[1] = char(0);
  int leastDigit = atoi( work );

  if (leastDigit & 1) {
    return -magnitude;
  } else {
    return magnitude;
  }

}
//////////////////////////////////////////////////
//
// Decode wind speed and direction from the
// ddfff string. Assumes that the YY message has
// already been decoded so we know what units
// the wind is in.
//
void Decoders::_decode_AA_Wind(const char *ddfff, double *u, double *v ){

  *u = _badVal; *v = _badVal;

  if (strlen(ddfff) < 5) return;
  if (NULL !=strstr(ddfff,"/")) return;
  //
  // Can't decode without units. Should never happen.
  //
  if (_unitsKnots == -1) return; // Can't decode without units.
                                 
  //
  // Wind direction rounded to the nearest 10 degrees.
  //
  work[0] = ddfff[0];
  work[1] = ddfff[1];
  work[2] = char(0);
  //
  double windDir = double(atoi(work)) * 10.0;
  //
  // Wind speed. If this is greater than 500 then
  // 500 is to be subtracted from it, and 5 degrees are to be added
  // to the wind direction.
  //
  work[0] = ddfff[2];
  work[1] = ddfff[3];
  work[2] = ddfff[4];
  work[3] = char(0);
  //
  double windSpeed = double(atoi(work));
  if (windSpeed > 500.0){
    windSpeed = windSpeed - 500.0;
    windDir = windDir + 5.0;
  }
  if (_unitsKnots){ // Convert to m/s
    windSpeed = windSpeed * 0.5145;
  }
  //
  // Convert to U,V
  //
  *u = PHYwind_u(windSpeed, windDir);
  *v = PHYwind_v(windSpeed, windDir);

  return;

}
//////////////////////////////////////////////
//
// Decode the 99 pressure level. Assumes the pressure is
// above 100hPa.
//
double Decoders::_decode_AA_99Pressure(const char *code_99PPP ){

  
  if (strlen(code_99PPP) < 5) return _badVal;
  if (strncmp("99",code_99PPP,2)) return _badVal;

  work[0] = code_99PPP[2];
  work[1] = code_99PPP[3];
  work[2] = code_99PPP[4];
  work[3] = char(0);
  
  double pres = atoi( work );

  if (pres < 500) pres = pres + 1000.0; // OK for surface data.

  return pres;

}
//////////////////////////////////////////////
//
// Decode a PPhhh message to give pressure in hPa
// and height in meters in the AA message.
//
void Decoders::_decode_AA_PPhhh(const char *PPhhh, double *press, double *ht ){

  *press = _badVal; *ht = _badVal;
  if (strlen(PPhhh) < 5) return;
  if (NULL != strstr(PPhhh,"/")) return;

  work[0] = PPhhh[0];
  work[1] = PPhhh[1];
  work[2]=char(0);

  int pcode = atoi( work );
  //
  // Decode the pressure from this coded integer.
  // This is done somewhat laboriously to avoid erroneous decodings.
  //
  switch (pcode ) {

  case 0 :
    *press = 1000; break;
  case 92:
    *press = 925; break;
  case 85:
    *press = 850; break;
  case 70:
    *press = 700; break;
  case 50:
    *press = 500; break;
  case 40:
    *press = 400; break;
  case 30:
    *press = 300; break;
  case 25:
    *press = 250; break;
  case 20:
    *press = 200; break;
  case 15:
    *press = 150; break;
  case 10:
    *press = 100; break;
  default : // Leave it as bad.
    break;
    
  }

  if (*press == _badVal) return; // Can't decode height without pressure.
  //
  // Decode the hhh part.
  //
  work[0] = PPhhh[2];
  work[1] = PPhhh[3];
  work[2]=  PPhhh[4];
  work[3]=char(0);

  int hcode = atoi( work );
  //
  // Supposedly, negative values have 500 added to them.
  // This seems ambiguous to me so I will ignore it. Niles.
  //
  int sign = 1;
  /*-------------------------
  if (hcode > 500) {
    hcode = hcode - 500;
    sign = -1;
  }
  ---------------------------*/
  //
  // Scaling depends on pressure.
  //
  if (*press > 500.0){
    *ht = hcode;
  } else {
    *ht = hcode * 10.0;
  }
  *ht = *ht * sign;
  //
  // Now - the real voodoo.
  //
  // The height we just got has, in fact, no
  // thousands or tens of thousands digits. What we
  // do have is an accurate pressure. So, what I think we have
  // to do is to cycle through the thousands up to 25,000m
  // and use the height that most closely matches the pressure height.
  //
  //
  double pht = PHYmb2meters( *press ); // Nominal height at this pressure.
  //
  double minErrsqrd = 1.0e12;
  double bestHeight = -1;
  for (int thousands = 0; thousands < 25; thousands++){
    //
    // Get our height with the thousands digits pre-prended.
    //
    double testHeight = *ht + thousands * 1000.0; 
    double err = pht -testHeight;
    double errSqrd = err*err;
    //
    // Pick off the best height - the one with the minimum error.
    //
    if (errSqrd < minErrsqrd){
      minErrsqrd = errSqrd;
      bestHeight = testHeight;
    }
  }

  *ht = bestHeight;

  return;

}

//////////////////////////////////////////////
//
// Decode a PPhhh message to give pressure in hPa
// and height in meters in the AA message.
//
void Decoders::_decode_CC_PPhhh(const char *PPhhh, double *press, double *ht ){

  *press = _badVal; *ht = _badVal;
  if (strlen(PPhhh) < 5) return;
  if (NULL != strstr(PPhhh,"/")) return;

  work[0] = PPhhh[0];
  work[1] = PPhhh[1];
  work[2]=char(0);

  int pcode = atoi( work );
  //
  // Decode the pressure from this coded integer.
  // This is done somewhat laboriously to avoid erroneous decodings.
  //
  switch (pcode ) {

  case 0 :
    *press = 100.0; break;
  case 92:
    *press = 92.5; break;
  case 85:
    *press = 85.0; break;
  case 70:
    *press = 70.0; break;
  case 50:
    *press = 50.0; break;
  case 40:
    *press = 40.0; break;
  case 30:
    *press = 30.0; break;
  case 25:
    *press = 25.0; break;
  case 20:
    *press = 20.0; break;
  case 15:
    *press = 15.0; break;
  case 10:
    *press = 10.0; break;
  default : // Leave it as bad.
    break;
    
  }

  if (*press == _badVal) return; // Can't decode height without pressure.
  //
  // Decode the hhh part.
  //
  work[0] = PPhhh[2];
  work[1] = PPhhh[3];
  work[2]=  PPhhh[4];
  work[3]=char(0);

  int hcode = atoi( work );
  //
  // Supposedly, negative values have 500 added to them.
  // This seems ambiguous to me so I will ignore it. Niles.
  //
  int sign = 1;
  /*-------------------------
  if (hcode > 500) {
    hcode = hcode - 500;
    sign = -1;
  }
  ---------------------------*/
  //
  // Scaling depends on pressure.
  //
  if (*press > 500.0){
    *ht = hcode;
  } else {
    *ht = hcode * 10.0;
  }
  *ht = *ht * sign;
  //
  // Now - the real voodoo.
  //
  // The height we just got has, in fact, no
  // thousands or tens of thousands digits. What we
  // do have is an accurate pressure. So, what I think we have
  // to do is to cycle through the thousands up to 25,000m
  // and use the height that most closely matches the pressure height.
  //
  //
  double pht = PHYmb2meters( *press ); // Nominal height at this pressure.
  //
  double minErrsqrd = 1.0e12;
  double bestHeight = -1;
  for (int thousands = 0; thousands < 25; thousands++){
    //
    // Get our height with the thousands digits pre-prended.
    //
    double testHeight = *ht + thousands * 1000.0; 
    double err = pht -testHeight;
    double errSqrd = err*err;
    //
    // Pick off the best height - the one with the minimum error.
    //
    if (errSqrd < minErrsqrd){
      minErrsqrd = errSqrd;
      bestHeight = testHeight;
    }
  }

  *ht = bestHeight;

  return;

}
//////////////////////////////////////////
//
// Get the location using the station ID and the lookup
// table from the parameter file.
//
int Decoders::_getLocation(){

  _lat = _badVal; 
  _lon = _badVal;
  _alt = _badVal;

  for (int i=0; i < _params->siteLocations_n; i++){
    if (_params->_siteLocations[i].soundingSiteId ==  _stationID){
      _lat = _params->_siteLocations[i].lat;
      _lon = _params->_siteLocations[i].lon;
      _alt = _params->_siteLocations[i].alt;
      _stationName = _params->_siteLocations[i].name;
      return 0; // Got it.
    }
  }

  return 1;

}

////////////////////////////////////////////////////
//
// Decode sounding source from BB mesages.
// From table 0265.
//
//
void  Decoders::_decode_BB_Source( const char *YYGGa ){

 _source = NULL;

 if (strlen(YYGGa) < 5) return;

 work[0] = YYGGa[4];
 work[1]=char(0);

 if (NULL != strstr(work,"/")) return;

 int sourceCode = atoi( work );
 switch( sourceCode ){

 case 0 :
   _source = "Pressure and wind instrument";
   break;
 case 1 :
   _source = "Optical theodolite";
   break;
 case 2 :
   _source = "Radiotheodolite";
   break;
 case 3 :
   _source = "Radar";
   break;
 case 4 :
   _source = "Failed pressure instrument";
   break;
 case 5 :
   _source = "VLF-Omega";
   break;
 case 6 :
   _source = "Loran-C";
   break;
 case 7 :
   _source = "Wind profiler";
   break;
 case 8 :
   _source = "Satellite navigation";
   break;
 default:
   break;
 }

 return;

}

//////////////////////////////////////////////////////
//
// Decoder to deal with nnPPP message.
//
double  Decoders::_decode_BB_nnPPP( const char *nnPPP ){

  if (NULL==nnPPP) return _badVal;
  if (strlen(nnPPP) < 5) return _badVal;
  //
  // nn should be encoded as 00, 11, 22, 33 .. 99
  // Check that the two characters are equal, if not,
  // don't consider this a valid message.
  //
  if (nnPPP[0] != nnPPP[1]) return _badVal;
  if (!(isdigit((int)nnPPP[0]))) return _badVal;
  if (NULL!=strstr(nnPPP,"/")) return _badVal;
  //
  work[0] = nnPPP[2];
  work[1] = nnPPP[3];
  work[2] = nnPPP[4];
  work[3] = char(0);


  
  double pres = atoi( work );

  if (pres < 50) pres = pres + 1000.0; // OK for surface data.
  //
  // Final check - make sure the standard height in the atmosphere associated
  // with this pressure is positive. Negative heights do not inspire
  // confidence.
  //
  if (PHYmb2meters( pres ) < 0.0) return _badVal;

  return pres;

}
/////////////////////////////////////////////////////
//
// Small routine that returns 1 if a string indicates the end
// of a section.
//
int Decoders::_decodeIsEndMessage( const char *p ){

  if (strlen(p) < 3) return 0;
  if (!(strncmp("NIL",p,3))) return 1;

  if (strlen(p) < 5) return 0;

  if (!(strncmp("21212",p,5))) return 1;
  if (!(strncmp("31313",p,5))) return 1;
  if (!(strncmp("41414",p,5))) return 1;

  if (!(strncmp("51515",p,5))) return 1;
  if (!(strncmp("52525",p,5))) return 1;
  if (!(strncmp("53535",p,5))) return 1;
  if (!(strncmp("54545",p,5))) return 1;
  if (!(strncmp("55555",p,5))) return 1;
  if (!(strncmp("56565",p,5))) return 1;
  if (!(strncmp("57575",p,5))) return 1;
  if (!(strncmp("58585",p,5))) return 1;
  if (!(strncmp("59595",p,5))) return 1;

  if (!(strncmp("61616",p,5))) return 1;
  if (!(strncmp("62626",p,5))) return 1;
  if (!(strncmp("63636",p,5))) return 1;
  if (!(strncmp("64646",p,5))) return 1;
  if (!(strncmp("65656",p,5))) return 1;
  if (!(strncmp("66666",p,5))) return 1;
  if (!(strncmp("67676",p,5))) return 1;
  if (!(strncmp("68686",p,5))) return 1;
  if (!(strncmp("69696",p,5))) return 1;

  return 0;
}

///////////////////////////////////////////////////////
//
// Decode the 44nPP and 55nPP message from AA type strings.
//
void  Decoders::_decode_AA_44nPP( const char *code_44nPP, int *n, double *pres ){

  *n = 0; *pres = _badVal;

  if (
      (code_44nPP == NULL) ||
      (strlen(code_44nPP) < 5) ||
      (NULL != strstr(code_44nPP,"/"))
      ){
    return;
  }
  //
  // Make sure the thing starts with either a '44' or a '55'
  //      
  if (!(
      ((code_44nPP[0] == '4') && (code_44nPP[1] == '4')) ||
      ((code_44nPP[0] == '5') && (code_44nPP[1] == '5')) 
      )){
    return;
  }
  //
  // Get n
  //
  work[0] = code_44nPP[2]; work[1]=char(0);
  *n = atoi( work );
  //
  // Get PP
  //
  work[0] = code_44nPP[3]; work[1]=code_44nPP[4];  work[2]=char(0);
  *pres = double(atoi( work )) * 10.0; // Report in tens of mb for AA,BB

  return;

}
///////////////////////////////////////////////////
//
// _decode_GetNext_AA_Pressure( double Pr ) returns the
// next pressure in the AA series or _badVal if the
// current pressure is unrecognised. Ascending with respect
// to altitude.
//
double Decoders::_decode_GetNext_AA_Pressure( double Pr ){

  if (Pr == 850.0) return 700.0;
  if (Pr == 700.0) return 500.0;
  if (Pr == 500.0) return 400.0;
  if (Pr == 400.0) return 300.0;
  if (Pr == 300.0) return 250.0;
  if (Pr == 250.0) return 200.0;
  if (Pr == 200.0) return 150.0;
  if (Pr == 150.0) return 100.0;

  return _badVal;

}
///////////////////////////////////////////////////////
//
// Decode the 44nPP and 55nPP message from CC type strings.
//
void  Decoders::_decode_CC_44nPP( const char *code_44nPP, int *n, double *pres ){

  *n = 0; *pres = _badVal;

  if (
      (code_44nPP == NULL) ||
      (strlen(code_44nPP) < 5) ||
      (NULL != strstr(code_44nPP,"/"))
      ){
    return;
  }
  //
  // Make sure the thing starts with either a '44' or a '55'
  //      
  if (!(
      ((code_44nPP[0] == '4') && (code_44nPP[1] == '4')) ||
      ((code_44nPP[0] == '5') && (code_44nPP[1] == '5')) 
      )){
    return;
  }
  //
  // Get n
  //
  work[0] = code_44nPP[2]; work[1]=char(0);
  *n = atoi( work );
  //
  // Get PP
  //
  work[0] = code_44nPP[3]; work[1]=code_44nPP[4];  work[2]=char(0);
  *pres = double(atoi( work ));

  return;

}
///////////////////////////////////////////////////
//
// _decode_GetNext_CC_Pressure( double Pr ) returns the
// next pressure in the CC series or _badVal if the
// current pressure is unrecognised. Ascending with respect
// to altitude.
//
double Decoders::_decode_GetNext_CC_Pressure( double Pr ){


  if (Pr == 70.0) return 50.0;
  if (Pr == 50.0) return 30.0;
  if (Pr == 30.0) return 20.0;
  if (Pr == 20.0) return 10.0;

  return _badVal;

}
/////////////////////////////////////////////////////
//
// Decoder for the rather different ITUUU messages.
//
//
int Decoders:: _decode_DD_ITUUU( const char *ITUUU, double *H1, double *H2, 
				 double *H3){

  *H1 = _badVal; *H2 = _badVal; *H3 = _badVal;

  //
  // Make sure we have a suitable string.
  //
  if (
      (ITUUU == NULL) ||
      (strlen(ITUUU) < 5)
      ){
    return 0;
  }
  //
  // Make sure the first character is a 1, 8 or 9.
  //
  if (
      (ITUUU[0] != '1') &&
      (ITUUU[0] != '8') &&
      (ITUUU[0] != '9')
      ){
    return 0;
  }
  //
  // Set up the scale and bias appropriately.
  //
  double scale=0.0,bias=0.0; // Init to avoid compiler warnings.

  if (ITUUU[0] == '1'){
    scale = 300.0; bias = 30000.0;
  }
  if (ITUUU[0] == '8'){
    scale = 500.0; bias = 0.0;
  }
  if (ITUUU[0] == '9'){
    scale = 300.0; bias = 0.0;
  }
  //
  // Decode the tens field.
  //
  work[0]=ITUUU[1]; work[1]=char(0);
  if (NULL!=strstr(work,"/")) return 0; // Can't get any alts without tens.

  int tens = atoi(work);
  //
  // Then the three unit fields.
  //
  int u1=-1, u2=-1, u3=-1;

  work[0]=ITUUU[2]; work[1]=char(0);
  if (NULL == strstr(work,"/"))   u1 = atoi(work);
  work[0]=ITUUU[3]; work[1]=char(0);
  if (NULL == strstr(work,"/"))   u2 = atoi(work);
  work[0]=ITUUU[4]; work[1]=char(0);
  if (NULL == strstr(work,"/"))   u3 = atoi(work);
  //
  // Write to output, if we got the units OK.
  //
  if (u1 != -1) *H1 = (10*tens +u1)*scale + bias;
  if (u2 != -1) *H2 = (10*tens +u2)*scale + bias;
  if (u3 != -1) *H3 = (10*tens +u3)*scale + bias;

  //
  // Sanity check.
  //
  if ((*H1 != _badVal) && (*H1 > 35000)) *H1 = _badVal;
  if ((*H2 != _badVal) && (*H2 > 35000)) *H1 = _badVal;
  if ((*H3 != _badVal) && (*H3 > 35000)) *H1 = _badVal;

  return 1; // Did OK, although all the altitudes may still be bad.

}

//////////////////////////////////////////////////////////
//
// Write the data out to SPDB, by interleaving or merging.
//
int Decoders::write(){

  if (!gotData()) return 0;
  //
  // Create and Init the object.
  //
  SoundingMerge S(_params);
  S.Init(_dataTime, _stationID, _stationName);  
  S.setMergeEpsilon( _params->soundingMergeEpsilon );
  //
  //  Set up a sounding data holder and pass it in to be added.
  //
  SoundingMerge::SoundingDataHolder toBeAdded;


  toBeAdded.height = _h;
  toBeAdded.u = _u;
  toBeAdded.v = _v;
  toBeAdded.w = NULL;
  toBeAdded.prs = _pressure;
  toBeAdded.relHum = _rh;
  toBeAdded.temp = _temp;
  toBeAdded.lat = _lat;
  toBeAdded.lon = _lon;
  toBeAdded.alt = _alt;
  toBeAdded.numPoints = _numPoints;
  toBeAdded.badVal = _badVal;

  if (_source == NULL){
    toBeAdded.source = "Unknown";
  } else {
    toBeAdded.source = _source;
  }

  char site[16];
  sprintf(site,"%d",_stationID);

  toBeAdded.siteName = site;

  return S.Merge( toBeAdded );


}




