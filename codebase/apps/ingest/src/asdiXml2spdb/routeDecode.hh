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

/************************************************************************
 * routeDecode.hh: routeDecode program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2005
 *
 * Jason Craig
 *
 ************************************************************************/

#ifndef routeDecode_HH
#define routeDecode_HH

#include <cstdio>
#include <vector>
#include <rapformats/ac_route.h>

#include "Params.hh"


class routeDecode
{
public:
  
  /* Constructor: Reads into memory FAA data files.
   *  Input: params containing specific decode instructions.
   *  Check isOk() to verify constructor succeded.
   */
  routeDecode(Params *tdrpParams);

  /* Destructor: Frees memory created by constructor.
   */
  ~routeDecode();

  /* Function to check on constructors success
   *  Should be checked after every Constructor
   */
  bool isOK();

  /* Function to decode asdi message
   * Inputs:
   *    message: a char array containing a FZ asdi message
   *                          (other messages are ignored)
   * Returns: 0 on Success
   */
  int decoder(char *message);


  /* Function to decode a route
   * Inputs:
   *    route: a char array containing just the route information
   * Returns: 0 on Success
   */
  int decodeRoute(char *route);

  /* Creates an array of ac_route_posn_t, filled with
   * a decoded route from previous decoder or decodeRoute call.
   * Outputs:
   *    arraySize: Number of elements in ac_route_posn_t array.
   * Returns: ac_route_posn_t array pointer.
   */
  ac_route_posn_t *getRouteArray(int *arraySize);

  /* Creates an array of ac_route_posn_t, filled with
   * a decoded route from previous decoder or decodeRoute call.
   * Inputs:
   *    stringSize: Max size of dest and dept char arrays.
   * Outputs:
   *    arraySize: Number of elements in ac_route_posn_t array.
   *   estArrival: Time of arrival parsed out from route.
   *      destStr: Destination Airport
   *      deptStr: Departure Airport
   * Returns: ac_route_posn_t array pointer.
   */
  ac_route_posn_t *getRouteArray(int *arraySize, int *estArrival, 
				 char *destStr, char *deptStr,
				 int stringSize);

  /* Lookup an airports lat and lon.
   * Inputs: 
   *      APT: a char array containing the airport code.
   * Outputs:
   *      lat: Latitude of the airport.
   *      lon: Longitude of the airport.
   * Returns: true if found.
   */
  bool lookupAirport(char *APT, float *lat, float *lon);

  /* Returns:  Time of arrival parsed out from route on
   *           previous decoder or decodeRoute call.
   */
  int getEstArrival();

  void print_route();

private:
  /*
   * vector<char *>    : Vector of char pointers
   * vector<float>     : Vector of floats
   * vector<char *> ** : Array of pointers to vector of char pointers
   * vector<float> **  : Array of pointers to vector of floats
   */

  const static int _internalStringLenLarge = 256;
  const static int _internalStringLen      = 128;
  const static int _internalStringLenSmall = 25;
  const static int _bufferSize = 4096;
  const static char EOL = char(10);
  const static char NUL = char(0);

  Params *_params;              // See Params.hh

  bool _isOK;                   // Signals constructor error

  int inputRoutePos;            // Current position in the inputRouteVec
  vector<char *> inputRouteVec; // A vector of the input message split on ".." and "./."
  typedef enum {
    FIRST_PEICE  = 0,
    REGULAR_PEICE = 1,
    LAST_PEICE = 2,
    ONLY_PEICE = 3
  } peice_type_t;
  int peiceVecPos;              // Current position in the peiceVec
  vector<char *> peiceVec;      // Vector of the current inputRouteVec splot on "."

  vector<char *> _routeNameVec; // Names of decoded route points
  vector<float> _routeLatVec;   // Lats of decoded route points
  vector<float> _routeLonVec;   // Lons of decoded route points
  int _estArrival;              // Estimated time of arrival (if known)
  char *_destName;              // Name of destination airport
  char *_deptName;              // Name of departure airport

  int _APTsize;
  char **_APTname;
  float *_APTlat;
  float *_APTlon;

  int _ARPsize;
  char **_ARPname;
  float *_ARPlat;
  float *_ARPlon;

  int _NAVsize;
  char **_NAVname;
  float *_NAVlat;
  float *_NAVlon;

  int _FIXsize;
  char **_FIXname;
  float *_FIXlat;
  float *_FIXlon;

  int _AWYsize;       // Size of the awy arrays
  char **_AWYname;    // Array of the awy names

  /* One Vector for each AWY name, which is the awy route, 
   * each point on the route has a Name, lat, and lon.
   */
  vector<char *> **_AWYnameArrayVec;
  vector<float> **_AWYlatArrayVec;   
  vector<float> **_AWYlonArrayVec;   

  int _SSDsize;      // Size of ssd arrays
  char **_SSDname;   // Array of the ssd names

  /* One Vector for each SSD name, which is the ssd route,
   * each point on the route has a Name, lat, and lon.
   */
  vector<char *> **_SSDnameArrayVec; 
  vector<float> **_SSDlatArrayVec;   
  vector<float> **_SSDlonArrayVec;   


  // Private functions //

  int _decodePeice(char *peice, peice_type_t type);
  void _clearRoute();

  bool _findAPT(char *part);
  bool _findARP(char *part);
  bool _findNAV(char *part);
  bool _findFIX(char *part);
  bool _findAWY(char *part, char *prevPart, char *nextPart);
  bool _findSSD(char *part);
  bool _findSSD(char *part, char *exit);
  bool _findSSD(char *entrance, char *part, char *exit);
  bool _findRadial(char *radial);

  bool _isAPT(char *part);
  bool _isAPT2(char *part);
  bool _isARP(char *part);
  bool _isARP2(char *part);
  bool _isLatLon(char *part);
  bool _isLatLon2(char *part);
  bool _isRadialFix(char *part);
  bool _isRadialIntersection(char *part);
  bool _isNAV(char *part);
  bool _isFIX(char *part);
  bool _isAWY(char *part);
  bool _isSSD(char *part);

  vector<char *> _splitString(char *instring, char *splitstring, int stringLen);
  void _extractString(char *instring, char *outstring);
  void _extractString(char *instring, char *stopPos, char *outstring);
  bool _isLetter(char c);
  bool _isDigit(char c);
};

#endif
