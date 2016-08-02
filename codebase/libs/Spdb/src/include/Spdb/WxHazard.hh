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
 * WxHazard.hh: Abstract object representing a weather hazard.  This
 *              is the base class for weather hazard objects stored in
 *              an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef WxHazard_HH
#define WxHazard_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <dataport/port_types.h>

#include <euclid/WorldPoint2D.hh>

using namespace std;


/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class WxHazard
{
 public:

  // Structure containing the weather hazard header information when
  // the hazard is stored in an SPDB database.  Every weather hazard
  // will be preceded by one of these headers so that the hazard type
  // for the hazard can be determined.

  typedef struct
  {
    si32 hazard_type;
    si32 spare;
  } spdb_hazard_header_t;
  
  // Id's which uniquely identify the valid hazards in an SPDB database.
  // When a new hazard is added, be sure to add it to the WxHazardFactory
  // class so it can be read from a database.

  typedef enum
  {
    CONVECTIVE_REGION_HAZARD = 300,
    CONVECTIVE_REGION_HAZARD_EXTENDED = 400
  } hazard_type_t;
  

  // Constructor

  WxHazard(hazard_type_t hazard_type, bool debug_flag);
  
  // Destructor

  virtual ~WxHazard(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Calculate the number of bytes occupied by this hazard when stored
  // in an SPDB database.

  virtual int getSpdbNumBytes(void) const = 0;
  
  // Write the hazard information to the given byte buffer in SPDB format.

  virtual void writeSpdb(ui08 *buffer) const = 0;
  
  // Print the contents of the hazard

  virtual void print(FILE *stream) const = 0;
  
  virtual void addPoint(WorldPoint2D *point) = 0; 

  /////////////////////
  // Utility methods //
  /////////////////////

  // Swaps the spdb_hazard_header_t structure from native format to
  // big-endian format.

  static void spdbHazardHeaderToBigend(spdb_hazard_header_t *header);
  
  // Swaps the spdb_hazard_header_t structure from big-endian format to
  // native format.

  static void spdbHazardHeaderToNative(spdb_hazard_header_t *header);
  

  ////////////////////
  // Access methods //
  ////////////////////

  hazard_type_t getHazardType(void) const
  {
    return _hazardType;
  }
  

 protected:
  
  // Debug flag

  bool _debugFlag;
  
  // Type of hazard

  hazard_type_t _hazardType;
  

 private:
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("WxHazard");
  }
  
};


#endif
