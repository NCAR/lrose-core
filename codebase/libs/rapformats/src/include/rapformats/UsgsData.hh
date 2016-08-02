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
/*
 * UsgsData.hh
 *
 * structs and defines for U.S. Geological Survey data
 *
 * Currently handles Volcanoes and Earthquakes point data
 *
 * Jason Craig, RAP, NCAR, Boulder, CO
 *
 * Nov 2006
 */

#ifndef USGSDATA_HH
#define USGSDATA_HH

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>

class UsgsData {

private:
public:

  static const si32 SRC_NAME_LEN = 80;
  static const si32 EVNT_NAME_LEN = 80;
  static const si32 EVENT_ID_LEN = 11;

  static const si32 NUM_SPARE_INTS = 4;
  static const si32 NUM_SPARE_FLOATS = 4;

  static const si32 NBYTES_32 = 92;
  
  static const fl32 VALUE_UNKNOWN;

  /*
   * Details what type of USGS message this is.
   */
  typedef enum
  {
    USGS_DATA_VOLCANO,    
    USGS_DATA_EARTHQUAKE   
  } usgsData_type_t;
    
  typedef enum
  {
    COLOR_CODE_GREEN,  // Volcano Aviation Color Codes
    COLOR_CODE_YELLOW,
    COLOR_CODE_ORANGE,
    COLOR_CODE_RED,
    COLOR_CODE_UNKNOWN,
    MAGNITUDE_DURATIAON,  // Earthquake Magnitude Types
    MAGNITUDE_LOCAL,
    MAGNITUDE_SURFACE_WAVE,
    MAGNITUDE_MOMENT,
    MAGNITUDE_BODY,
    MAGNITUDE_UNKOWN
  } magnitude_type_t;

  /*
   * See http://earthquake.usgs.gov/eqcenter/recenteqsww/glossary.php
   * For detailed info on Earthquake variables
   */
  typedef struct
  {
    ui32 data_type;        // see usgsData_type_t above
    si32 time;
    fl32 lat;
    fl32 lon;
    fl32 alt;              // (km) volcanoes = summit elevation, earthquakes = depth
    si32 version;
    fl32 magnitude;
    ui32 magnitude_type;   // see magnitude_type_t above
    fl32 horizontalError;  // (km)
    fl32 verticalError;    // (km)
    fl32 minDistance;      // (km)
    fl32 rmsTimeError;     // (seconds)
    fl32 azimuthalGap;     // (degrees)
    si32 numStations;
    si32 numPhases;
    fl32 spareFloats[NUM_SPARE_FLOATS];
    si32 spareInts[NUM_SPARE_INTS];
    char sourceName[SRC_NAME_LEN];
    char eventTitle[EVNT_NAME_LEN];
    char eventID[EVENT_ID_LEN];
  } usgsData_t;

  // constructor

  UsgsData();

  // destructor

  ~UsgsData();
  
  //
  //
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  void assemble();

  //
  // get the assembled buffer info
  //
  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  //
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  //
  int disassemble(const void *buf, int len);

  //
  // set methods
  //
  void setUsgsData(const usgsData_t &rep);
 
  //
  // get methods
  //
  fl32 getTime() { return _usgsData.time; }  
  const usgsData_t &getRep() const { return _usgsData; }
 
	 
  //
  // print usgsData
  //
  void print(ostream &out, string spacer = "") const;

protected:

private:
  
  //
  // data members 
  //
  MemBuf _memBuf;
  
  usgsData_t _usgsData; 
  
  //
  // byte swapping
  //
  static void _usgsData_to_BE(usgsData_t& rep);
  static void _usgsData_from_BE(usgsData_t& rep);

};

#endif /* USGSDATA_HH */
