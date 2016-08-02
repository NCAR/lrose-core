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
 * FltRoute.hh: Class representing a flight route as stored in an SPDB
 *              database.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FltRoute_HH
#define FltRoute_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <string>
#include <vector>

#include <dataport/port_types.h>

#include <Spdb/WayPoint.hh>
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

class FltRoute
{
 public:

  // Constructors

  FltRoute(const string& id = "",
	   const bool debug_flag = false);

  FltRoute(const char *id,
	   const bool debug_flag = false);

  FltRoute(const void *spdb_buffer,   // SPDB buffer as read directly from a
	                              // database (in big-endian format).
	   const bool debug_flag = false);
  
  // Destructor

  virtual ~FltRoute(void);
  
  // Add the given way point to the route.

  void addWayPoint(const WayPoint& way_point);
  
  // Calculate the number of bytes occupied by this route when stored
  // in an SPDB database.

  inline int getSpdbNumBytes(void) const
  {
    return sizeof(spdb_flt_route_t) +
      _wayPoints.size() * WayPoint::getSpdbNumBytes();
  }
  
  // Print the route to the given stream.

  void print(FILE *stream) const;
  
  // Write the route information to the given byte buffer in SPDB format.
  // Note that the data will be written to the buffer in big-endian format.
  // Note also that it is assumed that the given buffer is big enough for
  // the data.

  void writeSpdb(void *buffer) const;
  
  // Writes the flight route information to the indicated database.

  void writeToDatabase(const char *database_url,
		       const time_t valid_time,
		       const time_t expire_time,
		       const int data_type = 0) const;
  
 protected:
  
  // Debug flag

  bool _debugFlag;
  
  // Route information

  string _id;
  vector< WayPoint > _wayPoints;
  
  // Structure containing the flight route header information when
  // the route is stored in an SPDB database.  This header is followed
  // in the database by num_way_pts WayPoint::spdb_way_point_t structures.

  static const int MAX_FLT_ID_LEN = 16;
  
  typedef struct
  {
    char id[MAX_FLT_ID_LEN];      // The flight identifier
    si32 num_way_pts;
  } spdb_flt_route_t;
  
  // Swaps the spdb_flt_route_t structure from native format to
  // big-endian format.

  static void spdbToBigend(spdb_flt_route_t *flt_route);
  
  // Swaps the spdb_flt_route_t structure from big-endian format to
  // native format.

  static void spdbToNative(spdb_flt_route_t *flt_route);
  
 private:
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("FltRoute");
  }
  
};


#endif
