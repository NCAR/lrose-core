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
 * WayPoint.hh: Class representing a flight route as stored in an SPDB
 *              database.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef WayPoint_HH
#define WayPoint_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <string>

#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
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

class WayPoint
{
 public:

  /////////////////////
  // Constant values //
  /////////////////////

  // Set any invalid lat/lons to this value.

  static const double BAD_POSITION;

  // Set any invalid ETAs to this value.

  static const time_t BAD_ETA;
  
  //////////////////////////////
  // Constructors/destructors //
  //////////////////////////////

  // Constructors

  WayPoint(const string& label = "",
	   const time_t eta = BAD_ETA,
	   const double lat = BAD_POSITION,
	   const double lon = BAD_POSITION,
	   const bool debug_flag = false);
  
  WayPoint(const string& label,
	   const DateTime& eta,
	   const double lat = BAD_POSITION,
	   const double lon = BAD_POSITION,
	   const bool debug_flag = false);
  
  WayPoint(const void *spdb_buffer,         // SPDB buffer as read directly
                                            // from a database (in big-endian
                                            // format).
	   const bool debug_flag = false);
  
  // Destructor

  virtual ~WayPoint(void);
  
  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Calculate the number of bytes occupied by this hazard when stored
  // in an SPDB database.

  inline static int getSpdbNumBytes(void)
  {
    return sizeof(spdb_way_pt_t);
  }
  
  // Write the way point to the given byte buffer in SPDB format.  Note
  // that the data will be written to the buffer in big-endian format.

  void writeSpdb(void *buffer) const;
  
  // Print the way point to the given stream.

  void print(FILE *stream) const;
  
  ////////////////////
  // Access methods //
  ////////////////////

  const double& getLat(void) const
  {
    return _lat;
  }
  
  const double& getLon(void) const
  {
    return _lon;
  }
  
  const string& getLabel(void) const
  {
    return _label;
  }
  
  const DateTime& getEta(void) const
  {
    return _eta;
  }
  
  void setPosition(const double& lat, const double& lon)
  {
    _lat = lat;
    _lon = lon;
  }
  
  void setLabel(const string& label)
  {
    _label = label;
  }
  
  void setEta(const time_t& eta)
  {
    _eta = eta;
  }
  
  void setEta(const DateTime& eta)
  {
    _eta = eta;
  }
  

 protected:
  
  // Debug flag

  bool _debugFlag;
  
  // Way point information

  string _label;
  
  DateTime _eta;
  
  double _lat;
  double _lon;
  
  // Structure containing the way point information when the point is
  // stored in an SPDB database.

  static const int MAX_WAY_PT_LABEL_LEN = 8;
  
  typedef struct
  {
    char label[MAX_WAY_PT_LABEL_LEN];
    ti32 eta;
    si32 spare;
    fl32 lat;
    fl32 lon;
  } spdb_way_pt_t;
  
  // Swaps the spdb_way_pt_t structure from native format to
  // big-endian format.

  static void spdbToBigend(spdb_way_pt_t *way_pt);
  
  // Swaps the spdb_way_pt_t structure from big-endian format to
  // native format.

  static void spdbToNative(spdb_way_pt_t *way_pt);
  
 private:
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("WayPoint");
  }
  
};


#endif
