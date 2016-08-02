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
 * ConvRegionHazardExt.hh: Class representing a convective region hazard.  This
 *                         is a region, represented as a polygon, that contains
 *                         hazardous convective weather.  It is different than
 *                         ConvRegionHazard in that it includes centroid and
 *                         polygon sequence information from Titan.
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2015
 *
 * Gary Blackburn
 *
 ************************************************************************/

#ifndef ConvRegionHazardExt_HH
#define ConvRegionHazardExt_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <vector>
#include <string>

#include <dataport/port_types.h>
#include <euclid/WorldPolygon2D.hh>
#include <Spdb/WxHazard.hh>
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

class ConvRegionHazardExt : public WxHazard
{
 public:

  // Constructors

  ConvRegionHazardExt(double top = 0.0,
		   double speed = 0.0, double direction = 0.0,
		   double latitude = 0.0, double longitude = 0.0,
		   si32 simple_track_num = 0, si32 complex_track_num = 0,
                   si32 forecast_secs = 0,
		   bool debug_flag = false);
  
  ConvRegionHazardExt(void *buffer,
		   bool debug_flag = false);
  
  // Destructor

  ~ConvRegionHazardExt(void);
  
  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the hazard information to the given stream.

  void print(FILE *stream) const;
  
  // Retrieve the number of bytes occupied by this hazard when stored
  // in an SPDB database.

  int getSpdbNumBytes(void) const;
  
  // Write the hazard information to the given buffer in SPDB format.
  //
  // Note that the calling routine must call getSpdbNumBytes() and allocate
  // a large enough buffer before calling this routine.

  void writeSpdb(ui08 *buffer) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Add the given point to the end of the region polygon.
  //
  // Note that the pointer to this point is saved for the polygon so you
  // must not change the point value or delete the point object after
  // calling this routine.

  void addPoint(WorldPoint2D *point);
  
  // Set the top value for the region.  The top value should be given in
  // feet.

  void setTop(double top)
  {
    _top = top;
  }
  
  // Retrieves the top value for the region in feet.

  double getTop(void) const
  {
    return _top;
  }
  
  // Set the forecast valid time in seconds

  void setForecastSecs(si32 seconds)
  {
    _forecast_seconds = seconds;
  }
  
  // Set the speed for the region.  The speed value should be given in km/hr.

  void setSpeed(double speed)
  {
    _speed = speed;
  }
  
  // Set the direction for the region motion.  The direction value should be
  // given in deg, with North being 0.0 degrees and increasing clockwise.

  void setDirection(double direction)
  {
    _direction = direction;
  }
  
  // Sets the storm centroid lat/lons

  void setLatitude(double lat) 
  {
    _centroid_lat = lat;
  }
  
  void setlongitude(double lon) 
  {
    _centroid_lon = lon;
  }
  
  // Retrieves the polygon sequence number components

  si32 getSimpleTrackId(void) const
  {
    return _simple_track_num;
  }
  
  si32 getComplexTrackId(void) const
  {
    return _complex_track_num;
  }
  
  // Retrieves the speed for the region in km/hr.

  double getSpeed(void) const
  {
    return _speed;
  }

  // Get the forecast valid time in seconds

  si32 getForecastSecs(void) const
  {
    return _forecast_seconds;
  }
  
  
  // Retrieves the storm centroid components

  double getLatitude(void) const
  {
    return _centroid_lat;
  }
  
  double getLongitude(void) const
  {
    return _centroid_lon;
  }
  
  // Retrieves the direction for the region motion in degrees, with North
  // being 0.0 degrees and increasing clockwise.

  double getDirection(void) const
  {
    return _direction;
  }
  
  // Retrieves the polygon for the region

  WorldPolygon2D *getPolygon(void) const
  {
    return _polygon;
  }
  

 private:
  
  // Structure containing the weather hazard information when the
  // hazard is stored in an SPDB database.  In the database, the hazard
  // looks like this:
  //
  //             spdb_hazard_header_t
  //             spdb_header_t
  //             polygon point 0 point_t
  //             polygon point 1 point_t
  //                  .
  //                  .
  //                  .
  //             polygon point n-1 point_t

  typedef struct
  {
    fl32 lat;
    fl32 lon;
  } point_t;
  
  typedef struct
  {
    fl32 top;
    fl32 speed;
    fl32 direction;
    fl32 centroid_lat;
    fl32 centroid_lon;
    fl32 spare_fl32[2];
    si32 num_polygon_pts;
    si32 simple_track_num;
    si32 complex_track_num;
    si32 forecast_seconds;
    si32 spare_si32;
  } spdb_header_t;
  
  // Debug flag

  bool _debugFlag;
  
  // The convective region information

  WorldPolygon2D *_polygon;
  double _top;               // Top value for region in feet
  double _speed;             // Speed of region motion in km/hr
  double _direction;         // Direction of region motion in deg
  double  _centroid_lat;     // Titan centroid for XML MotionVectors
  double  _centroid_lon;     
  int     _simple_track_num; // Titan simple and complex storm id numbers
  int     _complex_track_num; 
  int     _forecast_seconds;
  
  // Swaps the spdb_header_t structure from native format to
  // big-endian format.

  static void _spdbHeaderToBigend(spdb_header_t *header);
  
  // Swaps the spdb_header_t structure from big-endian format to
  // native format.

  static void _spdbHeaderToNative(spdb_header_t *header);
  
  // Swaps the point_t structure from native format to big-endian format.

  static void _spdbPointToBigend(point_t *point);
  
  // Swaps the point_t structure from big-endian format to native format.

  static void _spdbPointToNative(point_t *point);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("ConvRegionHazardExt");
  }
  
};


#endif
